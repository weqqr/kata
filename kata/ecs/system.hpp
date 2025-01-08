#pragma once

#include <kata/ecs/registry.hpp>
#include <memory>
#include <unordered_map>
#include <vector>

namespace kata {
enum class SystemStage {
    Init,
    BeforeStep,
    Step,
    AfterStep,
};

class System {
public:
    virtual void run(Registry& reg) = 0;
};

class Schedule {
public:
    Schedule() = default;

    template<typename T>
    void add_system(SystemStage stage, T system)
    {
        std::unique_ptr system_ptr(system);

        auto it = m_systems.find(stage);
        if (it == m_systems.end()) {
            std::vector<std::unique_ptr<System>> stage_systems {
                system_ptr,
            };

            m_systems.insert(std::make_pair(stage, std::move(stage_systems)));

            return;
        }

        it->second.push_back(system_ptr);
    }

    void run_systems(SystemStage stage, Registry& reg);

private:
    std::unordered_map<SystemStage, std::vector<std::unique_ptr<System>>> m_systems;
};
}
