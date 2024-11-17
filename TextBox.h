#pragma once

#include "components/simple_scene.h"
#include "TextBox.h"

#define MENU_BACKGROUND_COLOR   0.5, 0.3, 0.2
#define MAX_BUTTONS_NUMBER		5

class TextBox {
public:
	unsigned int posX;
	unsigned int posY;
	float scale;
	glm::vec3 color;
	std::string text;
};
