#!/usr/bin/env python3

import argparse
import clangmodel
import cxxmodel

parser = argparse.ArgumentParser(description='Process some integers.')
'''parser.add_argument('integers', metavar='N', type=int, nargs='+',
                    help='an integer for the accumulator')
parser.add_argument('--sum', dest='accumulate', action='store_const',
                    const=sum, default=max,
                    help='sum the integers (default: find the max)')'''

parser.add_argument('-o', dest='output_hpp')
parser.add_argument('source_file')
parser.add_argument('-N', dest='namespace', default='')
#parser.add_argument('options', nargs='*')

args, unknown = parser.parse_known_args()

model = clangmodel.parse_file(args.source_file, unknown)

class HppGen:
    def __init__(self, output):
        self.indent_level = 0
        self.namespaces = []
        self.output = output

    def begin_class(self, class_):
        self.emit('struct %s {' % class_.name)
        self.indent()

    def declare_field(self, name, type):
        self.emit('%s %s;' % (type, name))

    def emit(self, *args, **kwargs):
        print('    ' * self.indent_level, end='', file=self.output)
        print(*args, **kwargs, file=self.output)

    def end_class(self):
        self.unindent()
        self.emit('};')
        self.emit()

    def indent(self):
        self.indent_level += 1

    def push_namespace(self, ns_name):
        self.namespaces = [ns_name] + self.namespaces

        self.emit('namespace %s {' % ns_name)

    def pop_namespace(self):
        self.namespaces = self.namespaces[1:]

        self.emit('}')

    def unindent(self):
        self.indent_level -= 1

class Traverser:
    def __init__(self, hpp_gen):
        self.hpp = hpp_gen

    def process_class(self, class_):
        self.hpp.begin_class(class_)

        for member in class_.members:
            if isinstance(member, cxxmodel.Class):
                self.process_class(member)
            elif isinstance(member, cxxmodel.Field):
                self.hpp.declare_field(member.name, member.type_str)

        self.hpp.end_class()

    def walk_namespace(self, ns):
        for member in ns.members:
            if isinstance(member, cxxmodel.Class):
                #self.process_class(member)
                pass
            elif isinstance(member, cxxmodel.Namespace):
                self.hpp.push_namespace(member.name)

                self.walk_namespace(member)

                self.hpp.pop_namespace()
            else:
                print('unk', member)

with open(args.output_hpp, 'w') as hpp:
    hpp_gen = HppGen(hpp)

    t = Traverser(hpp_gen)
    hpp_gen.push_namespace(args.namespace)      # FIXME: buggy is namespace==''
    t.walk_namespace(model.find_namespace(args.namespace))
    hpp_gen.pop_namespace()
