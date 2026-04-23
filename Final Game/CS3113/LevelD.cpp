#include "LevelD.h"

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
} // namespace

LevelD::LevelD() : Scene{{0.0f}, nullptr} {}
LevelD::LevelD(Vector2 origin, const char *bgHexCode) : Scene{origin, bgHexCode} {}

LevelD::~LevelD() { shutdown(); }

void LevelD::playerDeathAnimationHelper() {
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

void LevelD::initialise() {
  mGameState.nextSceneID = NO_SCENE;
  mHasGun = false;
  mIsShooting = false;
  mIsDashing = false;
  mIsReloading = false;
  mBulletsRemaining = MAX_BULLETS;
  mReloadTimer = 0.0f;
  mIsLifeLossTransitioning = false;
  mLifeLossTransitionTimer = 0.0f;
  mIsPlayerDying = false;
  mPlayerDeathAnimTimer = 0.0f;
  mPendingLifeLossSceneID = NO_SCENE;

  mGameState.bgm = LoadMusicStream("assets/game/boss_fight.mp3");
  SetMusicVolume(mGameState.bgm, 0.33f);
  PlayMusicStream(mGameState.bgm);
  mGameState.jumpSound = LoadSound("assets/game/Jump.wav");
  mLevelClearSound = LoadSound("assets/game/level_up.mp3");
  mEvilLaughSound = LoadSound("assets/game/evil_laugh.mp3");
  mBullets.clear();
  mBulletLifetimes.clear();
  mBullets.resize(MAX_BULLETS, nullptr);
  mBulletLifetimes.resize(MAX_BULLETS, 0.0f);

  for (int i = 0; i < MAX_BULLETS; i++) {
    mBullets[i] = new Entity({0.0f, 0.0f}, {BULLET_SCALE, BULLET_SCALE},
                             "assets/game/bullet.png", NONE);
    mBullets[i]->setAcceleration({0.0f, 0.0f});
    mBullets[i]->setSpeed((int)BULLET_SPEED);
    mBullets[i]->setColliderDimensions({BULLET_SCALE * 0.65f, BULLET_SCALE * 0.65f});
    mBullets[i]->deactivate();
  }

  mGameState.map = new Map(LEVEL_D_WIDTH, LEVEL_D_HEIGHT, (unsigned int *)mLevelData,
                           "assets/game/kenney_tinyDungeon/Tilemap/tilemap_packed.png",
                           TILE_DIMENSION, 12, 11, mOrigin,
                           {2, 3, 4, 5, 6, 14, 16, 17, 18, 27,65, 90});

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

  mGameState.xochitl = new Entity({mOrigin.x - 300.0f, mOrigin.y - 200.0f},
                                  {250.0f * sizeRatio, 250.0f},
                                  "assets/game/walk.png", ATLAS, {6, 8},
                                  xochitlAnimationAtlas, PLAYER);

  mGameState.xochitl->setJumpingPower(550.0f);
  mGameState.xochitl->setColliderDimensions({mGameState.xochitl->getScale().x / 3.0f,
                                            mGameState.xochitl->getScale().y / 2.0f});
  mGameState.xochitl->setSpeed(PLAYER_NORMAL_SPEED);
  mGameState.xochitl->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});

  if (mGorgonEnemy == nullptr) mGorgonEnemy = new GorgonEnemy();
  if (!mPreserveGorgonStateOnInitialise) {
    mGorgonEnemy->initialise(mOrigin, {mOrigin.x + 260.0f, mOrigin.y - 140.0f});
    mGorgonEnemy->setAutoRespawn(false);
  }

  mPreserveGorgonStateOnInitialise = false;
}

