#pragma once

#include <SDL.h>

struct AnimationComponent {
	int numFrames;
	int currentFrame;
	int frameSpeedRate;
	bool isLoop;
	int startTime;

	AnimationComponent(int numFrames = 0, int frameSpeedRate = 0, bool isLoop = true) {
		this->numFrames = numFrames;
		this->currentFrame = currentFrame;
		this->frameSpeedRate = frameSpeedRate;
		this->isLoop = isLoop;
		this->startTime = SDL_GetTicks();

	}
};