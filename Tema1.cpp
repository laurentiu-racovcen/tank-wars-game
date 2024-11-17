#include "lab_m1/Tema1/Tema1.h"

#include <vector>
#include <iostream>

#include "lab_m1/Tema1/transform2D.h"
#include "lab_m1/Tema1/Object2D.h"

// text library
#define GLT_IMPLEMENTATION
#include "lab_m1/Tema1/libs/gltext/gltext.h"

// environment colors
#define SKY_COLOR         0.5705, 0.688, 0.7529
#define TERRAIN_COLOR     0.984, 0.8078, 0.290

// scale factor on OY-axis
#define SCALE_Y           250

// the function of the terrain
#define TERRAIN_FUNCTION  0.49 * sin(x) + \
                          0.15 * sin(0.8 * x) + \
                          0.3 * sin(3.3 * x) + 1.7f

// start and end coordinates of the function on X-axis
#define FUNCTION_START_X  23.5f
#define FUNCTION_END_X    30.5f

// thresholds
#define SLIDING_THRESHOLD 1.2f
#define TERRAIN_COLLISION_THRESHOLD 2.0f
#define TANK_COLLISION_THRESHOLD DEFAULT_TANK_SIZE/2

#define EXPLOSION_RADIUS 80.0f

#define GAME_TITLE_COLOR 1, 1, 1

using namespace std;
using namespace m1;

Tema1::Tema1()
{
}

Tema1::~Tema1()
{
}

void Tema1::Init()
{
    glm::ivec2 resolution = window->GetResolution();
    camera = GetSceneCamera();
    camera->SetOrthographic(0, (float)resolution.x, 0, (float)resolution.y, 0.01f, 400);
    camera->SetPosition(glm::vec3(0, 0, 50));
    cameraPosition = glm::vec3(0, 0, 50);
    cameraIsShaking = false;
    cameraShakeDirection = 'N';
    camera->SetRotation(glm::vec3(0, 0, 0));
    camera->Update();
    GetCameraInput()->SetActive(false);

    // add square mesh for terrain generation
    glm::vec3 corner = glm::vec3(0, 0, 0);
    float squareSide = 1;

    Mesh* square = object2D::CreateSquare("square", corner, squareSide, glm::vec3(TERRAIN_COLOR), true);
    AddMeshToList(square);

    // initialize terrain_points vector
    for (unsigned int i = 0; i < MAX_TERRAIN_POINTS_NR; i++) {
        terrainPoints[i] = glm::vec2(0.0f, 0.0f);
    }

    // initialize terrain points number
    terrainPointsNr = 0;

    // initialize window segment size on X-axis
    windowSegmentSizeX = 2;

    // Fill the terrain points vector
    FillTerrainVector(FUNCTION_START_X, FUNCTION_END_X);

    // initialize tanks number
    tanksNumber = 2;

    // initialize tank size
    tankScale = 1;
    theme = "Simple";
    roundsNumber = 1;

    // initialize tanks position
    tanks[0].positionX = TANK1_INITIAL_X_POS;
    tanks[1].positionX = TANK2_INITIAL_X_POS;

    for (size_t i = 0; i < tanksNumber; i++) {
        // initialize tank's Y position
        tanks[i].positionY = GetPositionY(tanks[i].positionX);

        // center of the tank's bottom side
        tanks[i].cx = 50;
        
        // initialize turret angle to point upwards
        tanks[i].turretAngle = 0;

        // initialize turret's position
        tanks[i].turretPosition = glm::vec2(tanks[i].positionX, tanks[i].positionY);

        // initialize tanks' health
        tanks[i].health = 100;

    }

    // add tanks' meshes
    AddTankMesh(0, glm::vec3 (TANK0_COLOR));
    AddTankMesh(1, glm::vec3 (TANK1_COLOR));

    // add tank turret mesh
    AddTankTurretMesh();

    // add health bar mesh
    AddHealthBarBorderMesh();
    AddHealthBarMesh();

    // add and init tank projectile mesh
    AddTankProjectileMesh();
    InitTanksProjectilesData();

    AddProjectileTrajectoryMesh();

    AddMenuMeshes();

    // when opening the game, firstly show the menu
    showingMenu = true;

    // get window's resolution
    window_width = SimpleScene::window->props.resolution.x;
    window_height = SimpleScene::window->props.resolution.y;

    unsigned int sectionsNumber = 3;

    menu.sectionNameColor = glm::vec3(SECTION_NAME_COLOR);

    menu.initMenuData(sectionsNumber, window_width/2, window_height/2.3f, window_width/2.3f, window_height/2);

    // 2 buttons for tank size + 2*9 for tanks colors
    // 5 texts for: tank size, "-", "+", 2 tanks strings
    menu.sections[0] = MenuSection("Select*Tanks*Size", 0, 2 + 2 * 9, 5);

    /* Theme section */
    // 4 buttons for 4 themes
    // 4 texts for 4 themes
    menu.sections[1] = MenuSection("Theme-Selection", 1, 4, 4);

    /* Number of rounds section */
    // 2 buttons for round incremention/decrementation
    // 3 texts for "-", "+" and rounds number
    menu.sections[2] = MenuSection("Number-of-rounds", 2, 2, 3);

    // initialize sections' buttons and texts
    for (size_t i = 0; i < menu.sectionsNumber; i++) {
        menu.sections[i].initTextBoxes(window_width, window_height);
        menu.sections[i].updateTexts(tankScale, roundsNumber);
    }

    // open menu's first section
    menu.currentSection = 0;

    // initialize glText
    gltInit();

}

