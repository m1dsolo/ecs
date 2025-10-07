#pragma once

#include <algorithm>

#include <ecs/entity.hpp>

namespace wheel {

// type erasure for resource
class ResourceInterface {
public:
    virtual ~ResourceInterface() = default;
};

template <typename ResourceType>
class Resource : public ResourceInterface {
public:
    ResourceType resource;

    Resource(ResourceType&& resource) : resource(std::move(resource)) {}
    
    Resource(const ResourceType& resource) : resource(resource) {}
};

}  // namespace wheel
