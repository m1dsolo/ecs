<div align="center">

<h1>A Simple C++23 ECS framework</h1>

[![license](https://img.shields.io/badge/License-MIT-blue.svg)](https://github.com/m1dsolo/ecs/blob/main/LICENSE)

</div>

## Table of Contents

- [Introduction](#Introduction)
- [Installation](#Installation)
- [Usage](#Usage)
  - [Entities](#Entities)
  - [Components](#Components)
  - [Systems](#Systems)
  - [Events](#Events)
  - [Entity-Based Events](#Entity-Based-Events)
  - [Entity Copying](#Entity-Copying)
  - [Resources](#Resources)
- [Todo](#Todo)
- [License](#License)

## Introduction

A simple, lightweight Entity Component System (ECS) framework crafted with C++23
 — boasting just a few hundred lines of code,
making it exceptionally accessible for learning, customization, or direct modification to suit your needs.
Following the ECS architectural pattern, this library delivers a flexible and efficient solution for
managing game objects (entities), their state data (components), and core game logic (systems).

## Installation

To use this ECS framework in your project, you can either clone the repository or include it as a submodule.

For example, using git submodule:

```bash
cd your_project_directory
git submodule add https://github.com/m1dsolo/ecs.git third_party/ecs
```

Then modify your `CMakeLists.txt` to include the ECS framework:

```cmake
add_subdirectory(third_party/ecs)

target_include_directories(your_project PRIVATE third_party/ecs/include)
target_link_libraries(your_project PRIVATE ecs)
```

## Usage

Here only list the basic usage.
For detailed usage, please refer to my [game project](https://github.com/m1dsolo/game) based on this ECS library.

### Basic Setup

Include the main header and use the `wheel` namespace:

```cpp
#include <ecs/ecs.hpp>

using namespace wheel;

// Create an ECS instance
ECS ecs;
```

### Entities

Entities are unique identifiers for game objects. You can create, check, and remove them:

```cpp
// Create entities
Entity entity0 = ecs.add_entity();
Entity entity1 = ecs.add_entity();

// Check entity existence
bool exists = ecs.has_entity(entity0); // true

// Count active entities
size_t count = ecs.count_entities(); // 2

// Remove an entity
ecs.remove_entity(entity0);
```

### Components

Components are plain data structures that store entity state. Attach them when creating entities or later:

```cpp
// Define components
struct NameComponent {
    std::string name;
};

struct HPComponent {
    int hp;
};

// Create entity with components
Entity entity = ecs.add_entity(
    NameComponent{"Player"},
    HPComponent{100}
);

// Check component existence
bool has_hp = ecs.has_component<HPComponent>(entity); // true
bool has_both = ecs.has_components<NameComponent, HPComponent>(entity); // true

// Retrieve components
const auto& name = ecs.get_component<NameComponent>(entity);
auto& hp = ecs.get_component<HPComponent>(entity); // Mutable access

// Modify component data
hp.hp = 80;
```

### Systems

Systems are functions that operate on entities with specific components. They contain game logic:

```cpp
// Define a system (recovers HP for entities with both Name and HP components)
struct RecoverHPSystem {
    void operator()(ECS& ecs) {
        // Get all entities with required components
        for (auto entity : ecs.get_entities<NameComponent, HPComponent>()) {
            auto& hp = ecs.get_component<HPComponent>(entity);
            hp.hp++; // Heal 1 HP per update
        }
    }
};

// Add system to ECS
ecs.add_system<RecoverHPSystem>();

// Update all active systems
ecs.update(); // HP components will be incremented

// System management
ecs.pause_system<RecoverHPSystem>();  // Pause system execution
ecs.resume_system<RecoverHPSystem>(); // Resume system execution
ecs.remove_system<RecoverHPSystem>(); // Remove system entirely
```

### Events

Events are temporary messages passed between systems. They are cleared after each update cycle:

```cpp
// Define an event type
struct DamageEvent {
    Entity source;  // Who dealt damage
    Entity target;  // Who received damage
    int damage;     // Amount of damage
};

// Send events
ecs.emplace_event<DamageEvent>(attacker, target, 50);
ecs.emplace_event<DamageEvent>(player, enemy, 30);

// Process events in a system
ecs.update(); // Triggers event processing phase

if (ecs.has_event<DamageEvent>()) {
    for (auto [source, target, damage] : ecs.get_events<DamageEvent>()) {
        auto& target_hp = ecs.get_component<HPComponent>(target);
        target_hp.hp -= damage; // Apply damage
    }
}

// Events are cleared after processing
ecs.update();
bool has_events = ecs.has_event<DamageEvent>(); // false
```

### Entity-Based Events

Send events targeted at specific entities (automatically converted to components temporarily).
They are cleared after each update cycle:

```cpp
// Define an entity-specific event
struct GetHitEvent {
    int damage;
};

// Send event to a specific entity
ecs.add_entity_event(entity, GetHitEvent{10});

// Event is processed during update
ecs.update();

// Access the event as a temporary component
if (ecs.has_component<GetHitEvent>(entity)) {
    auto& hit = ecs.get_component<GetHitEvent>(entity);
    // Handle hit (e.g., reduce HP)
}

// Event component is removed after processing
ecs.update();
bool has_hit = ecs.has_component<GetHitEvent>(entity); // false
```

### Entity Copying

Create copies of entities with all their components:

```cpp
// Create original entity
Entity original = ecs.add_entity(
    NameComponent{"Warrior"},
    HPComponent{150}
);

// Copy the entity
Entity copy = ecs.copy_entity(original);

// Copies have new IDs but identical components
bool same_id = (original == copy); // false
auto& copy_name = ecs.get_component<NameComponent>(copy);
bool same_name = (copy_name.name == "Warrior"); // true
```

### Resources

Resources are global data accessible to all systems (e.g., game configs):

```cpp
// Define a resource
struct GameResource {
    int max_players;
    std::string game_name;
};

// Add a resource
ecs.add_resource(GameResource{4, "Adventure Quest"});

// Check and retrieve resources
if (ecs.has_resource<GameResource>()) {
    const auto& config = ecs.get_resource<GameResource>();
    // Use resource data (e.g., config.max_players)
}

// Remove a resource
ecs.remove_resource<GameResource>();
```

## Todo

- system scheduler
- multithreading acceleration

## License

[MIT](LICENSE) © m1dsolo
