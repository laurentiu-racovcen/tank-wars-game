#include "lab_m1/Tema1/Tema1.h"

#include <vector>
#include <iostream>

#include "lab_m1/Tema1/transform2D.h"
#include "lab_m1/Tema1/object2D.h"

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
#define COLLISION_THRESHOLD 2.0f

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

    // initialize window segment size on OX-axis
    windowSegmentSizeX = 2;

    // Fill the terrain points vector
    FillTerrainVector(FUNCTION_START_X, FUNCTION_END_X);

    // initialize tanks position
    tank1.positionX = TANK1_INITIAL_X_POS;
    tank1.positionY = GetPositionY(tank1.positionX);

    tank2.positionX = TANK2_INITIAL_X_POS;
    tank2.positionY = GetPositionY(tank2.positionX);

    // center of the tanks bottom side
    tank1.cx = 50;
    tank2.cx = 50;

    // initialize turret angle to point upwards
    tank1.turretAngle = 0;
    tank2.turretAngle = 0;

    // initialize turrets position
    tank1.turretPosition = glm::vec2(tank1.positionX, tank1.positionY);
    tank2.turretPosition = glm::vec2(tank2.positionX, tank2.positionY);

    // initialize tanks' health
    tank1.health = 100;
    tank2.health = 100;

    // add tanks' meshes
    AddTank1Mesh();
    AddTank2Mesh();

    // add tank turret mesh
    AddTankTurretMesh();

    // add tank projectile mesh
    AddTankProjectileMesh();

    InitTanksProjectilesData();
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
    for (size_t i = 0; i < MAX_PROJECTILES_NR; i++) {
        tank1.projectiles[i].ResetProjectile();
        tank2.projectiles[i].ResetProjectile();
    }
}

void Tema1::CreateMesh(const char* name, const std::vector<VertexFormat>& vertices, const std::vector<unsigned int>& indices)
{
    unsigned int VAO = 0;
    // Create and bind the VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    unsigned int VBO = 0;
    // Create and bind the VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Send vertices data into the VBO buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

    unsigned int IBO = 0;
    // Create and bind the IBO
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

    // Send indices data into the IBO buffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), &indices[0], GL_STATIC_DRAW);

    // Set vertex position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), 0);

    // Set vertex normal attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(sizeof(glm::vec3)));

    // Set texture coordinate attribute
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(2 * sizeof(glm::vec3)));

    // Set vertex color attribute
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(2 * sizeof(glm::vec3) + sizeof(glm::vec2)));

    // Unbind the VAO
    glBindVertexArray(0);

    // Check for OpenGL errors
    if (GetOpenGLError() == GL_INVALID_OPERATION)
    {
        cout << "\tOpenGL error." << std::endl;
    }

    // Mesh information is saved into a Mesh object
    meshes[name] = new Mesh(name);
    meshes[name]->InitFromBuffer(VAO, static_cast<unsigned int>(indices.size()));
}

