#!/usr/bin/env python3
#  Copyright 2016 Google Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS-IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from concurrent import futures

import itertools
import sys
import re
import pygraphviz as gv
import ply.lex as lex
import ply.yacc as yacc
from functools import lru_cache as memoize

diagnostic_header_pattern = re.compile(r'[^ ]+\.[^ ]+:[0-9]+:[0-9]+: ([^ ]*): (.*)')
in_file_included_from_pattern = re.compile('In file included from .*:')
in_instantiation_of_template_pattern = re.compile('in instantiation of (.*) (?:requested|required) here')
static_warning_marked_deprecated_here_pattern = re.compile('\'static_warning\' has been explicitly marked deprecated here')

class Diagnostic:
    def __init__(self, kind, message):
        self.kind = kind
        self.message = message
        self.template_instantiation_trace = []

tokens = (
    'LPAREN',
    'RPAREN',
    'LBRACKET',
    'RBRACKET',
    'LBRACE',
    'RBRACE',
    'LESS_THAN',
    'GREATER_THAN',
    'DOUBLE_COLON',
    'COMMA',
    'IDENTIFIER',
    'ASTERISK',
    'AMPERSAND',
)

t_LPAREN = r'\('
t_RPAREN = r'\)'
t_LBRACKET = r'\['
t_RBRACKET = r'\]'
t_LBRACE = r'}'
t_RBRACE = r'{'
t_LESS_THAN = r'<'
t_GREATER_THAN = r'>'
t_DOUBLE_COLON = r'::'
t_COMMA = r','
t_ASTERISK = r'\*'
t_AMPERSAND = r'&'
# We conflate numbers as identifiers too, we don't care about the difference.
t_IDENTIFIER = r'[a-zA-Z0-9_]+'

t_ignore = ' \t'

def t_error(t):
    raise Exception("Illegal character '%s' followed by %s" % (t.value[0], t.value[1:]))

class LayoutNeedsMultipleLinesException(Exception):
    pass

class AstNode:
    def __str__(self):
        return ''.join(self)

class TerminalAstNode(AstNode):
    def __init__(self, s):
        self.s = s
        self.is_multiline = (s == '\n')
        # last_line_length is the string length if s is not a multiline string.
        # For multiline strings ending in a newline, this is 0.
        if self.is_multiline:
            self.first_line_length = 0
            self.last_line_length = 0
            self.max_line_length = 0
        else:
            # This never happens ATM, so we don't handle it.
            assert '\n' not in s

            self.first_line_length = len(s)
            self.last_line_length = len(s)
            self.max_line_length = len(s)

    def __iter__(self):
        return iter((self.s,))

class NonTerminalAstNode(AstNode):
    def __init__(self, children_ast_nodes):
        self.children_ast_nodes = children_ast_nodes
        first_line_length = 0
        last_line_length = 0
        is_multiline = False
        max_line_length = 0
        for node in children_ast_nodes:
            if node.is_multiline:
                last_line_length = node.last_line_length
                max_line_length = max(max_line_length, last_line_length + node.first_line_length, node.max_line_length)
                is_multiline = True
            else:
                last_line_length += node.last_line_length
                max_line_length = max(max_line_length, last_line_length)

        self.first_line_length = first_line_length
        self.last_line_length = last_line_length
        self.is_multiline = is_multiline
        self.max_line_length = max_line_length

    def __iter__(self):
        return itertools.chain(*self.children_ast_nodes)

max_line_length = 80
# Size of an indent in spaces.
single_indent_length = 4

class TerminalNodeFactory():
    def __init__(self, s):
        self.s = s

    def __call__(self, current_indent, current_line_length, inside_meta_type, last_token_was_type_wrapper, accept_single_line_only):
        return TerminalAstNode(self.s)

# 'balanced_string' nodes evaluate to a function (or a callable object) taking these parameters:
#     current_indent (integer): the indentation in the current line (spaces only)
#     current_line_length (integer): the number of preceding characters in the current line (>=current_indent)
#     inside_meta_type (boolean): whether we're inside a Type<...>
#     last_token_was_type_wrapper (boolean): whether the immediately-preceding token was the identifier 'Type'
#  and returning an AstNode
# 'comma_separated_balanced_string' nodes evaluate to a tuple of such functions

def p_comma_separated_balanced_string_empty(p):
    'comma_separated_balanced_string : '
    p[0] = tuple()

def p_comma_separated_balanced_string_not_empty(p):
    'comma_separated_balanced_string : COMMA balanced_string comma_separated_balanced_string'
    p[0] = (
        p[2],
        *(p[3])
    )

def p_optional_balanced_string_empty(p):
    'optional_balanced_string : '
    p[0] = TerminalNodeFactory('')

def p_optional_balanced_string_not_empty(p):
    'optional_balanced_string : balanced_string'
    p[0] = p[1]

