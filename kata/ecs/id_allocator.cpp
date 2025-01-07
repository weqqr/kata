#include <kata/ecs/id_allocator.hpp>

namespace kata {
EntityID IDAllocator::allocate()
{
    m_last_id++;

    return m_last_id;
}

void IDAllocator::free(EntityID)
{
    // FIXME: ID could be generational (32-bit ID + 32-bit generation index),
    //        but flat uint64_t range seems to be good enough.
}
}
