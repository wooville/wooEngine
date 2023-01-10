#pragma once

#include <glm/glm.hpp>
#include <SDL.h>

struct ProjectileEmitterComponent {
	glm::vec2 projectileVelocity;
	int repeatFreq;
	int projectileDuration;
	int hitDamage;
	bool isFriendly;
	int lastEmissionTime;

	ProjectileEmitterComponent(glm::vec2 projectileVelocity = glm::vec2(0), int repeatFreq = 0, int projectileDuration = 10000, int hitDamage = 10, bool isFriendly = false) {
		this->projectileVelocity = projectileVelocity;
		this->repeatFreq = repeatFreq;
		this->projectileDuration = projectileDuration;
		this->hitDamage = hitDamage;
		this->isFriendly = isFriendly;
		this->lastEmissionTime = SDL_GetTicks();
	}
};