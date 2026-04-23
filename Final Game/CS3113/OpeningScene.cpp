#include "OpeningScene.h"

OpeningScene::OpeningScene() : Scene{{0.0f}, "#060606"} {}
OpeningScene::OpeningScene(Vector2 origin, const char *bgHexCode) : Scene{origin, bgHexCode} {}

OpeningScene::~OpeningScene() { shutdown(); }

void OpeningScene::initialise()
{
    mGameState.xochitl = nullptr;
    mGameState.map = nullptr;
    mGameState.nextSceneID = NO_SCENE;
    mSceneTimer = 0.0f;

    if (mHasLoadedIntroMusic)
    {
        StopMusicStream(mGameState.bgm);
        UnloadMusicStream(mGameState.bgm);
        mHasLoadedIntroMusic = false;
    }

    mGameState.bgm = LoadMusicStream("assets/game/intro_music.mp3");
    SetMusicVolume(mGameState.bgm, 0.33f);
    PlayMusicStream(mGameState.bgm);
    mHasLoadedIntroMusic = true;
}

void OpeningScene::update(float deltaTime)
{
    if (mHasLoadedIntroMusic)
        UpdateMusicStream(mGameState.bgm);

    mSceneTimer += deltaTime;

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || mSceneTimer >= AUTO_CONTINUE_TIME)
        mGameState.nextSceneID = LEVEL_B_SCENE;
}

void OpeningScene::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));

    if (mSceneTimer >= 0.5f)
        DrawText("Long ago, the kingdom knew peace... until silence fell over the royal halls.",
                 mOrigin.x - 500, mOrigin.y - 190, 24, LIGHTGRAY);

    if (mSceneTimer >= 2.2f)
        DrawText("The princess has been taken.",
                 mOrigin.x - 170, mOrigin.y - 150, 34, GOLD);

    if (mSceneTimer >= 4.1f)
        DrawText("Not by war... not by chance...", mOrigin.x - 250, mOrigin.y - 80, 24, WHITE);

    if (mSceneTimer >= 5.7f)
        DrawText("but by something that dwells beneath the world of men.",
                 mOrigin.x - 330, mOrigin.y - 45, 24, WHITE);

    if (mSceneTimer >= 7.7f)
        DrawText("No knight returned to tell the tale.",
                 mOrigin.x - 220, mOrigin.y + 15, 24, LIGHTGRAY);

    if (mSceneTimer >= 9.6f)
        DrawText("But you! The brave knight can do it.",
                 mOrigin.x - 230, mOrigin.y + 75, 28, RED);

    if (mSceneTimer >= 11.5f)
        DrawText("Bravery has a price.", mOrigin.x - 155, mOrigin.y + 125, 26, WHITE);

    if (mSceneTimer >= 13.4f)
        DrawText("And in this dungeon... it is always paid in full.",
                 mOrigin.x - 290, mOrigin.y + 160, 26, ORANGE);

    if (mSceneTimer >= 15.5f)
        DrawText("Press ENTER to descend", mOrigin.x - 160, mOrigin.y + 235, 28, GREEN);
}

void OpeningScene::shutdown()
{
    if (!mHasLoadedIntroMusic) return;

    StopMusicStream(mGameState.bgm);
    UnloadMusicStream(mGameState.bgm);
    mHasLoadedIntroMusic = false;
}
