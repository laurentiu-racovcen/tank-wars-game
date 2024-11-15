#include "lab_m1/Tema1/Tema1.h"

#include <vector>
#include <iostream>

#include "lab_m1/Tema1/transform2D.h"
#include "lab_m1/Tema1/Object2D.h"

// environment colors
#define SKY_COLOR         0.5705, 0.688, 0.7529
#define TERRAIN_COLOR     0.984, 0.8078, 0.290

// scale factor on OY-axis
#define SCALE_Y           250

// the function of the terrain
#define TERRAIN_FUNCTION  0.49 * sin(x) + \
                          0.15 * sin(0.8 * x) + \
                          0.3 * sin(3.3 * x) + 1.7f

// start and end coordinates of the function on OX-axis
#define FUNCTION_START_X  23.5f
#define FUNCTION_END_X    30.5f

// thresholds
#define SLIDING_THRESHOLD 1.2f
#define TERRAIN_COLLISION_THRESHOLD 2.0f
#define TANK_COLLISION_THRESHOLD TANK_SIZE/2

#define EXPLOSION_RADIUS 80.0f

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
            modelMatrix *= transform2D::Translate(tanks[i].positionX, tanks[i].positionY + TANK_SIZE * 2.5f);
            modelMatrix *= transform2D::Scale(TANK_SIZE / 2, TANK_SIZE / 2);
            RenderMesh2D(meshes["health-bar-border"], shaders["VertexColor"], modelMatrix);

            // tank health bar
            modelMatrix = glm::mat3(1);
            // move the health bar to the left by "consumed health/2"
            modelMatrix *= transform2D::Translate(-(((MAX_HEALTH_POINTS - tanks[i].health) / MAX_HEALTH_POINTS) * 5.4 * TANK_SIZE / 2) / 2, 0);
            // translate the health bar above the tank
            modelMatrix *= transform2D::Translate(tanks[i].positionX, tanks[i].positionY + TANK_SIZE * 2.5f);
            modelMatrix *= transform2D::Scale((TANK_SIZE / 2) * (tanks[i].health / MAX_HEALTH_POINTS), TANK_SIZE / 2);
            RenderMesh2D(meshes["health-bar"], shaders["VertexColor"], modelMatrix);

            // tank
            modelMatrix = glm::mat3(1);
            modelMatrix *= transform2D::Translate(tanks[i].positionX, tanks[i].positionY - 3); // -3 for the tank to enter the terrain surface
            modelMatrix *= transform2D::Rotate(tanks[i].rotationAngle);
            modelMatrix *= transform2D::Scale(TANK_SIZE, TANK_SIZE);

            string meshID = "tank-" + to_string(i); // e.g. tank-0, tank-1, etc
            RenderMesh2D(meshes[meshID.c_str()], shaders["VertexColor"], modelMatrix);

            // tank turret
            modelMatrix = glm::mat3(1);
            modelMatrix *= transform2D::Translate(tanks[i].positionX, tanks[i].positionY - 3);
            modelMatrix *= transform2D::Rotate(tanks[i].rotationAngle);
            modelMatrix *= transform2D::Translate(0, 0.8f * TANK_SIZE);
            modelMatrix *= transform2D::Rotate(tanks[i].turretAngle - tanks[i].rotationAngle);
            modelMatrix *= transform2D::Scale(TURRET_WIDTH, TURRET_LENGTH);
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
        if (tanks[i].isNearProjectile(projectileX)) {
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
                            if (tanks[k].isNearProjectile(tanks[i].projectiles[j].x)) {
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
                        modelMatrix *= transform2D::Scale(PROJECTILE_SIZE, PROJECTILE_SIZE);
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

void Tema1::Update(float deltaTimeSeconds)
{
    UpdateTerrain(deltaTimeSeconds);
    GenerateTerrain();
    RenderTanksComponents(deltaTimeSeconds);
    DrawProjectileTrajectories();
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
