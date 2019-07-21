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

from absl.testing import parameterized
from fruit_test_common import *

COMMON_DEFINITIONS = '''
    #include "test_common.h"
    
    #define IN_FRUIT_CPP_FILE 1
    #include <fruit/impl/data_structures/semistatic_graph.templates.h>
    
    using namespace std;
    using namespace fruit::impl;
    
    using Graph = SemistaticGraph<int, const char*>;
    using node_iterator = Graph::node_iterator;
    using edge_iterator = Graph::edge_iterator;
    
    vector<int> no_neighbors{};
    
    struct SimpleNode {
      int id;
      const char* value;
      const vector<int>* neighbors;
      bool is_terminal;
      
      int getId() { return id; }
      const char* getValue() { return value; }
      bool isTerminal() { return is_terminal; }
      vector<int>::const_iterator getEdgesBegin() { return neighbors->begin(); }
      vector<int>::const_iterator getEdgesEnd() { return neighbors->end(); }
    };
    '''

class TestSemistaticGraph(parameterized.TestCase):
    def test_empty(self):
        source = '''
            int main() {
              MemoryPool memory_pool;
              vector<SimpleNode> values{};
              
              Graph graph(values.begin(), values.end(), memory_pool);
              Assert(graph.find(0) == graph.end());
              Assert(graph.find(2) == graph.end());
              Assert(graph.find(5) == graph.end());
              const Graph& cgraph = graph;
              Assert(cgraph.find(0) == cgraph.end());
              Assert(cgraph.find(2) == cgraph.end());
              Assert(cgraph.find(5) == cgraph.end());
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_1_node_no_edges(self):
        source = '''
            int main() {
              MemoryPool memory_pool;
              vector<SimpleNode> values{{2, "foo", &no_neighbors, false}};
            
              Graph graph(values.begin(), values.end(), memory_pool);
              Assert(graph.find(0) == graph.end());
              Assert(!(graph.find(2) == graph.end()));
              Assert(graph.at(2).getNode() == string("foo"));
              Assert(graph.at(2).isTerminal() == false);
              Assert(graph.find(5) == graph.end());
              const Graph& cgraph = graph;
              Assert(cgraph.find(0) == cgraph.end());
              Assert(!(cgraph.find(2) == cgraph.end()));
              Assert(cgraph.find(2).getNode() == string("foo"));
              Assert(cgraph.find(2).isTerminal() == false);
              Assert(cgraph.find(5) == cgraph.end());
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_1_node_no_edges_terminal(self):
        source = '''
            int main() {
              MemoryPool memory_pool;
              vector<SimpleNode> values{{2, "foo", &no_neighbors, true}};
              
              Graph graph(values.begin(), values.end(), memory_pool);
              Assert(graph.find(0) == graph.end());
              Assert(!(graph.find(2) == graph.end()));
              Assert(graph.at(2).getNode() == string("foo"));
              Assert(graph.at(2).isTerminal() == true);
              Assert(graph.find(5) == graph.end());
              const Graph& cgraph = graph;  
              Assert(cgraph.find(0) == cgraph.end());
              Assert(!(cgraph.find(2) == cgraph.end()));
              Assert(cgraph.find(2).getNode() == string("foo"));
              Assert(cgraph.find(2).isTerminal() == true);
              Assert(cgraph.find(5) == cgraph.end());
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_1_node_self_edge(self):
        source = '''
            int main() {
              MemoryPool memory_pool;
              vector<int> neighbors = {2};
              vector<SimpleNode> values{{2, "foo", &neighbors, false}};
              
              Graph graph(values.begin(), values.end(), memory_pool);
              Assert(graph.find(0) == graph.end());
              Assert(!(graph.find(2) == graph.end()));
              Assert(graph.at(2).getNode() == string("foo"));
              Assert(graph.at(2).isTerminal() == false);
              edge_iterator itr = graph.at(2).neighborsBegin();
              (void)itr;
              Assert(itr.getNodeIterator(graph.begin()).getNode() == string("foo"));
              Assert(itr.getNodeIterator(graph.begin()).isTerminal() == false);
              Assert(graph.find(5) == graph.end());
              const Graph& cgraph = graph;
              Assert(cgraph.find(0) == cgraph.end());
              Assert(!(cgraph.find(2) == cgraph.end()));
              Assert(cgraph.find(2).getNode() == string("foo"));
              Assert(cgraph.find(2).isTerminal() == false);
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_2_nodes_one_edge(self):
        source = '''
            int main() {
              MemoryPool memory_pool;
              vector<int> neighbors = {2};
              vector<SimpleNode> values{{2, "foo", &no_neighbors, false}, {3, "bar", &neighbors, false}};
              
              Graph graph(values.begin(), values.end(), memory_pool);
              Assert(graph.find(0) == graph.end());
              Assert(!(graph.find(2) == graph.end()));
              Assert(graph.at(2).getNode() == string("foo"));
              Assert(graph.at(2).isTerminal() == false);
              Assert(graph.at(3).getNode() == string("bar"));
              Assert(graph.at(3).isTerminal() == false);
              edge_iterator itr = graph.at(3).neighborsBegin();
              (void)itr;
              Assert(itr.getNodeIterator(graph.begin()).getNode() == string("foo"));
              Assert(itr.getNodeIterator(graph.begin()).isTerminal() == false);
              Assert(graph.find(5) == graph.end());
              const Graph& cgraph = graph;
              Assert(cgraph.find(0) == cgraph.end());
              Assert(!(cgraph.find(2) == cgraph.end()));
              Assert(cgraph.find(2).getNode() == string("foo"));
              Assert(cgraph.find(2).isTerminal() == false);
              Assert(cgraph.find(3).getNode() == string("bar"));
              Assert(cgraph.find(3).isTerminal() == false);
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_3_nodes_two_edges(self):
        source = '''
            int main() {
              MemoryPool memory_pool;
              vector<int> neighbors = {2, 4};
              vector<SimpleNode> values{{2, "foo", &no_neighbors, false}, {3, "bar", &neighbors, false}, {4, "baz", &no_neighbors, true}};
              
              Graph graph(values.begin(), values.end(), memory_pool);
              Assert(graph.find(0) == graph.end());
              Assert(!(graph.find(2) == graph.end()));
              Assert(graph.at(2).getNode() == string("foo"));
              Assert(graph.at(2).isTerminal() == false);
              Assert(graph.at(3).getNode() == string("bar"));
              Assert(graph.at(3).isTerminal() == false);
              edge_iterator itr = graph.at(3).neighborsBegin();
              Assert(itr.getNodeIterator(graph.begin()).getNode() == string("foo"));
              Assert(itr.getNodeIterator(graph.begin()).isTerminal() == false);
              ++itr;
              Assert(itr.getNodeIterator(graph.begin()).getNode() == string("baz"));
              Assert(itr.getNodeIterator(graph.begin()).isTerminal() == true);
              Assert(graph.at(4).getNode() == string("baz"));
              Assert(graph.at(4).isTerminal() == true);
              Assert(graph.find(5) == graph.end());
              const Graph& cgraph = graph;
              Assert(cgraph.find(0) == cgraph.end());
              Assert(!(cgraph.find(2) == cgraph.end()));
              Assert(cgraph.find(2).getNode() == string("foo"));
              Assert(cgraph.find(2).isTerminal() == false);
              Assert(cgraph.find(3).getNode() == string("bar"));
              Assert(cgraph.find(3).isTerminal() == false);
              Assert(cgraph.find(4).getNode() == string("baz"));
              Assert(cgraph.find(4).isTerminal() == true);
              Assert(cgraph.find(5) == cgraph.end());
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_add_node(self):
        source = '''
            int main() {
              MemoryPool memory_pool;
              vector<SimpleNode> old_values{{2, "foo", &no_neighbors, false}, {4, "baz", &no_neighbors, true}};
              
              Graph old_graph(old_values.begin(), old_values.end(), memory_pool);
              vector<int> neighbors = {2, 4};
              vector<SimpleNode> new_values{{3, "bar", &neighbors, false}};
              
              Graph graph(old_graph, new_values.begin(), new_values.end(), memory_pool);
              Assert(graph.find(0) == graph.end());
              Assert(!(graph.find(2) == graph.end()));
              Assert(graph.at(2).getNode() == string("foo"));
              Assert(graph.at(2).isTerminal() == false);
              Assert(graph.at(3).getNode() == string("bar"));
              Assert(graph.at(3).isTerminal() == false);
              edge_iterator itr = graph.at(3).neighborsBegin();
              Assert(itr.getNodeIterator(graph.begin()).getNode() == string("foo"));
              Assert(itr.getNodeIterator(graph.begin()).isTerminal() == false);
              ++itr;
              Assert(itr.getNodeIterator(graph.begin()).getNode() == string("baz"));
              Assert(itr.getNodeIterator(graph.begin()).isTerminal() == true);
              Assert(graph.at(4).getNode() == string("baz"));
              Assert(graph.at(4).isTerminal() == true);
              Assert(graph.find(5) == graph.end());
              const Graph& cgraph = graph;
              Assert(cgraph.find(0) == cgraph.end());
              Assert(!(cgraph.find(2) == cgraph.end()));
              Assert(cgraph.find(2).getNode() == string("foo"));
              Assert(cgraph.find(2).isTerminal() == false);
              Assert(cgraph.find(3).getNode() == string("bar"));
              Assert(cgraph.find(3).isTerminal() == false);
              Assert(cgraph.find(4).getNode() == string("baz"));
              Assert(cgraph.find(4).isTerminal() == true);
              Assert(cgraph.find(5) == cgraph.end());
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_set_terminal(self):
        source = '''
            int main() {
              MemoryPool memory_pool;
              vector<int> neighbors = {2, 4};
              vector<SimpleNode> values{{2, "foo", &no_neighbors, false}, {3, "bar", &neighbors, false}, {4, "baz", &no_neighbors, true}};
              
              Graph graph(values.begin(), values.end(), memory_pool);
              graph.find(3).setTerminal();
              Assert(graph.find(0) == graph.end());
              Assert(!(graph.find(2) == graph.end()));
              Assert(graph.at(2).getNode() == string("foo"));
              Assert(graph.at(2).isTerminal() == false);
              Assert(graph.at(3).getNode() == string("bar"));
              Assert(graph.at(3).isTerminal() == true);
              Assert(graph.at(4).getNode() == string("baz"));
              Assert(graph.at(4).isTerminal() == true);
              Assert(graph.find(5) == graph.end());
              const Graph& cgraph = graph;
              Assert(cgraph.find(0) == cgraph.end());
              Assert(!(cgraph.find(2) == cgraph.end()));
              Assert(cgraph.find(2).getNode() == string("foo"));
              Assert(cgraph.find(3).getNode() == string("bar"));
              Assert(cgraph.find(4).getNode() == string("baz"));
              Assert(cgraph.find(5) == cgraph.end());
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_move_constructor(self):
        source = '''
            int main() {
              MemoryPool memory_pool;
              vector<int> neighbors = {2};
              vector<SimpleNode> values{{2, "foo", &no_neighbors, false}, {3, "bar", &neighbors, false}};
              
              Graph graph1(values.begin(), values.end(), memory_pool);
              Graph graph = std::move(graph1);
              Assert(graph.find(0) == graph.end());
              Assert(!(graph.find(2) == graph.end()));
              Assert(graph.at(2).getNode() == string("foo"));
              Assert(graph.at(2).isTerminal() == false);
              Assert(graph.at(3).getNode() == string("bar"));
              Assert(graph.at(3).isTerminal() == false);
              edge_iterator itr = graph.at(3).neighborsBegin();
              (void)itr;
              Assert(itr.getNodeIterator(graph.begin()).getNode() == string("foo"));
              Assert(itr.getNodeIterator(graph.begin()).isTerminal() == false);
              Assert(graph.find(5) == graph.end());
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_move_assignment(self):
        source = '''
            int main() {
              MemoryPool memory_pool;
              vector<int> neighbors = {2};
              vector<SimpleNode> values{{2, "foo", &no_neighbors, false}, {3, "bar", &neighbors, false}};
              
              Graph graph1(values.begin(), values.end(), memory_pool);
              Graph graph;
              graph = std::move(graph1);
              Assert(graph.find(0) == graph.end());
              Assert(!(graph.find(2) == graph.end()));
              Assert(graph.at(2).getNode() == string("foo"));
              Assert(graph.at(2).isTerminal() == false);
              Assert(graph.at(3).getNode() == string("bar"));
              Assert(graph.at(3).isTerminal() == false);
              edge_iterator itr = graph.at(3).neighborsBegin();
              (void)itr;
              Assert(itr.getNodeIterator(graph.begin()).getNode() == string("foo"));
              Assert(itr.getNodeIterator(graph.begin()).isTerminal() == false);
              Assert(graph.find(5) == graph.end());
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_incomplete_graph(self):
        source = '''
            int main() {
              MemoryPool memory_pool;
              vector<int> neighbors = {2};
              vector<SimpleNode> values{{1, "foo", &neighbors, false}};
            
              Graph graph(values.begin(), values.end(), memory_pool);
              Assert(!(graph.find(1) == graph.end()));
              Assert(graph.at(1).getNode() == string("foo"));
              Assert(graph.at(1).isTerminal() == false);
              Assert(graph.find(2) == graph.end());
              const Graph& cgraph = graph;
              Assert(!(cgraph.find(1) == cgraph.end()));
              Assert(cgraph.find(1).getNode() == string("foo"));
              Assert(cgraph.find(1).isTerminal() == false);
              Assert(cgraph.find(2) == cgraph.end());
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

if __name__ == '__main__':
    absltest.main()
