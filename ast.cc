#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <clang-c/Index.h>

#include <json.hpp>

using nlohmann::json;

struct VisitClientData {
    unsigned int level;
    json& model;
    json* node;
    bool print_all = false;
    bool record_classes = false;
};

void show_spell(const CXCursor &cursor) {
    CXString spell = clang_getCursorSpelling(cursor);
    printf("  Text: %s\n", clang_getCString(spell));
    clang_disposeString(spell);
}

void show_type(const CXCursor &cursor) {
    CXType type = clang_getCursorType(cursor);
    CXString typeName = clang_getTypeSpelling(type);
    CXTypeKind typeKind = type.kind;
    CXString typeKindName = clang_getTypeKindSpelling(typeKind);
    printf("  Type: %s\n", clang_getCString(typeName));
    printf("  TypeKind: %s\n", clang_getCString(typeKindName));
    clang_disposeString(typeName);
    clang_disposeString(typeKindName);
}

void show_linkage(const CXCursor &cursor) {
    CXLinkageKind linkage = clang_getCursorLinkage(cursor);
    const char *linkageName;
    switch (linkage) {
        case CXLinkage_Invalid:        linkageName = "Invalid"; break;
        case CXLinkage_NoLinkage:      linkageName = "NoLinkage"; break;
        case CXLinkage_Internal:       linkageName = "Internal"; break;
        case CXLinkage_UniqueExternal: linkageName = "UniqueExternal"; break;
        case CXLinkage_External:       linkageName = "External"; break;
        default:                       linkageName = "Unknown"; break;
    }
    printf("  Linkage: %s\n", linkageName);
}

void show_parent(const CXCursor &cursor, const CXCursor &parent) {
    CXCursor semaParent = clang_getCursorSemanticParent(cursor);
    CXCursor lexParent  = clang_getCursorLexicalParent(cursor);
    CXString parentName = clang_getCursorSpelling(parent);
    CXString semaParentName = clang_getCursorSpelling(semaParent);
    CXString lexParentName = clang_getCursorSpelling(lexParent);
    printf("  Parent: parent:%s semantic:%s lexicial:%s\n",
                 clang_getCString(parentName),
                 clang_getCString(semaParentName),
                 clang_getCString(lexParentName));

    clang_disposeString(parentName);
    clang_disposeString(semaParentName);
    clang_disposeString(lexParentName);
}

void show_location(const CXCursor &cursor) {
    CXSourceLocation loc = clang_getCursorLocation(cursor);
    CXFile file;
    unsigned line, column, offset;
    clang_getSpellingLocation(loc, &file, &line, &column, &offset);
    CXString fileName = clang_getFileName(file);
    printf("  Location: %s:%d:%d:%d\n",
                 clang_getCString(fileName), line, column, offset);
    clang_disposeString(fileName);
}

void show_usr(const CXCursor &cursor) {
    CXString usr = clang_getCursorUSR(cursor);
    printf("  USR: %s\n", clang_getCString(usr));
    clang_disposeString(usr);
}

void show_cursor_kind(const CXCursor &cursor) {
    CXCursorKind curKind  = clang_getCursorKind(cursor);
    CXString curKindName  = clang_getCursorKindSpelling(curKind);

    const char *type;
    if (clang_isAttribute(curKind))            type = "Attribute";
    else if (clang_isDeclaration(curKind))     type = "Declaration";
    else if (clang_isExpression(curKind))      type = "Expression";
    else if (clang_isInvalid(curKind))         type = "Invalid";
    else if (clang_isPreprocessing(curKind))   type = "Preprocessing";
    else if (clang_isReference(curKind))       type = "Reference";
    else if (clang_isStatement(curKind))       type = "Statement";
    else if (clang_isTranslationUnit(curKind)) type = "TranslationUnit";
    else if (clang_isUnexposed(curKind))       type = "Unexposed";
    else                                       type = "Unknown";

    printf("  CursorKind: %s\n", clang_getCString(curKindName));
    printf("  CursorKindType: %s\n", type);
    clang_disposeString(curKindName);
}

void show_included_file(const CXCursor &cursor) {
    CXFile included = clang_getIncludedFile(cursor);
    if (included == 0) return;

    CXString includedFileName = clang_getFileName(included);
    printf(" included file: %s\n", clang_getCString(includedFileName));
    clang_disposeString(includedFileName);
}

bool is_desirable(CXCursor cursor) {
    CXSourceLocation loc = clang_getCursorLocation(cursor);
    CXFile file;
    unsigned line, column, offset;
    clang_getSpellingLocation(loc, &file, &line, &column, &offset);
    CXString fileName = clang_getFileName(file);
    assert(clang_getCString(fileName) != nullptr);
    bool desirable = strcmp("proto.hpp", clang_getCString(fileName)) == 0;
    clang_disposeString(fileName);
    return desirable;
}

static CXChildVisitResult dump_cursor(CXCursor cursor, CXCursor parent, CXClientData client_data) {
    unsigned int level = *reinterpret_cast<unsigned int*>(client_data);
    printf("  Level: %d\n", level);

    show_spell(cursor);
    show_linkage(cursor);
    show_cursor_kind(cursor);
    show_type(cursor);
    show_parent(cursor, parent);
    show_location(cursor);
    show_usr(cursor);
    show_included_file(cursor);
    printf("\n");

    // visit children recursively
    unsigned next = level + 1;
    clang_visitChildren(cursor, dump_cursor, &next);

    return CXChildVisit_Continue;
}

