#include <ecs/ecs.hpp>

namespace wheel {

void ECS::update() {
    current_events_map_ = std::move(next_events_map_);
    current_entity_events_ = std::move(next_entity_events_);
    for (auto& func : delayed_functions_) {
        func();
    }
    delayed_functions_.clear();

    for (auto system : systems_) {
        const auto& info = system_infos_map_.at(system);
        if (info.active) {
            info.func();
        }
    }
    
    for (auto [entity, cid] : current_entity_events_) {
        remove_component_(entity, cid);
    }
}

Entity ECS::copy_entity(Entity entity) {
    if (!has_entity(entity)) return NullEntity;

    auto new_entity = EntityGenerator::generate();
    entity_components_[new_entity] = {};

    for (auto cid : entity_components_.at(entity)) {
        copy_component_(entity, new_entity, cid);
    }

    return new_entity;
}

void ECS::remove_entity(Entity entity) {
    if (!has_entity(entity)) {
        return;
    }

    for (auto cid : entity_components_.at(entity)) {
        cid2containers_.at(cid)->remove(entity);
    }
    entity_components_.erase(entity);
}

bool ECS::has_entity(Entity entity) const {
    return entity_components_.count(entity);
}

size_t ECS::count_entities() const {
    return entity_components_.size();
}

void ECS::pause_system(const SystemID& system_id) {
    if (system_infos_map_.count(system_id)) {
        system_infos_map_.at(system_id).active = false;
    }
}

void ECS::resume_system(const SystemID& system_id) {
    if (system_infos_map_.count(system_id)) {
        system_infos_map_.at(system_id).active = true;
    }
}

void ECS::clear() {
    clear_systems();
    clear_entities();
    clear_events();
    clear_entity_events();
}

void ECS::clear_entities() {
    entity_components_.clear();
    cid2containers_.clear();
    EntityGenerator::clear();
}

void ECS::clear_systems() {
    systems_.clear();
}

void ECS::clear_events() {
    current_events_map_.clear();
    next_events_map_.clear();
}

void ECS::clear_entity_events() {
    current_entity_events_.clear();
    next_entity_events_.clear();
}

void ECS::copy_component_(Entity src_entity, Entity dst_entity, ComponentID cid) {
    entity_components_.at(dst_entity).emplace(cid);
    cid2containers_.at(cid)->copy(src_entity, dst_entity);
}

void ECS::remove_component_(Entity entity, ComponentID cid) {
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

}  // namespace wheel