void Tema1::AddTank1Mesh() {
    // generate disk surface using k triangles with the tank
    unsigned int k = 20;
    vector<VertexFormat> vertices;
    vector<unsigned int> indices;

    // add origin of (x,y) = (0, 0.8)
    vertices.push_back(VertexFormat(glm::vec3(0, 0.8f, 0), glm::vec3(TANK1_COLOR)));

    float disk_radius = 0.6f;
    // insert all the vertices of the disk
    for (unsigned int i = 1; i <= k; i++) {
        vertices.push_back(VertexFormat(glm::vec3(disk_radius * cos(((float)i / k) * 2 * 3.14f), disk_radius * sin(((float)i / k) * 2 * 3.14f) + 0.8f, 0),
            glm::vec3(TANK1_COLOR)));
    }

    // insert all the indices of the disk
    for (unsigned int i = 2; i <= k; i++) {
        indices.push_back(i);
        indices.push_back(0);
        indices.push_back(i - 1);
    }

    // add last triangle indices of the disk
    indices.push_back(1);
    indices.push_back(0);
    indices.push_back(k);

    // Middle and bottom part
    // Define tank vertices
    vector<VertexFormat> mid_bottom_tank_vertices
    {
        // Bottom part (slightly darker than the upper part)
        VertexFormat(glm::vec3(-1, 0, 0), glm::vec3(TANK1_COLOR) - glm::vec3(0.1f, 0.1f, 0.1f)),       // Bottom-left    21
        VertexFormat(glm::vec3(1, 0, 0), glm::vec3(TANK1_COLOR) - glm::vec3(0.1f, 0.1f, 0.1f)),        // Bottom-right   22
        VertexFormat(glm::vec3(1.2f, 0.2, 0), glm::vec3(TANK1_COLOR) - glm::vec3(0.1f, 0.1f, 0.1f)),   // Top-right      23
        VertexFormat(glm::vec3(-1.2f, 0.2, 0), glm::vec3(TANK1_COLOR) - glm::vec3(0.1f, 0.1f, 0.1f)),  // Top-left       24

        // Upper part
        VertexFormat(glm::vec3(-1.4f, 0.2f, 0), glm::vec3(TANK1_COLOR)), // Bottom-left    25
        VertexFormat(glm::vec3(1.4f, 0.2f, 0), glm::vec3(TANK1_COLOR)),  // Bottom-right   26
        VertexFormat(glm::vec3(1.2f, 0.8, 0), glm::vec3(TANK1_COLOR)),   // Top-right      27
        VertexFormat(glm::vec3(-1.2f, 0.8, 0), glm::vec3(TANK1_COLOR)),  // Top-left       28
    };

    vector<unsigned int> mid_bottom_tank_indices =
    {
        // bottom part
        21, 22, 23,
        21, 23, 24,

        // upper part
        25, 26, 27,
        25, 27, 28
    };

    // insert middle and bottom tank vertices and indices
    vertices.insert(vertices.end(), mid_bottom_tank_vertices.begin(), mid_bottom_tank_vertices.end());
    indices.insert(indices.end(), mid_bottom_tank_indices.begin(), mid_bottom_tank_indices.end());

    // Actually create the mesh from the data
    CreateMesh("tank1", vertices, indices);
}

void Tema1::AddTank2Mesh() {
    // generate disk surface using k triangles with the tank
    unsigned int k = 20;
    vector<VertexFormat> vertices;
    vector<unsigned int> indices;

    // add origin of (x,y) = (0, 0.8)
    vertices.push_back(VertexFormat(glm::vec3(0, 0.8f, 0), glm::vec3(TANK2_COLOR)));

    float disk_radius = 0.6f;
    // insert all the vertices of the disk
    for (unsigned int i = 1; i <= k; i++) {
        vertices.push_back(VertexFormat(glm::vec3(disk_radius * cos(((float)i / k) * 2 * 3.14f), disk_radius * sin(((float)i / k) * 2 * 3.14f) + 0.8f, 0),
            glm::vec3(TANK2_COLOR)));
    }

    // insert all the indices of the disk
    for (unsigned int i = 2; i <= k; i++) {
        indices.push_back(i);
        indices.push_back(0);
        indices.push_back(i - 1);
    }

    // add last triangle indices of the disk
    indices.push_back(1);
    indices.push_back(0);
    indices.push_back(k);

    // Middle and bottom part
    // Define tank vertices
    vector<VertexFormat> mid_bottom_tank_vertices
    {
        // Bottom part (slightly darker than the upper part)
        VertexFormat(glm::vec3(-1, 0, 0), glm::vec3(TANK2_COLOR) - glm::vec3(0.1f, 0.1f, 0.1f)),       // Bottom-left    21
        VertexFormat(glm::vec3(1, 0, 0), glm::vec3(TANK2_COLOR) - glm::vec3(0.1f, 0.1f, 0.1f)),        // Bottom-right   22
        VertexFormat(glm::vec3(1.2f, 0.2, 0), glm::vec3(TANK2_COLOR) - glm::vec3(0.1f, 0.1f, 0.1f)),   // Top-right      23
        VertexFormat(glm::vec3(-1.2f, 0.2, 0), glm::vec3(TANK2_COLOR) - glm::vec3(0.1f, 0.1f, 0.1f)),  // Top-left       24

        // Upper part
        VertexFormat(glm::vec3(-1.4f, 0.2f, 0), glm::vec3(TANK2_COLOR)), // Bottom-left    25
        VertexFormat(glm::vec3(1.4f, 0.2f, 0), glm::vec3(TANK2_COLOR)),  // Bottom-right   26
        VertexFormat(glm::vec3(1.2f, 0.8, 0), glm::vec3(TANK2_COLOR)),   // Top-right      27
        VertexFormat(glm::vec3(-1.2f, 0.8, 0), glm::vec3(TANK2_COLOR)),  // Top-left       28
    };

    vector<unsigned int> mid_bottom_tank_indices =
    {
        // bottom part
        21, 22, 23,
        21, 23, 24,

        // upper part
        25, 26, 27,
        25, 27, 28
    };

    // insert middle and bottom tank vertices and indices
    vertices.insert(vertices.end(), mid_bottom_tank_vertices.begin(), mid_bottom_tank_vertices.end());
    indices.insert(indices.end(), mid_bottom_tank_indices.begin(), mid_bottom_tank_indices.end());

    // Actually create the mesh from the data
    CreateMesh("tank2", vertices, indices);
}

