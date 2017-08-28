//
// Created by minexew on 8/25/17.
//

#ifndef PROJECT_FIELD_HPP
#define PROJECT_FIELD_HPP

namespace reflector2 {
    class Field {
        std::string name;
        const Type* type;
    };

    class Type {
        std::string name;
    };

    class Struct : public Type {
        const std::span<Field&> fields;
    };

    // Only value-based API for now
    template <class T>
    constexpr Type* reflectType();

    //template <typename T>
    //class TypeReflection {};
}

namespace reflector2 {
    /*template <>
    class TypeReflection<agdg::Character> {
        using Type = ;
    };*/

    template <>
    constexpr Type* reflectType<agdg::Character>() {
        static constexpr Field level { "level", reflectType<int16_t>(); };
        static constexpr Field gid { "gid", reflectType<uint64_t>(); };

        static constexpr Struct agdg_Character {"Character", span<Field&> {level, gid}};
        return &agdg_Character;
    }
}

#endif //PROJECT_FIELD_HPP
