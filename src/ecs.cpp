#include <ecs/ecs.hpp>

namespace wheel {

void ECS::update() {
    for (auto system : systems_) {
        const auto& info = system_infos_map_.at(system);
        if (info.active) {
            info.func();
        }
    }
    std::swap(current_events_map_, next_events_map_);
    next_events_map_.clear();
}

Entity ECS::copy_entity(Entity entity) {
    if (!has_entity(entity)) return NullEntity;

    auto new_entity = EntityGenerator::generate();
    entity_components_[new_entity] = {};

    for (auto cid : entity_components_.at(entity)) {
        cid2containers_.at(cid)->copy(entity, new_entity);
    }

    return new_entity;
}

void ECS::del_entity(Entity entity) {
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

}  // namespace wheel
