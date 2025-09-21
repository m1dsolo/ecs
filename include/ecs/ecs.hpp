#pragma once

#include <ecs/component_container.hpp>
#include <ecs/entity.hpp>
#include <ecs/entity_generator.hpp>
#include <ecs/sparse_set.hpp>

#include <algorithm>
#include <any>
#include <functional>
#include <ranges>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace wheel {

using ComponentID = std::type_index;
using EventID = std::type_index;
using SystemID = std::type_index;

class ECS {
    friend class ComponentContainer;

public:
    ECS() = default;
    ~ECS() = default;
    ECS(const ECS&) = delete;

    void update();

    template <typename... ComponentTypes>
    Entity add_entity(ComponentTypes&&... components);

    template <typename... ComponentTypes>
    Entity add_entity(Entity entity, ComponentTypes&&... components);

    void del_entity(Entity entity);
    bool has_entity(Entity entity) const;

    template <typename... ComponentTypes>
    auto get_entities() const;

    template <typename... ComponentTypes>
    Entity get_entity() const;

    std::size_t count_entities() const;

    template <typename ComponentType>
    void add_component(Entity entity, ComponentType&& component);

    template <typename... ComponentTypes>
    void add_components(Entity entity, ComponentTypes&&... components);

    template <typename ComponentType>
    void del_component();

    template <typename ComponentType>
    void del_component(Entity entity);

    void del_component(Entity entity, ComponentID component_id);

    template <typename... ComponentTypes>
    void del_components();

    template <typename... ComponentTypes>
    void del_components(Entity entity);

    template <typename... ComponentIDs> requires (std::is_convertible_v<ComponentIDs, ComponentID> && ...)
    void del_components(Entity entity, ComponentIDs&&... component_ids);

    template <typename ComponentType>
    bool has_component(Entity entity) const;

    template <typename ComponentType>
    bool has_component() const;

    template <typename... ComponentTypes>
    bool has_components(Entity entity) const;

    template <typename... ComponentTypes>
    bool has_components() const;

    template <typename ComponentType>
    ComponentType& get_component();

    template <typename ComponentType>
    ComponentType& get_component(Entity entity);

    template <typename... ComponentTypes>
    auto get_components();

    template <typename... ComponentTypes>
    auto get_components(Entity entity);

    template <typename... ComponentTypes>
    void exclude_components();

    template <typename ComponentType>
    void exclude_component();

    template <typename... ComponentTypes>
    auto get_entity_and_components();

    template <typename SystemType>
    SystemID get_system_id() {
        return typeid(SystemType);
    }

    template <typename SystemType>
    void add_system();

    template <typename... SystemTypes>
    void add_systems();

    template <typename SystemType>
    void del_system();

    template <typename... SystemTypes>
    void del_systems();

    template <typename SystemType>
    void pause_system();

    void pause_system(const SystemID& system_id);

    template <typename... SystemType>
    void pause_systems();

    template <typename SystemType>
    void resume_system();

    void resume_system(const SystemID& system_id);

    template <typename... SystemType>
    void resume_systems();

    template <typename EventType>
    void add_event(EventType&& event);
    
    template <typename EventType, typename... Args>
    void emplace_event(Args&&... args);

    template <typename EventType>
    bool has_event() const;

    template <typename EventType>
    auto get_events();

    void clear();
    void clear_entities();
    void clear_systems();
    void clear_events();

private:
    template <typename ComponentType>
    void add_component(Entity entity, const std::type_index& component_id, ComponentType&& component);

    std::unordered_map<Entity, std::unordered_map<ComponentID, size_t>> entity2components_;
    std::unordered_map<ComponentID, SparseSet<Entity>> component2entities_;
    std::unordered_map<ComponentID, ComponentContainer> component2containers_;
    std::unordered_set<ComponentID> excluded_component_ids_;

    struct SystemInfo {
        std::function<void()> func;
        bool active{true};
    };
    std::vector<SystemID> systems_;
    std::unordered_map<SystemID, SystemInfo> system_infos_map_;

    std::unordered_map<EventID, std::vector<std::any>> current_frame_events_map_, next_frame_events_map_;
};


