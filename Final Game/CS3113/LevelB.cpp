#include "LevelB.h"

namespace {
Vector2 getAimFromDirection(Direction direction) {
   switch (direction) {
   case LEFT:
      return {-1.0f, 0.0f};
   case UP:
      return {0.0f, -1.0f};
   case RIGHT:
      return {1.0f, 0.0f};
   case DOWN:
      return {0.0f, 1.0f};
   default:
      return {1.0f, 0.0f};
   }
}
}

LevelB::LevelB() : Scene{{0.0f}, nullptr} {}
LevelB::LevelB(Vector2 origin, const char *bgHexCode)
      : Scene{origin, bgHexCode} {}

LevelB::~LevelB() { shutdown(); }

void LevelB::initialise() {
   mGameState.nextSceneID = NO_SCENE;
   mHasGun = false;
   mIsShooting = false;
   mIsDashing = false;
   mIsReloading = false;
   mBulletsRemaining = MAX_BULLETS;
   mReloadTimer = 0.0f;
   mBulletFireCooldown = 0.0f;
   mIsLifeLossTransitioning = false;
   mLifeLossTransitionTimer = 0.0f;
   mIsPlayerDying = false;
   mPlayerDeathAnimTimer = 0.0f;
   mPendingLifeLossSceneID = NO_SCENE;
   mTutorialStep = 0;

   mGameState.bgm = LoadMusicStream("assets/game/time_for_adventure.mp3");
   SetMusicVolume(mGameState.bgm, 0.33f);
   PlayMusicStream(mGameState.bgm);
   mLevelClearSound = LoadSound("assets/game/level_up.mp3");
   mBullets.clear();
   mBulletLifetimes.clear();
   mBullets.resize(MAX_BULLETS, nullptr);
   mBulletLifetimes.resize(MAX_BULLETS, 0.0f);

   for (int i = 0; i < MAX_BULLETS; i++) {
      mBullets[i] =
            new Entity({0.0f, 0.0f}, {BULLET_SCALE, BULLET_SCALE},
                            "assets/game/bullet.png", NONE);

      mBullets[i]->setAcceleration({0.0f, 0.0f});
      mBullets[i]->setSpeed((int)BULLET_SPEED);
      mBullets[i]->setColliderDimensions(
            {BULLET_SCALE * 0.65f, BULLET_SCALE * 0.65f});
      mBullets[i]->deactivate();
   }

   /*
       ----------- MAP -----------
   */
   mGameState.map = new Map(
         LEVEL_B_WIDTH, LEVEL_B_HEIGHT, // map grid cols & rows
         (unsigned int *)mLevelData,    // grid data
         "assets/game/kenney_tinyDungeon/Tilemap/tilemap_packed.png", // texture
                                                                                                    // filepath
         TILE_DIMENSION,                                              // tile size
         12, 11,                                 // texture cols & rows
         mOrigin,                                // in-game origin
         {2, 3, 4, 5, 6, 14, 16, 17, 18, 27, 90} // solid tile IDs (walls)
   );

   /*
       ----------- PROTAGONIST -----------
   */
   std::map<Direction, std::vector<int>> xochitlAnimationAtlas = {
         {DOWN, {0, 1, 2, 3, 4, 5, 6, 7}},
         {LEFT, {8, 9, 10, 11, 12, 13, 14, 15}},
         {UP, {24, 25, 26, 27, 28, 29, 30, 31}},
         {RIGHT, {40, 41, 42, 43, 44, 45, 46, 47}},
   };

   mNormalWalkAnimationAtlas = xochitlAnimationAtlas;

   mDashAnimationAtlas = {
         {DOWN, {0, 1, 2, 3, 4, 5, 6, 7}},
         {LEFT, {8, 9, 10, 11, 12, 13, 14, 15}},
         {UP, {24, 25, 26, 27, 28, 29, 30, 31}},
         {RIGHT, {40, 41, 42, 43, 44, 45, 46, 47}},
   };

   mGunDashAnimationAtlas = {
         {DOWN, {0, 1, 2, 3, 4, 5, 6, 7}},
         {LEFT, {8, 9, 10, 11, 12, 13, 14, 15}},
         {UP, {24, 25, 26, 27, 28, 29, 30, 31}},
         {RIGHT, {40, 41, 42, 43, 44, 45, 46, 47}},
   };

   mWalkGunAnimationAtlas = {
         {DOWN, {0, 1, 2, 3, 4, 5, 6, 7}},
         {LEFT, {8, 9, 10, 11, 12, 13, 14, 15}},
         {UP, {24, 25, 26, 27, 28, 29, 30, 31}},
         {RIGHT, {40, 41, 42, 43, 44, 45, 46, 47}},
   };

   mGunAnimationAtlas = {
         // 8x8 order: down, leftdown, leftup, up, rightup, rightdown, right, left
         {DOWN, {0, 1, 2, 3, 4, 5, 6, 7}},
         {UP, {24, 25, 26, 27, 28, 29, 30, 31}},
         {RIGHT, {48, 49, 50, 51, 52, 53, 54, 55}},
         {LEFT, {56, 57, 58, 59, 60, 61, 62, 63}},
   };

         mGunDeathAnimationAtlas = {
            {DOWN, {0, 1, 2, 3, 4, 5, 6, 7}},
            {LEFT, {8, 9, 10, 11, 12, 13, 14, 15}},
            {UP, {24, 25, 26, 27, 28, 29, 30, 31}},
            {RIGHT, {40, 41, 42, 43, 44, 45, 46, 47}},
         };

   float sizeRatio = 48.0f / 64.0f;

   mGameState.xochitl =
         new Entity({mOrigin.x - 400.0f, mOrigin.y - 100.0f}, // position
                         {250.0f * sizeRatio, 250.0f},             // scale
                         "assets/game/walk.png", // texture file address
                         ATLAS,                  // single image or atlas?
                         {6, 8},                 // atlas dimensions
                         xochitlAnimationAtlas,  // actual atlas
                         PLAYER                  // entity type
         );

   mGameState.xochitl->setColliderDimensions(
         {mGameState.xochitl->getScale().x / 3.0f,
          mGameState.xochitl->getScale().y / 2.0f});
   mGameState.xochitl->setSpeed(PLAYER_NORMAL_SPEED);
   mGameState.xochitl->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});

      std::map<Direction, std::vector<int>> princessIdleAnimationAtlas = {
         {DOWN, {0, 1}},
         {LEFT, {0, 1}},
         {UP, {0, 1}},
         {RIGHT, {0, 1}},
      };

      if (mPrincess == nullptr) {
      mPrincess = new Entity(
         {mOrigin.x + 850.0f, mOrigin.y - 150.0f},
         {145.0f, 145.0f},
         "assets/game/princess.png",
         ATLAS,
         {1, 2},
         princessIdleAnimationAtlas,
         PLAYER
      );

      mPrincess->setAcceleration({0.0f, 0.0f});
      mPrincess->setMovement({0.0f, 0.0f});
      mPrincess->setSpeed(0);
      mPrincess->setFrameSpeed(2);
      mPrincess->setAnimateWhenStill(true);
      mPrincess->setDirection(DOWN);
      }

      if (mSkeletonEnemy == nullptr) mSkeletonEnemy = new SkeletonEnemy();
      mSkeletonEnemy->initialise(
         mOrigin,
         {mOrigin.x + 150.0f, mOrigin.y + 400.0f}
      );
}

