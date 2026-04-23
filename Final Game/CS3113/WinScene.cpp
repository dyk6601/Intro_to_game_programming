#include "WinScene.h"

WinScene::WinScene() : Scene{{0.0f}, "#053B06"} {}
WinScene::WinScene(Vector2 origin, const char *bgHexCode) : Scene{origin, bgHexCode} {}

WinScene::~WinScene() { shutdown(); }

void WinScene::initialise()
{
    mGameState.nextSceneID = NO_SCENE;
}

void WinScene::update(float deltaTime)
{
    (void) deltaTime;

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))
    {
        if (mGameState.sharedLives != nullptr) *mGameState.sharedLives = 3;
        mGameState.nextSceneID = MAIN_MENU_SCENE;
    }
}

void WinScene::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));

    DrawText("YOU WIN", mOrigin.x - 130, mOrigin.y - 80, 70, GREEN);
    DrawText("Press ENTER for Main Menu", mOrigin.x - 230, mOrigin.y + 10, 35, WHITE);
}

void WinScene::shutdown()
{
}
