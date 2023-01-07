#pragma once

#include "../ECS/ECS.h"
#include "../EventBus/EventBus.h"
#include "../Events/KeyPressedEvent.h"
#include "SDL.h"

class KeyboardControlSystem : public System {
public:
	KeyboardControlSystem() {
		//RequireComponent<>;
	}

	void SubscribeToEvents(std::unique_ptr<EventBus>& eventBus) {
		eventBus->SubscribeToEvent<KeyPressedEvent>(this, &KeyboardControlSystem::OnKeyPressed);
	}

	void OnKeyPressed(KeyPressedEvent& event) {
		std::string keyCode = std::to_string(event.symbol);
		std::string keySymbol(1, event.symbol);
		Logger::Log("Key pressed: [" + keyCode + "] " + keySymbol);
	}

	void Update() {

	}
};