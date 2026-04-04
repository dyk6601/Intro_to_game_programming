#include "LevelB.h"

LevelB::LevelB()                                      : Scene { {0.0f}, nullptr   } {}
LevelB::LevelB(Vector2 origin, const char *bgHexCode) : Scene { origin, bgHexCode } {}

LevelB::~LevelB() { shutdown(); }

void LevelB::initialise()
{
   mGameState.nextSceneID = NO_SCENE;
   mIsLevelTransitioning = false;
   mLevelTransitionTimer = 0.0f;

   mGameState.bgm = LoadMusicStream("assets/game/time_for_adventure.mp3");
   SetMusicVolume(mGameState.bgm, 0.33f);
   PlayMusicStream(mGameState.bgm);
   mGameState.jumpSound = LoadSound("assets/game/Jump.wav");
   mLevelClearSound = LoadSound("assets/game/level_up.mp3");
   mPowerUpSound = LoadSound("assets/game/power_up.mp3");
   

   /*
      ----------- MAP -----------
   */
   mGameState.map = new Map(
      LEVEL_B_WIDTH, LEVEL_B_HEIGHT, // map grid cols & rows
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
      {mOrigin.x - 400.0f, mOrigin.y - 200.0f}, // position
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

   // ----------- FROG  -----------
   std::map<Direction, std::vector<int>> frogAnimationAtlas = {
      {DOWN,  {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10}},
      {LEFT,  {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10}},
      {RIGHT, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10}},
      {UP,    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10}},
   };

   mFrog = new Entity(
      {mOrigin.x - 100.0f, mOrigin.y - 460.0f}, // position
      {95.0f, 95.0f},                            // scale
      "assets/game/frog.png",                  // texture
      ATLAS,                                    // animated atlas
      {1, 11},                                  // 1 row, 11 frames
      frogAnimationAtlas,
      NPC                                       // entity type
   );

   mFrog->setColliderDimensions({
      mFrog->getScale().x * 0.60f,
      mFrog->getScale().y * 0.70f
   });
   mFrog->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});
   mFrog->setSpeed(100);
   mFrog->setFrameSpeed(12);
   mFrog->setAIType(WANDERER);
   mFrog->setAIState(WALKING);
}

void LevelB::update(float deltaTime)
{
   UpdateMusicStream(mGameState.bgm);

   if (mIsLevelTransitioning)
   {
      mLevelTransitionTimer -= deltaTime;
      if (mLevelTransitionTimer <= 0.0f) mGameState.nextSceneID = LEVEL_C_SCENE;
      return;
   }

   mGameState.xochitl->update(
      deltaTime,      // delta time / fixed timestep
      nullptr,        // player
      mGameState.map, // map
      nullptr,        // collidable entities
      0               // col. entity count
   );

   if (mFrog)
   {
      mFrog->update(
         deltaTime,
         mGameState.xochitl,
         mGameState.map,
         nullptr,
         0
      );

      if (mFrog->isCollidingLeft())       mFrog->setDirection(RIGHT);
      else if (mFrog->isCollidingRight()) mFrog->setDirection(LEFT);

      // Check Collision
      if (mGameState.sharedLives && mGameState.xochitl->hasCollidedWith(mFrog))
      {
         const float playerBottom = mGameState.xochitl->getPosition().y + (mGameState.xochitl->getScale().y / 2.0f);
         const float frogTop = mFrog->getPosition().y - (mFrog->getScale().y / 2.0f);
         const float frogUpperSectionBottom = frogTop + (mFrog->getScale().y * 0.40f);

         // Top-hit logic: if player touches upper section of frog, remove frog.
         if (playerBottom <= frogUpperSectionBottom)
         {
            PlaySound(mPowerUpSound);
            mFrog->deactivate();
            mGameState.xochitl->setJumpingPower(850.0f);
            return;
         }

         (*mGameState.sharedLives)--;

         if (*mGameState.sharedLives <= 0) mGameState.nextSceneID = LOSE_SCENE;
         else mGameState.nextSceneID = LEVEL_B_SCENE;

         return;
      }
   }

   // death trigger
   if (mGameState.xochitl->getPosition().y > END_GAME_THRESHOLD && mGameState.sharedLives)
   {
      (*mGameState.sharedLives)--;

      if (*mGameState.sharedLives <= 0) mGameState.nextSceneID = LOSE_SCENE;
      else mGameState.nextSceneID = LEVEL_B_SCENE;
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
      if (currentTile == 1)
      {
         PlaySound(mLevelClearSound);
         mIsLevelTransitioning = true;
         mLevelTransitionTimer = 0.9f;
      }
   }

}

void LevelB::render()
{
   ClearBackground(ColorFromHex(mBGColourHexCode));

   mGameState.map->render();
   if (mFrog) mFrog->render();
   mGameState.xochitl->render();

   if (mGameState.sharedLives)
   {
      float uiOffsetX = -520.0f;
      DrawText(TextFormat("Lives: %d", *mGameState.sharedLives), (int)(mGameState.xochitl->getPosition().x + uiOffsetX), 550, 30, WHITE);
   }
   float uiOffsetX = -520.0f;
   DrawText(TextFormat("Levels Press: 1,2,3"), (int)(mGameState.xochitl->getPosition().x + uiOffsetX), 0, 30, GREEN);

   DrawText(TextFormat("Step on Frog for special powers?!"), (int)(mGameState.xochitl->getPosition().x), 550, 30, RED);
}

void LevelB::shutdown()
{
   delete mFrog;
   mFrog = nullptr;

   delete mGameState.xochitl;
   delete mGameState.map;

   UnloadMusicStream(mGameState.bgm);
   UnloadSound(mGameState.jumpSound);
   UnloadSound(mLevelClearSound);
   UnloadSound(mPowerUpSound);
}