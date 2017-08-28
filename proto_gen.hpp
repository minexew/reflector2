namespace agdg {
struct SHA3_224 {
};

struct CHello {
    agdg::SHA3_224 token;
};

struct CEnterWorld {
    std::string characterName;
};

struct CZoneLoaded {
};

struct CPlayerMovement {
    glm::vec3 pos;
    glm::vec3 dir;
};

struct CPong {
};

struct CChatSay {
    std::string text;
};

struct SHello {
    std::vector<std::string> characters;
};

struct SLoadZone {
    std::string zoneName;
    agdg::SHA3_224 zoneRef;
};

struct SZoneState {
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

struct SPing {
};

struct SEntitySpawn {
    struct Entity {
        int32_t eid;
        uint32_t flags;
        std::string name;
        glm::vec3 pos;
        glm::vec3 dir;
    };
    
    agdg::SEntitySpawn::Entity entity;
};

struct SEntityDespawn {
    int32_t eid;
};

struct SEntityUpdate {
    int32_t eid;
    glm::vec3 pos;
    glm::vec3 dir;
    uint32_t latency;
};

struct SChatSay {
    int32_t eid;
    std::string text;
    bool html;
};

}
