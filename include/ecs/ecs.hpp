#pragma once

#include <ecs/entity.hpp>
#include <ecs/entity_generator.hpp>
#include <ecs/component_container.hpp>
#include <ecs/event_container.hpp>

#include <algorithm>
#include <functional>
#include <ranges>
#include <typeindex>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <memory>

namespace wheel {

using ComponentID = std::type_index;
using SystemID = std::type_index;
using EventID = std::type_index;

class ECS {
public:
    ECS() = default;
    ~ECS() = default;
    ECS(const ECS&) = delete;

    void update();

    template <typename... ComponentTypes>
    Entity add_entity(ComponentTypes&&... components);

    Entity copy_entity(Entity entity);

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

    template <typename... ComponentTypes>
    void del_components();

    template <typename... ComponentTypes>
    void del_components(Entity entity);

    template <typename... ComponentIDs> requires (std::is_convertible_v<ComponentIDs, ComponentID> && ...)
    void del_components(Entity entity, ComponentIDs&&... cids);

    template <typename ComponentType>
    bool has_component(Entity entity) const;

    template <typename... ComponentTypes>
    bool has_components(Entity entity) const;

    template <typename ComponentType>
    ComponentType& get_component() const;

    template <typename ComponentType>
    ComponentType& get_component(Entity entity) const;

    template <typename... ComponentTypes>
    auto get_components() const;

    template <typename... ComponentTypes>
    auto get_components(Entity entity) const;

    template <typename... ComponentTypes>
    auto get_entity_and_components() const;

    template <typename SystemType>
    SystemID get_system_id() const {
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
    std::span<const EventType> get_events() const;

    void clear();
    void clear_entities();
    void clear_systems();
    void clear_events();

private:
    template <typename ComponentType>
    void add_component_(Entity entity, ComponentType&& component);

    std::unordered_map<Entity, std::unordered_set<ComponentID>> entity_components_;
    std::unordered_map<ComponentID, std::unique_ptr<ComponentContainerInterface>> cid2containers_;

    struct SystemInfo {
        std::function<void()> func;
        bool active{true};
    };
    std::vector<SystemID> systems_;
    std::unordered_map<SystemID, SystemInfo> system_infos_map_;

    std::unordered_map<EventID, std::unique_ptr<EventContainerInterface>> current_events_map_, next_events_map_;
};


template <typename... ComponentTypes>
Entity ECS::add_entity(ComponentTypes&&... components) {
    auto entity = EntityGenerator::generate();
    entity_components_[entity];
    add_components(entity, std::forward<ComponentTypes>(components)...);
    return entity;
}

template <typename... ComponentTypes>
auto ECS::get_entities() const {
    return entity_components_ |
        std::views::keys |
        std::views::filter([this](Entity entity) {
            const auto& components = entity_components_.at(entity);
            return (components.contains(typeid(ComponentTypes)) && ...);
        });
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
    add_component_(entity, std::forward<ComponentType>(component));
}

template <typename... ComponentTypes>
void ECS::add_components(Entity entity, ComponentTypes&&... components) {
    auto components_tuple = std::make_tuple(std::forward<ComponentTypes>(components)...);
    std::apply([this, entity](auto&&... component) {
        (add_component_(entity, std::forward<ComponentTypes>(component)), ...);
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
    ComponentID cid = typeid(ComponentType);

    auto it = entity_components_.find(entity);
    if (it == entity_components_.end()) {
        return;
    }
    auto& components = it->second;
    if (!components.contains(cid)) {
        return;
    }

    if (auto it = cid2containers_.find(cid); it != cid2containers_.end()) {
        it->second->remove(entity);
        // if (it->second->size() == 0) {
        //     cid2containers_.erase(it);
        // }
    }

    components.erase(cid);
    // if (components.empty()) {
    //     entity_components_.erase(it);
    // }
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
void ECS::del_components(Entity entity, ComponentIDs&&... cids) {
    (del_component(entity, std::forward<ComponentIDs>(cids)), ...);
}

template <typename ComponentType>
bool ECS::has_component(Entity entity) const {
    return entity_components_.count(entity) && entity_components_.at(entity).contains(typeid(ComponentType));
}

template <typename... ComponentTypes>
bool ECS::has_components(Entity entity) const {
    return (has_component<ComponentTypes>(entity) && ...);
}

template <typename ComponentType>
ComponentType& ECS::get_component() const {
    ComponentID cid = typeid(ComponentType);
    auto& container = static_cast<ComponentContainer<ComponentType>&>(*cid2containers_.at(cid));
    return container.get_first();
}

template <typename ComponentType>
ComponentType& ECS::get_component(Entity entity) const {
    ComponentID cid = typeid(ComponentType);
    auto& container = static_cast<ComponentContainer<ComponentType>&>(*cid2containers_.at(cid));
    return container.get(entity);
}

template <typename... ComponentTypes>
auto ECS::get_components() const {
    auto entities = get_entities<ComponentTypes...>();
    return std::views::zip(
        entities |
        std::views::transform([&](Entity entity) -> ComponentTypes& {
            return get_component<ComponentTypes>(entity);
        })
    ...);
}

template <typename... ComponentTypes>
auto ECS::get_components(Entity entity) const {
    return std::make_tuple(std::ref(get_component<ComponentTypes>(entity))...);
}

template <typename... ComponentTypes>
auto ECS::get_entity_and_components() const {
    auto entities = get_entities<ComponentTypes...>();
    return std::views::zip(
        entities,
        entities |
        std::views::transform([&](Entity entity) -> ComponentTypes& {
            return get_component<ComponentTypes>(entity);
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
    EventID eid = typeid(EventType);
    if (!next_events_map_.count(eid)) {
        next_events_map_[eid] = std::make_unique<EventContainer<EventType>>();
    }
    auto& container = static_cast<EventContainer<EventType>&>(*next_events_map_[eid]);
    container.events.emplace_back(std::forward<EventType>(event));
}

template <typename EventType, typename... Args>
void ECS::emplace_event(Args&&... args) {
    EventID eid = typeid(EventType);
    if (!next_events_map_.count(eid)) {
        next_events_map_[eid] = std::make_unique<EventContainer<EventType>>();
    }
    auto& container = static_cast<EventContainer<EventType>&>(*next_events_map_[eid]);
    container.events.emplace_back(std::forward<Args>(args)...);
}

template <typename EventType>
bool ECS::has_event() const {
    EventID eid = typeid(EventType);
    return next_events_map_.count(eid) > 0 && next_events_map_.at(eid)->size() > 0;
}

template <typename EventType>
std::span<const EventType> ECS::get_events() const {
    EventID eid = typeid(EventType);
    if (auto it = current_events_map_.find(eid); it == current_events_map_.end()) {
        return {};
    } else {
        return static_cast<const EventContainer<EventType>&>(*current_events_map_.at(eid)).events;
    }
}

template <typename ComponentType>
void ECS::add_component_(Entity entity, ComponentType&& component) {
    ComponentID cid = typeid(ComponentType);
    if (entity_components_[entity].count(cid)) {
        return;
    }

    if (!cid2containers_.count(cid)) {
        cid2containers_[cid] = std::make_unique<ComponentContainer<ComponentType>>();
    }
    auto& container = static_cast<ComponentContainer<ComponentType>&>(*cid2containers_[cid]);
    container.add(entity, std::forward<ComponentType>(component));

    entity_components_[entity].emplace(cid);
}

}  // namespace wheel
