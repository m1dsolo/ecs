#pragma once

#include <ecs/entity.hpp>

namespace wheel {

class EntityGenerator {
public:
    EntityGenerator() = default;

    static Entity generate() { return next_entity_++; }
    static void clear() { next_entity_ = 0; }

private:
    static inline Entity next_entity_ = 0;
};

}  // namespace wheel
