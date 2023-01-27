#pragma once
#include "../ECS/ECS.h"
#include "../AssetStore/AssetStore.h"
#include <SDL.h>
#include <memory>
#include <sol/sol.hpp>

class LevelLoader {
public:
	LevelLoader();
	~LevelLoader();

	void LoadLevel(sol::state& lua, const std::unique_ptr<Registry>& registry, std::unique_ptr<AssetStore>& assetStore, SDL_Renderer* renderer, int level);
};