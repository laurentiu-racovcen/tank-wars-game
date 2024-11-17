#include "lab_m1/Tema1/menu/MenuSection.h"

#define COLOR_PURPLE  0.5,  0.4,   0.6
#define COLOR_BROWN   0.5,  0.3,   0.2
#define COLOR_GREEN   0.62, 0.87, 0.61
#define COLOR_BLUE    0.47, 0.70, 0.81
#define COLOR_WHITE   1,    1,    1

void MenuSection::initTextBoxes(int windowWidth, int windowHeight) {
	if (this->sectionNr == 0) {
		// Current section is "Tank Proprieties"

		// tank size minus
		textBoxes[0].text = "--";
		textBoxes[0].posX = 1.0f * windowWidth / 2.5;
		textBoxes[0].posY = 1.0f * windowHeight / 1.75f;
		textBoxes[0].scale = 3.7f;
		textBoxes[0].color = glm::vec3(COLOR_BROWN);
		textBoxes[0].isAligned = false;

		// tank size plus
		textBoxes[1].text = "++";
		textBoxes[1].posX = 1.0f * windowWidth / 1.77f;
		textBoxes[1].posY = 1.0f * windowHeight / 1.75f;
		textBoxes[1].scale = 3.7f;
		textBoxes[1].color = glm::vec3(COLOR_GREEN);
		textBoxes[1].isAligned = false;

		// tank size
		textBoxes[2].text = "";
		textBoxes[2].posX = 0; // is aligned, don't need posX
		textBoxes[2].posY = 1.0f * windowHeight / 1.65f;
		textBoxes[2].scale = 5.5f;
		textBoxes[2].color = glm::vec3(COLOR_WHITE);
		textBoxes[2].isAligned = true;

	} else if (this->sectionNr == 1) {
		// Current section is "Theme"
		textBoxes[0].text = "Simple";
		textBoxes[1].text = "Jungle";
		textBoxes[2].text = "Mud";
		textBoxes[3].text = "Ice";
	} else if (this->sectionNr == 2) {
		// Current section is "Number of rounds"

		// rounds nr minus
		textBoxes[0].text = "--";
		textBoxes[0].posX = 1.0f * windowWidth / 2.5;
		textBoxes[0].posY = 1.0f * windowHeight / 1.75f;
		textBoxes[0].scale = 3.7f;
		textBoxes[0].color = glm::vec3(COLOR_BROWN);
		textBoxes[0].isAligned = false;

		// rounds nr plus
		textBoxes[1].text = "++";
		textBoxes[1].posX = 1.0f * windowWidth / 1.77f;
		textBoxes[1].posY = 1.0f * windowHeight / 1.75f;
		textBoxes[1].scale = 3.7f;
		textBoxes[1].color = glm::vec3(COLOR_GREEN);
		textBoxes[1].isAligned = false;

		// rounds number text
		textBoxes[2].text = "";
		textBoxes[2].posX = 0; // is aligned, don't need posX
		textBoxes[2].posY = 1.0f * windowHeight / 1.65f;
		textBoxes[2].scale = 5.5f;
		textBoxes[2].color = glm::vec3(COLOR_WHITE);
		textBoxes[2].isAligned = true;
	}
}

void MenuSection::updateTexts(unsigned int tankSize, unsigned int roundsNumber) {
	if (this->sectionNr == 0) {
		// Current section is "Tank Size"

		// update tankSize
		textBoxes[2].text = std::to_string(tankSize);
	} else if (this->sectionNr == 2) {
		// Current section is "Number of rounds"

		// update roundsNumber
		textBoxes[2].text = std::to_string(roundsNumber);
	}
}

