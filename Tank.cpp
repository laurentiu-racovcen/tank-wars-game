#include "lab_m1/Tema1/Tank.h"

bool Tank::isNearProjectile(float projectileX, unsigned int tankScale)
{
	float startTankAreaX = this->positionX - 1.4f * tankScale*DEFAULT_TANK_SIZE;
	float endTankAreaX = this->positionX + 1.4f * tankScale * DEFAULT_TANK_SIZE;
	if ((projectileX >= startTankAreaX) && (projectileX <= endTankAreaX)) {
		return true;
	}
	else {
		return false;
	}
}

void Tank::collidesWithProjectile() {
	health -= PROJECTILE_DAMAGE;
}
