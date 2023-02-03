#pragma once

#include "../ECS/ECS.h"
#include "../Components/TransformComponent.h"
#include "../Components/CameraFollowComponent.h"
#include "SDL.h"

class CameraMovementSystem : public System {
public:
	CameraMovementSystem() {
		RequireComponent<TransformComponent>();
		RequireComponent<CameraFollowComponent>();
	}

	void Update(SDL_Rect& camera) {
		for (auto entity : GetSystemEntities()) {
			auto transform = entity.GetComponent<TransformComponent>();

			if (transform.position.x + (camera.w / 2) < Game::mapWidth) {
				camera.x = transform.position.x - (Game::logicalWindowWidth / 2);
			}
			
			if (transform.position.y + (camera.h / 2) < Game::mapHeight) {
				camera.y = transform.position.y - (Game::logicalWindowHeight / 2);
			}

			// reset camera position if out of bounds
			camera.x = (camera.x < 0) ? 0 : camera.x;
			camera.y = (camera.y < 0) ? 0 : camera.y;
			camera.x = (camera.x > camera.w) ? camera.w : camera.x;
			camera.y = (camera.y > camera.h) ? camera.w : camera.y;
		}
	}
};