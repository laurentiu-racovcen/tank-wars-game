#pragma once

#include "Projectile.h"

#define TANK1_COLOR          0.5, 0.3, 0.2
#define TANK2_COLOR          0.5, 0.4, 0.6
#define TURRET_COLOR         0.2, 0.2, 0.2
#define TANK_SIZE            40
#define TANK_SPEED           250
#define TANK1_INITIAL_X_POS  200
#define TANK2_INITIAL_X_POS  1800
#define TURRET_WIDTH         1.0f * TANK_SIZE / 7
#define TURRET_LENGTH        TANK_SIZE * 1.35f
#define PROJECTILE_SIZE      1.0f * TANK_SIZE / 7
#define MAX_PROJECTILES_NR   100

class Tank {
	public:
		float positionX;
		float positionY;
		float rotationAngle;
		float cx;
		float turretAngle;
		Projectile projectiles[MAX_PROJECTILES_NR];
};
