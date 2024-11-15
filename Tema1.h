#pragma once

#include "components/simple_scene.h"
#include "Tank.h"

// for 1920x1080 resolution is enough
#define MAX_TERRAIN_POINTS_NR 2000
#define MAX_TANKS_NR 4

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
        void AddTankMesh(unsigned int index, glm::vec3 color);
        void AddTankTurretMesh();
        void AddTankProjectileMesh();
        void AddProjectileTrajectoryMesh();
        void AddHealthBarBorderMesh();
        void AddHealthBarMesh();
        void DrawProjectileTrajectories();

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
        void CameraShake(float deltaTimeSeconds);

        void FillTerrainVector(float startX, float endX);
        void UpdateTerrain(float deltaTimeSeconds);
        void GenerateTerrain();
        float TerrainFunction(float x);
        void InitTanksProjectilesData();
        void RenderTanksComponents(float deltaTimeSeconds);
        void RenderTanksProjectiles(float deltaTimeSeconds);
        float GetPositionY(float x);
        float GetTankAngle(float x);
        float GetProjectilePositionY(float y0, float initialSpeedY, float t);
        void ProjectileTerrainCollision(float x);
        bool IsAnyTankNearProjectile(float x);

    protected:
        glm::mat3 modelMatrix;
        glm::vec2 terrainPoints[MAX_TERRAIN_POINTS_NR];
        unsigned int terrainPointsNr;
        float windowSegmentSizeX;
        unsigned int tanksNumber;
        Tank tanks[MAX_TANKS_NR];
        gfxc::Camera* camera;
        glm::vec3 cameraPosition;
        bool cameraIsShaking;
        char cameraShakeDirection;
        bool gameStarted;
    };
}   // namespace m1
