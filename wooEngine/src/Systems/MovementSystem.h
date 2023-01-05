#ifndef MOVEMENTSYSTEM_H
#define MOVEMENTSYSTEM_H

#include "../ECS/ECS.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"

class MovementSystem : public System {
public:
	MovementSystem() {
		 RequireComponent<TransformComponent>();
		 RequireComponent<RigidBodyComponent>();
	}

	void Update(double deltaTime) {
		for (auto entity : GetSystemEntities()) {
			//update position based on velocity
			auto& transform = entity.GetComponent<TransformComponent>();
			const auto& rigidBody = entity.GetComponent<RigidBodyComponent>();

			transform.position.x += rigidBody.velocity.x*deltaTime;
			transform.position.y += rigidBody.velocity.y*deltaTime;
		}
	}
};

#endif