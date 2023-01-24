#pragma once

#include "../ECS/ECS.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/ProjectileEmitterComponent.h"
#include "../Components/HealthComponent.h"
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdlrenderer.h>
#include <imgui/imgui_impl_sdl.h>

class RenderGUISystem : public System {
public:
	RenderGUISystem() = default;

	void Update(SDL_Renderer* renderer, const std::unique_ptr<Registry>& registry, SDL_Rect& camera) {
		// refresh frame
		ImGui_ImplSDLRenderer_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();
		
		if (ImGui::Begin("Spawn Enemies")) {
			static int posX = 0;
			static int posY = 0;
			static int scaleX = 1;
			static int scaleY = 1;
			static int velX = 0;
			static int velY = 0;
			static int health = 100;
			static float rotation = 0.0;
			static float projAngle = 0.0;
			static float projSpeed = 0.0;
			static int projRepeat = 0;
			static int projDuration = 0;
			const char* sprites[] = {"tank-image", "truck-image", "axolot-image"};
			static int spriteIndex = 0;

			if (ImGui::CollapsingHeader("Sprite", ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::Combo("texture id", &spriteIndex, sprites, IM_ARRAYSIZE(sprites));
			}
			ImGui::Spacing();

			if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::InputInt("position x", &posX);
				ImGui::InputInt("position y", &posY);
				ImGui::SliderInt("scale x", &scaleX, 1, 10);
				ImGui::SliderInt("scale y", &scaleY, 1, 10);
				ImGui::SliderAngle("rotation", &rotation, 0, 360);
			}
			ImGui::Spacing();

			if (ImGui::CollapsingHeader("Rigid Body", ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::InputInt("velocity x", &velX);
				ImGui::InputInt("velocity y", &velY);
			}
			ImGui::Spacing();

			if (ImGui::CollapsingHeader("Projectile Emitter", ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::SliderAngle("angle (deg)", &projAngle, 0, 360);
				ImGui::SliderFloat("speed (px/sec)", &projSpeed, 10, 500);
				ImGui::InputInt("repeat (sec)", &projRepeat);
				ImGui::InputInt("duration (sec)", &projDuration);
			}
			ImGui::Spacing();

			if (ImGui::CollapsingHeader("Health", ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::SliderInt("%", &health, 0, 100);
			}

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			if (ImGui::Button("Spawn enemy")) {
				Entity enemy = registry->CreateEntity();
				enemy.Group("enemies");
				enemy.AddComponent<TransformComponent>(glm::vec2(posX, posY), glm::vec2(scaleX, scaleY), glm::degrees(rotation));
				enemy.AddComponent<RigidBodyComponent>(glm::vec2(velX, velY));
				enemy.AddComponent<SpriteComponent>(sprites[spriteIndex], 32, 32, 1);
				enemy.AddComponent<BoxColliderComponent>(25, 20, glm::vec2(5, 5));

				double projVelX = cos(projAngle) * projSpeed;
				double projVelY = sin(projAngle) * projSpeed;

				enemy.AddComponent<ProjectileEmitterComponent>(glm::vec2(projVelX , projVelY), projRepeat * 1000, projDuration * 1000, 10, false);
				enemy.AddComponent<HealthComponent>(health);

				posX = posY = scaleX = scaleY = rotation = projAngle = 0;
				scaleX = scaleY = 1;
				projRepeat = projDuration = 10;
				projSpeed = 100;
				health = 100;
			}
		}
		ImGui::End();

		ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoNav;
		ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always, ImVec2(0, 0));
		ImGui::SetNextWindowBgAlpha(0.9f);
		if (ImGui::Begin("Coordinates", NULL, windowFlags)) {
			static float logicalMousePosX, logicalMousePosY;
			SDL_RenderWindowToLogical(renderer, ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y, &logicalMousePosX, &logicalMousePosY);
			ImGui::Text(
				"Map coordinates (x=%.1f, y=%.1f)",
				logicalMousePosX + camera.x,
				logicalMousePosY + camera.y
			);
		}
		ImGui::End();

		// render
		ImGui::Render();
		ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
	}
};