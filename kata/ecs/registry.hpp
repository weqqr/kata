#pragma once

#include <any>
#include <array>
#include <assert.h>
#include <kata/ecs/id_allocator.hpp>
#include <span>
#include <typeindex>
#include <vector>

namespace kata {
class Archetype {
public:
    Archetype(Archetype const&) = delete;
    Archetype& operator=(Archetype const&) = delete;

    Archetype(Archetype&&) = default;
    Archetype& operator=(Archetype&&) = default;

    template<typename... Components>
    static Archetype create()
    {
        Archetype archetype {};

        archetype.m_column_types = {
            std::type_index(typeid(Components))...
        };

        (archetype.m_columns.push_back(std::move(std::vector<Components>{})), ...);

        return archetype;
    }

    template<typename T>
    std::vector<T>& column_for_type()
    {
        auto it = std::find(m_column_types.begin(), m_column_types.end(), std::type_index(typeid(T)));

        assert(it != m_column_types.end());

        auto column_vector_index = it - m_column_types.begin();
        auto& column = m_columns[column_vector_index];
        auto& column_vector = std::any_cast<std::vector<T>&>(column);

        return column_vector;
    }

    bool matches_exactly(std::span<std::type_index> types) const
    {
        if (types.size() != m_column_types.size()) {
            return false;
        }

        for (auto type : types) {
            auto it = std::find(m_column_types.begin(), m_column_types.end(), type);

            if (it == m_column_types.end()) {
                return false;
            }
        }

        return true;
    }

    bool contains_components(std::span<std::type_index> types) const {
        for (auto type : types) {
            auto it = std::find(m_column_types.begin(), m_column_types.end(), type);

            if (it == m_column_types.end()) {
                return false;
            }
        }

        return true;
    }

    template<typename... Components>
    void write_column(EntityID id, Components... components)
    {
        std::tuple<std::vector<Components>&...> columns {
            column_for_type<Components>()...
        };

        (std::get<std::vector<Components>&>(columns).push_back(std::move(components)), ...);

        m_id_column.push_back(id);

        m_size++;
    }

    size_t size() const
    {
        return m_size;
    }

private:
    Archetype() = default;

    std::vector<std::type_index> m_column_types {};
    std::vector<EntityID> m_id_column {};
    std::vector<std::any> m_columns {};
    size_t m_size {};
};

class Registry {
public:
    Registry() = default;

    template<typename... Components>
    EntityID spawn_with(Components... components)
    {
        std::array<std::type_index, sizeof...(Components)> type_indexes {
            std::type_index(typeid(Components))...
        };

        EntityID id = m_id_allocator.allocate();

        Archetype* exact_archetype { nullptr };

        for (auto& archetype : m_archetypes) {
            if (archetype.matches_exactly(type_indexes)) {
                exact_archetype = &archetype;
            }
        }

        if (!exact_archetype) {
            auto archetype = Archetype::create<Components...>();
            m_archetypes.push_back(std::move(archetype));
            exact_archetype = &m_archetypes.back();
        }

        assert(exact_archetype != nullptr);

        exact_archetype->write_column(id, std::forward<Components>(components)...);

        return id;
    }

    template<typename... Components, typename F>
    void query(F f)
    {
        std::array<std::type_index, sizeof...(Components)> type_indexes {
            std::type_index(typeid(Components))...
        };

        for (auto& archetype : m_archetypes) {
            if (!archetype.contains_components(type_indexes)) {
                continue;
            }

            std::tuple<std::vector<Components>&...> columns {
                archetype.column_for_type<Components>()...
            };

            for (size_t i = 0; i < archetype.size(); i++) {
                std::tuple<Components&...> components {
                    std::get<std::vector<Components>&>(columns)[i]...
                };

                f(std::get<Components&>(components)...);
            }
        }
    }

private:
    void allocate_id();

    std::vector<Archetype> m_archetypes;
    IDAllocator m_id_allocator {};
};

}
