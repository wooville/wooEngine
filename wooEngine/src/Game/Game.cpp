#include "Game.h"
#include "../Logger/Logger.h"
#include "../ECS/ECS.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/AnimationComponent.h"
#include "../Components/BoxColliderComponent.h"
#include "../Systems/MovementSystem.h"
#include "../Systems/RenderSystem.h"
#include "../Systems/AnimationSystem.h"
#include "../Systems/CollisionSystem.h"
#include "../Systems/RenderColliderSystem.h"
#include "../Systems/DamageSystem.h"
#include "../Systems/KeyboardControlSystem.h"
#include <SDL.h>
#include <SDL_image.h>
#include <glm/glm.hpp>
#include <iostream>
#include <memory>
#include <fstream>

Game::Game() {
	isRunning = false;
	isDebug = false;
	registry = std::make_unique<Registry>();
	assetStore = std::make_unique<AssetStore>();
	eventBus = std::make_unique<EventBus>();
	Logger::Log("Game constructor called.");
}

Game::~Game() {
	Logger::Log("Game destructor called.");
}

void Game::Initialize() {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		Logger::Err("Error initializing SDL.");
		return;
	}

	SDL_DisplayMode displayMode;
	SDL_GetCurrentDisplayMode(0, &displayMode);

	windowWidth = 800;  // displayMode.w;
	windowHeight = 600; // displayMode.h;
	window = SDL_CreateWindow(NULL,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		windowWidth,
		windowHeight,
		SDL_WINDOW_BORDERLESS);

	if (!window) {
		Logger::Err("Error creating SDL window.");
		return;
	}

	renderer = SDL_CreateRenderer(window,
		-1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC	//combine flags with bitwise OR
	);

	if (!renderer) {
		Logger::Err("Error creating SDL renderer.");
		return;
	}
	SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
	SDL_RenderSetLogicalSize(renderer, windowWidth, windowHeight);

	isRunning = true;
}

void Game::Run() {
	Setup();
	while (isRunning) {
		ProcessInput();
		Update();
		Render();
	}
}

void Game::ProcessInput() {
	SDL_Event sdlEvent;

	while(SDL_PollEvent(&sdlEvent)){
		switch (sdlEvent.type) {
		case SDL_QUIT :
			isRunning = false;
			break;
		case SDL_KEYDOWN:
			if (sdlEvent.key.keysym.sym == SDLK_ESCAPE) {
				isRunning = false;
			}
			if (sdlEvent.key.keysym.sym == SDLK_d) {	//toggle debug mode
				isDebug = !isDebug;
			}
			eventBus->EmitEvent<KeyPressedEvent>(sdlEvent.key.keysym.sym);
			break;
		}
	}
}

