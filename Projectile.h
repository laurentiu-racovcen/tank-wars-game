#pragma once

#define GRAVITATIONAL_FORCE  9.81f
#define PROJECTILE_INITIAL_SPEED	 2.0f

class Projectile {
	public:
		float initialPositionX;
		float currentPositionX;
		float launchTime;
		float currentPositionY;
		bool isIdle;
};
