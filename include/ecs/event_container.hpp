#pragma once

#include <ecs/entity.hpp>

#include <vector>

namespace wheel {

// type erasure for event container
class IEventContainer {
public:
    virtual ~IEventContainer() = default;

    virtual size_t size() const = 0;
};

template <typename EventType>
class EventContainer : public IEventContainer {
public:
    size_t size() const override {
        return events.size();
    }

    std::vector<EventType> events;
};

}  // namespace wheel