void Tema1::AddTankTurretMesh()
{
    vector<VertexFormat> vertices
    {
        VertexFormat(glm::vec3(-0.5, 0, 0), glm::vec3(TURRET_COLOR)),  // Bottom-left    0
        VertexFormat(glm::vec3(0.5, 0, 0), glm::vec3(TURRET_COLOR)),   // Bottom-right   1
        VertexFormat(glm::vec3(0.5, 1, 0), glm::vec3(TURRET_COLOR)),   // Top-right      2
        VertexFormat(glm::vec3(-0.5, 1, 0), glm::vec3(TURRET_COLOR))   // Top-left       3
    };

    vector<unsigned int> indices
    {
        0, 1, 2,
        0, 2, 3
    };

    // Create the mesh from the data
    CreateMesh("tank-turret", vertices, indices);
}

void Tema1::AddTankProjectileMesh()
{
    unsigned int k = 25;
    vector<VertexFormat> vertices;
    vector<unsigned int> indices;
    // initialize the first vertex (x,y) = (1,0) in the vertices vector

    // add origin of (x,y) = (0, 0)
    vertices.push_back(VertexFormat(glm::vec3(0, 0, 0), glm::vec3(0, 1, 1)));

    // insert all the vertices of the disk
    for (unsigned int i = 1; i <= k; i++) {
        vertices.push_back(VertexFormat(glm::vec3(cos(((float)i / k) * 2 * 3.14f), sin(((float)i / k) * 2 * 3.14f), 0),
            glm::vec3(0, 1, 1)));
    }

    // insert all the indices of the disk
    for (unsigned int i = 2; i <= k; i++) {
        indices.push_back(i);
        indices.push_back(0);
        indices.push_back(i - 1);
    }

    // add last triangle indices of the disk
    indices.push_back(1);
    indices.push_back(0);
    indices.push_back(k);

    // Actually create the mesh from the data
    CreateMesh("projectile", vertices, indices);
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

    // check if any of the array points have a higher difference than the limit
    bool isHigherPoint = false;
    for (unsigned int i = 0; (i < terrainPointsNr) && !isHigherPoint; i++) {
        float dy = abs(terrainPoints[i].y - terrainPoints[i + 1].y);
        //if (i==500) cout <<"y1=" << terrainPoints[i].y << "y2=" << terrainPoints[i+1].y << " diff= " << dy << endl;
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
    if (tank1.health > 0) {
        // update tanks' Y coordinates
        tank1.positionY = GetPositionY(tank1.positionX);

        // compute tanks rotation angle
        if (tank1.positionY == 0) {
            tank1.rotationAngle = 0;
        }
        else {
            tank1.rotationAngle = GetTankAngle(tank1.positionX);
        }

        // tank1
        modelMatrix = glm::mat3(1);
        modelMatrix *= transform2D::Translate(tank1.positionX, tank1.positionY - 3); // -3 for the tank to enter in terrain surface
        modelMatrix *= transform2D::Rotate(tank1.rotationAngle);
        modelMatrix *= transform2D::Scale(TANK_SIZE, TANK_SIZE);
        RenderMesh2D(meshes["tank1"], shaders["VertexColor"], modelMatrix);

        // tank1 turret
        modelMatrix = glm::mat3(1);
        modelMatrix *= transform2D::Translate(tank1.positionX, tank1.positionY - 3);
        modelMatrix *= transform2D::Rotate(tank1.rotationAngle);
        modelMatrix *= transform2D::Translate(0, 0.8f * TANK_SIZE);
        modelMatrix *= transform2D::Rotate(tank1.turretAngle - tank1.rotationAngle);
        modelMatrix *= transform2D::Scale(TURRET_WIDTH, TURRET_LENGTH);
        RenderMesh2D(meshes["tank-turret"], shaders["VertexColor"], modelMatrix);

        // update turret's position
        tank1.turretPosition = glm::vec2(modelMatrix[2][0], modelMatrix[2][1]);
    }

    if (tank2.health > 0) {
        tank2.positionY = GetPositionY(tank2.positionX);

        if (tank2.positionY == 0) {
            tank2.rotationAngle = 0;
        }
        else {
            tank2.rotationAngle = GetTankAngle(tank2.positionX);
        }

        // tank2
        modelMatrix = glm::mat3(1);
        modelMatrix *= transform2D::Translate(tank2.positionX, tank2.positionY - 3); // -3 for the tank to enter in the terrain surface
        modelMatrix *= transform2D::Rotate(tank2.rotationAngle);
        modelMatrix *= transform2D::Scale(TANK_SIZE, TANK_SIZE);
        RenderMesh2D(meshes["tank2"], shaders["VertexColor"], modelMatrix);

        // tank2 turret
        modelMatrix = glm::mat3(1);
        modelMatrix *= transform2D::Translate(tank2.positionX, tank2.positionY - 3);
        modelMatrix *= transform2D::Rotate(tank2.rotationAngle);
        modelMatrix *= transform2D::Translate(0, 0.8f * TANK_SIZE);
        modelMatrix *= transform2D::Rotate(tank2.turretAngle - tank2.rotationAngle);
        modelMatrix *= transform2D::Scale(TURRET_WIDTH, TURRET_LENGTH);
        RenderMesh2D(meshes["tank-turret"], shaders["VertexColor"], modelMatrix);

        // update turret's position
        tank2.turretPosition = glm::vec2(modelMatrix[2][0], modelMatrix[2][1]);
    }

    RenderTanksProjectiles(deltaTimeSeconds);
}

float Tema1::GetProjectilePositionY(float y0, float initialSpeedY, float t) {
    return -GRAVITATIONAL_FORCE * (t * t / 2) + initialSpeedY * t + y0;
}

void Tema1::ProjectileTerrainCollision(float x) {
    // decrease terrain height
    int collisionPointsNr = 2*(EXPLOSION_RADIUS / windowSegmentSizeX);
    int collisionIndex = x / windowSegmentSizeX;

    if ((collisionIndex >= 0) && (collisionIndex <= terrainPointsNr)) {
        //cout << "col idx = " << collisionIndex << "\n";
        //cout << "start i = " << collisionIndex - collisionPointsNr / 2 << "\n";
        // if the radius is outside the smaller limit, make changes starting with terrainPoints[0]
        for (size_t i = std::max(0, collisionIndex - collisionPointsNr / 2); i <= collisionIndex + collisionPointsNr / 2; i++) {
            float dx = abs(terrainPoints[collisionIndex].x - terrainPoints[i].x);
            float dy = sqrt(EXPLOSION_RADIUS * EXPLOSION_RADIUS - dx * dx);
            //cout << "x=" << terrainPoints[i].x << "y=" << terrainPoints[i].y << "\n";
            //cout << "dx=" << dx << "dy=" << dy << "\n";

            // update y-coordinate corresponding to the collision location
            //float max_y = terrainPoints[collisionIndex].y + EXPLOSION_RADIUS;
            //float min_y = terrainPoints[collisionIndex].y - EXPLOSION_RADIUS;
            //if ((terrainPoints[i].y < max_y && terrainPoints[i].y > min_y)) {
            terrainPoints[i].y -= dy;
        }
    }
}

bool inTanksArea(float projectileX, float tankX) {
    float startTankAreaX = tankX - 1.4f * TANK_SIZE;
    float endTankAreaX = tankX + 1.4f * TANK_SIZE;
    if ((projectileX >= startTankAreaX) && (projectileX <= endTankAreaX)) {
        return true;
    } else {
        return false;
    }
}

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
    //camera->MoveRight(200*deltaTimeSeconds);
}