template <typename... ComponentTypes>
Entity ECS::add_entity(ComponentTypes&&... components) {
    return add_entity(EntityGenerator::generate(), std::forward<ComponentTypes>(components)...);
}

template <typename... ComponentTypes>
Entity ECS::add_entity(Entity entity, ComponentTypes&&... components) {
    EntityGenerator::set_next_entity(std::max(EntityGenerator::next_entity(), entity + 1));
    if (!entity2components_.count(entity)) {
        entity2components_[entity] = {};
    }
    if constexpr (sizeof...(ComponentTypes) > 0) {
        add_components(entity, std::forward<ComponentTypes>(components)...);
    }
    return entity;
}

template <typename... ComponentTypes>
auto ECS::get_entities() const {
    // return all entities regardless of whether exclude
    if constexpr (sizeof...(ComponentTypes) == 0) {
        return entity2components_ | std::views::keys;
    } else {
        // exclude entities with excluded_components
        std::unordered_set<std::type_index> input_component_ids{typeid(ComponentTypes)...};
        auto all_entities = entity2components_ |
            std::views::keys |
            std::views::filter([this, input_component_ids](Entity entity) {
                const auto& component_ids = entity2components_.at(entity) | std::views::keys;
                return std::ranges::none_of(component_ids, [this, input_component_ids](const auto& component_id) {
                    return excluded_component_ids_.count(component_id) && !input_component_ids.count(component_id);
                });
            });

        // filter entities with required components
        return all_entities | std::views::filter([this](Entity entity) {
            const auto& components = entity2components_.at(entity);
            return (components.contains(typeid(ComponentTypes)) && ...);
        });
    }
}

template <typename... ComponentTypes>
Entity ECS::get_entity() const {
    auto entities = get_entities<ComponentTypes...>();
    auto it = entities.begin();
    if (it != entities.end()) {
        return *it;
    }
    return NullEntity;
}

template <typename ComponentType>
void ECS::add_component(Entity entity, ComponentType&& component) {
    add_component(entity, typeid(ComponentType), std::forward<ComponentType>(component));
}

template <typename... ComponentTypes>
void ECS::add_components(Entity entity, ComponentTypes&&... components) {
    auto components_tuple = std::make_tuple(std::forward<ComponentTypes>(components)...);
    std::apply([this, entity](auto&&... component) {
        (add_component(entity, std::forward<ComponentTypes>(component)), ...);
    }, components_tuple);
}

template <typename ComponentType>
void ECS::del_component() {
    for (auto& entity : get_entities<ComponentType>()) {
        del_component<ComponentType>(entity);
    }
}

template <typename ComponentType>
void ECS::del_component(Entity entity) {
    return del_component(entity, typeid(ComponentType));
}

template <typename... ComponentTypes>
void ECS::del_components() {
    (del_component<ComponentTypes>(), ...);
}

template <typename... ComponentTypes>
void ECS::del_components(Entity entity) {
    (del_component<ComponentTypes>(entity), ...);
}

template <typename... ComponentIDs> requires (std::is_convertible_v<ComponentIDs, ComponentID> && ...)
void ECS::del_components(Entity entity, ComponentIDs&&... component_ids) {
    (del_component(entity, std::forward<ComponentIDs>(component_ids)), ...);
}

template <typename ComponentType>
bool ECS::has_component(Entity entity) const {
    return has_components<ComponentType>(entity);
}

template <typename ComponentType>
bool ECS::has_component() const {
    return has_components<ComponentType>();
}

template <typename... ComponentTypes>
bool ECS::has_components(Entity entity) const {
    return entity2components_.count(entity) && ((entity2components_.at(entity).contains(typeid(ComponentTypes)) && ...));
}

template <typename... ComponentTypes>
bool ECS::has_components() const {
    return ((component2containers_.count(typeid(ComponentTypes)) && component2containers_.at(typeid(ComponentTypes)).size() > 0) && ...);
}

template <typename ComponentType>
ComponentType& ECS::get_component() {
    return std::any_cast<ComponentType&>(component2containers_.at(typeid(ComponentType)).components.at(0));
}

template <typename ComponentType>
ComponentType& ECS::get_component(Entity entity) {
    size_t idx = entity2components_.at(entity).at(typeid(ComponentType));
    return std::any_cast<ComponentType&>(component2containers_.at(typeid(ComponentType)).components.at(idx));
}

