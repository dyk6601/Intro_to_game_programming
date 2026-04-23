#include "GorgonEnemy.h"

GorgonEnemy::GorgonEnemy() {}

GorgonEnemy::~GorgonEnemy() { shutdown(); }

void GorgonEnemy::deathAnimationHelper()
{
  std::vector<int> deathFrames = {0, 1, 2};

  mDeathAnimationAtlas = {
      {DOWN, deathFrames},
      {LEFT, deathFrames},
      {UP, deathFrames},
      {RIGHT, deathFrames},
  };

  int deathFrameCount = 3;
  mDeathTimer = (float)deathFrameCount / (float)DEATH_FRAME_SPEED;
}

void GorgonEnemy::spawnAttackProjectile()
{
  if (mGorgon == nullptr || mAttackProjectile == nullptr) return;

  Vector2 forward = {0.0f, 0.0f};
  if (mGorgon->getDirection() == LEFT) forward = {1.0f, 0.0f};
  else if (mGorgon->getDirection() == RIGHT) forward = {-1.0f, 0.0f};
  else if (mGorgon->getDirection() == UP) forward = {0.0f, -1.0f};
  else forward = {0.0f, 1.0f};

  Vector2 spawnPosition = Vector2Add(
      mGorgon->getPosition(),
      Vector2Scale(forward, PROJECTILE_FORWARD_OFFSET));
  spawnPosition.y += 30.0f;

  mAttackProjectile->setPosition(spawnPosition);
  mAttackProjectile->setMovement(forward);
  mAttackProjectile->setAngle(atan2f(forward.y, forward.x) * 57.29578f);
  mAttackProjectile->activate();
  mProjectileLifetime = PROJECTILE_LIFETIME;
  mHasFiredProjectileThisAttack = true;
}

void GorgonEnemy::initialise(Vector2 origin)
{
  initialise(origin, {origin.x + 200.0f, origin.y - 120.0f});
}

void GorgonEnemy::initialise(Vector2 origin, Vector2 spawnPosition)
{
  (void)origin;
  shutdown();

  mAnimationState = GORGON_IDLE;
  mHitPoints = HIT_POINTS;
  mIsDead = false;

  mBaseSpawnPosition = spawnPosition;
  mSpawnPosition = mBaseSpawnPosition;

  mDamageTimer = 0.0f;
  mDeathTimer = 0.0f;
  mAttackTimer = 0.0f;
  mAttackCooldownTimer = 0.0f;

  mDidFinishAttackThisFrame = false;
  mAutoRespawn = true;
  mHasFiredProjectileThisAttack = false;
  mProjectileLifetime = 0.0f;

  std::vector<int> gorgonFrames = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
  std::vector<int> gorgonAttackFrames = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

  mIdleAnimationAtlas = {
      {DOWN, gorgonFrames},
      {LEFT, gorgonFrames},
      {UP, gorgonFrames},
      {RIGHT, gorgonFrames},
  };
  mMoveAnimationAtlas = mIdleAnimationAtlas;
  mAttackAnimationAtlas = {
      {DOWN, gorgonAttackFrames},
      {LEFT, gorgonAttackFrames},
      {UP, gorgonAttackFrames},
      {RIGHT, gorgonAttackFrames},
  };
  mTakeDamageAnimationAtlas = mIdleAnimationAtlas;
  deathAnimationHelper();

  mGorgon = new Entity(
      mSpawnPosition,
      {205.0f, 205.0f},
      "assets/game/gorgon/walk.png",
      ATLAS,
      {1, 13},
      mIdleAnimationAtlas,
      PLAYER
  );

  mGorgon->setColliderDimensions(
      {mGorgon->getScale().x * 0.42f, mGorgon->getScale().y * 0.70f});
  mGorgon->setAcceleration({0.0f, 0.0f});
  mGorgon->setSpeed(SPEED);
  mGorgon->setFrameSpeed(12);
  mGorgon->setDirection(LEFT);

  mAttackProjectile = new Entity(
      {0.0f, 0.0f},
      {PROJECTILE_SCALE, PROJECTILE_SCALE},
      "assets/game/66.png",
      NONE
  );
  mAttackProjectile->setAcceleration({0.0f, 0.0f});
  mAttackProjectile->setSpeed((int)PROJECTILE_SPEED);
  mAttackProjectile->setColliderDimensions(
      {PROJECTILE_SCALE * PROJECTILE_HITBOX_SCALE,
       PROJECTILE_SCALE * PROJECTILE_HITBOX_SCALE});
  mAttackProjectile->deactivate();
}

