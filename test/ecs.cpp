#include <ecs/ecs.hpp>

#include <gtest/gtest.h>

using namespace wheel;

struct NameComponent {
    std::string name;
};

struct HPComponent {
    int hp;
};

struct RecoverHPSystem {
    void operator()(ECS& ecs) {
        for (auto entity : ecs.get_entities<NameComponent, HPComponent>()) {
            auto& hp = ecs.get_component<HPComponent>(entity);
            hp.hp++;
        }
    }
};

struct DamageEvent {
    Entity source;
    Entity target;
    int damage;
};

struct GetHitEvent {
    int damage;
};

struct GameResource {
    int max_players;
    std::string game_name;
};

class ECSTest : public ::testing::Test {
protected:
    void SetUp() override {
        ecs.clear();
    }

    ECS ecs;
};

TEST_F(ECSTest, Entity) {
    Entity entity0 = ecs.add_entity();
    Entity entity1 = ecs.add_entity();
    
    EXPECT_NE(entity0, entity1);
    EXPECT_EQ(ecs.count_entities(), 2);
    EXPECT_TRUE(ecs.has_entity(entity0));
    EXPECT_TRUE(ecs.has_entity(entity1));

    ecs.remove_entity(entity0);
    EXPECT_EQ(ecs.count_entities(), 1);
    EXPECT_FALSE(ecs.has_entity(entity0));
    EXPECT_TRUE(ecs.has_entity(entity1));
}

TEST_F(ECSTest, Component) {
    Entity entity0 = ecs.add_entity(
        NameComponent{"entity0"},
        HPComponent{100}
    );
    Entity entity1 = ecs.add_entity(
        NameComponent{"entity1"}
    );

    ASSERT_TRUE((ecs.has_components<NameComponent, HPComponent>(entity0)));
    ASSERT_TRUE(ecs.has_component<NameComponent>(entity1));
    EXPECT_FALSE(ecs.has_component<HPComponent>(entity1));

    const auto& name0 = ecs.get_component<NameComponent>(entity0);
    const auto& hp0 = ecs.get_component<HPComponent>(entity0);
    const auto& name1 = ecs.get_component<NameComponent>(entity1);
    EXPECT_EQ(name0.name, "entity0");
    EXPECT_EQ(hp0.hp, 100);
    EXPECT_EQ(name1.name, "entity1");
}

TEST_F(ECSTest, System) {
    ecs.add_system<RecoverHPSystem>();
    Entity entity0 = ecs.add_entity(
        NameComponent{"entity0"},
        HPComponent{100}
    );
    Entity entity1 = ecs.add_entity(
        NameComponent{"entity1"},
        HPComponent{200}
    );
    ecs.update();
    const auto& hp0 = ecs.get_component<HPComponent>(entity0);
    const auto& hp1 = ecs.get_component<HPComponent>(entity1);
    EXPECT_EQ(hp0.hp, 101);
    EXPECT_EQ(hp1.hp, 201);
    ecs.pause_system<RecoverHPSystem>();
    ecs.update();
    EXPECT_EQ(hp0.hp, 101);
    EXPECT_EQ(hp1.hp, 201);
    ecs.resume_system<RecoverHPSystem>();
    ecs.update();
    EXPECT_EQ(hp0.hp, 102);
    EXPECT_EQ(hp1.hp, 202);

    ecs.remove_system<RecoverHPSystem>();
    ecs.update();
    EXPECT_EQ(hp0.hp, 102);
    EXPECT_EQ(hp1.hp, 202);
}

TEST_F(ECSTest, Event) {
    Entity entity0 = ecs.add_entity(
        NameComponent{"entity0"},
        HPComponent{100}
    );
    Entity entity1 = ecs.add_entity(
        NameComponent{"entity1"},
        HPComponent{100}
    );

    ecs.emplace_event<DamageEvent>(entity1, entity0, 50);
    ecs.emplace_event<DamageEvent>(entity0, entity1, 30);
    EXPECT_FALSE(ecs.has_event<DamageEvent>());

    ecs.update();

    ASSERT_TRUE(ecs.has_event<DamageEvent>());
    for (auto [source, target, damage] : ecs.get_events<DamageEvent>()) {
        auto& target_hp = ecs.get_component<HPComponent>(target);
        target_hp.hp -= damage;
    }
    const auto& hp0 = ecs.get_component<HPComponent>(entity0);
    const auto& hp1 = ecs.get_component<HPComponent>(entity1);
    EXPECT_EQ(hp0.hp, 50);
    EXPECT_EQ(hp1.hp, 70);

    ecs.update();

    EXPECT_FALSE(ecs.has_event<DamageEvent>());
}

TEST_F(ECSTest, EntityEvent) {
    Entity entity0 = ecs.add_entity(
        NameComponent{"entity0"},
        HPComponent{100}
    );
    ecs.add_entity_event(entity0, GetHitEvent{10});
    EXPECT_FALSE(ecs.has_component<GetHitEvent>(entity0));

    ecs.update();

    ASSERT_TRUE(ecs.has_component<GetHitEvent>(entity0));
    const auto& get_hit_event = ecs.get_component<GetHitEvent>(entity0);
    EXPECT_EQ(get_hit_event.damage, 10);

    ecs.update();

    EXPECT_FALSE(ecs.has_component<GetHitEvent>(entity0));
}

TEST_F(ECSTest, CopyEntity) {
    Entity entity0 = ecs.add_entity(
        NameComponent{"entity0"},
        HPComponent{100}
    );
    Entity entity1 = ecs.copy_entity(entity0);
    EXPECT_NE(entity0, entity1);
    ASSERT_TRUE(ecs.has_entity(entity1));
    ASSERT_TRUE(ecs.has_component<NameComponent>(entity1));
    ASSERT_TRUE(ecs.has_component<HPComponent>(entity1));
    const auto& name1 = ecs.get_component<NameComponent>(entity1);
    const auto& hp1 = ecs.get_component<HPComponent>(entity1);
    EXPECT_EQ(name1.name, "entity0");
    EXPECT_EQ(hp1.hp, 100);
}

TEST_F(ECSTest, Resource) {
    ecs.add_resource(GameResource{4, "Test Game"});
    ASSERT_TRUE(ecs.has_resource<GameResource>());
    const auto& config = ecs.get_resource<GameResource>();
    EXPECT_EQ(config.max_players, 4);
    EXPECT_EQ(config.game_name, "Test Game");
    ecs.remove_resource<GameResource>();
    EXPECT_FALSE(ecs.has_resource<GameResource>());
}