float Tema1::TerrainFunction(float x)
{
    return (float) TERRAIN_FUNCTION;
}

void Tema1::FillTerrainVector(float startX, float endX)
{
    terrainPointsNr = 1920 / windowSegmentSizeX + 1; // 1 extra point to complete the graph
    float functionSegmentSizeX = (endX - startX) / terrainPointsNr;

    for (size_t i = 0; i < terrainPointsNr; i++) {
        // store all (x, y) points in terrainPoints vector
        float x = i * windowSegmentSizeX;
        float y = SCALE_Y * TerrainFunction(startX + i * functionSegmentSizeX);
        terrainPoints[i] = glm::vec2(x, y);
    }
}

void Tema1::InitTanksProjectilesData()
{
    for (size_t i = 0; i < tanksNumber; i++) {
        for (size_t j = 0; j < MAX_PROJECTILES_NR; j++) {
            tanks[i].projectiles[j].ResetProjectile();
        }
    }
}

void Tema1::FrameStart()
{
    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(SKY_COLOR, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::ivec2 resolution = window->GetResolution();
    // Sets the screen area where to draw
    glViewport(0, 0, resolution.x, resolution.y);
}

void Tema1::UpdateTerrain(float deltaTimeSeconds)
{
    /* ------------- Sliding animation ------------- */

    // check if any of the array points have a higher difference than the threshold
    bool isHigherPoint = false;
    for (unsigned int i = 0; (i < terrainPointsNr) && (!isHigherPoint); i++) {
        float dy = abs(terrainPoints[i].y - terrainPoints[i + 1].y);
        // check if is higher than the limit
        if (dy > SLIDING_THRESHOLD) {
            isHigherPoint = true;
            //cout << "diff = " << dy << "\n";
        }
    }
    if (!isHigherPoint) cout << "Done sliding!\n";

    for (unsigned int i = 0; i < terrainPointsNr; i++) {
        /* landslide animation */
        // deltaTime limitation in order to NOT get too much terrain difference on lag/freeze
        if (isHigherPoint && deltaTimeSeconds < 0.1f) {
            // compute the height difference and set the landslide speed
            float dy = abs(terrainPoints[i].y - terrainPoints[i + 1].y);
            float landslide_speed = 40.0f / 3;

            // modify points' heights
            if (terrainPoints[i].y > terrainPoints[i + 1].y) {
                terrainPoints[i].y -= dy * landslide_speed * deltaTimeSeconds;
                terrainPoints[i + 1].y += dy * landslide_speed * deltaTimeSeconds;
            }
            else if (terrainPoints[i].y < terrainPoints[i + 1].y) {
                terrainPoints[i].y += dy * landslide_speed * deltaTimeSeconds;
                terrainPoints[i + 1].y -= dy * landslide_speed * deltaTimeSeconds;
            }
        }
    }
}

void Tema1::GenerateTerrain()
{
    for (unsigned int i = 0; i < terrainPointsNr; i++) {
        // first point
        float x1 = terrainPoints[i].x;
        float y1 = terrainPoints[i].y;

        // second point
        float x2 = terrainPoints[i+1].x;
        float y2 = terrainPoints[i+1].y;

        modelMatrix = glm::mat3(1);
        modelMatrix *= transform2D::Translate(x1, 0);
        modelMatrix *= transform2D::Skew(glm::vec2(x1, y1), glm::vec2(x2, y2));
        modelMatrix *= transform2D::Scale(windowSegmentSizeX, y1);
        modelMatrix *= transform2D::Translate(0, 0);

        RenderMesh2D(meshes["square"], shaders["VertexColor"], modelMatrix);
    }
}

float Tema1::GetPositionY(float x)
{
    float aproxIndex = x / windowSegmentSizeX;

    // get the indexes of A and B points in the terrainPoints array
    unsigned int indexA = trunc(aproxIndex);
    unsigned int indexB = indexA + 1;

    // extract A and B points from the array
    glm::vec2 A = terrainPoints[indexA];
    glm::vec2 B = terrainPoints[indexB];
    float t = (x - A.x) / (B.x - A.x);

    // limit tanks' height to y = 0
    if (A.y + t * (B.y - A.y) < 0) {
        return 0;
    }

    return A.y + t * (B.y - A.y);
}

float Tema1::GetTankAngle(float x)
{
    float aproxIndex = x / windowSegmentSizeX;

    // get the indexes of A and B points in the terrainPoints array
    unsigned int indexA = trunc(aproxIndex);
    unsigned int indexB = indexA + 1;

    // extract A and B points from the array
    glm::vec2 A = terrainPoints[indexA];
    glm::vec2 B = terrainPoints[indexB];

    float dx = B.x - A.x;
    float dy = B.y - A.y;

    return atan2f(dy, dx);
}

void Tema1::RenderTanksComponents(float deltaTimeSeconds)
{
    for (size_t i = 0; i < tanksNumber; i++) {
        if (tanks[i].health > 0) {
            // update tanks' Y coordinates
            tanks[i].positionY = GetPositionY(tanks[i].positionX);

            // compute tanks rotation angle
            if (tanks[i].positionY == 0) {
                tanks[i].rotationAngle = 0;
            }
            else {
                tanks[i].rotationAngle = GetTankAngle(tanks[i].positionX);
            }

            // tank health bar border
            modelMatrix = glm::mat3(1);
            modelMatrix *= transform2D::Translate(tanks[i].positionX, tanks[i].positionY + tankScale*DEFAULT_TANK_SIZE * 2.5f);
            modelMatrix *= transform2D::Scale(tankScale*DEFAULT_TANK_SIZE / 2, tankScale*DEFAULT_TANK_SIZE / 2);
            RenderMesh2D(meshes["health-bar-border"], shaders["VertexColor"], modelMatrix);

            // tank health bar
            modelMatrix = glm::mat3(1);
            // move the health bar to the left by "consumed health/2"
            modelMatrix *= transform2D::Translate(-(((MAX_HEALTH_POINTS - tanks[i].health) / MAX_HEALTH_POINTS) * 5.4 * tankScale*DEFAULT_TANK_SIZE / 2) / 2, 0);
            // translate the health bar above the tank
            modelMatrix *= transform2D::Translate(tanks[i].positionX, tanks[i].positionY + tankScale*DEFAULT_TANK_SIZE * 2.5f);
            modelMatrix *= transform2D::Scale((tankScale*DEFAULT_TANK_SIZE / 2) * (tanks[i].health / MAX_HEALTH_POINTS), tankScale*DEFAULT_TANK_SIZE / 2);
            RenderMesh2D(meshes["health-bar"], shaders["VertexColor"], modelMatrix);

            // tank
            modelMatrix = glm::mat3(1);
            modelMatrix *= transform2D::Translate(tanks[i].positionX, tanks[i].positionY - 3); // -3 for the tank to enter the terrain surface
            modelMatrix *= transform2D::Rotate(tanks[i].rotationAngle);
            modelMatrix *= transform2D::Scale(tankScale*DEFAULT_TANK_SIZE, tankScale*DEFAULT_TANK_SIZE);

            string meshID = "tank-" + to_string(i); // e.g. tank-0, tank-1, etc
            RenderMesh2D(meshes[meshID.c_str()], shaders["VertexColor"], modelMatrix);

            // tank turret
            modelMatrix = glm::mat3(1);
            modelMatrix *= transform2D::Translate(tanks[i].positionX, tanks[i].positionY - 3);
            modelMatrix *= transform2D::Rotate(tanks[i].rotationAngle);
            modelMatrix *= transform2D::Translate(0, 0.8f * tankScale * DEFAULT_TANK_SIZE);
            modelMatrix *= transform2D::Rotate(tanks[i].turretAngle - tanks[i].rotationAngle);
            modelMatrix *= transform2D::Scale(TURRET_WIDTH*tankScale, TURRET_LENGTH*tankScale);
            RenderMesh2D(meshes["tank-turret"], shaders["VertexColor"], modelMatrix);

            // update turret's position
            tanks[i].turretPosition = glm::vec2(modelMatrix[2][0], modelMatrix[2][1]);
        }
    }

    RenderTanksProjectiles(deltaTimeSeconds);
}

float Tema1::GetProjectilePositionY(float y0, float initialSpeedY, float t) {
    return -GRAVITATIONAL_FORCE * (t * t / 2) + initialSpeedY * t + y0;
}

void Tema1::ProjectileTerrainCollision(float x) {
    // locally decrease terrain height
    int collisionPointsNr = 2*(EXPLOSION_RADIUS / windowSegmentSizeX);
    int collisionIndex = x / windowSegmentSizeX;

    if ((collisionIndex >= 0) && (collisionIndex <= terrainPointsNr)) {
        // if the radius is outside the smaller limit (0), make changes starting with terrainPoints[0]
        for (size_t i = std::max(0, collisionIndex - collisionPointsNr / 2); i <= collisionIndex + collisionPointsNr / 2; i++) {
            float dx = abs(terrainPoints[collisionIndex].x - terrainPoints[i].x);
            float dy = sqrt(EXPLOSION_RADIUS * EXPLOSION_RADIUS - dx * dx);
            // update y-coordinate corresponding to the collision location
            terrainPoints[i].y -= dy;
        }
    }
}

/* Generates up-down movement in case of projectile collision */
void Tema1::CameraShake(float deltaTimeSeconds) {
    if ((cameraShakeDirection == 'N')) {
        if ((cameraPosition.y < 18)) {
            // go to north
            float newX = cameraPosition.x;
            float newY = cameraPosition.y + 3;
            glm::vec3 newCameraPosition = glm::vec3(newX, newY, cameraPosition.z);
            camera->SetPosition(newCameraPosition);
            cameraPosition = newCameraPosition;
        } else {
            // change camera direction to south
            cameraShakeDirection = 'S';
        }
    } else if ((cameraShakeDirection == 'S')) {
        if ((cameraPosition.y >= 0)) {
            // go to south
            float newX = cameraPosition.x;
            float newY = cameraPosition.y - 3;
            glm::vec3 newCameraPosition = glm::vec3(newX, newY, cameraPosition.z);
            camera->SetPosition(newCameraPosition);
            cameraPosition = newCameraPosition;
        }
        else {
            // change camera direction to south
            cameraShakeDirection = 0;
        }
    } else {
        // the camera becomes static
        cameraIsShaking = false;
        //cout << "shake done!";
    }
}

bool Tema1::IsAnyTankNearProjectile(float projectileX) {
    for (size_t i = 0; i < tanksNumber; i++) {
        if (tanks[i].isNearProjectile(projectileX, tankScale)) {
            return true;
        }
    }
    return false;
}

void Tema1::RenderTanksProjectiles(float deltaTimeSeconds)
{
    if (cameraIsShaking) {
        CameraShake(deltaTimeSeconds);
    }

    // compute the position of all tanks' projectiles

    for (size_t i = 0; i < tanksNumber; i++) {
        for (size_t j = 0; j < MAX_PROJECTILES_NR; j++) {
            if (!tanks[i].projectiles[j].isIdle) {
                // the projectile has been launched, now it is moving
                // check terrain limits
                if ((tanks[i].projectiles[j].x >= 0) && (tanks[i].projectiles[j].x <= windowSegmentSizeX * terrainPointsNr)) {
                    float projectileToGroundDiff = tanks[i].projectiles[j].y - GetPositionY(tanks[i].projectiles[j].x);

                    if ((projectileToGroundDiff < TANK_COLLISION_THRESHOLD) && (IsAnyTankNearProjectile(tanks[i].projectiles[j].x))) {
                        // check if the projectile is near any tank and modify tank's health if it collides
                        for (size_t k = 0; k < tanksNumber; k++) {
                            if (tanks[k].isNearProjectile(tanks[i].projectiles[j].x, tankScale)) {
                                tanks[k].collidesWithProjectile();
                                // reset projectile
                                tanks[i].projectiles[j].ResetProjectile();
                                // perform camera shake
                                cameraIsShaking = true;
                                cameraShakeDirection = 'N';
                                cout << "new health of tank" << k << ": " << tanks[k].health << "\n";
                            }
                        }
                    }
                    else if (projectileToGroundDiff < TERRAIN_COLLISION_THRESHOLD) {
                        ProjectileTerrainCollision(tanks[i].projectiles[j].x);
                        tanks[i].projectiles[j].ResetProjectile();
                        // perform camera shake
                        cameraIsShaking = true;
                        cameraShakeDirection = 'N';
                    }
                    else {
                        modelMatrix = glm::mat3(1);

                        // update projectile's attributes of movement
                        tanks[i].projectiles[j].time += 5 * deltaTimeSeconds;
                        tanks[i].projectiles[j].x = tanks[i].projectiles[j].x0 + tanks[i].projectiles[j].time * tanks[i].projectiles[j].initialSpeedX;
                        tanks[i].projectiles[j].y = GetProjectilePositionY(tanks[i].projectiles[j].y0, tanks[i].projectiles[j].initialSpeedY, tanks[i].projectiles[j].time);

                        modelMatrix *= transform2D::Translate(tanks[i].projectiles[j].x, tanks[i].projectiles[j].y);
                        modelMatrix *= transform2D::Scale(PROJECTILE_SIZE*tankScale, PROJECTILE_SIZE*tankScale);
                        RenderMesh2D(meshes["projectile"], shaders["VertexColor"], modelMatrix);
                    }
                }
                else {
                    // the projectile is outside the terrain limits, reset the projectile
                    tanks[i].projectiles[j].ResetProjectile();
                }
            }
        }
    }
}

void Tema1::RenderText(string text, float posX, float posY, float scale, glm::vec3 color) {
    glDisable(GL_DEPTH_TEST);

    // Creating text
    GLTtext* text_struct = gltCreateText();
    gltSetText(text_struct, text.c_str());

    // Begin text drawing
    gltBeginDraw();

    gltColor(color.x, color.y, color.z, 1.0f);
    gltDrawText2D(text_struct, posX, posY, scale);

    glEnable(GL_DEPTH_TEST);

    //// Finish drawing text
    //gltEndDraw();

    //// Deleting text
    //gltDeleteText(text);

    //// Destroy glText
    //gltTerminate();
}

void Tema1::RenderMenuBg(float deltaTimeSeconds)
{
    modelMatrix = glm::mat3(1);
    modelMatrix *= transform2D::Translate(menu.posX, menu.posY);
    modelMatrix *= transform2D::Scale(menu.width, menu.height);
    RenderMesh2D(meshes["menu-background"], shaders["VertexColor"], modelMatrix);

    RenderText("Tank-Wars", 1.0f*menu.posX/1.525f, 1.0f*menu.posY/ 2.8f, 8, glm::vec3(GAME_TITLE_COLOR));

}

void Tema1::RenderSection(float deltaTimeSeconds)
{
    /* render section background */
    modelMatrix = glm::mat3(1);
    modelMatrix *= transform2D::Translate(menu.posX, menu.posY);
    modelMatrix *= transform2D::Scale(menu.width - 30, menu.height - 30);
    RenderMesh2D(meshes["menu-section-background"], shaders["VertexColor"], modelMatrix);

    /* render section name */
    RenderText(menu.sections[menu.currentSection].name, 1.0f*window_width / 2.8f, 1.0f*window_height / 2.7f, 3.5f, menu.sectionNameColor);

    for (size_t i = 0; i < menu.sections[menu.currentSection].textBoxesNr; i++) {
        if (menu.sections[menu.currentSection].textBoxes[i].text != "") {
            /* render button text */
            RenderText(menu.sections[menu.currentSection].textBoxes[i].text,
                menu.sections[menu.currentSection].textBoxes[i].posX,
                menu.sections[menu.currentSection].textBoxes[i].posY,
                menu.sections[menu.currentSection].textBoxes[i].scale,
                menu.sections[menu.currentSection].textBoxes[i].color);
        }
    }

    RenderMenuBg(deltaTimeSeconds);
}

//void Tema1::RenderButton(Button button) {
//    modelMatrix = glm::mat3(1);
//    modelMatrix *= transform2D::Translate(button.posX, button.posY);
//    modelMatrix *= transform2D::Scale(button.scaleX, button.scaleY);
//
//    RenderMesh2D(meshes["button-" + button.color], shaders["VertexColor"], modelMatrix);
//}

void Tema1::RenderMenuArrows(float deltaTimeSeconds)
{
    window_width = SimpleScene::window->props.resolution.x;
    window_height = SimpleScene::window->props.resolution.y;

    // show current section's buttons
    if (menu.currentSection == 0) {
        // add only right menu arrow
        modelMatrix = glm::mat3(1);
        modelMatrix *= transform2D::Translate(1.0f * window_width / 1.355f, 1.0f * window_height / 2.8f);
        modelMatrix *= transform2D::Scale(1.0f * window_width / 9, 1.0f * menu.height / 3.5f);
        RenderMesh2D(meshes["menu-arrow"], shaders["VertexColor"], modelMatrix);

        //int right_arrow_x = modelMatrix[2][0];
        //int right_arrow_y = modelMatrix[2][1];
        //cout << "right arrow x = " << right_arrow_x << "\n" << "right arrow y = " << right_arrow_y << "\n";

    } else if (menu.currentSection > 0) {

        // add left menu arrow
        modelMatrix = glm::mat3(1);
        modelMatrix *= transform2D::Translate(1.0f * window_width / 3.79f, 1.0f * window_height / 2.0f);
        modelMatrix *= transform2D::Scale(1.0f * window_width / 9, 1.0f * menu.height / 3.5f);
        modelMatrix *= transform2D::Rotate(M_PI);
        RenderMesh2D(meshes["menu-arrow"], shaders["VertexColor"], modelMatrix);

        // add right menu arrow
        modelMatrix = glm::mat3(1);
        modelMatrix *= transform2D::Translate(1.0f * window_width / 1.355f, 1.0f * window_height / 2.8f);
        modelMatrix *= transform2D::Scale(1.0f * window_width / 9, 1.0f * menu.height / 3.5f);
        RenderMesh2D(meshes["menu-arrow"], shaders["VertexColor"], modelMatrix);
    }

    //// render current sections' buttons
    //for (size_t i = 0; i < menu.sections[menu.currentSection].buttonsNumber; i++) {
    //    RenderButton(menu.sections[menu.currentSection].buttons[i]);
    //}

    // render section background
    RenderSection(deltaTimeSeconds);
}

bool Tema1::isArrowClicked(unsigned int arrowID, int cursorX, int cursorY)
{
    window_width = SimpleScene::window->props.resolution.x;
    window_height = SimpleScene::window->props.resolution.y;

    // relative to left-bottom corner
    float arrowScaleX = 1.0f * window_width / 9;
    float arrowScaleY = 1.0f * menu.height / 3.5f;
    if (arrowID == 0) {
        cursorY = window_height - cursorY;
        // checking left arrow
        float arrowPosX = 1.0f * window_width / 3.79f;
        float arrowPosY = window_height - (1.0f * window_height / 2.0f);
        
        if ((cursorX < arrowPosX) && (cursorX > arrowPosX - arrowScaleX)) {
            // check Y coordinate
            if ((cursorY < arrowPosY) && (cursorY > arrowPosY - arrowScaleY)) {
                // check Y coordinate
                return true;
            }
        }
    } else if (arrowID == 1) {
        // checking right arrow
        float arrowPosX = (1.0f * window_width / 1.355f);
        float arrowPosY = window_height - (1.0f * window_height / 2.8f);
        if ((cursorX > arrowPosX) && (cursorX < arrowPosX + arrowScaleX)) {
            // check Y coordinate
            if ((cursorY < arrowPosY) && (cursorY > arrowPosY - arrowScaleY)) {
                // check Y coordinate
                return true;
            }
        }
    }
    return false;
}

// return the mouse-clicked textbox
int Tema1::GetClickedArrow(unsigned int cursorX, unsigned int cursorY) {
    for (size_t i = 0; i < 2; i++) {
        if (isArrowClicked(i, cursorX, cursorY)) {
            return i;
        }
    }
    return -1;
}

bool Tema1::isTextboxClicked(TextBox textbox, int cursorX, int cursorY)
{
    if ((cursorX > textbox.posX) && (cursorX < textbox.posX + textbox.scale * textbox.text.length() * 9)) {
        // cursor is in the width limits of this textbox
        cout << "good X!\n";
        if ((cursorY > textbox.posY) && (cursorY < textbox.posY + textbox.scale * 16)) {
            // cursor is in the width limits of this textbox
            cout << "The textbox -- " << textbox.text << " -- has been clicked!\n";
            return true;
        }
    }
    return false;
}

// return the mouse-clicked textbox
int Tema1::GetClickedTextBox(unsigned int cursorX, unsigned int cursorY) {
    for (size_t i = 0; i < menu.sections[menu.currentSection].textBoxesNr; i++) {
        if (isTextboxClicked(menu.sections[menu.currentSection].textBoxes[i], cursorX, cursorY)) {
            return i;
        }
    }
    return -1;
}

void Tema1::TextBoxAction(unsigned int TextBoxIndex) {
    if (menu.currentSection == 0) {
        // tankSize section
        if (TextBoxIndex == 0) {
            // decrement tankSize
            if (tankScale > 1) {
                tankScale--;
            }
        } else if (TextBoxIndex == 1) {
            // increment tankSize
            if (tankScale < 5) {
                tankScale++;
            }
        }
    } else if (menu.currentSection == 1) {
        // theme section
        if (TextBoxIndex == 0) {
            // select Simple Theme
            theme = "Simple";
        } else if (TextBoxIndex == 1) {
            // select Jungle Theme
            theme = "Jungle";
        } else if (TextBoxIndex == 2) {
            // select Mud Theme
            theme = "Mud";
        } else if (TextBoxIndex == 2) {
            // select Ice Theme
            theme = "Ice";
        }
    } else if (menu.currentSection == 2) {
        // Number of rounds section
        if (TextBoxIndex == 0) {
            // decrement rounds number
            if (roundsNumber > 1) {
                roundsNumber--;
            }
        } else if (TextBoxIndex == 1) {
            if (roundsNumber < 10) {
                // increment rounds number
                roundsNumber++;
            }
        }
    }
}

void Tema1::Update(float deltaTimeSeconds)
{
    if (showingMenu) {
        // get window's resolution
        window_width = SimpleScene::window->props.resolution.x;
        window_height = SimpleScene::window->props.resolution.y;
        
        for (size_t i = 0; i < menu.sectionsNumber; i++) {
            menu.sections[i].updateTexts(tankScale, roundsNumber);
        }
        menu.updateData(window_width / 2, window_height / 2.3f, window_width / 2.3f, window_height / 2);

        // render the menu
        RenderMenuArrows(deltaTimeSeconds);
    } else {
        UpdateTerrain(deltaTimeSeconds);
        GenerateTerrain();
        RenderTanksComponents(deltaTimeSeconds);
        DrawProjectileTrajectories(tankScale);
    }
}

void Tema1::FrameEnd()
{
}

void Tema1::OnInputUpdate(float deltaTime, int mods)
{
    if (tanks[0].health > 0) {
        // left-right movement (with OX-axis window limits)
        if ((window->KeyHold(GLFW_KEY_A) == true) && (tanks[0].positionX - deltaTime * TANK_SPEED > 0)) {
            tanks[0].positionX -= deltaTime * TANK_SPEED;
        }
        else if ((window->KeyHold(GLFW_KEY_D) == true) && (tanks[0].positionX + deltaTime * TANK_SPEED < 1920)) {
            tanks[0].positionX += deltaTime * TANK_SPEED;
        }

        if ((window->KeyHold(GLFW_KEY_W) == true) && tanks[0].turretAngle < M_PI_2 * 0.8f) {
            tanks[0].turretAngle += 2 * deltaTime;
        }
        else if ((window->KeyHold(GLFW_KEY_S) == true) && tanks[0].turretAngle > -M_PI_2 * 0.8f) {
            tanks[0].turretAngle -= 2 * deltaTime;
        }
    }

    if (tanks[1].health > 0) {
        if ((window->KeyHold(GLFW_KEY_LEFT) == true) && (tanks[1].positionX - deltaTime * TANK_SPEED > 0)) {
            tanks[1].positionX -= deltaTime * TANK_SPEED;
        }
        else if ((window->KeyHold(GLFW_KEY_RIGHT) == true) && (tanks[1].positionX + deltaTime * TANK_SPEED < 1920)) {
            tanks[1].positionX += deltaTime * TANK_SPEED;
        }

        if ((window->KeyHold(GLFW_KEY_UP) == true) && tanks[1].turretAngle > -M_PI_2*0.8f) {
            tanks[1].turretAngle -= 2 * deltaTime;
        }
        else if ((window->KeyHold(GLFW_KEY_DOWN) == true) && tanks[1].turretAngle < M_PI_2*0.8f) {
            tanks[1].turretAngle += 2 * deltaTime;
        }
    }
}

void Tema1::OnKeyPress(int key, int mods)
{
    if ((key == GLFW_KEY_SPACE) && (tanks[0].health > 0)) {
        // launch an available projectile from tank1
        for (size_t i = 0; i < MAX_PROJECTILES_NR; i++) {
            // launch the first available idle projectile
            if (tanks[0].projectiles[i].isIdle) {
                tanks[0].projectiles[i].isIdle = false;
                tanks[0].projectiles[i].x0 = tanks[0].turretPosition.x + TURRET_LENGTH * cos(tanks[0].turretAngle + M_PI_2);
                tanks[0].projectiles[i].y0 = tanks[0].turretPosition.y + TURRET_LENGTH * sin(tanks[0].turretAngle + M_PI_2);
                tanks[0].projectiles[i].x = tanks[0].projectiles[i].x0;
                tanks[0].projectiles[i].y = tanks[0].projectiles[i].y0;
                tanks[0].projectiles[i].initialSpeedX = PROJECTILE_INITIAL_SPEED * cos(tanks[0].turretAngle + M_PI_2);
                tanks[0].projectiles[i].initialSpeedY = PROJECTILE_INITIAL_SPEED * sin(tanks[0].turretAngle + M_PI_2);
                break;
            }
        }
    }

    if ((key == GLFW_KEY_ENTER) && (tanks[1].health > 0)) {
        // launch an available projectile from tank2
        for (size_t i = 0; i < MAX_PROJECTILES_NR; i++) {
            // launch the first available idle projectile
            if (tanks[1].projectiles[i].isIdle) {
                tanks[1].projectiles[i].isIdle = false;
                tanks[1].projectiles[i].x0 = tanks[1].turretPosition.x + TURRET_LENGTH * cos(tanks[1].turretAngle + M_PI_2);
                tanks[1].projectiles[i].y0 = tanks[1].turretPosition.y + TURRET_LENGTH * sin(tanks[1].turretAngle + M_PI_2);
                tanks[1].projectiles[i].x = tanks[1].projectiles[i].x0;
                tanks[1].projectiles[i].y = tanks[1].projectiles[i].y0;
                tanks[1].projectiles[i].initialSpeedX = PROJECTILE_INITIAL_SPEED * cos(tanks[1].turretAngle + M_PI_2);
                tanks[1].projectiles[i].initialSpeedY = PROJECTILE_INITIAL_SPEED * sin(tanks[1].turretAngle + M_PI_2);
                break;
            }
        }
    }
}


void Tema1::OnKeyRelease(int key, int mods)
{
}


void Tema1::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
}


