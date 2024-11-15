#pragma once

#include <vector>
#include "Button.h"

#define MENU_BACKGROUND_COLOR   0.5, 0.3, 0.2
#define MAX_BUTTONS_NUMBER		5

class MenuSection {
	public:
		std::string name;
		unsigned int buttonsNumber;
		std::vector<Button> buttons;
		MenuSection(unsigned int buttonsNr) {
			this->buttonsNumber = buttonsNr;
		}
};

