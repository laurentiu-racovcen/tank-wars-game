#include "lab_m1/Tema1/Tema1.h"

#include <vector>
#include <iostream>

#include "lab_m1/Tema1/transform2D.h"
#include "lab_m1/Tema1/object2D.h"

using namespace std;
using namespace m1;


/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */


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

    Mesh* square = object2D::CreateSquare("square", corner, squareSide, glm::vec3(1, 1, 0), true);
    AddMeshToList(square);

    // initialize terrain_points vector
    for (unsigned int i = 0; i < MAX_TERRAIN_POINTS_NR; i++) {
        terrainPoints[i] = glm::vec2(0.0f, 0.0f);
    }

    // initialize terrain points number
    terrainPointsNr = 0;

    // initialize window segment size on X-axis
    windowSegmentSizeX = 0;

    // Fill the terrain points vector
    windowSegmentSizeX = 10;
    FillTerrainVector(23.5f, 30.5f);

    // initialize tanks position
    tank1.positionX = terrainPoints[0].x;
    tank1.positionY = terrainPoints[0].y;

    tank2.positionX = terrainPoints[180].x;
    tank2.positionY = terrainPoints[180].y;

    // center of the tanks bottom side
    tank1.cx = 50;
    tank2.cx = 50;

    Mesh* tank1 = object2D::CreateSquare("tank1", corner, squareSide, glm::vec3(0, 1, 0), true);
    AddMeshToList(tank1);

    Mesh* tank2 = object2D::CreateSquare("tank2", corner, squareSide, glm::vec3(0, 0, 1), true);
    AddMeshToList(tank2);
}


void Tema1::FrameStart()
{
    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::ivec2 resolution = window->GetResolution();
    // Sets the screen area where to draw
    glViewport(0, 0, resolution.x, resolution.y);
}

float Tema1::TerrainFunction(float x)
{
    return (float) 0.49 * sin(x) + 0.15 * sin(0.8*x) + 0.3 * sin(3.3*x) + 1.2f;
}

void Tema1::FillTerrainVector(float startX, float endX)
{
    terrainPointsNr = 1920 / windowSegmentSizeX + 1; // 1 extra point to complete the graph
    float functionSegmentSizeX = (endX - startX) / terrainPointsNr;

    // scale factor on Y-axis
    float scaleY = 250;

    for (size_t i = 0; i < terrainPointsNr; i++) {
        // store all (x, y) points in terrainPoints vector
        float x = i * windowSegmentSizeX;
        float y = scaleY * TerrainFunction(startX + i * functionSegmentSizeX);
        terrainPoints[i] = glm::vec2(x, y);
    }
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

void Tema1::RenderTanks()
{
    modelMatrix = glm::mat3(1);
    modelMatrix *= transform2D::Translate(tank1.positionX, tank1.positionY);
    modelMatrix *= transform2D::Translate(-tank1.cx, 0);
    modelMatrix *= transform2D::Scale(100, 100);
    modelMatrix *= transform2D::Translate(0, 0);
    RenderMesh2D(meshes["tank1"], shaders["VertexColor"], modelMatrix);

    modelMatrix = glm::mat3(1);
    modelMatrix *= transform2D::Translate(tank2.positionX, tank2.positionY);
    modelMatrix *= transform2D::Translate(-tank2.cx, 0);
    modelMatrix *= transform2D::Scale(100, 100);
    modelMatrix *= transform2D::Translate(0, 0);
    RenderMesh2D(meshes["tank2"], shaders["VertexColor"], modelMatrix);
}

void Tema1::Update(float deltaTimeSeconds)
{
    GenerateTerrain();
    RenderTanks();
}


void Tema1::FrameEnd()
{
}


/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */


void Tema1::OnInputUpdate(float deltaTime, int mods)
{
}


void Tema1::OnKeyPress(int key, int mods)
{
    // Add key press event
}


void Tema1::OnKeyRelease(int key, int mods)
{
    // Add key release event
}


void Tema1::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    // Add mouse move event
}


void Tema1::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button press event
}


void Tema1::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
}


void Tema1::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}


void Tema1::OnWindowResize(int width, int height)
{
}
