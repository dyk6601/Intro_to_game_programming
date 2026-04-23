#include "LevelA.h"

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

LevelA::LevelA() : Scene{{0.0f}, nullptr} {}
LevelA::LevelA(Vector2 origin, const char *bgHexCode)
    : Scene{origin, bgHexCode} {}

LevelA::~LevelA() { shutdown(); }

void LevelA::playerDeathAnimationHelper() {
  mIsPlayerDying = true;
  mGameState.xochitl->resetMovement();
  mGameState.xochitl->setSpeed(0);
  PlaySound(mEvilLaughSound);

  constexpr int PLAYER_DEATH_FRAME_SPEED = 4;
  Direction currentDirection = mGameState.xochitl->getDirection();
  int deathFrameCount = 8;
  mPlayerDeathAnimTimer =
      (float)deathFrameCount / (float)PLAYER_DEATH_FRAME_SPEED;

  mGameState.xochitl->setTexture("assets/game/adventurer/Death/Gun/death_Gun.png");
  mGameState.xochitl->setSpriteSheetDimensions({6, 8});
  mGameState.xochitl->setDirection(currentDirection);
  mGameState.xochitl->setAnimationAtlas(mGunDeathAnimationAtlas);
  mGameState.xochitl->resetAnimation();
  mGameState.xochitl->setAnimateWhenStill(true);
  mGameState.xochitl->setFrameSpeed(PLAYER_DEATH_FRAME_SPEED);
}

void LevelA::configureWaves() {
  mWaveStartDelays.clear();
  mSpawnWaveIndices.clear();
  mSpawnEnemyTypes.clear();
  mSpawnPositions.clear();
  mSpawnTimesFromWaveStart.clear();
  mSpawnHasTriggered.clear();

  // Delay time before each wave begins.
  mWaveStartDelays.push_back(1.0f); // Wave 0
  mWaveStartDelays.push_back(2.0f); // Wave 1
  mWaveStartDelays.push_back(2.4f); // Wave 2

  // Wave 1 
  addWaveSpawn(0, EnemyType::SKELETON, {mOrigin.x + 320.0f, mOrigin.y - 180.0f},
               0.0f);
  addWaveSpawn(0, EnemyType::SKELETON, {mOrigin.x - 260.0f, mOrigin.y + 20.0f},
               1.25f);
  addWaveSpawn(0, EnemyType::SKELETON, {mOrigin.x + 180.0f, mOrigin.y + 90.0f},
               0.55f);
  addWaveSpawn(0, EnemyType::SKELETON, {mOrigin.x - 340.0f, mOrigin.y - 120.0f},
               1.8f);

  // Wave 2 
  addWaveSpawn(1, EnemyType::SKELETON, {mOrigin.x + 210.0f, mOrigin.y - 130.0f},
               0.0f);
  addWaveSpawn(1, EnemyType::VAMPIRE, {mOrigin.x + 60.0f, mOrigin.y + 10.0f},
               0.8f);
  addWaveSpawn(1, EnemyType::SKELETON, {mOrigin.x - 160.0f, mOrigin.y - 170.0f},
               1.6f);
  addWaveSpawn(1, EnemyType::SKELETON, {mOrigin.x - 300.0f, mOrigin.y + 70.0f},
               1.05f);
  addWaveSpawn(1, EnemyType::VAMPIRE, {mOrigin.x + 280.0f, mOrigin.y - 40.0f},
               2.1f);

  // Wave 3 
  addWaveSpawn(2, EnemyType::VAMPIRE, {mOrigin.x + 300.0f, mOrigin.y - 190.0f},
               0.0f);
  addWaveSpawn(2, EnemyType::SKELETON, {mOrigin.x + 120.0f, mOrigin.y - 90.0f},
               0.6f);
  addWaveSpawn(2, EnemyType::SKELETON, {mOrigin.x - 280.0f, mOrigin.y + 10.0f},
               1.2f);
  addWaveSpawn(2, EnemyType::VAMPIRE, {mOrigin.x - 80.0f, mOrigin.y - 150.0f},
               2.2f);
}

void LevelA::startWave(int waveIndex) {
  if (waveIndex < 0 || waveIndex >= (int)mWaveStartDelays.size())
    return;

  mCurrentWaveIndex = waveIndex;
  mCurrentWaveElapsed = 0.0f;

  for (size_t i = 0; i < mSpawnHasTriggered.size(); i++) {
    if (mSpawnWaveIndices[i] == waveIndex)
      mSpawnHasTriggered[i] = false;
  }
}

