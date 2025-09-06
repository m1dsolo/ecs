#include <ecs/component_container.hpp>
#include <ecs/ecs.hpp>

namespace wheel {

ComponentContainer::ComponentContainer(ECS& ecs, std::type_index component_id)
    : ecs(ecs), component_id(component_id) {}

bool ComponentContainer::del(size_t idx) {
    if (idx < components.size()) {
        auto& entity2components = ecs.entity2components_;
        if (idx < components.size() - 1) {
            components[idx] = std::move(components.back());
            invert_index[idx] = std::move(invert_index.back());

            entity2components.at(invert_index[idx]).at(component_id) = idx;
        }
        components.pop_back();
        invert_index.pop_back();

        return true;
    }
    return false;
}

}  // namespace wheel
