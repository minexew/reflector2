class Namespace:
    def __init__(self, name):
        self.members = []
        self.name = name

    def add_member(self, member):
        self.members.append(member)

    def find_member(self, name, type=None):
        for m in self.members:
            if (type is None or isinstance(m, type)) and m.name == name:
                return m

    def find_namespace(self, path):
        start, delimiter, end = path.partition('::')

        if not start:
            return self

        ns = self.find_member(start, Namespace)
        if ns:
            return ns.find_namespace(end)
        else:
            raise KeyError('ehh')


class Type:
    def __init__(self):
        pass


class Field:
    def __init__(self, type_str, name):
        self.annotations = []
        self.type_str = type_str
        self.name = name

    def add_annotation(self, annotation):
        self.annotations.append(annotation)

class Class(Type):
    def __init__(self, name, is_struct=False):
        self.annotations = []
        self.is_struct = is_struct
        self.name = name
        self.members = []

    def add_annotation(self, annotation):
        self.annotations.append(annotation)

    def add_member(self, members):
        self.members.append(members)