void Game::LoadLevel(int level) {
	registry->AddSystem<MovementSystem>();
	registry->AddSystem<RenderSystem>();
	registry->AddSystem<AnimationSystem>();
	registry->AddSystem<CollisionSystem>();
	registry->AddSystem<RenderColliderSystem>();
	registry->AddSystem<DamageSystem>();
	registry->AddSystem<KeyboardControlSystem>();

	// populate asset store
	assetStore->AddTexture(renderer, "tank-image", "./assets/images/tank-panther-right.png");
	assetStore->AddTexture(renderer, "truck-image", "./assets/images/truck-ford-right.png");
	assetStore->AddTexture(renderer, "chopper-image", "./assets/images/chopper.png");
	assetStore->AddTexture(renderer, "radar-image", "./assets/images/radar.png");
	// tilemap
	assetStore->AddTexture(renderer, "jungle-tilemap", "./assets/tilemaps/jungle.png");

	//read map
	int tileSize = 32;
	double tileScale = 2.0;
	int mapNumCols = 25;
	int mapNumRows = 20;

	std::fstream mapFile;
	mapFile.open("./assets/tilemaps/jungle.map");

	for (int y = 0; y < mapNumRows; y++) {
		for (int x = 0; x < mapNumCols; x++) {
			char ch;
			mapFile.get(ch);
			int srcRectY = std::atoi(&ch) * tileSize;
			mapFile.get(ch);
			int srcRectX = std::atoi(&ch) * tileSize;
			mapFile.ignore();

			Entity tile = registry->CreateEntity();
			tile.AddComponent<TransformComponent>(glm::vec2(x * tileSize * tileScale, y * tileSize * tileScale), glm::vec2(tileScale, tileScale));
			tile.AddComponent<SpriteComponent>("jungle-tilemap", tileSize, tileSize, 0, srcRectX, srcRectY);
		}
	}
	mapFile.close();

	Entity tank = registry->CreateEntity();
	Entity truck = registry->CreateEntity();
	Entity chopper = registry->CreateEntity();
	Entity radar = registry->CreateEntity();

	tank.AddComponent<TransformComponent>(glm::vec2(10.0, 100.0), glm::vec2(1.0, 1.0));
	tank.AddComponent<RigidBodyComponent>(glm::vec2(45.0, 0.0));
	tank.AddComponent<SpriteComponent>("tank-image", 32, 32, 2);
	tank.AddComponent<BoxColliderComponent>(32, 32);

	truck.AddComponent<TransformComponent>(glm::vec2(500.0, 100.0), glm::vec2(1.0, 1.0));
	truck.AddComponent<RigidBodyComponent>(glm::vec2(-55.0, 0.0));
	truck.AddComponent<SpriteComponent>("truck-image", 32, 32, 1);
	truck.AddComponent<BoxColliderComponent>(32, 32);

	chopper.AddComponent<TransformComponent>(glm::vec2(10.0, 10.0), glm::vec2(1.0, 1.0));
	chopper.AddComponent<RigidBodyComponent>(glm::vec2(10.0, 0.0));
	chopper.AddComponent<SpriteComponent>("chopper-image", 32, 32, 1);
	chopper.AddComponent<AnimationComponent>(2, 10, true);

	radar.AddComponent<TransformComponent>(glm::vec2(windowWidth - 74, 10.0), glm::vec2(1.0, 1.0));
	radar.AddComponent<RigidBodyComponent>(glm::vec2(0.0, 0.0));
	radar.AddComponent<SpriteComponent>("radar-image", 64, 64, 2);
	radar.AddComponent<AnimationComponent>(8, 5, true);

	
}

void Game::Setup() {
	LoadLevel(1);
}

void Game::Update() {
	// wait until current frame has passed into next frame to execute (framerate cap)
	int timeToWait = MILLISECS_PER_FRAME - (SDL_GetTicks() - millisecsPreviousFrame);
	if (timeToWait > 0 && timeToWait <= MILLISECS_PER_FRAME) {
		SDL_Delay(timeToWait);
	}

	// time since previous frame
	double deltaTime = (SDL_GetTicks() - millisecsPreviousFrame) / 1000.0;

	// current frame time
	millisecsPreviousFrame = SDL_GetTicks();
	
	// reset all event handlers for current frame
	eventBus->Reset();

	// subscribe to events for all systems for current frame
	registry->GetSystem<DamageSystem>().SubscribeToEvents(eventBus);
	registry->GetSystem<KeyboardControlSystem>().SubscribeToEvents(eventBus);

	// update registry to process entities
	registry->Update();

	// invoke systems that need to update
	registry->GetSystem<MovementSystem>().Update(deltaTime);
	registry->GetSystem<AnimationSystem>().Update();
	registry->GetSystem<CollisionSystem>().Update(eventBus);
	//CollisionSystem.Update();
	//DamageSystem.Update();
}

void Game::Render() {
	SDL_SetRenderDrawColor(renderer, 21, 21, 21, 255);
	SDL_RenderClear(renderer);

	// invoke systems that need to render
	registry->GetSystem<RenderSystem>().Update(renderer, assetStore);
	if (isDebug) {
		registry->GetSystem<RenderColliderSystem>().Update(renderer);
	}

	SDL_RenderPresent(renderer);
}

void Game::Destroy() {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}