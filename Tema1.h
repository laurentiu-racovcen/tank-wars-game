#pragma once

#include "components/simple_scene.h"
#include "Tank.h"

// for 1920x1080 resolution is enough
#define MAX_TERRAIN_POINTS_NR 2000

namespace m1
{
    class Tema1 : public gfxc::SimpleScene
    {
    public:
        Tema1();
        ~Tema1();

        void Init() override;

    private:
        void CreateMesh(const char* name, const std::vector<VertexFormat>& vertices, const std::vector<unsigned int>& indices);
        void AddTank1Mesh();
        void AddTank2Mesh();
        void AddTankTurretMesh();

        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;

        void FillTerrainVector(float startX, float endX);
        void GenerateTerrain(float deltaTimeSeconds);
        float TerrainFunction(float x);
        void RenderTanks();
        float GetTankPositionY(float x);
        float GetTankAngle(float x);

    protected:
        glm::mat3 modelMatrix;
        glm::vec2 terrainPoints[MAX_TERRAIN_POINTS_NR];
        unsigned int terrainPointsNr;
        float windowSegmentSizeX;
        Tank tank1;
        Tank tank2;
    };
}   // namespace m1
