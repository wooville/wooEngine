#ifndef RENDERSYSTEM_H
#define RENDERSYSTEM_H

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

	

	void Update(SDL_Renderer* renderer, std::unique_ptr<AssetStore>& assetStore) {
		//sort entities by z-index

		//organize into struct that couples transform and sprite components
		struct RenderableEntity {
			TransformComponent transformComponent;
			SpriteComponent spriteComponent;
		};
		std::vector<RenderableEntity> renderableEntities;

		//populate vector with relevant entities
		for (auto entity : GetSystemEntities()) {
			if (entity.HasComponent<TransformComponent>() && entity.HasComponent<SpriteComponent>()) {
				RenderableEntity renderableEntity;
				renderableEntity.transformComponent = entity.GetComponent<TransformComponent>();
				renderableEntity.spriteComponent = entity.GetComponent<SpriteComponent>();

				renderableEntities.emplace_back(renderableEntity);
			}
		}

		//sort by zIndex
		std::sort(renderableEntities.begin(), renderableEntities.end(), [](const RenderableEntity& a, const RenderableEntity& b) {
			return a.spriteComponent.zIndex < b.spriteComponent.zIndex;
		});

		//render entities
		for (auto entity : renderableEntities) {
			//update position based on velocity
			const auto& transform = entity.transformComponent;
			const auto& sprite = entity.spriteComponent;

			// set src rect of original sprite texture
			SDL_Rect srcRect = sprite.srcRect;

			SDL_Rect dstRect = {
				static_cast<int>(transform.position.x),
				static_cast<int>(transform.position.y),
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
				SDL_FLIP_NONE
			);

		}
	}
};

#endif