#include <kata/ecs/system.hpp>

namespace kata {
void Schedule::run_systems(SystemStage stage, Registry &reg)
{
    auto it = m_systems.find(stage);
    if (it == m_systems.end()) {
        return;
    }

    for (auto& system : it->second) {
        system->run(reg);
    }
}
}
