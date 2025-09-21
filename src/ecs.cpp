#include <ecs/ecs.hpp>

namespace wheel {

void ECS::update() {
    for (auto system : systems_) {
        const auto& info = system_infos_map_.at(system);
        if (info.active) {
            info.func();
        }
    }
    std::swap(current_frame_events_map_, next_frame_events_map_);
    next_frame_events_map_.clear();
}

void ECS::del_entity(Entity entity) {
    if (!has_entity(entity)) return;

    for (auto& [component_id, idx] : entity2components_.at(entity)) {
        component2entities_.at(component_id).del(entity);
        component2containers_.at(component_id).del(idx);
    }
    entity2components_.erase(entity);
}

bool ECS::has_entity(Entity entity) const {
    return entity2components_.count(entity);
}

size_t ECS::count_entities() const {
    return entity2components_.size();
}

void ECS::del_component(Entity entity, ComponentID component_id) {
    size_t idx = entity2components_.at(entity).at(component_id);
    component2entities_.at(component_id).del(entity);
    component2containers_.at(component_id).del(idx);
    entity2components_.at(entity).erase(component_id);
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
    entity2components_.clear();
    component2entities_.clear();
    component2containers_.clear();
    EntityGenerator::set_next_entity(wheel::NullEntity + 1);
}

void ECS::clear_systems() {
    systems_.clear();
}

void ECS::clear_events() {
    current_frame_events_map_.clear();
    next_frame_events_map_.clear();
}

}  // namespace wheel
