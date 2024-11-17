#pragma once

#include "lab_m1/Tema1/tank/Projectile.h"
#include "components/simple_scene.h"

#define TANK0_COLOR          0.5, 0.3, 0.2
#define TANK1_COLOR          0.5, 0.4, 0.6
#define TURRET_COLOR         0.2, 0.2, 0.2
#define HEALTH_BAR_COLOR     1, 1, 1
#define DEFAULT_TANK_SIZE    20
#define TANK_SPEED           250
#define TANK1_INITIAL_X_POS  200
#define TANK2_INITIAL_X_POS  1800
#define TURRET_WIDTH         1.0f * DEFAULT_TANK_SIZE / 7
#define TURRET_LENGTH        DEFAULT_TANK_SIZE * 1.35f
#define PROJECTILE_SIZE      1.0f * DEFAULT_TANK_SIZE / 5.5f
#define MAX_PROJECTILES_NR   30
#define MAX_HEALTH_POINTS    100

class Tank {
	public:
		float positionX;
		float positionY;
		float rotationAngle;
		float cx;
		float turretAngle;
		float health;
		glm::vec2 turretPosition;
		Projectile projectiles[MAX_PROJECTILES_NR];
	public:
		bool isNearProjectile(float projectileX, unsigned int tankScale);
		void collidesWithProjectile();
};
