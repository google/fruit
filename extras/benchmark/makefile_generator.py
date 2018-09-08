# Copyright 2016 Google Inc. All Rights Reserved.
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


def generate_makefile(cpp_files, executable_name, compile_command, link_command, link_command_suffix):
    assert executable_name + '.cpp' in cpp_files, '%s.cpp in %s' % (executable_name, cpp_files)

    link_rule_template = """
{executable_name}: {object_files}
\t{link_command} {object_files} -o {executable_name} {link_command_suffix}
"""
    compile_rule_template = """
{name}.o: {name}.cpp
\t{compile_command} -c {name}.cpp -o {name}.o
"""

    clean_rule_template = """
clean:
\trm -f {object_files} {dep_files} {executable_name}
"""

    dep_file_deps = """
%.d: ;
"""

    dep_files_includes_template = """
include {dep_files}
"""

    compile_rules = []
    object_files = []
    dep_files = []
    for cpp_file in cpp_files:
        assert cpp_file.endswith('.cpp')
        source = cpp_file[:-len('.cpp')]

        compile_rule = compile_rule_template.format(
            name=source,
            compile_command=compile_command)
        compile_rules.append(compile_rule)
        object_files.append('%s.o' % source)
        dep_files.append('%s.d' % source)

    link_rule = link_rule_template.format(
        object_files=' '.join(object_files),
        link_command=link_command,
        link_command_suffix=link_command_suffix,
        executable_name=executable_name)

    clean_rule = clean_rule_template.format(
        object_files=' '.join(object_files),
        executable_name=executable_name,
        dep_files=' '.join(dep_files))

    dep_files_includes = dep_files_includes_template.format(dep_files=' '.join(dep_files))

    # We put the link rule first so that it's the default Make target.
    return link_rule + ''.join(compile_rules) + clean_rule + dep_file_deps + dep_files_includes
