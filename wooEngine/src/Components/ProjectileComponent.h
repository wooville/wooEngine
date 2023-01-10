#pragma once

#include "../ECS/ECS.h"

struct ProjectileComponent {
	bool isFriendly;
	int hitDamage;
	int duration;
	int startTime;

	ProjectileComponent(bool isFriendly = false, int hitDamage = 0, int duration = 10000) {
		this->isFriendly = isFriendly;
		this->hitDamage = hitDamage;
		this->duration = duration;
		startTime = SDL_GetTicks();
	}
};