void ProjectileTankCollision(Tank &tank) {
    tank.health -= PROJECTILE_DAMAGE;
}

void Tema1::RenderTanksProjectiles(float deltaTimeSeconds)
{
    if (cameraIsShaking) {
        CameraShake(deltaTimeSeconds);
    }

    // position of the tank1 projectiles
    for (size_t i = 0; i < MAX_PROJECTILES_NR; i++) {
        if (!tank1.projectiles[i].isIdle) {
            if ((tank1.projectiles[i].x >= 0) && (tank1.projectiles[i].x <= windowSegmentSizeX * terrainPointsNr)) {
                // the projectile has been launched
                float projectileToGroundDiff = tank1.projectiles[i].y - GetPositionY(tank1.projectiles[i].x);
                if ((projectileToGroundDiff < COLLISION_THRESHOLD) && inTanksArea(tank1.projectiles[i].x, tank2.positionX)) {
                    ProjectileTankCollision(tank2);
                    // reset projectile
                    tank1.projectiles[i].ResetProjectile();
                    // perform camera shake
                    cameraIsShaking = true;
                    cameraShakeDirection = 'N';
                    cout << "new health of tank2: " << tank2.health << "\n";
                } else if (projectileToGroundDiff < COLLISION_THRESHOLD) {
                    ProjectileTerrainCollision(tank1.projectiles[i].x);
                    tank1.projectiles[i].ResetProjectile();
                    // perform camera shake
                    cameraIsShaking = true;
                    cameraShakeDirection = 'N';
                } else {
                    modelMatrix = glm::mat3(1);

                    // update projectile's attributes of movement
                    tank1.projectiles[i].time += 5 * deltaTimeSeconds;
                    tank1.projectiles[i].x = tank1.projectiles[i].x0 + tank1.projectiles[i].time * tank1.projectiles[i].initialSpeedX;
                    tank1.projectiles[i].y = GetProjectilePositionY(tank1.projectiles[i].y0, tank1.projectiles[i].initialSpeedY, tank1.projectiles[i].time);

                    modelMatrix *= transform2D::Translate(tank1.projectiles[i].x, tank1.projectiles[i].y);
                    modelMatrix *= transform2D::Scale(PROJECTILE_SIZE, PROJECTILE_SIZE);
                    RenderMesh2D(meshes["projectile"], shaders["VertexColor"], modelMatrix);
                }
            }
        }
    }
    
    // position of the tank2 projectiles
    for (size_t i = 0; i < MAX_PROJECTILES_NR; i++) {
        if (!tank2.projectiles[i].isIdle) {
            if ((tank2.projectiles[i].x >= 0) && (tank2.projectiles[i].x <= windowSegmentSizeX * terrainPointsNr)) {
                // the projectile has been launched
                float projectileToGroundDiff = tank2.projectiles[i].y - GetPositionY(tank2.projectiles[i].x);
                if ((projectileToGroundDiff < COLLISION_THRESHOLD) && inTanksArea(tank2.projectiles[i].x, tank1.positionX)) {
                    ProjectileTankCollision(tank1);
                    // reset projectile
                    tank2.projectiles[i].ResetProjectile();
                    // perform camera shake
                    cameraIsShaking = true;
                    cameraShakeDirection = 'N';
                    cout << "new health of tank1: " << tank1.health << "\n";
                }
                else if (projectileToGroundDiff < COLLISION_THRESHOLD) {
                    ProjectileTerrainCollision(tank2.projectiles[i].x);
                    tank2.projectiles[i].ResetProjectile();
                    // perform camera shake
                    cameraIsShaking = true;
                    cameraShakeDirection = 'N';
                }
                else {
                    modelMatrix = glm::mat3(1);

                    // update projectile's attributes of movement
                    tank2.projectiles[i].time += 5 * deltaTimeSeconds;
                    tank2.projectiles[i].x = tank2.projectiles[i].x0 + tank2.projectiles[i].time * tank2.projectiles[i].initialSpeedX;
                    tank2.projectiles[i].y = GetProjectilePositionY(tank2.projectiles[i].y0, tank2.projectiles[i].initialSpeedY, tank2.projectiles[i].time);

                    modelMatrix *= transform2D::Translate(tank2.projectiles[i].x, tank2.projectiles[i].y);
                    modelMatrix *= transform2D::Scale(PROJECTILE_SIZE, PROJECTILE_SIZE);
                    RenderMesh2D(meshes["projectile"], shaders["VertexColor"], modelMatrix);
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
}

void Tema1::FrameEnd()
{
}

void Tema1::OnInputUpdate(float deltaTime, int mods)
{
    if (tank1.health > 0) {
        // left-right movement (with OX-axis window limits)
        if ((window->KeyHold(GLFW_KEY_A) == true) && (tank1.positionX - deltaTime * TANK_SPEED > 0)) {
            tank1.positionX -= deltaTime * TANK_SPEED;
        }
        else if ((window->KeyHold(GLFW_KEY_D) == true) && (tank1.positionX + deltaTime * TANK_SPEED < 1920)) {
            tank1.positionX += deltaTime * TANK_SPEED;
        }

        if (window->KeyHold(GLFW_KEY_W) == true) {
            tank1.turretAngle += 2 * deltaTime;
        }
        else if (window->KeyHold(GLFW_KEY_S) == true) {
            tank1.turretAngle -= 2 * deltaTime;
        }
    }

    if (tank2.health > 0) {
        if ((window->KeyHold(GLFW_KEY_LEFT) == true) && (tank2.positionX - deltaTime * TANK_SPEED > 0)) {
            tank2.positionX -= deltaTime * TANK_SPEED;
        }
        else if ((window->KeyHold(GLFW_KEY_RIGHT) == true) && (tank2.positionX + deltaTime * TANK_SPEED < 1920)) {
            tank2.positionX += deltaTime * TANK_SPEED;
        }

        if (window->KeyHold(GLFW_KEY_UP) == true) {
            tank2.turretAngle -= 2 * deltaTime;
        }
        else if (window->KeyHold(GLFW_KEY_DOWN) == true) {
            tank2.turretAngle += 2 * deltaTime;
        }
    }
}

void Tema1::OnKeyPress(int key, int mods)
{
    if ((key == GLFW_KEY_SPACE) && (tank1.health > 0)) {
        // launch an available projectile from tank1
        for (size_t i = 0; i < MAX_PROJECTILES_NR; i++) {
            // launch the first available idle projectile
            if (tank1.projectiles[i].isIdle) {
                tank1.projectiles[i].isIdle = false;
                //tank2.projectiles[i].x0 = tank2.positionX * cos(tank2.rotationAngle) - tank2.positionY * sin(tank2.rotationAngle);
                //tank2.projectiles[i].y0 = tank2.positionX * sin(tank2.rotationAngle) + tank2.positionY * cos(tank2.rotationAngle);
                tank1.projectiles[i].x0 = tank1.turretPosition.x + TURRET_LENGTH * cos(tank1.turretAngle + M_PI_2);
                tank1.projectiles[i].y0 = tank1.turretPosition.y + TURRET_LENGTH * sin(tank1.turretAngle + M_PI_2);
                tank1.projectiles[i].x = tank1.projectiles[i].x0;
                tank1.projectiles[i].y = tank1.projectiles[i].y0;
                tank1.projectiles[i].initialSpeedX = PROJECTILE_INITIAL_SPEED * cos(tank1.turretAngle + M_PI_2);
                tank1.projectiles[i].initialSpeedY = PROJECTILE_INITIAL_SPEED * sin(tank1.turretAngle + M_PI_2);
                break;
            }
        }
    }

    if ((key == GLFW_KEY_ENTER) && (tank2.health > 0)) {
        // launch an available projectile from tank2
        for (size_t i = 0; i < MAX_PROJECTILES_NR; i++) {
            // launch the first available idle projectile
            if (tank2.projectiles[i].isIdle) {
                tank2.projectiles[i].isIdle = false;
                //tank2.projectiles[i].x0 = tank2.positionX * cos(tank2.rotationAngle) - tank2.positionY * sin(tank2.rotationAngle);
                //tank2.projectiles[i].y0 = tank2.positionX * sin(tank2.rotationAngle) + tank2.positionY * cos(tank2.rotationAngle);
                tank2.projectiles[i].x0 = tank2.turretPosition.x + TURRET_LENGTH * cos(tank2.turretAngle + M_PI_2);
                tank2.projectiles[i].y0 = tank2.turretPosition.y + TURRET_LENGTH * sin(tank2.turretAngle + M_PI_2);
                tank2.projectiles[i].x = tank2.projectiles[i].x0;
                tank2.projectiles[i].y = tank2.projectiles[i].y0;
                tank2.projectiles[i].initialSpeedX = PROJECTILE_INITIAL_SPEED * cos(tank2.turretAngle + M_PI_2);
                tank2.projectiles[i].initialSpeedY = PROJECTILE_INITIAL_SPEED * sin(tank2.turretAngle + M_PI_2);
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
