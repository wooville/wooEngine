#pragma once

#include "../ECS/ECS.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../EventBus/EventBus.h"
#include "../Events/CollisionEvent.h"

class MovementSystem : public System {
public:
	MovementSystem() {
		 RequireComponent<TransformComponent>();
		 RequireComponent<RigidBodyComponent>();
	}

	void SubscribeToEvents(std::unique_ptr<EventBus>& eventBus) {
		eventBus->SubscribeToEvent<CollisionEvent>(this, &MovementSystem::onCollision);
	}

	void onCollision(CollisionEvent& event) {
		Entity a = event.a;
		Entity b = event.b;
		//Logger::Log("Movement system received event collision between entities " + std::to_string(a.GetId()) + " and " + std::to_string(b.GetId()));

		if (a.BelongsToGroup("enemies") && b.BelongsToGroup("obstacles")) {
			onEnemyObstacleCollision(a, b);
		}
		if (b.BelongsToGroup("enemies") && a.BelongsToGroup("obstacles")) {
			onEnemyObstacleCollision(b, a);
		}
	}

	void onEnemyObstacleCollision(Entity enemy, Entity obstacle) {
		if (enemy.HasComponent<RigidBodyComponent>() && enemy.HasComponent<SpriteComponent>()) {
			auto& rigidbody = enemy.GetComponent<RigidBodyComponent>();
			auto& sprite = enemy.GetComponent<SpriteComponent>();
			Logger::Log("Movement system received event collision between entities " + std::to_string(enemy.GetId()) + " and " + std::to_string(obstacle.GetId()));

			if (rigidbody.velocity.x != 0) {
				rigidbody.velocity.x *= -1;
				sprite.flip = (sprite.flip == SDL_FLIP_NONE) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
			}

			if (rigidbody.velocity.y != 0) {
				rigidbody.velocity.y *= -1;
				sprite.flip = (sprite.flip == SDL_FLIP_NONE) ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE;
			}
		}
	}

	void Update(double deltaTime) {
		for (auto entity : GetSystemEntities()) {
			//update position based on velocity
			auto& transform = entity.GetComponent<TransformComponent>();
			auto& rigidbody = entity.GetComponent<RigidBodyComponent>();

			transform.position.x += rigidbody.velocity.x*deltaTime;
			transform.position.y += rigidbody.velocity.y*deltaTime;

			if (entity.HasTag("player")) {
				int paddingTop = 8;
				int paddingRight = 8;
				int paddingDown = 8;
				int paddingLeft = 8;
				transform.position.x = transform.position.x < paddingLeft ? paddingLeft : transform.position.x;
				transform.position.x = transform.position.x > Game::mapWidth - paddingRight ? Game::mapWidth - paddingRight : transform.position.x;
				transform.position.y = transform.position.y < paddingLeft ? paddingLeft : transform.position.y;
				transform.position.y = transform.position.y > Game::mapWidth - paddingRight ? Game::mapWidth - paddingRight : transform.position.y;
			}

			bool isEntityOutsideMap = (
				transform.position.x < 0 ||
				transform.position.x > Game::mapWidth ||
				transform.position.y < 0 ||
				transform.position.y > Game::mapHeight
			);

			if (isEntityOutsideMap && !entity.HasTag("player")) {
				entity.Kill();
			}
		}
	}
};