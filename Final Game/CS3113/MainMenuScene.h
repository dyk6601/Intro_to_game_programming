#ifndef MAIN_MENU_SCENE_H
#define MAIN_MENU_SCENE_H

#include "Scene.h"

class MainMenuScene : public Scene
{
public:
    MainMenuScene();
    MainMenuScene(Vector2 origin, const char *bgHexCode);
    ~MainMenuScene();

    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
};

#endif