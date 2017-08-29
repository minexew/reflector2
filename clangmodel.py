from clang.cindex import CursorKind, Index
from pprint import pprint
import cxxmodel

def dump_node(node, depth=0):
    indent = '  ' * depth

    if depth >= 10:
        print(indent + '...')
        return

    print(indent + '- kind', node.kind)
    print(indent + '  type', node.type.spelling)
    print(indent + '  type.kind', node.type.kind)
    print(indent + '  usr', node.get_usr())
    print(indent + '  spelling', node.spelling)
    print(indent + '  location', node.location)
    print(indent + '  extent.start', node.extent.start)
    print(indent + '  extent.end', node.extent.end)
    print(indent + '  is_definition', node.is_definition())
    if node.kind == CursorKind.ENUM_CONSTANT_DECL:
        print(indent + '  enum_value', node.enum_value)
    #print(indent + '  get_definition', node.get_definition())
    print()

    for child in node.get_children():
        dump_node(child, depth + 1)


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

    def process_enum(self, node):
        enum = cxxmodel.Enum(node.spelling)

        for child in node.get_children():
            if child.kind == CursorKind.ENUM_CONSTANT_DECL:
                enum.add_value(child.spelling, child.enum_value)

        return enum

    def walk_namespace_children(self, parent):
        for node in parent.get_children():
            if node.kind == CursorKind.NAMESPACE:
                #print('clangmodel: ns', node.spelling)
                prev_namespace = self.namespace
                prev_namespace_full_path = self.namespace_full_path

                self.namespace = self.get_or_create_namespace(node.spelling)
                self.namespace_full_path += '::' + self.namespace.name

                self.walk_namespace_children(node)

                self.namespace_full_path = prev_namespace_full_path
                self.namespace = prev_namespace
            elif node.kind == CursorKind.STRUCT_DECL:
                self.namespace.add_member(self.process_class(node, True))
            else:
                print('clangmodel.AstReconstructor.walk_namespace_children: unk kind', node.kind)

    def walk_struct_children(self, struct, parent):
        for node in parent.get_children():
            if node.kind == CursorKind.ANNOTATE_ATTR:
                struct.add_annotation(node.spelling)
            elif node.kind == CursorKind.ENUM_DECL:
                struct.add_member(self.process_enum(node))
            elif node.kind == CursorKind.FIELD_DECL:
                struct.add_member(cxxmodel.Field(node.type.spelling, node.spelling))
            elif node.kind == CursorKind.STRUCT_DECL:
                struct.add_member(self.process_class(node, True))
            elif node.kind == CursorKind.VAR_DECL:
                dump_node(node)
            else:
                print('clangmodel.AstReconstructor.walk_struct_children: unk kind', node.kind, 'in struct', struct.name)

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
