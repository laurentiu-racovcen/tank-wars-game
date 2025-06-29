#pragma once

#include <vector>
#include "game/textbox/TextBox.h"

#define MENU_SECTION_BACKGROUND_COLOR   0.909, 0.89, 0.87
#define MAX_BUTTONS_NUMBER		        5
#define SECTION_NAME_COLOR              0.6,   0.6,  0.6
class MenuSection {
	public:
		std::string name;
		unsigned int sectionNr;
		unsigned int textBoxesNr;
		std::vector<TextBox> textBoxes;

		MenuSection(){};
		MenuSection(std::string name, unsigned int sectionNr, unsigned int textBoxesNr) {
			this->name = name;
			this->textBoxesNr = textBoxesNr;
			this->sectionNr = sectionNr;
			for (size_t i = 0; i < textBoxesNr; i++) {
				this->textBoxes.push_back(TextBox());
			}
		}
		void initTextBoxes(int windowWidth, int windowHeight);
		void updateTexts(unsigned int tankSize, unsigned int roundsNumber);
};
