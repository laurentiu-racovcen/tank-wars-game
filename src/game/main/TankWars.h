#pragma once

#include "components/simple_scene.h"
#include "game/tank/Tank.h"
#include "game/menu/Menu.h"
#include <vector>

// for 1920x1080 resolution is enough
#define MAX_TERRAIN_POINTS_NR 2000
#define MAX_TANKS_NR 4
#define CLOUD_DISKS_NUMBER 100

namespace m1
{
    class TankWars : public gfxc::SimpleScene
    {
    public:
        TankWars();
        ~TankWars();

        void Init() override;

    private:
        void CreateMesh(const char* name, const std::vector<VertexFormat>& vertices, const std::vector<unsigned int>& indices);
        void AddAllMeshes();
        void AddTankMesh(unsigned int index, glm::vec3 color);
        void AddTankTurretMesh();
        void AddTankProjectileMesh();
        void AddProjectileTrajectoryMesh();
        void AddHealthBarBorderMesh();
        void AddHealthBarMesh();
        void DrawProjectileTrajectories(unsigned int tankScale);
        void AddMenuMeshes();
        void AddCloudMesh();

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
        void InitTanks();
        void RenderTanksComponents(float deltaTimeSeconds);
        void RenderTanksProjectiles(float deltaTimeSeconds);
        void RenderMenuBg(float deltaTimeSeconds);
        void RenderSection(float deltaTimeSeconds);
        void RenderMenu(float deltaTimeSeconds);
        void RenderText(std::string text, float posX, float posY, float scale, glm::vec3 color, bool isAligned);
        void RenderPlayerScores();
        void RenderClouds(float deltaTimeSeconds);
        float GetPositionY(float x);
        float GetTankAngle(float x);
        float GetProjectilePositionY(float y0, float initialSpeedY, float t);
        void ProjectileTerrainCollision(float x);
        bool IsAnyTankNearProjectile(float x);

        int GetClickedSectionTextBox(unsigned int cursorX, unsigned int cursorY);
        int GetClickedArrow(unsigned int cursorX, unsigned int cursorY);
        void SectionTextBoxAction(unsigned int TextBoxIndex);
        bool isTextboxClicked(TextBox textbox, int cursorX, int cursorY);
        bool isArrowClicked(unsigned int arrowID, int cursorX, int cursorY);
        bool isPlayAgainClicked(int cursorX, int cursorY);
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
        Menu menu;
        bool showingMenu;
        int window_width;
        int window_height;
        unsigned int tankScale;
        unsigned int roundsNumber;
        unsigned int currentRound;
        unsigned int tank0Score;
        unsigned int tank1Score;
        bool gameFinished;
        TextBox playAgainTextBox;
        std::vector<glm::vec2> clouds;
        float cloudStep;
    };
}   // namespace m1
