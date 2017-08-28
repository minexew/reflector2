#!/usr/bin/env python3

import argparse
import json

parser = argparse.ArgumentParser(description='Process some integers.')
'''parser.add_argument('integers', metavar='N', type=int, nargs='+',
                    help='an integer for the accumulator')
parser.add_argument('--sum', dest='accumulate', action='store_const',
                    const=sum, default=max,
                    help='sum the integers (default: find the max)')'''

parser.add_argument('model_json')
parser.add_argument('output_hpp')

args = parser.parse_args()

with open(args.model_json) as f:
    model = json.load(f)

class HppGen:
    def __init__(self, output):
        self.indent_level = 0
        self.namespaces = []
        self.output = output

    def begin_class(self, name):
        self.emit('struct %s {' % name)
        self.indent()

    def declare_field(self, name, type):
        self.emit('%s %s;' % (type, name))

    def emit(self, *args, **kwargs):
        print('    ' * self.indent_level, end='', file=self.output)
        print(*args, **kwargs, file=self.output)

    def end_class(self):
        self.unindent()
        self.emit('};')

    def indent(self):
        self.indent_level += 1

    def push_namespace(self, ns):
        self.namespaces = [ns] + self.namespaces

        self.emit('namespace %s {' % ns)

    def pop_namespace(self):
        self.namespaces = self.namespaces[1:]

        self.emit('}')

    def unindent(self):
        self.indent_level -= 1

class Traverser:
    def __init__(self, hpp_gen):
        self.hpp = hpp_gen

    def walk_children(self, parent):
        for node in parent['children']:
            if node['kind'] == 'field_decl':
                self.hpp.declare_field(node['name'], node['type_string1'])
            elif node['kind'] == 'namespace':
                self.hpp.push_namespace(node['name'])

                self.walk_children(node)

                self.hpp.pop_namespace()
            elif node['kind'] == 'struct':
                self.hpp.begin_class(node['name'])

                self.walk_children(node)

                self.hpp.end_class()
            else:
                print('unk', node['kind'])

with open(args.output_hpp, 'w') as hpp:
    hpp_gen = HppGen(hpp)

    t = Traverser(hpp_gen)
    t.walk_children(model)
