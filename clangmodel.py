from clang.cindex import CursorKind, Index
from pprint import pprint
import cxxmodel

class AstReconstructor:
    def __init__(self):
        self.namespace = cxxmodel.Namespace(None)
        self.namespace_full_path = ''

    def get_or_create_namespace(self, name):
        existing = self.namespace.find_member(name, cxxmodel.Namespace)

        if existing is not None: return existing

        new = cxxmodel.Namespace(name)
        self.namespace.add_member(new)
        return new

    def process_class(self, node, is_struct):
        struct = cxxmodel.Class(node.spelling, is_struct)
        self.walk_struct_children(struct, node)
        return struct

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
                self.namespace.add_member(self.process_class(node, True))

    def walk_struct_children(self, struct, parent):
        for node in parent.get_children():
            if node.kind == CursorKind.ANNOTATE_ATTR:
                struct.add_annotation(node.spelling)
            elif node.kind == CursorKind.FIELD_DECL:
                struct.add_member(cxxmodel.Field(node.type.spelling, node.spelling))
            elif node.kind == CursorKind.STRUCT_DECL:
                struct.add_member(self.process_class(node, True))
            else:
                print('walk_struct_children: kind', node.kind)

def get_diag_info(diag):
    return { 'severity' : diag.severity,
             'location' : diag.location,
             'spelling' : diag.spelling,
             'ranges' : diag.ranges,
             'fixits' : diag.fixits }

def parse_file(path, args):
    index = Index.create()
    tu = index.parse(path, args)
    if not tu:
        raise Exception()

    pprint(('diags', list(map(get_diag_info, tu.diagnostics))))
    #pprint(('nodes', get_info(tu.cursor)))

    ar = AstReconstructor()
    ar.walk_namespace_children(tu.cursor)
    return ar.namespace
