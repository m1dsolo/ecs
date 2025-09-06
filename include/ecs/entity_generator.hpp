#pragma once

#include <ecs/entity.hpp>

namespace wheel {

class EntityGenerator {
public:
    EntityGenerator() = default;

    static Entity generate() { return next_entity_++; }
    static Entity next_entity() { return next_entity_; }
    static void set_next_entity(Entity entity) { next_entity_ = entity; }

private:
    static inline Entity next_entity_ = 1;
};

}  // namespace wheel