void LevelA::addWaveSpawn(int waveIndex, EnemyType type, Vector2 position,
                          float spawnTimeFromWaveStart) {
  mSpawnWaveIndices.push_back(waveIndex);
  mSpawnEnemyTypes.push_back(type);
  mSpawnPositions.push_back(position);
  mSpawnTimesFromWaveStart.push_back(spawnTimeFromWaveStart);
  mSpawnHasTriggered.push_back(false);
}

void LevelA::spawnConfiguredEnemy(EnemyType enemyType, Vector2 spawnPosition) {
  if (enemyType == EnemyType::SKELETON) {
    SkeletonEnemy *enemy = new SkeletonEnemy();
    enemy->initialise(mOrigin, spawnPosition);
    enemy->setAutoRespawn(false);
    mSkeletonEnemies.push_back(enemy);
    return;
  }

  VampireEnemy *enemy = new VampireEnemy();
  enemy->initialise(mOrigin, spawnPosition);
  enemy->setAutoRespawn(false);
  mVampireEnemies.push_back(enemy);
}

bool LevelA::allAliveEnemiesCleared() const {
  return mSkeletonEnemies.empty() && mVampireEnemies.empty();
}

bool LevelA::isCurrentWaveComplete() const {
  return isWaveSpawnScheduleFinished(mCurrentWaveIndex) && allAliveEnemiesCleared();
}

bool LevelA::isWaveSpawnScheduleFinished(int waveIndex) const {
  for (size_t i = 0; i < mSpawnWaveIndices.size(); i++) {
    if (mSpawnWaveIndices[i] == waveIndex && !mSpawnHasTriggered[i])
      return false;
  }

  return true;
}

void LevelA::cleanupInactiveEnemies() {
  for (size_t i = 0; i < mSkeletonEnemies.size();) {
    if (mSkeletonEnemies[i] && mSkeletonEnemies[i]->isActive()) {
      i++;
      continue;
    }

    delete mSkeletonEnemies[i];
    mSkeletonEnemies.erase(mSkeletonEnemies.begin() + i);
  }

  for (size_t i = 0; i < mVampireEnemies.size();) {
    if (mVampireEnemies[i] && mVampireEnemies[i]->isActive()) {
      i++;
      continue;
    }

    delete mVampireEnemies[i];
    mVampireEnemies.erase(mVampireEnemies.begin() + i);
  }
}

void LevelA::updateWaveSpawner(float deltaTime) {
  if (mAllWavesCompleted)
    return;

  if (mWaitingToStartWave) {
    mWaveStartTimer -= deltaTime;
    if (mWaveStartTimer <= 0.0f) {
      mWaitingToStartWave = false;
      startWave(mNextWaveIndexToStart);
      mNextWaveIndexToStart++;
    }
    return;
  }

  mCurrentWaveElapsed += deltaTime;

  for (size_t i = 0; i < mSpawnWaveIndices.size(); i++) {
    if (mSpawnWaveIndices[i] != mCurrentWaveIndex)
      continue;
    if (mSpawnHasTriggered[i])
      continue;

    if (mCurrentWaveElapsed >= mSpawnTimesFromWaveStart[i]) {
      spawnConfiguredEnemy(mSpawnEnemyTypes[i], mSpawnPositions[i]);
      mSpawnHasTriggered[i] = true;
    }
  }

  if (!isCurrentWaveComplete())
    return;

  PlaySound(mWaveClearSound);

  if (mNextWaveIndexToStart < (int)mWaveStartDelays.size()) {
    mWaitingToStartWave = true;
    mWaveStartTimer = mWaveStartDelays[mNextWaveIndexToStart];
    return;
  }

  mAllWavesCompleted = true;
  mIsLevelTransitioning = true;
  mLevelTransitionTimer = 0.9f;
}

