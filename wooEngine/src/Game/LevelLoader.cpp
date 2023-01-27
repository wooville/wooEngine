#include "LevelLoader.h"
#include "Game.h"
#include "../ECS/ECS.h"
#include "../AssetStore/AssetStore.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/AnimationComponent.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/KeyboardControllerComponent.h"
#include "../Components/CameraFollowComponent.h"
#include "../Components/ProjectileEmitterComponent.h"
#include "../Components/HealthComponent.h"
#include "../Components/TextLabelComponent.h"
#include "../Components/ScriptComponent.h"
#include <string>
#include <memory>
#include <fstream>

LevelLoader::LevelLoader() {
	Logger::Log("LevelLoader constructor called.");
}

LevelLoader::~LevelLoader() {
	Logger::Log("LevelLoader destructor called.");
}

void LevelLoader::LoadLevel(sol::state& lua, const std::unique_ptr<Registry>& registry, std::unique_ptr<AssetStore>& assetStore, SDL_Renderer* renderer, int levelNum) {
	sol::load_result script = lua.load_file("./assets/scripts/level" + std::to_string(levelNum) + ".lua");

	if (!script.valid()) {
		sol::error err = script;
		std::string errorMessage = err.what();
		Logger::Err("Error loading lua script: " + errorMessage);
		return;
	}

	lua.script_file("./assets/scripts/level" + std::to_string(levelNum) + ".lua");
	Logger::Log("LevelLoader opened level" + std::to_string(levelNum) + ".lua");

	// table containing values for current level
	sol::table level = lua["Level"];

	// read assets
	sol::table assets = level["assets"];

	int i = 0;
	while (true) {
		sol::optional<sol::table> hasAsset = assets[i];
		if (hasAsset == sol::nullopt) {
			break;
		}
		sol::table asset = assets[i];
		std::string assetType = asset["Type"];
		if (assetType != "texture") {
			assetStore->AddTexture(renderer, asset["id"], asset["file"]);
		}
		if (assetType != "font") {
			assetStore->AddFont(asset["id"], asset["file"], asset["font_size"]);
		}
		std::string assetId = asset["id"];
		Logger::Log("New " + assetType + " asset loaded to asset store, id: " + assetId);
		i++;
	}

	// read tilemap
	sol::table map = level["tilemap"];
	std::string mapFilePath = map["map_file"];
	std::string mapTextureAssetId = map["texture_asset_id"];
	int mapNumRows = map["num_rows"];
	int mapNumCols = map["num_cols"];
	int tileSize = map["tile_size"];
	double tileScale = map["scale"];

	std::fstream mapFile;
	mapFile.open(mapFilePath);

	for (int y = 0; y < mapNumRows; y++) {
		for (int x = 0; x < mapNumCols; x++) {
			char ch;
			mapFile.get(ch);
			int srcRectY = std::atoi(&ch) * tileSize;
			mapFile.get(ch);
			int srcRectX = std::atoi(&ch) * tileSize;
			mapFile.ignore();

			Entity tile = registry->CreateEntity();
			tile.Group("tiles");
			tile.AddComponent<TransformComponent>(glm::vec2(x * tileSize * tileScale, y * tileSize * tileScale), glm::vec2(tileScale, tileScale));
			tile.AddComponent<SpriteComponent>(mapTextureAssetId, tileSize, tileSize, 0, false, srcRectX, srcRectY);
		}
	}
	mapFile.close();
	Game::mapWidth = mapNumCols * tileSize * tileScale;
	Game::mapHeight = mapNumRows * tileSize * tileScale;

	// read entities and components
	sol::table entities = level["entities"];

	i = 0;
	while (true) {
		sol::optional<sol::table> hasEntity = entities[i];
		if (hasEntity == sol::nullopt) {
			break;
		}

		sol::table entity = entities[i];
		
		Entity newEntity = registry->CreateEntity();

		// tag entity
		sol::optional<std::string> hasTag = entity["tag"];
		if (hasTag != sol::nullopt) {
			newEntity.Tag(entity["tag"]);
		}

		// add entity to group
		sol::optional<std::string> hasGroup = entity["group"];
		if (hasGroup != sol::nullopt) {
			newEntity.Group(entity["group"]);
		}

		// add components
		sol::optional<sol::table> hasComponents = entity["components"];
		if (hasComponents != sol::nullopt) {
			// transform component
			sol::optional<sol::table> hasTransform = entity["components"]["transform"];
			if (hasTransform != sol::nullopt) {
				newEntity.AddComponent<TransformComponent>(
					glm::vec2(
						entity["components"]["transform"]["position"]["x"],
						entity["components"]["transform"]["position"]["y"]
						),
					glm::vec2(
						entity["components"]["transform"]["scale"]["x"].get_or(1.0),
						entity["components"]["transform"]["scale"]["y"].get_or(1.0)
						),
					entity["components"]["transform"]["rotation"].get_or(0.0)
					);
			}

			// rigidbody component
			sol::optional<sol::table> hasRigidBody = entity["components"]["rigidbody"];
			if (hasRigidBody != sol::nullopt) {
				newEntity.AddComponent<RigidBodyComponent>(
					glm::vec2(
						entity["components"]["rigidbody"]["velocity"]["x"].get_or(0.0),
						entity["components"]["rigidbody"]["velocity"]["y"].get_or(0.0)
						)
					);
			}

			// sprite component
			sol::optional<sol::table> hasSprite = entity["components"]["sprite"];
			if (hasSprite != sol::nullopt) {
				newEntity.AddComponent<SpriteComponent>(
					entity["components"]["sprite"]["texture_asset_id"],
					entity["components"]["sprite"]["width"],
					entity["components"]["sprite"]["height"],
					entity["components"]["sprite"]["z_index"].get_or(1),
					entity["components"]["sprite"]["fixed"].get_or(false),
					entity["components"]["sprite"]["src_rect_x"].get_or(0),
					entity["components"]["sprite"]["src_rect_y"].get_or(0)
					);
			}

			// animation component
			sol::optional<sol::table> hasAnimation = entity["components"]["animation"];
			if (hasAnimation != sol::nullopt) {
				newEntity.AddComponent<AnimationComponent>(
					entity["components"]["animation"]["num_frames"].get_or(1),
					entity["components"]["animation"]["speed_rate"].get_or(1)
					);
			}

			// boxcollider component
			sol::optional<sol::table> hasCollider = entity["components"]["boxcollider"];
			if (hasCollider != sol::nullopt) {
				newEntity.AddComponent<BoxColliderComponent>(
					entity["components"]["boxcollider"]["width"],
					entity["components"]["boxcollider"]["height"],
					glm::vec2(
						entity["components"]["boxcollider"]["offset"]["x"].get_or(0),
						entity["components"]["boxcollider"]["offset"]["y"].get_or(0)
						)
					);
			}

			// health component
			sol::optional<sol::table> hasHealth = entity["components"]["health"];
			if (hasHealth != sol::nullopt) {
				newEntity.AddComponent<HealthComponent>(
					static_cast<int>(entity["components"]["health"]["health_val"].get_or(100))
					);
			}

			// projectileemitter component
			sol::optional<sol::table> hasProjectileEmitter = entity["components"]["projectile_emitter"];
			if (hasProjectileEmitter != sol::nullopt) {
				newEntity.AddComponent<ProjectileEmitterComponent>(
					glm::vec2(
						entity["components"]["projectile_emitter"]["projectile_velocity"]["x"],
						entity["components"]["projectile_emitter"]["projectile_velocity"]["y"]
						),
					entity["components"]["projectile_emitter"]["repeat_frequency"].get_or(1) * 1000,
					entity["components"]["projectile_emitter"]["projectile_duration"].get_or(10000) * 10000,
					static_cast<int>(entity["components"]["projectile_emitter"]["hit_damage"].get_or(10)),
					entity["components"]["projectile_emitter"]["friendly"].get_or(false)
					);
			}

			// camerafollow component
			sol::optional<sol::table> hasCameraFollow = entity["components"]["camera_follow"];
			if (hasCameraFollow != sol::nullopt) {
				newEntity.AddComponent<CameraFollowComponent>();
			}

			// keyboardcontroller component
			sol::optional<sol::table> hasKeyboardController = entity["components"]["keyboard_controller"];
			if (hasKeyboardController != sol::nullopt) {
				newEntity.AddComponent<KeyboardControllerComponent>(
					glm::vec2(
						entity["components"]["keyboard_controller"]["up_velocity"]["x"],
						entity["components"]["keyboard_controller"]["up_velocity"]["y"]
						),
					glm::vec2(
						entity["components"]["keyboard_controller"]["right_velocity"]["x"],
						entity["components"]["keyboard_controller"]["right_velocity"]["y"]
						),
					glm::vec2(
						entity["components"]["keyboard_controller"]["down_velocity"]["x"],
						entity["components"]["keyboard_controller"]["down_velocity"]["y"]
						),
					glm::vec2(
						entity["components"]["keyboard_controller"]["left_velocity"]["x"],
						entity["components"]["keyboard_controller"]["left_velocity"]["y"]
						)
					);
			}

			// scripts
			sol::optional<sol::table> script = entity["components"]["on_update_script"];
			if (script != sol::nullopt) {
				sol::function func = entity["components"]["on_update_script"][0];
				newEntity.AddComponent<ScriptComponent>(func);
			}
		}

		i++;
	}
}