void LevelB::update(float deltaTime) {
   UpdateMusicStream(mGameState.bgm);
   mBulletFireCooldown -= deltaTime;

   if (mIsPlayerDying && mGameState.xochitl) {
      mGameState.xochitl->resetMovement();
      mGameState.xochitl->setSpeed(0);
      mGameState.xochitl->update(deltaTime, nullptr, mGameState.map, nullptr, 0);

      mPlayerDeathAnimTimer -= deltaTime;
      if (mPlayerDeathAnimTimer <= 0.0f)
         mGameState.nextSceneID = LOSE_SCENE;

      return;
   }

   if (mTutorialStep == 0 && IsKeyPressed(KEY_W)) mTutorialStep = 1;
   if (mTutorialStep == 1 && IsKeyPressed(KEY_SPACE)) mTutorialStep = 2;
   if (mTutorialStep == 2 && mHasGun) mTutorialStep = 3;
   if (mTutorialStep == 3 && IsKeyDown(KEY_LEFT_SHIFT)) mTutorialStep = 4;

   ///
   if (!mHasGun && IsKeyPressed(KEY_F) && mGameState.xochitl) {
      mHasGun = true;
      mIsDashing = false;
      mIsReloading = false;
      mBulletsRemaining = MAX_BULLETS;
      mGameState.xochitl->setSpeed(PLAYER_NORMAL_SPEED);
      mGameState.xochitl->setTexture(
            "assets/game/adventurer/Walk/Gun/Walk_Gun.png");
      mGameState.xochitl->setSpriteSheetDimensions({6, 8});
      mGameState.xochitl->setAnimationAtlas(mWalkGunAnimationAtlas);
   }

   if (!mHasGun && mGameState.xochitl) {
      bool wantsDash = IsKeyDown(KEY_SPACE);
      std::map<Direction, std::vector<int>> currentDashAnimationAtlas =
            mDashAnimationAtlas;

      if (wantsDash && !mIsDashing) {
         mIsDashing = true;
         mGameState.xochitl->setSpeed(PLAYER_DASH_SPEED);
         mGameState.xochitl->setTexture(
               "assets/game/adventurer/Dash/Normal/Dash.png");
         mGameState.xochitl->setSpriteSheetDimensions({6, 8});
         mGameState.xochitl->setAnimationAtlas(currentDashAnimationAtlas);
      } else if (wantsDash) {
         mGameState.xochitl->setAnimationAtlas(currentDashAnimationAtlas);
      } else if (!wantsDash && mIsDashing) {
         mIsDashing = false;
         mGameState.xochitl->setSpeed(PLAYER_NORMAL_SPEED);
         mGameState.xochitl->setTexture("assets/game/walk.png");
         mGameState.xochitl->setSpriteSheetDimensions({6, 8});
         mGameState.xochitl->setAnimationAtlas(mNormalWalkAnimationAtlas);
      }
   }

   if (mHasGun && mGameState.xochitl) {
      bool wantsDash = IsKeyDown(KEY_SPACE);
      bool wantsShooting = IsKeyDown(KEY_LEFT_SHIFT);

      if (mIsReloading) {
         wantsDash = false;
         wantsShooting = false;
         mGameState.xochitl->resetMovement();
         mGameState.xochitl->setAnimateWhenStill(true);

         mReloadTimer -= deltaTime;
         if (mReloadTimer <= 0.0f) {
            mIsReloading = false;
            mBulletsRemaining = MAX_BULLETS;
            mGameState.xochitl->setTexture(
                  "assets/game/adventurer/Walk/Gun/Walk_Gun.png");
            mGameState.xochitl->setSpriteSheetDimensions({6, 8});
            mGameState.xochitl->setAnimationAtlas(mWalkGunAnimationAtlas);
            mGameState.xochitl->setAnimateWhenStill(false);
         }
      }

      std::map<Direction, std::vector<int>> currentGunDashAnimationAtlas =
            mGunDashAnimationAtlas;

      if (!mIsReloading && wantsDash && !mIsDashing) {
         mIsDashing = true;
         mIsShooting = false;
         mGameState.xochitl->setAnimateWhenStill(false);
         mGameState.xochitl->setSpeed(PLAYER_DASH_SPEED);
         mGameState.xochitl->setTexture(
               "assets/game/adventurer/Dash/Gun/Dash_Gun.png");
         mGameState.xochitl->setSpriteSheetDimensions({6, 8});
         mGameState.xochitl->setAnimationAtlas(currentGunDashAnimationAtlas);
      } else if (!mIsReloading && wantsDash) {
         mGameState.xochitl->setAnimationAtlas(currentGunDashAnimationAtlas);
      } else if ((!wantsDash || mIsReloading) && mIsDashing) {
         mIsDashing = false;
         mGameState.xochitl->setSpeed(PLAYER_NORMAL_SPEED);
         mGameState.xochitl->setTexture(
               "assets/game/adventurer/Walk/Gun/Walk_Gun.png");
         mGameState.xochitl->setSpriteSheetDimensions({6, 8});
         mGameState.xochitl->setAnimationAtlas(mWalkGunAnimationAtlas);
      }

      if (mIsDashing)
         wantsShooting = false;

      std::map<Direction, std::vector<int>> currentGunShootingAtlas =
            mGunAnimationAtlas;

      if (wantsShooting && !mIsShooting) {
         mIsShooting = true;
         mGameState.xochitl->setTexture(
               "assets/game/adventurer/Walk_while_Shooting/Walk_while_Shooting.png");
         mGameState.xochitl->setSpriteSheetDimensions({8, 8});
         mGameState.xochitl->setAnimationAtlas(currentGunShootingAtlas);
         mGameState.xochitl->setAnimateWhenStill(true);
      } else if (wantsShooting) {
         mGameState.xochitl->setAnimationAtlas(currentGunShootingAtlas);
      } else if (!wantsShooting && mIsShooting) {
         mIsShooting = false;
         mGameState.xochitl->setTexture(
               "assets/game/adventurer/Walk/Gun/Walk_Gun.png");
         mGameState.xochitl->setSpriteSheetDimensions({6, 8});
         mGameState.xochitl->setAnimationAtlas(mWalkGunAnimationAtlas);
         mGameState.xochitl->setAnimateWhenStill(false);
      }

      if (wantsShooting && mBulletFireCooldown <= 0.0f) {
         mBulletFireCooldown = BULLET_FIRE_INTERVAL;

         if (mBulletsRemaining <= 0) {
            mIsReloading = true;
            mIsShooting = false;
            mIsDashing = false;
            mReloadTimer = RELOAD_DURATION;
            mGameState.xochitl->setSpeed(PLAYER_NORMAL_SPEED);
            mGameState.xochitl->setTexture("assets/game/adventurer/Reloading/Reloading.png");
            mGameState.xochitl->setSpriteSheetDimensions({6, 8});
            mGameState.xochitl->setAnimationAtlas(mWalkGunAnimationAtlas);
            mGameState.xochitl->setAnimateWhenStill(true);
            return;
         }

            Vector2 bulletMovement =
               getAimFromDirection(mGameState.xochitl->getDirection());

         Vector2 spawnPosition = mGameState.xochitl->getPosition();
         spawnPosition =
               Vector2Add(spawnPosition, Vector2Scale(bulletMovement, 55.0f));

         for (int i = 0; i < MAX_BULLETS; i++) {
            if (mBullets[i] == nullptr || mBullets[i]->isActive())
               continue;

            mBullets[i]->setPosition(spawnPosition);
            mBullets[i]->setMovement(bulletMovement);
            mBullets[i]->setAngle(atan2f(bulletMovement.y, bulletMovement.x) *
                                             57.29578f);
            mBullets[i]->activate();
            mBulletLifetimes[i] = BULLET_LIFETIME;
            mBulletsRemaining--;

            if (mBulletsRemaining <= 0) {
               mIsReloading = true;
               mIsShooting = false;
               mIsDashing = false;
               mReloadTimer = RELOAD_DURATION;
               mGameState.xochitl->setSpeed(PLAYER_NORMAL_SPEED);
               mGameState.xochitl->setTexture(
                     "assets/game/adventurer/Reloading/Reloading.png");
               mGameState.xochitl->setSpriteSheetDimensions({6, 8});
               mGameState.xochitl->setAnimationAtlas(mWalkGunAnimationAtlas);
               mGameState.xochitl->setAnimateWhenStill(true);
            }

            break;
         }
      }
   }

   if (mIsLifeLossTransitioning) {
      mLifeLossTransitionTimer -= deltaTime;
      if (mLifeLossTransitionTimer <= 0.0f)
         mGameState.nextSceneID = mPendingLifeLossSceneID;
      return;
   }

   mGameState.xochitl->update(deltaTime,      // delta time / fixed timestep
                                           nullptr,        // player
                                           mGameState.map, // map
                                           nullptr,        // collidable entities
                                           0               // col. entity count
   );

   if (mPrincess)
      mPrincess->update(deltaTime, nullptr, nullptr, nullptr, 0);

   if (mSkeletonEnemy)
      mSkeletonEnemy->update(deltaTime, mGameState.xochitl, mGameState.map);

   for (int i = 0; i < MAX_BULLETS; i++) {
      if (mBullets[i] == nullptr || !mBullets[i]->isActive())
         continue;

      mBullets[i]->update(deltaTime, nullptr, mGameState.map, nullptr, 0);
      mBulletLifetimes[i] -= deltaTime;

      bool bulletShouldDeactivate = false;

      if (mSkeletonEnemy && mSkeletonEnemy->processBulletHit(mBullets[i]))
         bulletShouldDeactivate = true;

      if (mBulletLifetimes[i] <= 0.0f || mBullets[i]->isCollidingLeft() ||
            mBullets[i]->isCollidingRight() || mBullets[i]->isCollidingTop() ||
            mBullets[i]->isCollidingBottom() || bulletShouldDeactivate) {
         mBullets[i]->deactivate();
      }
   }

   if (mSkeletonEnemy && mGameState.sharedLives &&
         mSkeletonEnemy->shouldDamagePlayer(mGameState.xochitl)) {
      (*mGameState.sharedLives)--;

      if (*mGameState.sharedLives <= 0) {
         mIsPlayerDying = true;
         mPlayerDeathAnimTimer = PLAYER_DEATH_ANIM_DURATION;
         mGameState.xochitl->resetMovement();
         mGameState.xochitl->setSpeed(0);
         mGameState.xochitl->setTexture(
               "assets/game/adventurer/Death/Gun/death_Gun.png");
         mGameState.xochitl->setSpriteSheetDimensions({6, 8});
         mGameState.xochitl->setAnimationAtlas(mGunDeathAnimationAtlas);
         mGameState.xochitl->setAnimateWhenStill(true);
         mGameState.xochitl->setFrameSpeed(18);
         return;
      } else {
         mPendingLifeLossSceneID = LEVEL_B_SCENE;
      }

      mIsLifeLossTransitioning = true;
      mLifeLossTransitionTimer = LIFE_LOSS_DELAY;

      return;
   }

   // death trigger
        if (mGameState.xochitl->getPosition().y > 800.0f &&
         mGameState.sharedLives) {
      (*mGameState.sharedLives)--;

      if (*mGameState.sharedLives <= 0) {
         mIsPlayerDying = true;
         mPlayerDeathAnimTimer = PLAYER_DEATH_ANIM_DURATION;
         mGameState.xochitl->resetMovement();
         mGameState.xochitl->setSpeed(0);
         mGameState.xochitl->setTexture(
               "assets/game/adventurer/Death/Gun/death_Gun.png");
         mGameState.xochitl->setSpriteSheetDimensions({6, 8});
         mGameState.xochitl->setAnimationAtlas(mGunDeathAnimationAtlas);
         mGameState.xochitl->setAnimateWhenStill(true);
         mGameState.xochitl->setFrameSpeed(18);
         return;
      } else {
         mPendingLifeLossSceneID = LEVEL_B_SCENE;
      }

      mIsLifeLossTransitioning = true;
      mLifeLossTransitionTimer = LIFE_LOSS_DELAY;
      return;
   }

   Vector2 feetPosition = {mGameState.xochitl->getPosition().x,
                                       mGameState.xochitl->getPosition().y +
                                             (mGameState.xochitl->getScale().y / 2.0f) -
                                             20.0f};

   int tileX = floor((feetPosition.x - mGameState.map->getLeftBoundary()) /
                              mGameState.map->getTileSize());
   int tileY = floor((feetPosition.y - mGameState.map->getTopBoundary()) /
                              mGameState.map->getTileSize());

   bool nearExitTile = false;
   for (int row = tileY - 1; row <= tileY + 1 && !nearExitTile; row++) {
      for (int col = tileX - 1; col <= tileX + 1; col++) {
         if (row < 0 || row >= mGameState.map->getMapRows() ||
             col < 0 || col >= mGameState.map->getMapColumns())
            continue;

         int tile = mGameState.map->getLevelData()[row * mGameState.map->getMapColumns() + col];
         if (tile == 47 || tile == 48) {
            nearExitTile = true;
            break;
         }
      }
   }

   if (nearExitTile) {
      PlaySound(mLevelClearSound);
      mGameState.nextSceneID = LEVEL_A_SCENE;
      return;
   }
}