void LevelA::initialise() {
  mGameState.nextSceneID = NO_SCENE;
  mHasGun = false;
  mIsShooting = false;
  mIsDashing = false;
  mIsReloading = false;
  mBulletsRemaining = MAX_BULLETS;
  mReloadTimer = 0.0f;
  mBulletFireCooldown = 0.0f;
  mIsLevelTransitioning = false;
  mLevelTransitionTimer = 0.0f;
  mIsLifeLossTransitioning = false;
  mLifeLossTransitionTimer = 0.0f;
  mIsPlayerDying = false;
  mPlayerDeathAnimTimer = 0.0f;
  mPendingLifeLossSceneID = NO_SCENE;

  mGameState.bgm = LoadMusicStream("assets/game/dungeon.mp3");
  SetMusicVolume(mGameState.bgm, 0.28f);
  PlayMusicStream(mGameState.bgm);
  mWaveClearSound = LoadSound("assets/game/wave_clear.mp3");
  mEvilLaughSound = LoadSound("assets/game/evil_laugh.mp3");
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
      LEVEL_WIDTH, LEVEL_HEIGHT,  // map grid cols & rows
      (unsigned int *)mLevelData, // grid data
      "assets/game/kenney_tinyDungeon/Tilemap/tilemap_packed.png", // texture
      TILE_DIMENSION,                                              // tile size
      12, 11,                                 // texture cols & rows
      mOrigin,                                // in-game origin
        {2, 3, 4, 5, 6, 7,9, 14, 16, 17, 18,19, 21, 33, 27, 90} // solid tile IDs (walls)
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
      new Entity({mOrigin.x - 300.0f, mOrigin.y - 200.0f}, // position
                 {250.0f * sizeRatio, 250.0f},             // scale
                 "assets/game/walk.png", // texture file address
                 ATLAS,                  // single image or atlas?
                 {6, 8},                 // atlas dimensions
                 xochitlAnimationAtlas,  // actual atlas
                 PLAYER                  // entity type
      );

  mGameState.xochitl->setColliderDimensions(
      {mGameState.xochitl->getScale().x / 2.0f,
       mGameState.xochitl->getScale().y / 2.0f});
  mGameState.xochitl->setSpeed(PLAYER_NORMAL_SPEED);
  mGameState.xochitl->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});

  mSkeletonEnemies.clear();
  mVampireEnemies.clear();
  mWaveStartDelays.clear();
  mSpawnWaveIndices.clear();
  mSpawnEnemyTypes.clear();
  mSpawnPositions.clear();
  mSpawnTimesFromWaveStart.clear();
  mSpawnHasTriggered.clear();
  mCurrentWaveIndex = -1;
  mCurrentWaveElapsed = 0.0f;
  mNextWaveIndexToStart = 0;
  mWaitingToStartWave = false;
  mWaveStartTimer = 0.0f;
  mAllWavesCompleted = false;

  configureWaves();

  if (!mWaveStartDelays.empty()) {
    mWaitingToStartWave = true;
    mWaveStartTimer = mWaveStartDelays[0];
  }
}

