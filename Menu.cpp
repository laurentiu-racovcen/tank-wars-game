#pragma once

#include "lab_m1/Tema1/Menu.h"

void Menu::initMenuData(unsigned int sectionsNumber, unsigned int posX, unsigned int posY, unsigned int width, unsigned int height) {
	this->sectionsNumber = sectionsNumber;
	this->posX = posX;
	this->posY = posY;
	this->width = width;
	this->height = height;
	this->backgroundColor = glm::vec3(MENU_BACKGROUND_COLOR);
	for (size_t i = 0; i < sectionsNumber; i++) {
		this->sections.push_back(MenuSection());
	}
}

void Menu::updateData(unsigned int posX, unsigned int posY, unsigned int width, unsigned int height) {
	this->posX = posX;
	this->posY = posY;
	this->width = width;
	this->height = height;
}
