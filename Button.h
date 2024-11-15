#pragma once

#include "components/simple_scene.h"
#include "Button.h"

#define MENU_BACKGROUND_COLOR   0.5, 0.3, 0.2
#define MAX_BUTTONS_NUMBER		5

class Button {
public:
	unsigned int posX;
	unsigned int posY;
	unsigned int width;
	unsigned int height;
	glm::vec3 color;
};

