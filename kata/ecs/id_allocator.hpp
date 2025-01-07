#pragma once

#include <cstdint>

namespace kata {
using EntityID = uint64_t;

class IDAllocator {
public:
    IDAllocator() = default;

    EntityID allocate();
    void free(EntityID id);

private:
    EntityID m_last_id { 0 };
};
}
