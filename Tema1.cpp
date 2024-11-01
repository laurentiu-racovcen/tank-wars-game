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

    Mesh* square1 = object2D::CreateSquare("square1", corner, squareSide, glm::vec3(1, 1, 0), true);
    AddMeshToList(square1);
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

float Tema1::TerrainFunction(float x) {
    return 0.49 * sin(x) + 0.15 * sin(0.8*x) + 0.3 * sin(3.3*x) + 1.2f;
}

void Tema1::GenerateTerrain() {
    // modifiable values
    float startX = 23.5f;
    float endX = 30.5f;
    float windowSegmentSizeX = 10;

    unsigned int nrPoints = 1920 / windowSegmentSizeX;
    float functionSegmentSizeX = (endX - startX) / nrPoints;

    // scale factor on Y-axis
    float scaleY = 250;

    for (size_t i = 0; i < nrPoints; i++) {

        // TODO: store all (x, y) coords in terrain_coords array

        // initialise this step's segment limits
        float y1 = scaleY * TerrainFunction(startX + i * functionSegmentSizeX);
        float y2 = scaleY * TerrainFunction(startX + (i+1) * functionSegmentSizeX);

        float x1 = i * windowSegmentSizeX;
        float x2 = (i + 1) * windowSegmentSizeX;

        modelMatrix = glm::mat3(1);
        modelMatrix *= transform2D::Translate(x1, 0);
        
        modelMatrix *= transform2D::Skew(glm::vec2(x1, y1), glm::vec2(x2, y2));
        modelMatrix *= transform2D::Scale(windowSegmentSizeX, y1);
        modelMatrix *= transform2D::Translate(0, 0);
        
        RenderMesh2D(meshes["square1"], shaders["VertexColor"], modelMatrix);
    }
}

void Tema1::Update(float deltaTimeSeconds)
{
    GenerateTerrain();
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