void LevelD::update(float deltaTime) {
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

  if (!mHasGun && mGameState.xochitl) {
    bool wantsDash = IsKeyDown(KEY_SPACE);
    std::map<Direction, std::vector<int>> currentDashAnimationAtlas = mDashAnimationAtlas;

    if (wantsDash && !mIsDashing) {
      mIsDashing = true;
      mGameState.xochitl->setSpeed(PLAYER_DASH_SPEED);
      mGameState.xochitl->setTexture("assets/game/adventurer/Dash/Normal/Dash.png");
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
        mGameState.xochitl->setTexture("assets/game/adventurer/Walk/Gun/Walk_Gun.png");
        mGameState.xochitl->setSpriteSheetDimensions({6, 8});
        mGameState.xochitl->setAnimationAtlas(mWalkGunAnimationAtlas);
        mGameState.xochitl->setAnimateWhenStill(false);
      }
    }

    std::map<Direction, std::vector<int>> currentGunDashAnimationAtlas = mGunDashAnimationAtlas;

    if (!mIsReloading && wantsDash && !mIsDashing) {
      mIsDashing = true;
      mIsShooting = false;
      mGameState.xochitl->setAnimateWhenStill(false);
      mGameState.xochitl->setSpeed(PLAYER_DASH_SPEED);
      mGameState.xochitl->setTexture("assets/game/adventurer/Dash/Gun/Dash_Gun.png");
      mGameState.xochitl->setSpriteSheetDimensions({6, 8});
      mGameState.xochitl->setAnimationAtlas(currentGunDashAnimationAtlas);
    } else if (!mIsReloading && wantsDash) {
      mGameState.xochitl->setAnimationAtlas(currentGunDashAnimationAtlas);
    } else if ((!wantsDash || mIsReloading) && mIsDashing) {
      mIsDashing = false;
      mGameState.xochitl->setSpeed(PLAYER_NORMAL_SPEED);
      mGameState.xochitl->setTexture("assets/game/adventurer/Walk/Gun/Walk_Gun.png");
      mGameState.xochitl->setSpriteSheetDimensions({6, 8});
      mGameState.xochitl->setAnimationAtlas(mWalkGunAnimationAtlas);
    }

    if (mIsDashing) wantsShooting = false;

    std::map<Direction, std::vector<int>> currentGunShootingAtlas = mGunAnimationAtlas;

    if (wantsShooting && !mIsShooting) {
      mIsShooting = true;
      mGameState.xochitl->setTexture("assets/game/adventurer/Walk_while_Shooting/Walk_while_Shooting.png");
      mGameState.xochitl->setSpriteSheetDimensions({8, 8});
      mGameState.xochitl->setAnimationAtlas(currentGunShootingAtlas);
      mGameState.xochitl->setAnimateWhenStill(true);
    } else if (wantsShooting) {
      mGameState.xochitl->setAnimationAtlas(currentGunShootingAtlas);
    } else if (!wantsShooting && mIsShooting) {
      mIsShooting = false;
      mGameState.xochitl->setTexture("assets/game/adventurer/Walk/Gun/Walk_Gun.png");
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

      Vector2 bulletMovement = getAimFromDirection(mGameState.xochitl->getDirection());

      Vector2 spawnPosition = mGameState.xochitl->getPosition();
      spawnPosition = Vector2Add(spawnPosition, Vector2Scale(bulletMovement, 55.0f));

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
          mGameState.xochitl->setTexture("assets/game/adventurer/Reloading/Reloading.png");
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

  mGameState.xochitl->update(deltaTime, nullptr, mGameState.map, nullptr, 0);

  if (mGorgonEnemy)
    mGorgonEnemy->update(deltaTime, mGameState.xochitl, mGameState.map);

  for (int i = 0; i < MAX_BULLETS; i++) {
    if (mBullets[i] == nullptr || !mBullets[i]->isActive())
      continue;

    mBullets[i]->update(deltaTime, nullptr, mGameState.map, nullptr, 0);
    mBulletLifetimes[i] -= deltaTime;

    bool bulletShouldDeactivate = false;
    if (mGorgonEnemy && mGorgonEnemy->processBulletHit(mBullets[i]))
      bulletShouldDeactivate = true;

    if (mBulletLifetimes[i] <= 0.0f || mBullets[i]->isCollidingLeft() ||
        mBullets[i]->isCollidingRight() || mBullets[i]->isCollidingTop() ||
        mBullets[i]->isCollidingBottom() || bulletShouldDeactivate) {
      mBullets[i]->deactivate();
    }
  }

  if (mGorgonEnemy && !mGorgonEnemy->isActive()) {
    PlaySound(mLevelClearSound);
    mGameState.nextSceneID = WIN_SCENE;
    return;
  }

  if (mGorgonEnemy && mGameState.sharedLives &&
      mGorgonEnemy->shouldDamagePlayer(mGameState.xochitl)) {
    (*mGameState.sharedLives)--;

    if (*mGameState.sharedLives <= 0) {
      playerDeathAnimationHelper();
      return;
    } else {
      mPendingLifeLossSceneID = LEVEL_D_SCENE;
      mPreserveGorgonStateOnInitialise = true;
    }

    mIsLifeLossTransitioning = true;
    mLifeLossTransitionTimer = LIFE_LOSS_DELAY;
    return;
  }

  if (mGameState.xochitl->getPosition().y > 800.0f && mGameState.sharedLives) {
    (*mGameState.sharedLives)--;

    if (*mGameState.sharedLives <= 0) {
      playerDeathAnimationHelper();
      return;
    } else {
      mPendingLifeLossSceneID = LEVEL_D_SCENE;
      mPreserveGorgonStateOnInitialise = true;
    }

    mIsLifeLossTransitioning = true;
    mLifeLossTransitionTimer = LIFE_LOSS_DELAY;
    return;
  }
}

void LevelD::render() {
  ClearBackground(ColorFromHex(mBGColourHexCode));

  mGameState.map->render();

  for (int i = 0; i < MAX_BULLETS; i++) {
    if (mBullets[i] == nullptr || !mBullets[i]->isActive())
      continue;
    mBullets[i]->render();
  }

  if (mGorgonEnemy)
    mGorgonEnemy->render();

  mGameState.xochitl->render();

  if (mGorgonEnemy && mGorgonEnemy->isActive()) {
    int maxHP = mGorgonEnemy->getMaxHitPoints();
    int currentHP = mGorgonEnemy->getHitPoints();

    if (maxHP > 0) {
      float hpRatio = (float)currentHP / (float)maxHP;
      if (hpRatio < 0.0f) hpRatio = 0.0f;
      if (hpRatio > 1.0f) hpRatio = 1.0f;

      int barX = (int)(mGameState.xochitl->getPosition().x - 210.0f);
      int barY = 24;
      int barWidth = 420;
      int barHeight = 24;

      DrawRectangle(barX, barY, barWidth, barHeight, DARKGRAY);
      DrawRectangle(barX + 2, barY + 2,
                    (int)((barWidth - 4) * hpRatio), barHeight - 4, RED);
      DrawRectangleLines(barX, barY, barWidth, barHeight, WHITE);
      DrawText(TextFormat("%d / %d", currentHP, maxHP),
               barX + barWidth + 16, barY + 2, 20, WHITE);
    }
  }

  if (mGameState.sharedLives) {
    float uiOffsetX = -520.0f;
    if (mHasGun) {
      if (mIsReloading)
        DrawText("RELOADING...", (int)(mGameState.xochitl->getPosition().x + uiOffsetX), 510, 30, RED);
      else
        DrawText(TextFormat("Bullets: %d", mBulletsRemaining),
                 (int)(mGameState.xochitl->getPosition().x + uiOffsetX), 510, 30, WHITE);
    }
    DrawText(TextFormat("Lives: %d", *mGameState.sharedLives),
             (int)(mGameState.xochitl->getPosition().x + uiOffsetX), 550, 30, BLUE);
  }
}

void LevelD::shutdown() {
  for (int i = 0; i < MAX_BULLETS; i++) {
    delete mBullets[i];
    mBullets[i] = nullptr;
  }
  mBullets.clear();
  mBulletLifetimes.clear();

  if (mGorgonEnemy) {
    mGorgonEnemy->shutdown();
    delete mGorgonEnemy;
    mGorgonEnemy = nullptr;
  }

  delete mGameState.xochitl;
  delete mGameState.map;

  UnloadMusicStream(mGameState.bgm);
  UnloadSound(mGameState.jumpSound);
  UnloadSound(mLevelClearSound);
  UnloadSound(mEvilLaughSound);
}
