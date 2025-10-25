#pragma once

#include <ecs/entity.hpp>
#include <ecs/sparse_set.hpp>

namespace wheel {

// type erasure for component container
class IComponentContainer {
public:
    virtual ~IComponentContainer() = default;

    virtual void remove(Entity entity) = 0;

    virtual bool has(Entity entity) const = 0;

    virtual size_t size() const = 0;

    virtual void copy(Entity src_entity, Entity dst_entity) = 0;
};

template <typename ComponentType>
class ComponentContainer : public IComponentContainer {
public:
    void remove(Entity entity) override {
        if (!entities_.has(entity)) return;

        auto idx = entities_.get_index(entity);
        entities_.remove(entity);

        if (idx < components_.size() - 1) {
            components_[idx] = std::move(components_.back());
        }
        components_.pop_back();
    }

    bool has(Entity entity) const override {
        return entities_.has(entity);
    }

    size_t size() const override {
        return components_.size();
    }

    void copy(Entity src_entity, Entity dst_entity) override {
        if (!entities_.has(src_entity)) {
            return;
        }

        const ComponentType& src_component = get(src_entity);

        add(dst_entity, src_component);
    }

    ComponentType& get(Entity entity) {
        auto idx = entities_.get_index(entity);
        return components_[idx];
    }

    ComponentType& get_first() {
        return components_.front();
    }

    template <typename T>
    void add(Entity entity, T&& component) {
        components_.emplace_back(component);
        entities_.add(entity);
    }

private:
    SparseSet<Entity> entities_;
    std::vector<ComponentType> components_;
};

}  // namespace wheel
