#include "Scene.h"

#ifndef LEVELA_H
#define LEVELA_H

constexpr int LEVEL_WIDTH = 21,
              LEVEL_HEIGHT = 8;


class LevelA : public Scene {
private:
    Entity *mRockhead = nullptr;

    bool mIsLevelTransitioning = false;
    float mLevelTransitionTimer = 0.0f;
    Sound mLevelClearSound = {};

    unsigned int mLevelData[LEVEL_WIDTH * LEVEL_HEIGHT] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0,0,
        0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0,0,
        0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 2, 0, 0, 0,0,0,0,0,0,0,0,
        0, 0, 1, 4, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0,0,0,0,0,0,7,0,
        0, 0, 0, 0, 0, 1, 2, 2, 4, 0, 0, 0, 1, 2,2,2,2,2,4,0,0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0,0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0,0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0,0
    };

public:
    static constexpr float TILE_DIMENSION       = 75.0f,
                        ACCELERATION_OF_GRAVITY = 981.0f,
                        END_GAME_THRESHOLD      = 800.0f;

    LevelA();
    LevelA(Vector2 origin, const char *bgHexCode);
    ~LevelA();
    
    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
};

#endif
