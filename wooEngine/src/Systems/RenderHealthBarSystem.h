#pragma once

#include "../ECS/ECS.h"
#include "../Components/HealthComponent.h"
#include "../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"
#include "../AssetStore/AssetStore.h"
#include <SDL.h>

class RenderHealthBarSystem : public System {
public:
	RenderHealthBarSystem() {
		RequireComponent<HealthComponent>();
		RequireComponent<TransformComponent>();
		RequireComponent<SpriteComponent>();
	}

	void Update(SDL_Renderer* renderer, std::unique_ptr<AssetStore>& assetStore, SDL_Rect& camera) {
		for (auto entity : GetSystemEntities()) {
			const auto transform = entity.GetComponent<TransformComponent>();
			const auto sprite = entity.GetComponent<SpriteComponent>();
			const auto health = entity.GetComponent<HealthComponent>();

			// red health bar
			SDL_Color healthBarColor = {255, 0, 0};

			int healthBarWidth = 15;
			int healthBarHeight = 5;
			double healthBarPosX = (transform.position.x + (sprite.width * transform.scale.x)) - camera.x;
			double healthBarPosY = (transform.position.y + (sprite.width * transform.scale.y)) - camera.y;

			SDL_Rect healthBarRectangle = {
				static_cast<int>(healthBarPosX),
				static_cast<int>(healthBarPosY),
				static_cast<int>(healthBarWidth * (health.health_val / 100.0)),
				static_cast<int>(healthBarHeight)
			};
			SDL_SetRenderDrawColor(renderer, healthBarColor.r, healthBarColor.g, healthBarColor.b, 255);
			SDL_RenderFillRect(renderer, &healthBarRectangle);

			// render label
			std::string healthText = std::to_string(health.health_val);
			SDL_Surface* surface = TTF_RenderText_Blended(assetStore->GetFont("normal-font-small"), healthText.c_str(), healthBarColor);
			SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
			int labelWidth = 0;
			int labelHeight = 0;
			SDL_QueryTexture(texture, NULL, NULL, &labelWidth, &labelHeight);
			SDL_Rect healthBarTextRectangle = {
				static_cast<int>(healthBarPosX),
				static_cast<int>(healthBarPosY) + 5,	//a little above bar
				labelWidth,
				labelHeight
			};

			SDL_RenderCopy(renderer, texture, NULL, &healthBarTextRectangle);
		}
	}
};