class BalancedStringTerminalNodeFactory():
    def __init__(self, first_token, node_factory):
        self.first_token = first_token
        self.node_factory = node_factory

    def __call__(self, current_indent, current_line_length, inside_meta_type, last_token_was_type_wrapper, accept_single_line_only):
        terminal_node = TerminalAstNode(self.first_token)
        non_terminal_node = self.node_factory(
            current_indent,
            current_line_length + len(self.first_token),
            inside_meta_type,
            self.first_token == 'Type',
            accept_single_line_only)
        if non_terminal_node is None:
            return None
        return NonTerminalAstNode((terminal_node, non_terminal_node))

def p_balanced_string_terminal(p):
    '''balanced_string : DOUBLE_COLON balanced_string
                       | IDENTIFIER optional_balanced_string
                       | ASTERISK optional_balanced_string
                       | AMPERSAND optional_balanced_string
    '''
    first_token = p[1]
    node_factory = p[2]

    p[0] = BalancedStringTerminalNodeFactory(first_token, node_factory)

def create_composite_node_from_factories(node_factory_inside_meta_type_pairs, current_line_length, accept_single_line_only):
    nodes = []
    for node_factory, current_indent, inside_meta_type in node_factory_inside_meta_type_pairs:
        node = node_factory(current_indent, current_line_length, inside_meta_type, False, accept_single_line_only)
        if node is None:
            return None
        nodes.append(node)
        if node.is_multiline:
            if accept_single_line_only:
                raise Exception('Unexpected multiline, due to factory: ' + node_factory)
            # Note that due to the way we break lines, the last line will have the same indent as the first.
            # So we don't need to update current_indent here.
            current_line_length = node.last_line_length
        else:
            current_line_length += node.last_line_length
    return NonTerminalAstNode(nodes)

def compute_layout(left_token, intermediate_node_factories, right_token, rhs_node_factory, current_indent, current_line_length, inside_meta_type, last_token_was_type_wrapper, accept_single_line_only):
    # We lay out the result in one of two ways:
    #
    # $previousIndent $previousContent LPAREN x1, x2, x3 RPAREN balanced_string
    #
    # Or:
    #
    # $previousIndent $previousContent LPAREN
    # $previousIndent $indent x1 ,
    # $previousIndent $indent x2 ,
    # $previousIndent $indent x3 RPAREN balanced_string

    entering_meta_type = last_token_was_type_wrapper

    # First, we try to use the first format if possible
    node_factory_inside_meta_type_pairs = [
        (TerminalNodeFactory(left_token), current_indent, inside_meta_type),
        *((intermediate_node_factory, current_indent, (inside_meta_type or entering_meta_type))
          for intermediate_node_factory in intermediate_node_factories),
        (TerminalNodeFactory(right_token), current_indent, inside_meta_type),
        (rhs_node_factory, current_indent, inside_meta_type),
    ]
    node_with_single_line_layout = create_composite_node_from_factories(node_factory_inside_meta_type_pairs, current_line_length, True)
    if node_with_single_line_layout is not None and node_with_single_line_layout.max_line_length <= max_line_length:
        assert not node_with_single_line_layout.is_multiline
        return node_with_single_line_layout

    if accept_single_line_only:
        return None

    # The result exceeds the line length, let's switch to the second one.
    node_factory_inside_meta_type_pairs = [
        (TerminalNodeFactory(left_token),
         current_indent,
         inside_meta_type)
    ]
    new_indent_length = current_indent + single_indent_length
    comma_node_factory_inside_meta_type_pair = (TerminalNodeFactory(','), current_indent, inside_meta_type or entering_meta_type)
    newline_node_factory_inside_meta_type_pair = (TerminalNodeFactory('\n'), current_indent, inside_meta_type or entering_meta_type)
    indent_node_factory_inside_meta_type_pair = (TerminalNodeFactory(' ' * new_indent_length), current_indent, inside_meta_type or entering_meta_type)
    for inner_node_factory in intermediate_node_factories:
        node_factory_inside_meta_type_pairs.append(newline_node_factory_inside_meta_type_pair)
        node_factory_inside_meta_type_pairs.append(indent_node_factory_inside_meta_type_pair)
        node_factory_inside_meta_type_pairs.append((inner_node_factory, new_indent_length, inside_meta_type or entering_meta_type))
        node_factory_inside_meta_type_pairs.append(comma_node_factory_inside_meta_type_pair)
    node_factory_inside_meta_type_pairs.pop()
    node_factory_inside_meta_type_pairs.append((TerminalNodeFactory(right_token), current_indent, inside_meta_type))
    node_factory_inside_meta_type_pairs.append((rhs_node_factory, current_indent, inside_meta_type))
    return create_composite_node_from_factories(node_factory_inside_meta_type_pairs, current_line_length, accept_single_line_only)


def p_balanced_string_with_balanced_token_no_comma_separated_elems(p):
    '''balanced_string : LPAREN    RPAREN       optional_balanced_string
                       | LBRACKET  RBRACKET     optional_balanced_string
                       | LBRACE    RBRACE       optional_balanced_string
                       | LESS_THAN GREATER_THAN optional_balanced_string
    '''
    p_1 = p[1]
    p_2 = p[2]
    p_3 = p[3]

    def result(current_indent, current_line_length, inside_meta_type, last_token_was_type_wrapper, accept_single_line_only):
        return compute_layout(p_1, [], p_2, p_3, current_indent, current_line_length, inside_meta_type, last_token_was_type_wrapper, accept_single_line_only)

    p[0] = result

