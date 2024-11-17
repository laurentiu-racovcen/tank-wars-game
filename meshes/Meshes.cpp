#pragma once

#include "lab_m1/Tema1/main/Tema1.h"
#include "lab_m1/Tema1/meshes/transform2D.h"

using namespace std;
using namespace m1;

// Mesh colors
#define COLOR_PURPLE  0.5,  0.4,   0.6
#define COLOR_BROWN   0.5,  0.3,   0.2
#define COLOR_GREEN   0.62, 0.87, 0.61
#define COLOR_BLUE    0.47, 0.70, 0.81
#define COLOR_WHITE   1,    1,    1

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
        printf("\tOpenGL error.\n");
    }

    // Mesh information is saved into a Mesh object
    meshes[name] = new Mesh(name);
    meshes[name]->InitFromBuffer(VAO, static_cast<unsigned int>(indices.size()));
}

void Tema1::AddTankMesh(unsigned int index, glm::vec3 color) {
    // generate disk surface using k triangles with the tank
    unsigned int k = 20;
    vector<VertexFormat> vertices;
    vector<unsigned int> indices;

    // add origin of (x,y) = (0, 0.8)
    vertices.push_back(VertexFormat(glm::vec3(0, 0.8f, 0), color));

    float disk_radius = 0.6f;
    // insert all the vertices of the disk
    for (unsigned int i = 1; i <= k; i++) {
        vertices.push_back(VertexFormat(glm::vec3(disk_radius * cos(((float)i / k) * 2 * 3.14f), disk_radius * sin(((float)i / k) * 2 * 3.14f) + 0.8f, 0),
            glm::vec3(color)));
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
        VertexFormat(glm::vec3(-1, 0, 0), glm::vec3(color) - glm::vec3(0.1f, 0.1f, 0.1f)),       // Bottom-left    21
        VertexFormat(glm::vec3(1, 0, 0), glm::vec3(color) - glm::vec3(0.1f, 0.1f, 0.1f)),        // Bottom-right   22
        VertexFormat(glm::vec3(1.2f, 0.2, 0), glm::vec3(color) - glm::vec3(0.1f, 0.1f, 0.1f)),   // Top-right      23
        VertexFormat(glm::vec3(-1.2f, 0.2, 0), glm::vec3(color) - glm::vec3(0.1f, 0.1f, 0.1f)),  // Top-left       24

        // Upper part
        VertexFormat(glm::vec3(-1.4f, 0.2f, 0), glm::vec3(color)), // Bottom-left    25
        VertexFormat(glm::vec3(1.4f, 0.2f, 0), glm::vec3(color)),  // Bottom-right   26
        VertexFormat(glm::vec3(1.2f, 0.8, 0), glm::vec3(color)),   // Top-right      27
        VertexFormat(glm::vec3(-1.2f, 0.8, 0), glm::vec3(color)),  // Top-left       28
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

    string meshID = "tank-" + to_string(index); // e.g. tank-0, tank-1, etc.

    // Actually create the mesh from the data
    CreateMesh(meshID.c_str(), vertices, indices);
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
    vertices.push_back(VertexFormat(glm::vec3(0, 0, 0), glm::vec3(PROJECTILE_COLOR)));

    // insert all the vertices of the disk
    for (unsigned int i = 1; i <= k; i++) {
        vertices.push_back(VertexFormat(glm::vec3(cos(((float)i / k) * 2 * 3.14f), sin(((float)i / k) * 2 * 3.14f), 0),
            glm::vec3(PROJECTILE_COLOR)));
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

void Tema1::AddProjectileTrajectoryMesh()
{
    unsigned int k = 25;
    vector<VertexFormat> vertices;
    vector<unsigned int> indices;
    // initialize the first vertex (x,y) = (1,0) in the vertices vector

    // add origin of (x,y) = (0, 0)
    vertices.push_back(VertexFormat(glm::vec3(0, 0, 0), glm::vec3(TRAJECTORY_COLOR)));

    // insert all the vertices of the disk
    for (unsigned int i = 1; i <= k; i++) {
        vertices.push_back(VertexFormat(glm::vec3(cos(((float)i / k) * 2 * 3.14f), sin(((float)i / k) * 2 * 3.14f), 0),
            glm::vec3(TRAJECTORY_COLOR)));
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
    CreateMesh("projectile-trajectory", vertices, indices);
}

void Tema1::AddHealthBarBorderMesh()
{
    vector<VertexFormat> vertices
    {
        VertexFormat(glm::vec3(-2.8f, 0, 0), glm::vec3(HEALTH_BAR_COLOR)),     // outside-Bottom-left    0
        VertexFormat(glm::vec3(2.8f, 0, 0), glm::vec3(HEALTH_BAR_COLOR)),      // outside-Bottom-right   1
        VertexFormat(glm::vec3(2.8f, 1.2f, 0), glm::vec3(HEALTH_BAR_COLOR)),   // outside-Top-right      2
        VertexFormat(glm::vec3(-2.8f, 1.2f, 0), glm::vec3(HEALTH_BAR_COLOR)),  // outside-Top-left       3

        VertexFormat(glm::vec3(-2.7f, 0.1f, 0), glm::vec3(HEALTH_BAR_COLOR)),  // inside-Bottom-left     4
        VertexFormat(glm::vec3(2.7f, 0.1f, 0), glm::vec3(HEALTH_BAR_COLOR)),   // inside-Bottom-right    5
        VertexFormat(glm::vec3(2.7f, 1.1f, 0), glm::vec3(HEALTH_BAR_COLOR)),   // inside-Top-right       6
        VertexFormat(glm::vec3(-2.7f, 1.1f, 0), glm::vec3(HEALTH_BAR_COLOR)),  // inside-Top-left        7

        VertexFormat(glm::vec3(-2.7f, 0, 0), glm::vec3(HEALTH_BAR_COLOR)),     // margin-Bottom-left     8
        VertexFormat(glm::vec3(2.7f, 0, 0), glm::vec3(HEALTH_BAR_COLOR)),      // margin-Bottom-right    9
        VertexFormat(glm::vec3(2.7f, 1.2f, 0), glm::vec3(HEALTH_BAR_COLOR)),   // margin-Top-right       10
        VertexFormat(glm::vec3(-2.7f, 1.2f, 0), glm::vec3(HEALTH_BAR_COLOR)),  // margin-Top-left        11
    };

    vector<unsigned int> indices
    {
        // bottom side
        4, 8, 5,
        8, 9, 5,

        // right side
        9, 1, 10,
        1, 2, 10,

        // top side
        11, 7, 6,
        11, 6, 10,

        // left side
        0, 8, 3,
        3, 8, 11
    };

    // Create the mesh from the data
    CreateMesh("health-bar-border", vertices, indices);
}

void Tema1::AddHealthBarMesh()
{
    vector<VertexFormat> vertices
    {
        VertexFormat(glm::vec3(-2.7f, 0.1f, 0), glm::vec3(HEALTH_BAR_COLOR)),  // inside-Bottom-left     0
        VertexFormat(glm::vec3(2.7f, 0.1f, 0), glm::vec3(HEALTH_BAR_COLOR)),   // inside-Bottom-right    1
        VertexFormat(glm::vec3(2.7f, 1.1f, 0), glm::vec3(HEALTH_BAR_COLOR)),   // inside-Top-right       2
        VertexFormat(glm::vec3(-2.7f, 1.1f, 0), glm::vec3(HEALTH_BAR_COLOR)),  // inside-Top-left        3
    };

    vector<unsigned int> indices
    {
        0, 1, 2,
        0, 2, 3,
    };

    // Create the mesh from the data
    CreateMesh("health-bar", vertices, indices);
}

void Tema1::DrawProjectileTrajectories(unsigned int tankScale)
{
    /* Draw trajectory for tank1 projectile */
    for (size_t i = 0; i < MAX_TANKS_NR; i++) {
        if (tanks[i].health > 0) {

            Projectile projectile = Projectile();
            vector<VertexFormat> vertices;
            vector<unsigned int> indices;

            // initialize projectile parameters
            projectile.x0 = tanks[i].turretPosition.x + TURRET_LENGTH*tankScale * cos(tanks[i].turretAngle + M_PI_2);
            projectile.y0 = tanks[i].turretPosition.y + TURRET_LENGTH*tankScale * sin(tanks[i].turretAngle + M_PI_2);
            projectile.x = projectile.x0;
            projectile.y = projectile.y0;
            projectile.initialSpeedX = PROJECTILE_INITIAL_SPEED * cos(tanks[i].turretAngle + M_PI_2);
            projectile.initialSpeedY = PROJECTILE_INITIAL_SPEED * sin(tanks[i].turretAngle + M_PI_2);

            for (size_t time = 0; time < TRAJECTORY_POINTS_NR; time++) {
                // update projectile's attributes of movement
                projectile.x = projectile.x0 + 0.5f * time * projectile.initialSpeedX;
                projectile.y = GetProjectilePositionY(projectile.y0, projectile.initialSpeedY, 0.5f * time);

                vertices.push_back(VertexFormat(glm::vec3(projectile.x, projectile.y, 0), glm::vec3(TRAJECTORY_COLOR)));
                indices.push_back(time);

                modelMatrix = glm::mat3(1);
                modelMatrix *= transform2D::Translate(projectile.x, projectile.y);
                modelMatrix *= transform2D::Scale(PROJECTILE_SIZE*tankScale / 2, PROJECTILE_SIZE*tankScale / 2);
                RenderMesh2D(meshes["projectile-trajectory"], shaders["VertexColor"], modelMatrix);
            }
        }
    }
}

void Tema1::AddMenuMeshes()
{
    vector<VertexFormat> vertices
    {
        VertexFormat(glm::vec3(-0.5f, -0.5f, 0), glm::vec3(MENU_BACKGROUND_COLOR)),   // Bottom-left     0
        VertexFormat(glm::vec3(0.5f, -0.5f, 0), glm::vec3(MENU_BACKGROUND_COLOR)),    // Bottom-right    1
        VertexFormat(glm::vec3(0.5f, 0.5f, 0), glm::vec3(MENU_BACKGROUND_COLOR)),     // Top-right       2
        VertexFormat(glm::vec3(-0.5f, 0.5f, 0), glm::vec3(MENU_BACKGROUND_COLOR)),    // Top-left        3
    };

    vector<unsigned int> indices
    {
        0, 1, 2,
        0, 2, 3,
    };

    // Create the mesh from the data
    CreateMesh("menu-background", vertices, indices);

    for (size_t i = 0; i < vertices.size(); i++){
        vertices[i].color = glm::vec3(MENU_SECTION_BACKGROUND_COLOR);
    }
    // Create the mesh from the data
    CreateMesh("menu-section-background", vertices, indices);

    /* ----------------- Arrow Mesh ----------------- */

    // add right arrow mesh
    vertices.clear();
    indices.clear();

    vertices = {
        VertexFormat(glm::vec3(0, 0, 0), glm::vec3(MENU_BACKGROUND_COLOR)),          // Bottom-left     0
        VertexFormat(glm::vec3(0.5f, 0.5f, 0), glm::vec3(MENU_BACKGROUND_COLOR)),    // Mid-right       1
        VertexFormat(glm::vec3(0, 1, 0), glm::vec3(MENU_BACKGROUND_COLOR)),          // Top-left        2
    };

    indices = {
        0, 1, 2,
        0, 2, 3,
    };

    // Create the arrow mesh from the data
    CreateMesh("menu-arrow", vertices, indices);

}

void Tema1::AddCloudMesh()
{
    unsigned int k = 35;
    vector<VertexFormat> vertices;
    vector<unsigned int> indices;
    // initialize the first vertex (x,y) = (1,0) in the vertices vector

    // add origin of (x,y) = (0, 0)
    vertices.push_back(VertexFormat(glm::vec3(0, 0, 0), glm::vec3(COLOR_WHITE)));

    // insert all the vertices of the disk
    for (unsigned int i = 1; i <= k; i++) {
        vertices.push_back(VertexFormat(glm::vec3(cos(((float)i / k) * 2 * 3.14f), sin(((float)i / k) * 2 * 3.14f), 0),
            glm::vec3(COLOR_WHITE)));
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
    CreateMesh("cloud", vertices, indices);
}
