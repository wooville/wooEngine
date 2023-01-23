#pragma once

#include "../ECS/ECS.h"
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdlrenderer.h>
#include <imgui/imgui_impl_sdl.h>

class RenderGUISystem : public System {
public:
	RenderGUISystem() = default;

	void Update() {
		// refresh frame
		ImGui_ImplSDLRenderer_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();
		
		if (ImGui::Begin("Spawn Enemies")) {
			ImGui::Text("This is a window for spawning new enemies");
		}
		ImGui::End();

		// render
		ImGui::Render();
		ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
	}
};