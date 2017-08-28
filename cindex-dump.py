#!/usr/bin/env python3

#===- cindex-dump.py - cindex/Python Source Dump -------------*- python -*--===#
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
#===------------------------------------------------------------------------===#

"""
A simple command line tool for dumping a source file using the Clang Index
Library.
"""

from clang.cindex import CursorKind, Index
from pprint import pprint

from optparse import OptionParser, OptionGroup

import cxxmodel

def get_diag_info(diag):
    return { 'severity' : diag.severity,
             'location' : diag.location,
             'spelling' : diag.spelling,
             'ranges' : diag.ranges,
             'fixits' : diag.fixits }

def get_cursor_id(cursor, cursor_list = []):
    if not opts.showIDs:
        return None

    if cursor is None:
        return None

    # FIXME: This is really slow. It would be nice if the index API exposed
    # something that let us hash cursors.
    for i,c in enumerate(cursor_list):
        if cursor == c:
            return i
    cursor_list.append(cursor)
    return len(cursor_list) - 1

def get_info(node, depth=0):
    if opts.maxDepth is not None and depth >= opts.maxDepth:
        children = None
    else:
        children = [get_info(c, depth+1)
                    for c in node.get_children()]
    return { 'id' : get_cursor_id(node),
             'kind' : node.kind,
             'usr' : node.get_usr(),
             'spelling' : node.spelling,
             'location' : node.location,
             'extent.start' : node.extent.start,
             'extent.end' : node.extent.end,
             'is_definition' : node.is_definition(),
             'definition id' : get_cursor_id(node.get_definition()),
             'children' : children }

class AstReconstructor:
    def __init__(self):
        self.namespace = cxxmodel.Namespace(None)
        self.namespace_full_path = ''

    def get_or_create_namespace(self, name):
        existing = self.namespace.find_member(name, cxxmodel.Namespace)

        if existing is not None: return existing

        new = cxxmodel.Namespace(name)
        self.namespace.add_member(new)
        print('+ns', self.namespace_full_path + '::' + new.name)
        return new

    def walk_namespace_children(self, parent):
        for node in parent.get_children():
            if node.kind == CursorKind.NAMESPACE:
                prev_namespace = self.namespace
                prev_namespace_full_path = self.namespace_full_path

                self.namespace = self.get_or_create_namespace(node.spelling)
                self.namespace_full_path += '::' + self.namespace.name

                self.walk_namespace_children(node)

                self.namespace_full_path = prev_namespace_full_path
                self.namespace = prev_namespace
            elif node.kind == CursorKind.STRUCT_DECL:
                print('struct', node.spelling)
                struct = cxxmodel.Class(node.spelling, True)

                self.walk_struct_children(struct, node)

    def walk_struct_children(self, struct, parent):
        for node in parent.get_children():
            if node.kind == CursorKind.ANNOTATE_ATTR:
                print('@' + node.spelling)
                struct.add_annotation(node.spelling)
            elif node.kind == CursorKind.FIELD_DECL:
                print(node.spelling)

def main():
    global opts

    parser = OptionParser("usage: %prog [options] {filename} [clang-args*]")
    parser.add_option("", "--show-ids", dest="showIDs",
                      help="Compute cursor IDs (very slow)",
                      action="store_true", default=False)
    parser.add_option("", "--max-depth", dest="maxDepth",
                      help="Limit cursor expansion to depth N",
                      metavar="N", type=int, default=None)
    parser.disable_interspersed_args()
    (opts, args) = parser.parse_args()

    if len(args) == 0:
        parser.error('invalid number arguments')

    index = Index.create()
    tu = index.parse(args[0], args[1:])
    if not tu:
        parser.error("unable to load input")

    pprint(('diags', list(map(get_diag_info, tu.diagnostics))))
    #pprint(('nodes', get_info(tu.cursor)))

    ar = AstReconstructor()
    ar.walk_namespace_children(tu.cursor)

if __name__ == '__main__':
    main()

