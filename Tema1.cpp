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
    auto camera = GetSceneCamera();
    camera->SetOrthographic(0, (float)resolution.x, 0, (float)resolution.y, 0.01f, 400);
    camera->SetPosition(glm::vec3(0, 0, 50));
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
    windowSegmentSizeX = 1;

    // Fill the terrain points vector
    FillTerrainVector(FUNCTION_START_X, FUNCTION_END_X);

    // initialize tanks position
    tank1.positionX = TANK1_INITIAL_X_POS;
    tank1.positionY = GetTankPositionY(tank1.positionX);

    tank2.positionX = TANK2_INITIAL_X_POS;
    tank2.positionY = GetTankPositionY(tank2.positionX);

    // center of the tanks bottom side
    tank1.cx = 50;
    tank2.cx = 50;

    // initialize turret angle to point upwards
    tank1.turretAngle = 0;
    tank2.turretAngle = 0;

    // add tanks' meshes
    AddTank1Mesh();
    AddTank2Mesh();

    // add tanks' turret mesh
    AddTankTurretMesh();
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

void Tema1::FrameStart()
{
    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(SKY_COLOR, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::ivec2 resolution = window->GetResolution();
    // Sets the screen area where to draw
    glViewport(0, 0, resolution.x, resolution.y);
}

void Tema1::GenerateTerrain()
{
    for (unsigned int i = 0; i < terrainPointsNr; i++) {

        // initialize this step's segment limits
        float x1 = terrainPoints[i].x;
        float y1 = terrainPoints[i].y;

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

float Tema1::GetTankPositionY(float x)
{
    float aproxIndex = x / windowSegmentSizeX;

    // get the indexes of A and B points in the terrainPoints array
    unsigned int indexA = trunc(aproxIndex);
    unsigned int indexB = indexA + 1;

    // extract A and B points from the array
    glm::vec2 A = terrainPoints[indexA];
    glm::vec2 B = terrainPoints[indexB];
    float t = (x - A.x) / (B.x - A.x);

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

void Tema1::RenderTanks()
{
    // update tanks' Y coordinates
    tank1.positionY = GetTankPositionY(tank1.positionX);
    tank2.positionY = GetTankPositionY(tank2.positionX);

    // compute tanks rotation angle
    float tank1Angle = GetTankAngle(tank1.positionX);
    float tank2Angle = GetTankAngle(tank2.positionX);

    // tank1
    modelMatrix = glm::mat3(1);
    modelMatrix *= transform2D::Translate(tank1.positionX, tank1.positionY - 3); // -3 for the tank to enter in terrain surface
    modelMatrix *= transform2D::Rotate(tank1Angle);
    modelMatrix *= transform2D::Scale(TANK_SIZE, TANK_SIZE);
    RenderMesh2D(meshes["tank1"], shaders["VertexColor"], modelMatrix);

    // tank1 turret
    modelMatrix = glm::mat3(1);
    modelMatrix *= transform2D::Translate(tank1.positionX, tank1.positionY - 3);
    modelMatrix *= transform2D::Rotate(tank1Angle);
    modelMatrix *= transform2D::Translate(0, 0.8f * TANK_SIZE);
    modelMatrix *= transform2D::Rotate(tank1.turretAngle - tank1Angle);
    modelMatrix *= transform2D::Scale(TURRET_WIDTH, TURRET_LENGTH);
    RenderMesh2D(meshes["tank-turret"], shaders["VertexColor"], modelMatrix);

    // tank2
    modelMatrix = glm::mat3(1);
    modelMatrix *= transform2D::Translate(tank2.positionX, tank2.positionY - 3); // -3 for the tank to enter in terrain surface
    modelMatrix *= transform2D::Rotate(tank2Angle);
    modelMatrix *= transform2D::Scale(TANK_SIZE, TANK_SIZE);
    RenderMesh2D(meshes["tank2"], shaders["VertexColor"], modelMatrix);

    // tank2 turret
    modelMatrix = glm::mat3(1);
    modelMatrix *= transform2D::Translate(tank2.positionX, tank2.positionY - 3);
    modelMatrix *= transform2D::Rotate(tank2Angle);
    modelMatrix *= transform2D::Translate(0, 0.8f * TANK_SIZE);
    modelMatrix *= transform2D::Rotate(tank2.turretAngle - tank2Angle);
    modelMatrix *= transform2D::Scale(TURRET_WIDTH, TURRET_LENGTH);
    RenderMesh2D(meshes["tank-turret"], shaders["VertexColor"], modelMatrix);
}

void Tema1::Update(float deltaTimeSeconds)
{
    GenerateTerrain();
    RenderTanks();
}

void Tema1::FrameEnd()
{
}

void Tema1::OnInputUpdate(float deltaTime, int mods)
{
    // left-right movement (with OX-axis window limits)
    if ((window->KeyHold(GLFW_KEY_A) == true) && (tank1.positionX - deltaTime * TANK_SPEED > 0)) {
        tank1.positionX -= deltaTime * TANK_SPEED;
    } else if ((window->KeyHold(GLFW_KEY_D) == true) && (tank1.positionX + deltaTime * TANK_SPEED < 1920)) {
        tank1.positionX += deltaTime * TANK_SPEED;
    }

    if ((window->KeyHold(GLFW_KEY_LEFT) == true) && (tank2.positionX - deltaTime * TANK_SPEED > 0)) {
        tank2.positionX -= deltaTime * TANK_SPEED;
    } else if ((window->KeyHold(GLFW_KEY_RIGHT) == true) && (tank2.positionX + deltaTime * TANK_SPEED < 1920)) {
        tank2.positionX += deltaTime * TANK_SPEED;
    }

    if (window->KeyHold(GLFW_KEY_W) == true) {
        tank1.turretAngle += 2 * deltaTime;
    } else if (window->KeyHold(GLFW_KEY_S) == true) {
        tank1.turretAngle -= 2 * deltaTime;
    }

    if (window->KeyHold(GLFW_KEY_UP) == true) {
        tank2.turretAngle -= 2 * deltaTime;
    } else if (window->KeyHold(GLFW_KEY_DOWN) == true) {
        tank2.turretAngle += 2 * deltaTime;
    }
}

void Tema1::OnKeyPress(int key, int mods)
{
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