void LevelA::update(float deltaTime) {
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
  //get gun
  if (!mHasGun && IsKeyPressed(KEY_F) && mGameState.xochitl) {
    mHasGun = true;
    mIsDashing = false;
    mIsReloading = false;
    mBulletsRemaining = MAX_BULLETS;
    mGameState.xochitl->setSpeed(PLAYER_NORMAL_SPEED);
    mGameState.xochitl->setTexture("assets/game/adventurer/Walk/Gun/Walk_Gun.png");
    mGameState.xochitl->setSpriteSheetDimensions({6, 8});
    mGameState.xochitl->setAnimationAtlas(mWalkGunAnimationAtlas);
  }
  //dash and shoot
  if (!mHasGun && mGameState.xochitl) {
    bool wantsDash = IsKeyDown(KEY_SPACE);
    std::map<Direction, std::vector<int>> currentDashAnimationAtlas = mDashAnimationAtlas;

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
        mBullets[i]->setAngle(atan2f(bulletMovement.y, bulletMovement.x) * 57.29578f);
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

  if (mIsLevelTransitioning) {
    mLevelTransitionTimer -= deltaTime;
    if (mLevelTransitionTimer <= 0.0f)
      mGameState.nextSceneID = LEVEL_C_SCENE;
    return;
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

  updateWaveSpawner(deltaTime);

  for (SkeletonEnemy *enemy : mSkeletonEnemies) {
    if (enemy)
      enemy->update(deltaTime, mGameState.xochitl, mGameState.map);
  }

  for (VampireEnemy *enemy : mVampireEnemies) {
    if (enemy)
      enemy->update(deltaTime, mGameState.xochitl, mGameState.map);
  }

  cleanupInactiveEnemies();

  for (int i = 0; i < MAX_BULLETS; i++) {
    if (mBullets[i] == nullptr || !mBullets[i]->isActive())
      continue;

    mBullets[i]->update(deltaTime, nullptr, mGameState.map, nullptr, 0);
    mBulletLifetimes[i] -= deltaTime;

    bool bulletShouldDeactivate = false;

    for (SkeletonEnemy *enemy : mSkeletonEnemies) {
      if (enemy && enemy->processBulletHit(mBullets[i])) {
        bulletShouldDeactivate = true;
        break;
      }
    }

    if (!bulletShouldDeactivate) {
      for (VampireEnemy *enemy : mVampireEnemies) {
        if (enemy && enemy->processBulletHit(mBullets[i])) {
          bulletShouldDeactivate = true;
          break;
        }
      }
    }

    if (mBulletLifetimes[i] <= 0.0f || mBullets[i]->isCollidingLeft() ||
        mBullets[i]->isCollidingRight() || mBullets[i]->isCollidingTop() ||
        mBullets[i]->isCollidingBottom() || bulletShouldDeactivate) {
      mBullets[i]->deactivate();
    }
  }

  bool shouldAnySkeletonDamagePlayer = false;
  for (SkeletonEnemy *enemy : mSkeletonEnemies) {
    if (enemy && enemy->shouldDamagePlayer(mGameState.xochitl)) {
      shouldAnySkeletonDamagePlayer = true;
      break;
    }
  }

  bool shouldVampireDamagePlayer = false;
  for (VampireEnemy *enemy : mVampireEnemies) {
    if (enemy && enemy->shouldDamagePlayer(mGameState.xochitl)) {
      shouldVampireDamagePlayer = true;
      break;
    }
  }

  if ((shouldAnySkeletonDamagePlayer || shouldVampireDamagePlayer) &&
      mGameState.sharedLives) {
    (*mGameState.sharedLives)--;

    if (*mGameState.sharedLives <= 0) {
      playerDeathAnimationHelper();
      return;
    } else {
      mPendingLifeLossSceneID = LEVEL_A_SCENE;
    }

    mIsLifeLossTransitioning = true;
    mLifeLossTransitionTimer = LIFE_LOSS_DELAY;

    return;
  }

  // death trigger
  if (mGameState.xochitl->getPosition().y > 800.0f && mGameState.sharedLives) {
    (*mGameState.sharedLives)--;

    if (*mGameState.sharedLives <= 0) {
      playerDeathAnimationHelper();
      return;
    } else {
      mPendingLifeLossSceneID = LEVEL_A_SCENE;
    }

    mIsLifeLossTransitioning = true;
    mLifeLossTransitionTimer = LIFE_LOSS_DELAY;
    return;
  }
}


void LevelA::render() {
  ClearBackground(ColorFromHex(mBGColourHexCode));

  mGameState.map->render();

  for (int i = 0; i < MAX_BULLETS; i++) {
    if (mBullets[i] == nullptr || !mBullets[i]->isActive())
      continue;
    mBullets[i]->render();
  }

  for (SkeletonEnemy *enemy : mSkeletonEnemies) {
    if (enemy)
      enemy->render();
  }

  for (VampireEnemy *enemy : mVampireEnemies) {
    if (enemy)
      enemy->render();
  }

  mGameState.xochitl->render();

  if (mGameState.sharedLives) {
    float uiOffsetX = -520.0f;
    int totalWaves = (int)mWaveStartDelays.size();
    int displayWaveNumber = 1;

    if (totalWaves > 0) {
      if (mAllWavesCompleted) {
        displayWaveNumber = totalWaves;
      } else if (mWaitingToStartWave) {
        displayWaveNumber = mNextWaveIndexToStart + 1;
      } else if (mCurrentWaveIndex >= 0) {
        displayWaveNumber = mCurrentWaveIndex + 1;
      }

      if (displayWaveNumber < 1)
        displayWaveNumber = 1;
      if (displayWaveNumber > totalWaves)
        displayWaveNumber = totalWaves;

      DrawText(TextFormat("Wave: %d/%d", displayWaveNumber, totalWaves),
               (int)(mGameState.xochitl->getPosition().x + uiOffsetX), 470,
               30, YELLOW);
    }

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
             BLUE);
  }
}

void LevelA::shutdown() {
  for (int i = 0; i < MAX_BULLETS; i++) {
    delete mBullets[i];
    mBullets[i] = nullptr;
  }
  mBullets.clear();
  mBulletLifetimes.clear();

  for (SkeletonEnemy *enemy : mSkeletonEnemies) {
    delete enemy;
  }
  mSkeletonEnemies.clear();

  for (VampireEnemy *enemy : mVampireEnemies) {
    delete enemy;
  }
  mVampireEnemies.clear();

  mWaveStartDelays.clear();
  mSpawnWaveIndices.clear();
  mSpawnEnemyTypes.clear();
  mSpawnPositions.clear();
  mSpawnTimesFromWaveStart.clear();
  mSpawnHasTriggered.clear();

  delete mGameState.xochitl;
  delete mGameState.map;

  UnloadMusicStream(mGameState.bgm);
  UnloadSound(mWaveClearSound);
  UnloadSound(mEvilLaughSound);
}