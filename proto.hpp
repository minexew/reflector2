
#include <string>
#include <vector>
#include <cstdint>

namespace glm {
    struct vec3 {};
}

namespace agdg {
    struct SHA3_224 {};
}

#define agdg_protocol_message_struct __attribute__((annotate("agdg.protocol.message_struct")))

namespace agdg {
// CLIENT -> SERVER

struct agdg_protocol_message_struct CHello {
    enum { code = 1 };

    SHA3_224 token;
};

struct agdg_protocol_message_struct CEnterWorld {
    enum { code = 2 };

    std::string characterName;
};

struct agdg_protocol_message_struct CZoneLoaded {
    enum { code = 3 };
};

struct agdg_protocol_message_struct CPlayerMovement {
    enum { code = 4 };

    glm::vec3 pos;
    glm::vec3 dir;
};

struct agdg_protocol_message_struct CPong {
    enum { code = 5 };
};

struct agdg_protocol_message_struct CChatSay {
    enum { code = 30 };

    std::string text;
};

// SERVER -> CLIENT

struct agdg_protocol_message_struct SHello {
    enum { code = 1 };

    std::vector<std::string> characters;
};

struct agdg_protocol_message_struct SLoadZone {
    enum { code = 2 };

    std::string zoneName;
    SHA3_224 zoneRef;
};

struct agdg_protocol_message_struct SZoneState {
    enum { code = 3 };

    struct Entity {
        uint32_t eid;
        uint32_t flags;
        std::string name;
        glm::vec3 pos;
        glm::vec3 dir;
    };

    int32_t playerEid;
    std::string playerName;
    glm::vec3 playerPos;
    glm::vec3 playerDir;
    std::vector<Entity> entities;
};

struct agdg_protocol_message_struct SPing {
    enum { code = 4 };
};

struct agdg_protocol_message_struct SEntitySpawn {
    enum { code = 20 };

    struct Entity {
        int32_t eid;
        uint32_t flags;
        std::string name;
        glm::vec3 pos;
        glm::vec3 dir;
    };

    Entity entity;
};

struct agdg_protocol_message_struct SEntityDespawn {
    enum { code = 21 };

    int32_t eid;
};

struct agdg_protocol_message_struct SEntityUpdate {
    enum { code = 22 };

    int32_t eid;
    glm::vec3 pos;
    glm::vec3 dir;
    uint32_t latency;
};

struct agdg_protocol_message_struct SChatSay {
    enum { code = 30 };

    int32_t eid;
    std::string text;
    bool html;
};

}