void LevelB::render() {
   ClearBackground(ColorFromHex(mBGColourHexCode));

   mGameState.map->render();

   for (int i = 0; i < MAX_BULLETS; i++) {
      if (mBullets[i] == nullptr || !mBullets[i]->isActive())
         continue;
      mBullets[i]->render();
   }

   if (mSkeletonEnemy)
      mSkeletonEnemy->render();
   if (mPrincess)
      mPrincess->render();
   mGameState.xochitl->render();

   if (mGameState.sharedLives) {
      float uiOffsetX = -520.0f;
      if (mHasGun) {
         if (mIsReloading)
            DrawText("RELOADING...",
                         (int)(mGameState.xochitl->getPosition().x + uiOffsetX), 510,
                         30, RED);
         else
            DrawText(TextFormat("Bullets: %d", mBulletsRemaining),
                         (int)(mGameState.xochitl->getPosition().x + uiOffsetX), 510,
                         30, WHITE);
      }
      DrawText(TextFormat("Lives: %d", *mGameState.sharedLives),
                   (int)(mGameState.xochitl->getPosition().x + uiOffsetX), 550, 30,
                   WHITE);
   }
 

   const int tutorialX = (int)(mGameState.xochitl->getPosition().x - 180.0f);
   if (mTutorialStep == 0)
      DrawText("Press W to move up", tutorialX, 20, 24, WHITE);
   else if (mTutorialStep == 1)
      DrawText("Press SPACE to dash", tutorialX, 20, 24, WHITE);
   else if (mTutorialStep == 2)
      DrawText("Press F to pick up gun", tutorialX, 20, 24, WHITE);
   else if (mTutorialStep == 3)
      DrawText("Hold LEFT SHIFT to shoot", tutorialX, 20, 24, WHITE);
   else if (mTutorialStep == 4)
      DrawText("Tutorial complete!", tutorialX, 20, 24, GREEN);
}

void LevelB::shutdown() {
   for (int i = 0; i < MAX_BULLETS; i++) {
      delete mBullets[i];
      mBullets[i] = nullptr;
   }
   mBullets.clear();
   mBulletLifetimes.clear();

   if (mSkeletonEnemy) {
      mSkeletonEnemy->shutdown();
      delete mSkeletonEnemy;
      mSkeletonEnemy = nullptr;
   }

   delete mPrincess;
   mPrincess = nullptr;

   delete mGameState.xochitl;
   delete mGameState.map;

   UnloadMusicStream(mGameState.bgm);
   UnloadSound(mLevelClearSound);
}