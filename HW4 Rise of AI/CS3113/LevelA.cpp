#include "LevelA.h"

LevelA::LevelA()                                      : Scene { {0.0f}, nullptr   } {}
LevelA::LevelA(Vector2 origin, const char *bgHexCode) : Scene { origin, bgHexCode } {}

LevelA::~LevelA() { shutdown(); }

void LevelA::initialise()
{
   mGameState.nextSceneID = NO_SCENE;
   mIsLevelTransitioning = false;
   mLevelTransitionTimer = 0.0f;

   mGameState.bgm = LoadMusicStream("assets/game/time_for_adventure.mp3");
   SetMusicVolume(mGameState.bgm, 0.33f);
   PlayMusicStream(mGameState.bgm);
   mGameState.jumpSound = LoadSound("assets/game/Jump.wav");
   mLevelClearSound = LoadSound("assets/game/level_up.mp3");

   /*
      ----------- MAP -----------
   */
   mGameState.map = new Map(
      LEVEL_WIDTH, LEVEL_HEIGHT,   // map grid cols & rows
      (unsigned int *) mLevelData, // grid data
      "assets/game/oak_woods_tileset.png", // texture filepath
      TILE_DIMENSION,              // tile size
      21, 15,                      // texture cols & rows (24x24 tiles)
      mOrigin                      // in-game origin
   );

   /*
      ----------- PROTAGONIST -----------
   */
   std::map<Direction, std::vector<int>> xochitlAnimationAtlas = {
      {DOWN,  { 0, 4, 8 }},
      {LEFT,  { 1, 5, 9 }},
      {RIGHT, { 2, 6, 10 }},
      {UP,    { 3, 7, 11 }},
   };

   float sizeRatio  = 45.0f / 116.0f;
   float widthBoost = 1.15f;

   mGameState.xochitl = new Entity(
      {mOrigin.x - 600.0f, mOrigin.y - 200.0f}, // position
      {140.0f * sizeRatio * widthBoost, 140.0f},// scale
      "assets/game/walking.png",                // texture file address
      ATLAS,                                    // single image or atlas?
      { 3, 4 },                                 // atlas dimensions
      xochitlAnimationAtlas,                    // actual atlas
      PLAYER                                    // entity type
   );

   mGameState.xochitl->setJumpingPower(550.0f);
   mGameState.xochitl->setColliderDimensions({
      mGameState.xochitl->getScale().x / 3.0f,
      mGameState.xochitl->getScale().y / 2.0f
   });
   mGameState.xochitl->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});

   //      ----------- rockhead -----------
 
   mRockhead = new Entity(
      {mOrigin.x + 260.0f, mOrigin.y - 50.0f}, // position
      {95.0f, 95.0f},                          // scale
      "assets/game/rockhead.png",             // texture
      NPC                                      // entity type
   );

   mRockhead->setColliderDimensions({
      mRockhead->getScale().x * 0.60f,
      mRockhead->getScale().y * 0.70f
   });
   mRockhead->setAcceleration({0.0f, 0.0f});
   mRockhead->setSpeed(300);
   mRockhead->setAIType(FLYER);
   mRockhead->setAIState(WALKING);
}

void LevelA::update(float deltaTime)
{
   UpdateMusicStream(mGameState.bgm);

   if (mIsLevelTransitioning)
   {
      mLevelTransitionTimer -= deltaTime;
      if (mLevelTransitionTimer <= 0.0f) mGameState.nextSceneID = 2;
      return;
   }

   mGameState.xochitl->update(
      deltaTime,      // delta time / fixed timestep
      nullptr,        // player
      mGameState.map, // map
      nullptr,        // collidable entities
      0               // col. entity count
   );

   if (mRockhead)
   {
      mRockhead->update(
         deltaTime,
         mGameState.xochitl,
         mGameState.map,
         nullptr,
         0
      );

      // Flyer bounce: reverse direction when tile collision happens.
      if (mRockhead->isCollidingLeft())      mRockhead->setDirection(RIGHT);
      else if (mRockhead->isCollidingRight()) mRockhead->setDirection(LEFT);

      // Touching rockhead costs 1 life.
      if (mGameState.sharedLives && mGameState.xochitl->hasCollidedWith(mRockhead))
      {
         (*mGameState.sharedLives)--;

         if (*mGameState.sharedLives <= 0) mGameState.nextSceneID = LOSE_SCENE;
         else mGameState.nextSceneID = LEVEL_A_SCENE;

         return;
      }
   }

   // death trigger
   if (mGameState.xochitl->getPosition().y > 800.0f && mGameState.sharedLives)
   {
      (*mGameState.sharedLives)--;

      if (*mGameState.sharedLives <= 0) mGameState.nextSceneID = LOSE_SCENE;
      else mGameState.nextSceneID = 1; // reload LevelA
   }

   Vector2 feetPosition = {
      mGameState.xochitl->getPosition().x,
      mGameState.xochitl->getPosition().y + (mGameState.xochitl->getScale().y / 2.0f) - 20.0f
   };

   int tileX = floor((feetPosition.x - mGameState.map->getLeftBoundary()) / mGameState.map->getTileSize());
   int tileY = floor((feetPosition.y - mGameState.map->getTopBoundary()) / mGameState.map->getTileSize());

   if (tileX >= 0 && tileX < mGameState.map->getMapColumns() &&
       tileY >= 0 && tileY < mGameState.map->getMapRows())
   {
      int currentTile = mGameState.map->getLevelData()[tileY * mGameState.map->getMapColumns() + tileX];
      if (currentTile == 7)
      {
         PlaySound(mLevelClearSound);
         mIsLevelTransitioning = true;
         mLevelTransitionTimer = 0.9f;
      }
   }
   
}

void LevelA::render()
{
   ClearBackground(ColorFromHex(mBGColourHexCode));

   mGameState.map->render();
   if (mRockhead) mRockhead->render();
   mGameState.xochitl->render();

   if (mGameState.sharedLives){
      float uiOffsetX = -520.0f;
      DrawText(TextFormat("Lives: %d", *mGameState.sharedLives), (int)(mGameState.xochitl->getPosition().x + uiOffsetX), 550, 30, WHITE);
   }
   float uiOffsetX = -150.0f;
   DrawText(TextFormat("Levels Press: 1,2,3"), (int)(mGameState.xochitl->getPosition().x + uiOffsetX), 550, 30, GREEN);
   
}

void LevelA::shutdown()
{
   delete mRockhead;
   mRockhead = nullptr;

   delete mGameState.xochitl;
   delete mGameState.map;

   UnloadMusicStream(mGameState.bgm);
   UnloadSound(mGameState.jumpSound);
   UnloadSound(mLevelClearSound);
}