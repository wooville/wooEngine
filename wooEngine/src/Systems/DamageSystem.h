#pragma once

#include "../ECS/ECS.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/ProjectileComponent.h"
#include "../Components/HealthComponent.h"
#include "../EventBus/EventBus.h"
#include "../Events/CollisionEvent.h"


class DamageSystem : public System {
public:
	DamageSystem() {
		RequireComponent<BoxColliderComponent>();
	}

	void SubscribeToEvents(std::unique_ptr<EventBus>& eventBus) {
		eventBus->SubscribeToEvent<CollisionEvent>(this, &DamageSystem::onCollision);
	}

	void onCollision(CollisionEvent& event) {
		Entity a = event.a;
		Entity b = event.b;
		//Logger::Log("Damage system received event collision between entities " + std::to_string(a.GetId()) + " and " + std::to_string(b.GetId()));
		
		if (a.BelongsToGroup("projectiles") && b.HasTag("player")) {
			onProjectilePlayerCollision(a, b);
		}
		if (b.BelongsToGroup("projectiles") && a.HasTag("player")) {
			onProjectilePlayerCollision(b, a);
		}

		if (a.BelongsToGroup("projectiles") && b.BelongsToGroup("enemies")) {
			onProjectileEnemyCollision(a, b);
		}
		if (b.BelongsToGroup("projectiles") && a.BelongsToGroup("enemies")) {
			onProjectileEnemyCollision(b, a);
		}
	}

	void onProjectilePlayerCollision(Entity projectile, Entity player) {
		auto projectileComponent = projectile.GetComponent<ProjectileComponent>();

		// update health, check health for kill condition, kill projectile entity
		if (!projectileComponent.isFriendly) {
			auto& health = player.GetComponent<HealthComponent>();

			health.health_val -= projectileComponent.hitDamage;
			//Logger::Log("Player received hit, new player health = " + std::to_string(health.health_val));

			if (health.health_val <= 0) {
				player.Kill();
			}

			projectile.Kill();
		}
	}

	void onProjectileEnemyCollision(Entity projectile, Entity enemy) {
		auto projectileComponent = projectile.GetComponent<ProjectileComponent>();

		// update health, check health for kill condition, kill projectile entity
		if (projectileComponent.isFriendly) {
			auto& health = enemy.GetComponent<HealthComponent>();

			health.health_val -= projectileComponent.hitDamage;
			//Logger::Log("Enemy id = " + std::to_string(enemy.GetId()) +" received hit, new enemy health = " + std::to_string(health.health_val));

			if (health.health_val <= 0) {
				enemy.Kill();
			}

			projectile.Kill();
		}
	}

	void Update() {

	}
};