#ifndef GAME_H
#define GAME_H

#include "../ECS/ECS.h"
#include "../AssetStore/AssetStore.h"
#include <SDL.h>

const int FPS = 60;
const int MILLISECS_PER_FRAME = 1000 / FPS;

class Game {
private:
	bool isRunning;
	bool isDebug;
	SDL_Window* window;
	SDL_Renderer* renderer;
	int millisecsPreviousFrame;
	
	std::unique_ptr<Registry> registry;
	std::unique_ptr<AssetStore> assetStore;
	
	void LoadTilemap(int rows, int cols, int tileSize, std::string mapfile);

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

	int windowWidth;
	int windowHeight;
};

#endif