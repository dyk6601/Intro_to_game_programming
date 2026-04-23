#include "LevelCToDScene.h"

LevelCToDScene::LevelCToDScene() : Scene{{0.0f}, "#05010A"} {}
LevelCToDScene::LevelCToDScene(Vector2 origin, const char *bgHexCode) : Scene{origin, bgHexCode} {}

LevelCToDScene::~LevelCToDScene() { shutdown(); }

void LevelCToDScene::initialise()
{
    mGameState.xochitl = nullptr;
    mGameState.map = nullptr;
    mGameState.nextSceneID = NO_SCENE;
    mSceneTimer = 0.0f;

    mGameState.bgm = LoadMusicStream("assets/game/levelc_music.mp3");
    SetMusicVolume(mGameState.bgm, 0.53f);
    PlayMusicStream(mGameState.bgm);
}

void LevelCToDScene::update(float deltaTime)
{
    UpdateMusicStream(mGameState.bgm);
    mSceneTimer += deltaTime;

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || mSceneTimer >= AUTO_CONTINUE_TIME)
        mGameState.nextSceneID = LEVEL_D_SCENE;
}

void LevelCToDScene::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));

    if (mSceneTimer >= 0.5f)
        DrawText("\"Princess...?\"", mOrigin.x - 120, mOrigin.y - 190, 40, WHITE);

    if (mSceneTimer >= 3.3f)
        DrawText("she laughs. \"Did you really think...\"", mOrigin.x - 285, mOrigin.y - 120, 30, LIGHTGRAY);

    if (mSceneTimer >= 5.5f)
        DrawText("\"...I needed saving?\"", mOrigin.x - 210, mOrigin.y - 70, 34, PURPLE);

    if (mSceneTimer >= 8.6f)
        DrawText("Her face--still smiling--splits in half.", mOrigin.x - 300, mOrigin.y + 5, 30, RED);

    if (mSceneTimer >= 10.8f)
        DrawText("Then--.", mOrigin.x - 180, mOrigin.y + 50, 32, WHITE);

    if (mSceneTimer >= 11.8f)
        DrawText("dark serpents rose from the ground", mOrigin.x - 200, mOrigin.y + 108, 38, LIGHTGRAY);

    if (mSceneTimer >= 14.9f)
        DrawText("and started to coil around her.", mOrigin.x - 330, mOrigin.y + 170, 34, PURPLE);

    if (mSceneTimer >= 16.7f)
        DrawText("Press ENTER to continue", mOrigin.x - 165, mOrigin.y + 245, 28, GREEN);
}

void LevelCToDScene::shutdown()
{
    UnloadMusicStream(mGameState.bgm);
}
