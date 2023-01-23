#pragma once

#include "../ECS/ECS.h"
#include "../AssetStore/AssetStore.h"
#include "../EventBus/EventBus.h"
#include <SDL.h>

const int FPS = 60;
const int MILLISECS_PER_FRAME = 1000 / FPS;

class Game {
private:
	bool isRunning;
	bool isDebug;
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Rect camera;
	int millisecsPreviousFrame;
	
	std::unique_ptr<Registry> registry;
	std::unique_ptr<AssetStore> assetStore;
	std::unique_ptr<EventBus> eventBus;

public:
	Game();
	~Game();
	void Initialize();
	void Run();
	void ProcessInput();
	void Setup();
	void Update();
	void Render();
	void Destroy();

	void LoadLevel(int level);

	static int windowWidth;
	static int windowHeight;
	static int logicalWindowWidth;
	static int logicalWindowHeight;
	static int mapWidth;
	static int mapHeight;
};