#pragma once

struct HealthComponent {
	int health_val;

	HealthComponent(int health_val = 0) {
		this->health_val = health_val;
	}
};