void GorgonEnemy::setAnimation(int newState)
{
  if (mGorgon == nullptr || mAnimationState == newState) return;

  mAnimationState = newState;

  if (newState == GORGON_IDLE)
  {
    mGorgon->setTexture("assets/game/gorgon/walk.png");
    mGorgon->setSpriteSheetDimensions({1, 13});
    mGorgon->setAnimationAtlas(mIdleAnimationAtlas);
    mGorgon->setFrameSpeed(10);
    mGorgon->setAnimateWhenStill(true);
  }
  else if (newState == GORGON_MOVING)
  {
    mGorgon->setTexture("assets/game/gorgon/walk.png");
    mGorgon->setSpriteSheetDimensions({1, 13});
    mGorgon->setAnimationAtlas(mMoveAnimationAtlas);
    mGorgon->setFrameSpeed(13);
    mGorgon->setAnimateWhenStill(false);
  }
  else if (newState == GORGON_ATTACKING)
  {
    mGorgon->setTexture("assets/game/Gorgon/Attack_3.png");
    mGorgon->setSpriteSheetDimensions({1, 10});
    mGorgon->setAnimationAtlas(mAttackAnimationAtlas);
    mGorgon->setFrameSpeed(12);
    mGorgon->setAnimateWhenStill(true);
  }
  else if (newState == GORGON_TAKING_DAMAGE)
  {
    mGorgon->setTexture("assets/game/gorgon/walk.png");
    mGorgon->setSpriteSheetDimensions({1, 13});
    mGorgon->setAnimationAtlas(mTakeDamageAnimationAtlas);
    mGorgon->setFrameSpeed(14);
    mGorgon->setAnimateWhenStill(true);
  }
  else if (newState == GORGON_DYING)
  {
    mGorgon->setTexture("assets/game/Gorgon/Dead.png");
    mGorgon->setSpriteSheetDimensions({1, 3});
    mGorgon->setAnimationAtlas(mDeathAnimationAtlas);
    mGorgon->setFrameSpeed(DEATH_FRAME_SPEED);
    mGorgon->setAnimateWhenStill(true);
  }

  mGorgon->resetAnimation();
}

