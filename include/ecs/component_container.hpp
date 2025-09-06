#pragma once

#include <ecs/entity.hpp>

#include <any>
#include <vector>
#include <typeindex>

namespace wheel {

class ECS;

struct ComponentContainer {
    ComponentContainer(ECS& ecs, std::type_index component_id);

    template <typename ComponentType>
    void add(ComponentType&& component, Entity entity) {
        components.emplace_back(std::forward<ComponentType>(component));
        invert_index.emplace_back(std::move(entity));
    }

    bool del(size_t idx);

    auto begin() { return components.begin(); }
    auto end() { return components.end(); }
    size_t size() const { return components.size(); }

    std::vector<std::any> components;
    std::vector<Entity> invert_index;
    std::type_index component_id;
    ECS& ecs;
};

}  // namespace wheel
