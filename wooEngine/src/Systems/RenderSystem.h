#pragma once

#include "../ECS/ECS.h"
#include "../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"
#include <SDL.h>
#include <algorithm>

class RenderSystem : public System {
public:
	RenderSystem() {
		RequireComponent<TransformComponent>();
		RequireComponent<SpriteComponent>();
	}

	

	void Update(SDL_Renderer* renderer, std::unique_ptr<AssetStore>& assetStore, SDL_Rect& camera) {
		// organize into struct that couples transform and sprite components
		struct RenderableEntity {
			TransformComponent transformComponent;
			SpriteComponent spriteComponent;
		};
		std::vector<RenderableEntity> renderableEntities;

		// populate vector with relevant entities
		for (auto entity : GetSystemEntities()) {
			if (entity.HasComponent<TransformComponent>() && entity.HasComponent<SpriteComponent>()) {
				RenderableEntity renderableEntity;
				renderableEntity.transformComponent = entity.GetComponent<TransformComponent>();
				renderableEntity.spriteComponent = entity.GetComponent<SpriteComponent>();

				// don't bother rendering entities outside of camera
				bool isEntityOutsideCameraView = (
					renderableEntity.transformComponent.position.x + (renderableEntity.transformComponent.scale.x * renderableEntity.spriteComponent.width) < camera.x ||
					renderableEntity.transformComponent.position.x  > camera.x + camera.w ||
					renderableEntity.transformComponent.position.y + (renderableEntity.transformComponent.scale.y * renderableEntity.spriteComponent.height) < camera.y ||
					renderableEntity.transformComponent.position.y > camera.y + camera.h
				);

				if (isEntityOutsideCameraView && !renderableEntity.spriteComponent.isFixed) {
					continue;
				}

				renderableEntities.emplace_back(renderableEntity);
			}
		}

		//sort by zIndex
		std::sort(renderableEntities.begin(), renderableEntities.end(), [](const RenderableEntity& a, const RenderableEntity& b) {
			return a.spriteComponent.zIndex < b.spriteComponent.zIndex;
		});

		// render entities
		for (auto entity : renderableEntities) {
			// update position based on velocity
			const auto& transform = entity.transformComponent;
			const auto& sprite = entity.spriteComponent;

			// rectangle to carve out of original sprite texture
			SDL_Rect srcRect = sprite.srcRect;

			// where to draw entity on map
			SDL_Rect dstRect = {
				static_cast<int>(transform.position.x - (sprite.isFixed ? 0 : camera.x)),
				static_cast<int>(transform.position.y - (sprite.isFixed ? 0 : camera.y)),
				static_cast<int>(sprite.width * transform.scale.x),
				static_cast<int>(sprite.height * transform.scale.y)
			};

			SDL_RenderCopyEx(
				renderer,
				assetStore->GetTexture(sprite.assetId),
				&srcRect,
				&dstRect,
				transform.rotation,
				NULL,
				sprite.flip
			);

		}
	}
};