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


class Class:
    def __init__(self, name, is_struct=False):
        self.annotations = []
        self.fields = []
        self.is_struct = is_struct
        self.name = name

    def add_annotation(self, annotation):
        self.annotations.append(annotation)
