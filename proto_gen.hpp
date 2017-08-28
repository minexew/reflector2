namespace agdg {
struct Character {
    int16_t level;
    uint64_t gid;
    int flags;
};
struct SClientHello {
    std::vector<Character> characters;
};
}
