#pragma once

#define GRAVITATIONAL_FORCE			 30.0f
#define PROJECTILE_INITIAL_SPEED	 150.0f
#define PROJECTILE_DAMAGE			 20.0f

class Projectile {
	public:
		float initialSpeedX;  // initial speed on X-axis
		float initialSpeedY;  // initial speed on Y-axis
		float x0;             // initial X coordinate of the projectile
		float y0;             // initial Y (height) of the projectile
		float time;           // elapsed time from launch
		float x;              // current X coordinate
		float y;              // current Y coordinate
		bool isIdle;          // true if the projectile has not been launched     
	public:
		void ResetProjectile();
};
