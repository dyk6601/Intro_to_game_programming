#include "Entity.h"

#ifndef SCENE_H
#define SCENE_H

struct GameState
{
    Entity *xochitl;
    Map *map;

    Music bgm;
    Sound jumpSound;

    int nextSceneID = -1;
    int *sharedLives = nullptr;
};

enum SceneID
{
    NO_SCENE        = -1,
    MAIN_MENU_SCENE = 0,
    LEVEL_A_SCENE   = 1,
    LEVEL_B_SCENE   = 2,
    LEVEL_C_SCENE   = 3,
    LOSE_SCENE      = 4,
    WIN_SCENE       = 5
};

class Scene 
{
protected:
    GameState mGameState;
    Vector2 mOrigin;
    const char *mBGColourHexCode = "#000000";
    
public:
    Scene();
    Scene(Vector2 origin, const char *bgHexCode);
    

    virtual void initialise() = 0;
    virtual void update(float deltaTime) = 0;
    virtual void render() = 0;
    virtual void shutdown() = 0;
    virtual void setSharedLives(int *lives) { mGameState.sharedLives = lives; }
    
    GameState   getState()           const { return mGameState; }
    Vector2     getOrigin()          const { return mOrigin;    }
    const char* getBGColourHexCode() const { return mBGColourHexCode; }
};

#endif