def p_balanced_string_with_balanced_token_some_comma_separated_elems(p):
    '''balanced_string : LPAREN    balanced_string comma_separated_balanced_string RPAREN       optional_balanced_string
                       | LBRACKET  balanced_string comma_separated_balanced_string RBRACKET     optional_balanced_string
                       | LBRACE    balanced_string comma_separated_balanced_string RBRACE       optional_balanced_string
                       | LESS_THAN balanced_string comma_separated_balanced_string GREATER_THAN optional_balanced_string
    '''
    p_1 = p[1]
    p_2 = p[2]
    p_3 = p[3]
    p_4 = p[4]
    p_5 = p[5]

    def result(current_indent, current_line_length, inside_meta_type, last_token_was_type_wrapper, accept_single_line_only):
        if not inside_meta_type:
            if p_1 == '(' and p_4 == ')':
                if len(p_3) == 0:
                    if isinstance(p_2, BalancedStringTerminalNodeFactory) and p_2.first_token == '*':
                        if isinstance(p_2.node_factory, TerminalNodeFactory) and p_2.node_factory.s == '':
                            # Special case: we're not inside a Type<...> and we've encountered a '(*)'.
                            # Discard it and just print the rhs.
                            return p_5(current_indent, current_line_length, inside_meta_type, False, accept_single_line_only)

        return compute_layout(p_1, (p_2, *(p_3)), p_4, p_5, current_indent, current_line_length, inside_meta_type, last_token_was_type_wrapper, accept_single_line_only)

    p[0] = result

def p_error(p):
    raise Exception("Syntax error when parsing meta type: ", p[:])

lexer = lex.lex()
parser = yacc.yacc(start='balanced_string')

strings_to_remove = re.compile(r'template class |template type alias |function template specialization |member class |member function |default argument for |fruit::impl::meta::|fruit::impl::|fruit::')

def do_simplify_template_trace_element(element):
    element, _ = re.subn(strings_to_remove, '', element)
    element = element.strip()
    if element[0] != '\'' or element[-1] != '\'':
        raise Exception('Expected single quotes in: ' + element)
    element = element[1:-1]
    if element.startswith('DoEval<') and element[-1] == '>':
        element = element[7:-1]
    result = ''.join(parser.parse(element, lexer)(0, 0, False, False, False))
    return result

@memoize(maxsize=1000)
def simplify_template_trace_element(element, executor):
    return executor.submit(do_simplify_template_trace_element, element)

def to_dot_left_justified_string(s):
    return '\\l'.join(s.splitlines() + [''])

def main():
    diagnostics = []

    with futures.ProcessPoolExecutor() as executor:
        lines = sys.stdin.readlines()
        for line_number, line in enumerate(lines):
            # Remove the newline
            line = line[:-1]

            matches = in_file_included_from_pattern.search(line)
            if matches:
                continue

            matches = diagnostic_header_pattern.search(line)
            if matches:
                diagnostic_kind, diagnostic_message = matches.groups()
                if diagnostic_kind == 'error':
                    diagnostics.append(Diagnostic(diagnostic_kind, diagnostic_message))
                    print('Processing diagnostic. (%s / %s) ' % (line_number, len(lines)), file=sys.stderr)
                elif diagnostic_kind == 'note':
                    matches = in_instantiation_of_template_pattern.search(diagnostic_message)
                    if matches:
                        if not diagnostics:
                            raise Exception('Found template instantiation note before any error diagnostic: %s' % diagnostic_message)
                        if 'in instantiation of template type alias' in line:
                            pass
                        else:
                            group = matches.groups()[0]
                            trace_element_future = simplify_template_trace_element(group, executor)
                            diagnostics[-1].template_instantiation_trace.append(trace_element_future)
                        continue

                    matches = static_warning_marked_deprecated_here_pattern.search(diagnostic_message)
                    if matches:
                        continue

                    raise Exception('Found unknown note: %s' % diagnostic_message)

        call_graph = {}
        graph = gv.AGraph(directed=True)

        for diagnostic_index, diagnostic in enumerate(diagnostics):
            if diagnostic_index % 10 == 0:
                print('Constructing dep graph: iteration %s/%s' % (diagnostic_index, len(diagnostics)), file=sys.stderr)

            template_instantiation_trace = [trace_element_future.result() for trace_element_future in diagnostic.template_instantiation_trace]
            for called, caller in zip(template_instantiation_trace[1:], template_instantiation_trace[2:]):
                if called in call_graph and call_graph[called] != caller:
                    # Avoid this edge, so that the resulting graph is a tree
                    continue
                graph.add_edge(to_dot_left_justified_string(caller), to_dot_left_justified_string(called))
                call_graph[called] = caller

        print(graph)

if __name__ == '__main__':
    main()
