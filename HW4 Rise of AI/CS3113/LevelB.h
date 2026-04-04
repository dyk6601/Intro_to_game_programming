#include "Scene.h"

#ifndef LEVELB_H
#define LEVELB_H

class LevelB : public Scene {
private:
    Entity *mFrog = nullptr;

    bool mIsLevelTransitioning = false;
    float mLevelTransitionTimer = 0.0f;
    Sound mLevelClearSound = {};
    Sound mPowerUpSound = {};

    static constexpr int LEVEL_B_WIDTH = 14,
                         LEVEL_B_HEIGHT = 8;

    unsigned int mLevelData[LEVEL_B_WIDTH * LEVEL_B_HEIGHT] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 6, 8, 9, 0, 0, 0, 0, 0, 1,
        0, 6, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 9, 0, 0, 0,
        0, 0, 0, 6, 7, 7, 7, 7, 7, 9, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

public:
    static constexpr float TILE_DIMENSION       = 75.0f,
                        ACCELERATION_OF_GRAVITY = 981.0f,
                        END_GAME_THRESHOLD      = 800.0f;

    LevelB();
    LevelB(Vector2 origin, const char *bgHexCode);
    ~LevelB();
    
    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
};

#endif