class AutoCXString {
public:
    AutoCXString(CXString cxs) : cxs{cxs} {}
    ~AutoCXString() { clang_disposeString(cxs); }

    const char* c_str() const { return clang_getCString(cxs); }
    operator CXString() const { return cxs; }

private:
    CXString cxs;
};

CXChildVisitResult visitChildrenCallback(CXCursor cursor, CXCursor parent, CXClientData client_data) {
    CXCursorKind curKind  = clang_getCursorKind(cursor);
    AutoCXString spell(clang_getCursorSpelling(cursor));

    auto& vcd = *reinterpret_cast<VisitClientData*>(client_data);
    auto& node = *vcd.node;

    if (vcd.print_all) {
        //fprintf(stderr, "[%d] %s\n", (int)curKind, clang_getCString(spell));
        unsigned int level = 0;
        dump_cursor(cursor, parent, &level);
    }

    switch (curKind) {
    case CXCursor_AnnotateAttr: {
        json ns {{"kind", "annotate_attr"}, {"name", spell.c_str()}};
        node.push_back(std::move(ns));
        break;
    }

    case CXCursor_FieldDecl: {
        auto type = clang_getCursorType(cursor);
        AutoCXString type_spell(clang_getTypeSpelling(type));
        bool print_all = vcd.print_all;
        json ns {{"kind", "field_decl"}, {"name", spell.c_str()}, {"type_string1", type_spell.c_str()}, {"children", json::array()}};
        vcd.node = &ns["children"];
        vcd.print_all = true;

        clang_visitChildren(cursor, visitChildrenCallback, client_data);

        vcd.print_all = print_all;
        vcd.node = &node;
        node.push_back(std::move(ns));
        break;
    }

    case CXCursor_Namespace: {
        // fast path to ignore std::*
        if (strcmp(spell.c_str(), "agdg") != 0)
            break;

        bool record_classes = vcd.record_classes;
        json ns {{"kind", "namespace"}, {"name", spell.c_str()}, {"children", json::array()}};
        vcd.node = &ns["children"];
        vcd.record_classes = true;

        clang_visitChildren(cursor, visitChildrenCallback, client_data);

        vcd.record_classes = (strcmp(spell.c_str(), "agdg") == 0);
        vcd.node = &node;
        node.push_back(std::move(ns));
        break;
    }

    case CXCursor_StructDecl: {
        if (!vcd.record_classes)
            break;

        json ns {{"kind", "struct"}, {"name", spell.c_str()}, {"children", json::array()}};
        vcd.node = &ns["children"];

        clang_visitChildren(cursor, visitChildrenCallback, client_data);

        vcd.node = &node;
        node.push_back(std::move(ns));
        break;
    }

    case CXCursor_TypeRef: {
        json ns {{"kind", "type_ref"}, {"name", spell.c_str()}};
        node.push_back(std::move(ns));
        break;
    }

    default:
    ;
    }

    return CXChildVisit_Continue;
}

void show_clang_version(void) {
    CXString version = clang_getClangVersion();
    printf("%s\n", clang_getCString(version));
    clang_disposeString(version);
}

void print_diagnostics(CXTranslationUnit tu) {
    size_t count = clang_getNumDiagnostics(tu);

    for (size_t i = 0; i < count; i++) {
        CXDiagnostic diag = clang_getDiagnostic(tu, i);
        CXString text = clang_formatDiagnostic(diag, clang_defaultDiagnosticDisplayOptions());
        fprintf(stderr, "%s", clang_getCString(text));
        /*CXString text = clang_getDiagnosticSpelling(diag);
        printf("%d: %s", clang_getDiagnosticSeverity(diag), clang_getCString(text));*/
        clang_disposeString(text);
        clang_disposeDiagnostic(tu);
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: ast CPP_file [args...]\n");
        exit(1);
    }

    json model;
    model["ast"] = json::array();

    show_clang_version();

    const auto filename = argv[1];

    // create index w/ excludeDeclsFromPCH = 1, displayDiagnostics=1.
    CXIndex index = clang_createIndex(1, 1);

    // load a *.ast file.
    CXTranslationUnit tu; //clang_createTranslationUnit(index, filename);
    /*if (tu == NULL) {
        printf("Cannot create translation unit\n");
        return 1;
    }*/
    auto error = clang_parseTranslationUnit2(index, filename, argv + 2, argc - 2, nullptr, 0, 0, &tu);

    if (error != 0) {
        fprintf(stderr, "ast: error %d\n", error);

        print_diagnostics(tu);
        return -1;
    }

    VisitClientData vcd {0, model, &model["children"]};
    CXCursor cursor = clang_getTranslationUnitCursor(tu);
    clang_visitChildren(cursor, visitChildrenCallback, &vcd);

    clang_disposeTranslationUnit(tu);
    clang_disposeIndex(index);

    puts(model.dump(4).c_str());
    return 0;
}
