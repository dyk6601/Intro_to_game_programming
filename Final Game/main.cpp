/**
* Author: Derek Kim
* Assignment: Final Project
* Date due: 2026-04-04, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "CS3113/LevelA.h"
#include "CS3113/LevelB.h"
#include "CS3113/LevelC.h"
#include "CS3113/LevelD.h"
#include "CS3113/LevelCToDScene.h"
#include "CS3113/MainMenuScene.h"
#include "CS3113/OpeningScene.h"
#include "CS3113/LoseScene.h"
#include "CS3113/WinScene.h"
#include "CS3113/ShaderProgram.h"
#include "CS3113/Effects.h"

// Global Constants
constexpr int SCREEN_WIDTH     = 1120,
              SCREEN_HEIGHT    = 600,
              FPS              = 120,
              NUMBER_OF_LEVELS = 9;

constexpr Vector2 ORIGIN      = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
            
constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;

int gLives = 3;
int gPreviousLives = 3;

float gLifeLossFlashTime = 0.28f;
float gLifeLossFlashTimer = 0.0f;

ShaderProgram gLifeLossShader;
Effects *gEffects = nullptr;

// Global Variables
AppStatus gAppStatus   = RUNNING;
float gPreviousTicks   = 0.0f,
      gTimeAccumulator = 0.0f;

Camera2D gCamera = { 0 };

Scene *gCurrentScene = nullptr;
std::vector<Scene*> gLevels = {};

LevelA *gLevelA = nullptr;
LevelB *gLevelB = nullptr;
LevelC *gLevelC = nullptr;
LevelD *gLevelD = nullptr;
LevelCToDScene *gLevelCToDScene = nullptr;
MainMenuScene *gMainMenu = nullptr;
OpeningScene *gOpeningScene = nullptr;
LoseScene *gLoseScene = nullptr;
WinScene *gWinScene = nullptr;

// Function Declarations
void switchToScene(Scene *scene);
void initialise();
void processInput();
void update();
void render();
void shutdown();
void startLifeLossFlash();
void applyProtagonistDamageShader();

void startLifeLossFlash()
{
    gLifeLossFlashTimer = gLifeLossFlashTime;
}

void applyProtagonistDamageShader()
{
    if (gCurrentScene == nullptr) return;

    Entity *player = gCurrentScene->getState().xochitl;
    if (player == nullptr) return;

    if (gLifeLossFlashTimer > 0.0f && gLifeLossShader.isLoaded())
        player->setRenderShader(gLifeLossShader.getShader());
    else
        player->clearRenderShader();
}

void switchToScene(Scene *scene)
{
    gCurrentScene = scene;
    gCurrentScene->initialise();

    applyProtagonistDamageShader();

    if (gCurrentScene->getState().xochitl != nullptr)
        gCamera.target = gCurrentScene->getState().xochitl->getPosition();
    else
        gCamera.target = ORIGIN;

    if (gEffects != nullptr)
    {
        if (gCurrentScene == gLevelA || gCurrentScene == gLevelB || gCurrentScene == gLevelD)
            gEffects->start(EffectType::TORCH_FLICKER);
        else
            gEffects->start(EffectType::NONE);
    }
}

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Scenes");
    InitAudioDevice();

    gMainMenu = new MainMenuScene(ORIGIN, "#011627");
    gOpeningScene = new OpeningScene(ORIGIN, "#0A0A0A");
    gLevelA = new LevelA(ORIGIN, "#C0897E");
    gLevelB = new LevelB(ORIGIN, "#011627");
    gLevelC = new LevelC(ORIGIN, "#533483");
    gLevelD = new LevelD(ORIGIN, "#C0897E");
    gLevelCToDScene = new LevelCToDScene(ORIGIN, "#05010A");
    gLoseScene = new LoseScene(ORIGIN, "#1F0000");
    gWinScene = new WinScene(ORIGIN, "#053B06");

    gLevelA->setSharedLives(&gLives);
    gLevelB->setSharedLives(&gLives);
    gLevelC->setSharedLives(&gLives);
    gLevelD->setSharedLives(&gLives);
    gLevelCToDScene->setSharedLives(&gLives);
    gOpeningScene->setSharedLives(&gLives);
    gLoseScene->setSharedLives(&gLives);
    gWinScene->setSharedLives(&gLives);

    gLevels.push_back(gMainMenu);
    gLevels.push_back(gLevelA);
    gLevels.push_back(gLevelB);
    gLevels.push_back(gLevelC);
    gLevels.push_back(gLoseScene);
    gLevels.push_back(gWinScene);
    gLevels.push_back(gOpeningScene);
    gLevels.push_back(gLevelD);
    gLevels.push_back(gLevelCToDScene);

    gLifeLossShader.load("shaders/vertex.glsl", "shaders/life_loss_yellow.glsl");
    gEffects = new Effects(ORIGIN, (float) SCREEN_WIDTH * 1.5f, (float) SCREEN_HEIGHT * 1.5f);

    switchToScene(gLevels[0]);

    gCamera.offset   = ORIGIN;
    gCamera.rotation = 0.0f;
    gCamera.zoom     = 1.0f;

    gPreviousLives = gLives;

    SetTargetFPS(FPS);
}

void processInput() 
{    
    if (IsKeyPressed(KEY_ONE))
    {
        switchToScene(gLevels[LEVEL_A_SCENE]);
        return;
    }
    if (IsKeyPressed(KEY_TWO))
    {
        switchToScene(gLevels[LEVEL_B_SCENE]);
        return;
    }
    if (IsKeyPressed(KEY_THREE))
    {
        switchToScene(gLevels[LEVEL_C_SCENE]);
        return;
    }
    if (IsKeyPressed(KEY_FOUR))
    {
        switchToScene(gLevels[LEVEL_D_SCENE]);
        return;
    }
    if (IsKeyPressed(KEY_FIVE))
    {
        switchToScene(gLevels[LEVEL_C_TO_D_SCENE]);
        return;
    }

    if (gCurrentScene->getState().xochitl == nullptr)
    {
        if (IsKeyPressed(KEY_Q) || WindowShouldClose()) gAppStatus = TERMINATED;
        return;
    }

    gCurrentScene->getState().xochitl->resetMovement();

    if      (IsKeyDown(KEY_A))  gCurrentScene->getState().xochitl->moveLeft();
    else if ((IsKeyDown(KEY_D))) gCurrentScene->getState().xochitl->moveRight();

    if      (IsKeyDown(KEY_W))  gCurrentScene->getState().xochitl->moveUp();
    else if (IsKeyDown(KEY_S))  gCurrentScene->getState().xochitl->moveDown();

    if (GetLength(gCurrentScene->getState().xochitl->getMovement()) > 1.0f) 
        gCurrentScene->getState().xochitl->normaliseMovement();

    if (IsKeyPressed(KEY_Q) || WindowShouldClose()) gAppStatus = TERMINATED;
}

void update() 
{
    float ticks = (float) GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks  = ticks;

    deltaTime += gTimeAccumulator;

    if (deltaTime < FIXED_TIMESTEP)
    {
        gTimeAccumulator = deltaTime;
        return;
    }

    while (deltaTime >= FIXED_TIMESTEP)
    {
        gCurrentScene->update(FIXED_TIMESTEP);
        deltaTime -= FIXED_TIMESTEP;

        if (gLives < gPreviousLives) startLifeLossFlash();
        gPreviousLives = gLives;

        gLifeLossFlashTimer -= FIXED_TIMESTEP;
        if (gLifeLossFlashTimer < 0.0f) gLifeLossFlashTimer = 0.0f;

        applyProtagonistDamageShader();

        if (gCurrentScene->getState().xochitl != nullptr)
        {
            Vector2 currentPlayerPosition = {
                gCurrentScene->getState().xochitl->getPosition().x,
                ORIGIN.y
            };

            panCamera(&gCamera, &currentPlayerPosition);
        }

        if (gEffects != nullptr)
            gEffects->update(FIXED_TIMESTEP, &gCamera.target);
    }
}

void render()
{
    BeginDrawing();
    BeginMode2D(gCamera);

    gCurrentScene->render();
    if (gEffects != nullptr) gEffects->render();

    EndMode2D();
    EndDrawing();
}

void shutdown() 
{
    delete gMainMenu;
    delete gOpeningScene;
    delete gLevelA;
    delete gLevelB;
    delete gLevelC;
    delete gLevelD;
    delete gLevelCToDScene;
    delete gLoseScene;
    delete gWinScene;

    gMainMenu = nullptr;
    gOpeningScene = nullptr;
    gLevelA = nullptr;
    gLevelB = nullptr;
    gLevelC = nullptr;
    gLevelD = nullptr;
    gLevelCToDScene = nullptr;
    gLoseScene = nullptr;
    gWinScene = nullptr;

    for (int i = 0; i < NUMBER_OF_LEVELS; i++) gLevels[i] = nullptr;

    gLifeLossShader.unload();

    delete gEffects;
    gEffects = nullptr;

    CloseAudioDevice();
    CloseWindow();
}

int main(void)
{
    initialise();

    while (gAppStatus == RUNNING)
    {
        processInput();
        update();

        if (gCurrentScene->getState().nextSceneID != NO_SCENE)
        {
            int id = gCurrentScene->getState().nextSceneID;
            switchToScene(gLevels[id]);
        }

        render();
    }

    shutdown();

    return 0;
}