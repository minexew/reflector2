
#include <string>
#include <vector>
#include <cstdint>

namespace glm {
    struct vec3 {};
}

namespace agdg {
    struct SHA3_224 {};
}

#define message_id(id_) __attribute__((annotate("agdg.protocol.message_id=" #id_)))

namespace agdg {
// CLIENT -> SERVER

struct message_id(1) CHello {
    SHA3_224 token;
};

struct message_id(2) CEnterWorld {
    std::string characterName;
};

struct message_id(3) CZoneLoaded {
};

struct message_id(4) CPlayerMovement {
    glm::vec3 pos;
    glm::vec3 dir;
};

struct message_id(5) CPong {
};

struct message_id(30) CChatSay {
    std::string text;
};

// SERVER -> CLIENT

struct message_id(1) SHello {
    std::vector<std::string> characters;
};

struct message_id(2) SLoadZone {
    std::string zoneName;
    SHA3_224 zoneRef;
};

struct message_id(3) SZoneState {
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

struct message_id(4) SPing {
};

struct message_id(20) SEntitySpawn {
    struct Entity {
        int32_t eid;
        uint32_t flags;
        std::string name;
        glm::vec3 pos;
        glm::vec3 dir;
    };

    Entity entity;
};

struct message_id(21) SEntityDespawn {
    int32_t eid;
};

struct message_id(22) SEntityUpdate {
    int32_t eid;
    glm::vec3 pos;
    glm::vec3 dir;
    uint32_t latency;
};

struct message_id(30) SChatSay {
    int32_t eid;
    std::string text;
    bool html;
};

}
