#include "MainMenuScene.h"

MainMenuScene::MainMenuScene() : Scene{{0.0f}, "#1B1B2F"} {}
MainMenuScene::MainMenuScene(Vector2 origin, const char *bgHexCode) : Scene{origin, bgHexCode} {}

MainMenuScene::~MainMenuScene() { shutdown(); }

void MainMenuScene::initialise()
{
    mGameState.xochitl = nullptr;
    mGameState.map = nullptr;
    mGameState.nextSceneID = NO_SCENE;
}

void MainMenuScene::update(float deltaTime)
{
    (void)deltaTime;
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))
    {
        mGameState.nextSceneID = LEVEL_A_SCENE;
    }
}

void MainMenuScene::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));

    DrawText("SOOZ'S PLATFORMER", mOrigin.x - 300, mOrigin.y - 90, 60, GREEN);
    DrawText("Press ENTER to start", mOrigin.x - 190, mOrigin.y, 35, WHITE);
}

void MainMenuScene::shutdown()
{
}