template <typename... ComponentTypes>
auto ECS::get_components() {
    auto entities = get_entities<ComponentTypes...>();
    return std::views::zip(
        entities |
        std::views::transform([&](Entity entity) -> ComponentTypes& {
            size_t idx = entity2components_.at(entity).at(typeid(ComponentTypes));
            auto& component = component2containers_.at(typeid(ComponentTypes)).components.at(idx);
            return std::any_cast<ComponentTypes&>(component);
        })
    ...);
}

template <typename... ComponentTypes>
auto ECS::get_components(Entity entity) {
    return std::make_tuple(std::ref(get_component<ComponentTypes>(entity))...);
}

template <typename... ComponentTypes>
void ECS::exclude_components() {
    (excluded_component_ids_.insert(typeid(ComponentTypes)), ...);
}

template <typename ComponentType>
void ECS::exclude_component() {
    excluded_component_ids_.insert(typeid(ComponentType));
}

template <typename... ComponentTypes>
auto ECS::get_entity_and_components() {
    auto entities = get_entities<ComponentTypes...>();
    return std::views::zip(
        entities,
        entities |
        std::views::transform([&](Entity entity) -> ComponentTypes& {
            size_t idx = entity2components_.at(entity).at(typeid(ComponentTypes));
            auto& component = component2containers_.at(typeid(ComponentTypes)).components.at(idx);
            return std::any_cast<ComponentTypes&>(component);
        })
    ...);
}

template <typename SystemType>
void ECS::add_system() {
    SystemID id = typeid(SystemType);
    system_infos_map_.emplace(id, SystemInfo{
        .func = [system = SystemType()]() mutable { system(); },
        .active = true
    });
    systems_.emplace_back(id);
}

template <typename... SystemTypes>
void ECS::add_systems() {
    (add_system<SystemTypes>(), ...);
}

template <typename SystemType>
void ECS::del_system() {
    SystemID id = typeid(SystemType);
    if (system_infos_map_.count(id)) {
        system_infos_map_.erase(id);
        systems_.erase(std::remove(systems_.begin(), systems_.end(), id), systems_.end());
    }
}

template <typename... SystemTypes>
void ECS::del_systems() {
    (del_system<SystemTypes>(), ...);
}

template <typename SystemType>
void ECS::pause_system() {
    SystemID id = typeid(SystemType);
    if (system_infos_map_.count(id)) {
        system_infos_map_.at(id).active = false;
    }
}

template <typename... SystemType>
void ECS::pause_systems() {
    (pause_system<SystemType>(), ...);
}

template <typename SystemType>
void ECS::resume_system() {
    SystemID id = typeid(SystemType);
    if (system_infos_map_.count(id)) {
        system_infos_map_.at(id).active = true;
    }
}

template <typename... SystemType>
void ECS::resume_systems() {
    (resume_system<SystemType>(), ...);
}

template <typename EventType>
void ECS::add_event(EventType&& event) {
    next_frame_events_map_[typeid(EventType)].emplace_back(std::forward<EventType>(event));
}

template <typename EventType, typename... Args>
void ECS::emplace_event(Args&&... args) {
    next_frame_events_map_[typeid(EventType)].emplace_back(EventType(std::forward<Args>(args)...));
}

template <typename EventType>
bool ECS::has_event() const {
    return current_frame_events_map_.count(typeid(EventType));
}

template <typename EventType>
auto ECS::get_events() {
    return current_frame_events_map_[typeid(EventType)] |
        std::views::transform([](std::any& event) -> EventType& { return std::any_cast<EventType&>(event); });
}

template <typename ComponentType>
void ECS::add_component(Entity entity, const std::type_index& component_id, ComponentType&& component) {
    if (entity2components_[entity].count(component_id)) {
        return;
    }

    component2entities_[component_id].add(entity);
    if (!component2containers_.count(component_id)) {
        component2containers_.emplace(component_id, ComponentContainer(*this, component_id));
    }
    auto& component_container = component2containers_.at(component_id);
    component_container.add(std::forward<ComponentType>(component), entity);
    entity2components_[entity][component_id] = component_container.size() - 1;
}

}  // namespace wheel
