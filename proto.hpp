
#include <vector>
#include <cstdint>

#define INTERESTING __attribute__((annotate("interesting")))

namespace agdg {
struct Character {
   int16_t level;
   INTERESTING uint64_t gid;
   int flags;
};

struct SClientHello {
   std::vector<Character> characters;
};
}
