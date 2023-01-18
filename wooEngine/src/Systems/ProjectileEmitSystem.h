#pragma once

#include "../ECS/ECS.h"
#include "../EventBus/EventBus.h"
#include "../Events/KeyPressedEvent.h"
#include "../Components/TransformComponent.h"
#include "../Components/ProjectileEmitterComponent.h"
#include "../Components/ProjectileComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/BoxColliderComponent.h"

class ProjectileEmitSystem : public System {
public:
	ProjectileEmitSystem() {
		RequireComponent<ProjectileEmitterComponent>();
		RequireComponent<TransformComponent>();
	}

	void SubscribeToEvents(std::unique_ptr<EventBus>& eventBus) {
		eventBus->SubscribeToEvent<KeyPressedEvent>(this, &ProjectileEmitSystem::onKeyPressed);
	}

	void onKeyPressed(KeyPressedEvent& event) {
		for (auto entity : GetSystemEntities()) {
			//only process input of player
			if (entity.HasComponent<CameraFollowComponent>()) {
				switch (event.symbol) {
				case SDLK_SPACE:
					auto& projectileEmitter = entity.GetComponent<ProjectileEmitterComponent>();
					auto transform = entity.GetComponent<TransformComponent>();
					auto rigidbody = entity.GetComponent<RigidBodyComponent>();

					glm::vec2 projectilePosition = transform.position;
					if (entity.HasComponent<SpriteComponent>()) {
						const auto sprite = entity.GetComponent<SpriteComponent>();
						projectilePosition.x += (transform.scale.x * sprite.width / 2);
						projectilePosition.y += (transform.scale.y * sprite.height / 2);
					}

					// adjust direction based on which way player is facing
					glm::vec2 projectileVelocity = projectileEmitter.projectileVelocity;
					int directionX = 0;
					int directionY = 0;

					if (rigidbody.velocity.x > 0) directionX = 1; 
					if (rigidbody.velocity.x < 0) directionX = -1;
					if (rigidbody.velocity.y > 0) directionY = 1;
					if (rigidbody.velocity.y < 0) directionY = -1;

					projectileVelocity.x = projectileEmitter.projectileVelocity.x * directionX;
					projectileVelocity.y = projectileEmitter.projectileVelocity.y * directionY;


					Entity projectile = entity.registry->CreateEntity();
					projectile.Group("projectiles");
					projectile.AddComponent<TransformComponent>(projectilePosition, glm::vec2(1.0, 1.0));
					projectile.AddComponent<RigidBodyComponent>(projectileVelocity);
					projectile.AddComponent<SpriteComponent>("bullet-image", 4, 4, 4);
					projectile.AddComponent<BoxColliderComponent>(4, 4);
					projectile.AddComponent<ProjectileComponent>(projectileEmitter.isFriendly, projectileEmitter.hitDamage, projectileEmitter.projectileDuration);

					break;
				}
			}
		}
	}

	void Update(std::unique_ptr<Registry>& registry) {
		for (auto entity : GetSystemEntities()) {
			auto& projectileEmitter = entity.GetComponent<ProjectileEmitterComponent>();
			auto transform = entity.GetComponent<TransformComponent>();

			if (projectileEmitter.repeatFreq == 0) {
				continue;
			}

			// check re-emit
			if (SDL_GetTicks() - projectileEmitter.lastEmissionTime > projectileEmitter.repeatFreq) {
				glm::vec2 projectilePosition = transform.position;
				if (entity.HasComponent<SpriteComponent>()) {
					const auto sprite = entity.GetComponent<SpriteComponent>();
					projectilePosition.x += (transform.scale.x * sprite.width / 2);
					projectilePosition.y += (transform.scale.y * sprite.height / 2);
				}

				Entity projectile = registry->CreateEntity();
				projectile.Group("projectiles");
				projectile.AddComponent<TransformComponent>(projectilePosition, glm::vec2(1.0, 1.0));
				projectile.AddComponent<RigidBodyComponent>(projectileEmitter.projectileVelocity);
				projectile.AddComponent<SpriteComponent>("bullet-image", 4, 4, 4);
				projectile.AddComponent<BoxColliderComponent>(4, 4);
				projectile.AddComponent<ProjectileComponent>(projectileEmitter.isFriendly, projectileEmitter.hitDamage, projectileEmitter.projectileDuration);

				// update last emission time
				projectileEmitter.lastEmissionTime = SDL_GetTicks();
			}
		}
	}
};