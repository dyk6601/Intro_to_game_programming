#ifndef OPENING_SCENE_H
#define OPENING_SCENE_H

#include "Scene.h"

class OpeningScene : public Scene
{
private:
    float mSceneTimer = 0.0f;
    bool mHasLoadedIntroMusic = false;

public:
    static constexpr float AUTO_CONTINUE_TIME = 29.0f;

    OpeningScene();
    OpeningScene(Vector2 origin, const char *bgHexCode);
    ~OpeningScene();

    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
};

#endif