void GorgonEnemy::update(float deltaTime, Entity *player, Map *map)
{
  if (mGorgon == nullptr || player == nullptr) return;

  mDidFinishAttackThisFrame = false;

  if (!mGorgon->isActive()) return;

  if (mIsDead)
  {
    setAnimation(GORGON_DYING);
    mGorgon->setMovement({0.0f, 0.0f});
    mDeathTimer -= deltaTime;
    mGorgon->update(deltaTime, player, map, nullptr, 0);

    if (mDeathTimer <= 0.0f)
    {
      if (!mAutoRespawn)
      {
        mGorgon->deactivate();
        mGorgon->setMovement({0.0f, 0.0f});
        return;
      }

      mIsDead = false;
      mHitPoints = HIT_POINTS;
      mDamageTimer = 0.0f;
      mDeathTimer = 0.0f;
      mAttackTimer = 0.0f;
      mAttackCooldownTimer = ATTACK_COOLDOWN;
      mDidFinishAttackThisFrame = false;
      mHasFiredProjectileThisAttack = false;
      mProjectileLifetime = 0.0f;

        mSpawnPosition = mBaseSpawnPosition;

      mGorgon->setPosition(mSpawnPosition);
      mGorgon->setDirection(LEFT);
      mGorgon->setMovement({0.0f, 0.0f});
      mGorgon->activate();
      if (mAttackProjectile) mAttackProjectile->deactivate();
      setAnimation(GORGON_IDLE);
    }

    return;
  }

  if (mAttackCooldownTimer > 0.0f)
    mAttackCooldownTimer -= deltaTime;

  if (mDamageTimer > 0.0f)
  {
    mDamageTimer -= deltaTime;
  }

  {
    Vector2 playerTrackingPosition = player->getPosition();
    playerTrackingPosition.y += PLAYER_TRACKING_Y_OFFSET;
    Vector2 toPlayer = Vector2Subtract(playerTrackingPosition, mGorgon->getPosition());
    float distanceToPlayer = GetLength(toPlayer);
    bool isAttackInProgress = (mAttackTimer > 0.0f);
    bool isPlayerInAttackRange = (distanceToPlayer <= ATTACK_DETECTION_RANGE);
    bool isPlayerInDetectionRange = (distanceToPlayer <= DETECTION_RANGE);
    const float Y_ALIGNMENT_THRESHOLD = 10.0f;

    if (toPlayer.x < 0.0f)      mGorgon->setDirection(RIGHT);
    else if (toPlayer.x > 0.0f) mGorgon->setDirection(LEFT);

    if (!isAttackInProgress && isPlayerInAttackRange && mAttackCooldownTimer <= 0.0f)
    {
      mAttackTimer = ATTACK_DURATION;
      mHasFiredProjectileThisAttack = false;
      isAttackInProgress = true;
    }

    if (isAttackInProgress)
    {
      setAnimation(GORGON_ATTACKING);
      mGorgon->setMovement({0.0f, 0.0f});
    }
    else if (isPlayerInAttackRange || isPlayerInDetectionRange)
    {
      setAnimation(GORGON_MOVING);

      Vector2 movement = {0.0f, 0.0f};

      if (fabsf(toPlayer.y) > Y_ALIGNMENT_THRESHOLD)
      {
        movement.y = (toPlayer.y < 0.0f) ? -1.0f : 1.0f;
      }
      else if (fabsf(toPlayer.x) > 0.0f)
      {
        movement.x = (toPlayer.x < 0.0f) ? -1.0f : 1.0f;
      }

      mGorgon->setMovement(movement);

    }
    /*
    else
    {
      setAnimation(GORGON_IDLE);
      mGorgon->setMovement({0.0f, 0.0f});
    }*/
  }

  if (mAttackTimer > 0.0f)
  {
    mAttackTimer -= deltaTime;

    if (!mHasFiredProjectileThisAttack &&
        mAttackTimer <= PROJECTILE_SPAWN_TRIGGER_TIME)
    {
      spawnAttackProjectile();
    }

    if (mAttackTimer <= 0.0f)
    {
      mAttackTimer = 0.0f;
      mDidFinishAttackThisFrame = true;
      mAttackCooldownTimer = ATTACK_COOLDOWN;
      mHasFiredProjectileThisAttack = false;

      if (mDamageTimer <= 0.0f)
        setAnimation(GORGON_IDLE);
    }
  }

  if (mAttackProjectile && mAttackProjectile->isActive())
  {
    mAttackProjectile->update(deltaTime, nullptr, map, nullptr, 0);
    mProjectileLifetime -= deltaTime;

    bool didProjectileExpireOrCollide =
        (mProjectileLifetime <= 0.0f ||
         mAttackProjectile->isCollidingLeft() ||
         mAttackProjectile->isCollidingRight() ||
         mAttackProjectile->isCollidingTop() ||
         mAttackProjectile->isCollidingBottom());

    if (didProjectileExpireOrCollide)
    {
      mAttackProjectile->deactivate();
    }
  }

  mGorgon->update(deltaTime, player, map, nullptr, 0);
}

bool GorgonEnemy::processBulletHit(Entity *bullet)
{
  if (mGorgon == nullptr || bullet == nullptr) return false;
  if (!mGorgon->isActive() || mIsDead) return false;
  if (!bullet->hasCollidedWith(mGorgon)) return false;

  mHitPoints--;

  if (mHitPoints <= 0)
  {
    mIsDead = true;
    deathAnimationHelper();
    setAnimation(GORGON_DYING);
    mGorgon->setMovement({0.0f, 0.0f});
    if (mAttackProjectile) mAttackProjectile->deactivate();
  }
  return true;
}

bool GorgonEnemy::shouldDamagePlayer(Entity *player)
{
  if (mGorgon == nullptr || player == nullptr) return false;
  if (!mGorgon->isActive() || mIsDead) return false;

  if (mAttackProjectile == nullptr || !mAttackProjectile->isActive()) return false;
  if (!mAttackProjectile->hasCollidedWith(player)) return false;

  mAttackProjectile->deactivate();
  return true;
}

void GorgonEnemy::render()
{
  if (mGorgon) mGorgon->render();
  if (mAttackProjectile && mAttackProjectile->isActive()) mAttackProjectile->render();
}

void GorgonEnemy::shutdown()
{
  delete mAttackProjectile;
  mAttackProjectile = nullptr;

  delete mGorgon;
  mGorgon = nullptr;
}

bool GorgonEnemy::isActive() const
{
  return mGorgon != nullptr && mGorgon->isActive();
}

int GorgonEnemy::getHitPoints() const
{
  return mHitPoints;
}

int GorgonEnemy::getMaxHitPoints() const
{
  return HIT_POINTS;
}

void GorgonEnemy::setAutoRespawn(bool shouldRespawn)
{
  mAutoRespawn = shouldRespawn;
}
