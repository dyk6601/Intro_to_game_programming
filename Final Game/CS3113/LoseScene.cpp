#include "LoseScene.h"

LoseScene::LoseScene() : Scene{{0.0f}, "#1F0000"} {}
LoseScene::LoseScene(Vector2 origin, const char *bgHexCode) : Scene{origin, bgHexCode} {}

LoseScene::~LoseScene() { shutdown(); }

void LoseScene::initialise()
{
    mGameState.nextSceneID = NO_SCENE;
}

void LoseScene::update(float deltaTime)
{
    (void) deltaTime;

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))
    {
        if (mGameState.sharedLives != nullptr) *mGameState.sharedLives = 3; // reset game lives
        mGameState.nextSceneID = MAIN_MENU_SCENE;                           
    }
}

void LoseScene::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));

    DrawText("YOU LOSE", mOrigin.x - 150, mOrigin.y - 80, 70, RED);
    DrawText("Press ENTER for Main Menu", mOrigin.x - 230, mOrigin.y + 10, 35, WHITE);
}

void LoseScene::shutdown()
{
}
