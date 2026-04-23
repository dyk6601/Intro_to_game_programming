#ifndef LEVELC_TO_D_SCENE_H
#define LEVELC_TO_D_SCENE_H

#include "Scene.h"

class LevelCToDScene : public Scene
{
private:
    float mSceneTimer = 0.0f;

public:
    static constexpr float AUTO_CONTINUE_TIME = 23.0f;

    LevelCToDScene();
    LevelCToDScene(Vector2 origin, const char *bgHexCode);
    ~LevelCToDScene();

    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
};

#endif
