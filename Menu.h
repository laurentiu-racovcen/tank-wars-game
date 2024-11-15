#pragma once

#include <vector>
#include "components/simple_scene.h"
#include "lab_m1/Tema1/MenuSection.h"

#define MENU_BACKGROUND_COLOR   0.5, 0.3, 0.2
#define MAX_SECTIONS_NUMBER		5

class Menu {
	public:
		unsigned int sectionsNumber;
		std::vector<MenuSection> sections;
		unsigned int posX;
		unsigned int posY;
		unsigned int width;
		unsigned int height;
		glm::vec3 backgroundColor;

		Menu(unsigned int sectionsNumber, unsigned int posX, unsigned int posY, unsigned int width, unsigned int height) {
			this->sectionsNumber = sectionsNumber;
			this->posX = posX;
			this->posY = posY;
			this->width = width;
			this->height = height;
			backgroundColor = glm::vec3(MENU_BACKGROUND_COLOR);
		}
};