void Tema1::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_2) {
        glm::ivec2 cursorCoords;
        cursorCoords = SimpleScene::window->GetCursorPosition();
        cout << "registered click at: " << cursorCoords.x << " " << cursorCoords.y << "\n";

        bool arrowClicked = false;
        /* check if an arrow was clicked */
        for (size_t i = 0; i < 2; i++) {
            if (isArrowClicked(i, cursorCoords.x, cursorCoords.y)) {
                arrowClicked = true;
                unsigned int arrowID = GetClickedArrow(cursorCoords.x, cursorCoords.y);
                if ((arrowID == 0) && (menu.currentSection > 0)) {
                    // pressed left arrow, go to the previous section
                    menu.currentSection--;
                }
                else if (arrowID == 1) {
                    // pressed right arrow
                    if (menu.currentSection == 2) {
                        // exit the menu, enter the game
                        showingMenu = false;
                    }
                    else {
                        // pressed right arrow, go to the next section
                        menu.currentSection++;
                    }
                }
                cout << "arrow " << i <<" clicked!\n";
            }
        }
        if (!arrowClicked) {
            unsigned int clickedTextBox = GetClickedTextBox(cursorCoords.x, cursorCoords.y);
            TextBoxAction(clickedTextBox);
        }
    }
}


void Tema1::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
}


void Tema1::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}


void Tema1::OnWindowResize(int width, int height)
{
}
