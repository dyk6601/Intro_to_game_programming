#include "SkeletonEnemy.h"

SkeletonEnemy::SkeletonEnemy() {}

SkeletonEnemy::~SkeletonEnemy() { shutdown(); }

void SkeletonEnemy::initialise(Vector2 origin)
{
  initialise(origin, {origin.x + 350.0f, origin.y - 200.0f});
}

void SkeletonEnemy::initialise(Vector2 origin, Vector2 spawnPosition)
{
  shutdown();

  mAnimationState = SKELETON_IDLE;
  mHitPoints = HIT_POINTS;
  mIsDead = false;

  mBaseSpawnPosition = spawnPosition;
  mSpawnPosition = mBaseSpawnPosition;

  mDamageTimer = 0.0f;
  mDeathTimer = 0.0f;
  mAttackTimer = 0.0f;
  mAttackCooldownTimer = 0.0f;

  mHasAppliedDamageThisAttack = false;
  mDidFinishAttackThisFrame = false;
  mAutoRespawn = true;

  mIdleAnimationAtlas = {
      {DOWN, {0, 1, 2, 3, 4, 5}},
      {LEFT, {0, 1, 2, 3, 4, 5}},
      {UP, {0, 1, 2, 3, 4, 5}},
      {RIGHT, {0, 1, 2, 3, 4, 5}},
  };

  mMoveAnimationAtlas = {
      {DOWN, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}},
      {LEFT, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}},
      {UP, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}},
      {RIGHT, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}},
  };

  mAttackAnimationAtlas = {
      {DOWN, {0, 1, 2, 3, 4, 5, 6, 7, 8}},
      {LEFT, {0, 1, 2, 3, 4, 5, 6, 7, 8}},
      {UP, {0, 1, 2, 3, 4, 5, 6, 7, 8}},
      {RIGHT, {0, 1, 2, 3, 4, 5, 6, 7, 8}},
  };

  mTakeDamageAnimationAtlas = {
      {DOWN, {0, 1, 2, 3, 4}},
      {LEFT, {0, 1, 2, 3, 4}},
      {UP, {0, 1, 2, 3, 4}},
      {RIGHT, {0, 1, 2, 3, 4}},
  };

  mDeathAnimationAtlas = {
      {DOWN, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}},
      {LEFT, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}},
      {UP, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}},
      {RIGHT, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}},
  };

  mSkeleton = new Entity(
      mSpawnPosition,
      {150.0f, 150.0f},
      "assets/game/skeleton/enemies-skeleton1_idle.png",
      ATLAS,
      {1, 6},
      mIdleAnimationAtlas,
      PLAYER
  );

  mSkeleton->setColliderDimensions(
      {mSkeleton->getScale().x * 0.42f, mSkeleton->getScale().y * 0.70f});
  mSkeleton->setAcceleration({0.0f, 0.0f});
  mSkeleton->setSpeed(SPEED);
  mSkeleton->setFrameSpeed(12);
  mSkeleton->setDirection(LEFT);
}

void SkeletonEnemy::setAnimation(int newState)
{
  if (mSkeleton == nullptr || mAnimationState == newState) return;

  mAnimationState = newState;

  if (newState == SKELETON_IDLE)
  {
    mSkeleton->setTexture("assets/game/skeleton/enemies-skeleton1_idle.png");
    mSkeleton->setSpriteSheetDimensions({1, 6});
    mSkeleton->setAnimationAtlas(mIdleAnimationAtlas);
    mSkeleton->setFrameSpeed(10);
    mSkeleton->setAnimateWhenStill(true);
  }
  else if (newState == SKELETON_MOVING)
  {
    mSkeleton->setTexture("assets/game/skeleton/enemies-skeleton1_movement.png");
    mSkeleton->setSpriteSheetDimensions({1, 10});
    mSkeleton->setAnimationAtlas(mMoveAnimationAtlas);
    mSkeleton->setFrameSpeed(12);
    mSkeleton->setAnimateWhenStill(false);
  }
  else if (newState == SKELETON_ATTACKING)
  {
    mSkeleton->setTexture("assets/game/skeleton/enemies-skeleton1_attack.png");
    mSkeleton->setSpriteSheetDimensions({1, 9});
    mSkeleton->setAnimationAtlas(mAttackAnimationAtlas);
    mSkeleton->setFrameSpeed(14);
    mSkeleton->setAnimateWhenStill(true);
  }
  else if (newState == SKELETON_TAKING_DAMAGE)
  {
    mSkeleton->setTexture("assets/game/skeleton/enemies-skeleton1_take_damage.png");
    mSkeleton->setSpriteSheetDimensions({1, 5});
    mSkeleton->setAnimationAtlas(mTakeDamageAnimationAtlas);
    mSkeleton->setFrameSpeed(14);
    mSkeleton->setAnimateWhenStill(true);
  }
  else if (newState == SKELETON_DYING)
  {
    mSkeleton->setTexture("assets/game/skeleton/enemies-skeleton1_death.png");
    mSkeleton->setSpriteSheetDimensions({1, 17});
    mSkeleton->setAnimationAtlas(mDeathAnimationAtlas);
    mSkeleton->setFrameSpeed(14);
    mSkeleton->setAnimateWhenStill(true);
  }
}

void SkeletonEnemy::update(float deltaTime, Entity *player, Map *map)
{
  if (mSkeleton == nullptr || player == nullptr) return;

  mDidFinishAttackThisFrame = false;

  if (!mSkeleton->isActive()) return;

  if (mIsDead)
  {
    setAnimation(SKELETON_DYING);
    mSkeleton->setMovement({0.0f, 0.0f});
    mDeathTimer -= deltaTime;
    mSkeleton->update(deltaTime, player, map, nullptr, 0);

    if (mDeathTimer <= 0.0f)
    {
      if (!mAutoRespawn)
      {
        mSkeleton->deactivate();
        mSkeleton->setMovement({0.0f, 0.0f});
        return;
      }

      mIsDead = false;
      mHitPoints = HIT_POINTS;
      mDamageTimer = 0.0f;
      mDeathTimer = 0.0f;
      mAttackTimer = 0.0f;
      mAttackCooldownTimer = ATTACK_COOLDOWN;
      mHasAppliedDamageThisAttack = false;
      mDidFinishAttackThisFrame = false;

        mSpawnPosition = mBaseSpawnPosition;

      mSkeleton->setPosition(mSpawnPosition);
      mSkeleton->setDirection(LEFT);
      mSkeleton->setMovement({0.0f, 0.0f});
      mSkeleton->activate();
      setAnimation(SKELETON_IDLE);
    }

    return;
  }

  if (mAttackCooldownTimer > 0.0f)
    mAttackCooldownTimer -= deltaTime;

  if (mDamageTimer > 0.0f)
  {
    mDamageTimer -= deltaTime;
    setAnimation(SKELETON_TAKING_DAMAGE);
    mSkeleton->setMovement({0.0f, 0.0f});
  }
  else
  {
    Vector2 toPlayer = Vector2Subtract(player->getPosition(), mSkeleton->getPosition());
    float distanceToPlayer = GetLength(toPlayer);

    if (toPlayer.x < 0.0f)      mSkeleton->setDirection(RIGHT);
    else if (toPlayer.x > 0.0f) mSkeleton->setDirection(LEFT);

    if (distanceToPlayer <= ATTACK_RANGE)
    {
      if (mAttackTimer <= 0.0f && mAttackCooldownTimer <= 0.0f)
      {
        mAttackTimer = ATTACK_DURATION;
        mHasAppliedDamageThisAttack = false;
        setAnimation(SKELETON_ATTACKING);
      }
      else if (mAttackTimer > 0.0f)
      {
        setAnimation(SKELETON_ATTACKING);
      }
      else
      {
        setAnimation(SKELETON_IDLE);
      }

      mSkeleton->setMovement({0.0f, 0.0f});
    }
    else if (distanceToPlayer <= DETECTION_RANGE)
    {
      mAttackTimer = 0.0f;
      mHasAppliedDamageThisAttack = false;
      setAnimation(SKELETON_MOVING);

      if (GetLength(toPlayer) > 0.0f)
        Normalise(&toPlayer);

      mSkeleton->setMovement(toPlayer);
    }
    else
    {
      mAttackTimer = 0.0f;
      mHasAppliedDamageThisAttack = false;
      setAnimation(SKELETON_IDLE);
      mSkeleton->setMovement({0.0f, 0.0f});
    }
  }

  if (mAttackTimer > 0.0f)
  {
    mAttackTimer -= deltaTime;

    if (mAttackTimer <= 0.0f)
    {
      mAttackTimer = 0.0f;
      mDidFinishAttackThisFrame = true;
      mAttackCooldownTimer = ATTACK_COOLDOWN;
      mHasAppliedDamageThisAttack = false;

      if (mDamageTimer <= 0.0f)
        setAnimation(SKELETON_IDLE);
    }
  }

  mSkeleton->update(deltaTime, player, map, nullptr, 0);
}

bool SkeletonEnemy::isPlayerInAttackHitbox(Entity *player) const
{
  if (mSkeleton == nullptr || player == nullptr) return false;

  Vector2 skeletonPosition = mSkeleton->getPosition();
  Vector2 playerPosition = player->getPosition();

  Vector2 skeletonCollider = mSkeleton->getColliderDimensions();
  Vector2 playerCollider = player->getColliderDimensions();

  float forwardOffset = 0.0f;
  // Keep hitbox direction aligned with the visual-facing compensation above.
  if (mSkeleton->getDirection() == LEFT) forwardOffset = ATTACK_HITBOX_FORWARD_OFFSET;
  else if (mSkeleton->getDirection() == RIGHT) forwardOffset = -ATTACK_HITBOX_FORWARD_OFFSET;

  float hitboxCenterX = skeletonPosition.x + forwardOffset;
  float hitboxCenterY = skeletonPosition.y;

  float reachX = ((skeletonCollider.x + playerCollider.x) / 2.0f) + ATTACK_HITBOX_X_PADDING;
  float reachY = ((skeletonCollider.y + playerCollider.y) / 2.0f) + ATTACK_HITBOX_Y_PADDING;

  float deltaX = fabs(playerPosition.x - hitboxCenterX);
  float deltaY = fabs(playerPosition.y - hitboxCenterY);

  return (deltaX <= reachX && deltaY <= reachY);
}

bool SkeletonEnemy::processBulletHit(Entity *bullet)
{
  if (mSkeleton == nullptr || bullet == nullptr) return false;
  if (!mSkeleton->isActive() || mIsDead) return false;
  if (!bullet->hasCollidedWith(mSkeleton)) return false;

  mHitPoints--;

  if (mHitPoints <= 0)
  {
    mIsDead = true;
    mDeathTimer = DEATH_ANIM_DURATION;
    setAnimation(SKELETON_DYING);
    mSkeleton->setMovement({0.0f, 0.0f});
  }
  else
  {
    mDamageTimer = DAMAGE_ANIM_DURATION;
    setAnimation(SKELETON_TAKING_DAMAGE);
    mSkeleton->setMovement({0.0f, 0.0f});
  }

  return true;
}

bool SkeletonEnemy::shouldDamagePlayer(Entity *player)
{
  if (mSkeleton == nullptr || player == nullptr) return false;
  if (!mSkeleton->isActive() || mIsDead) return false;

  if (mAnimationState != SKELETON_ATTACKING) return false;
  if (mHasAppliedDamageThisAttack) return false;
  if (mAttackTimer <= 0.0f || mAttackTimer > ATTACK_DURATION * 0.10f) return false;
  if (!isPlayerInAttackHitbox(player)) return false;

  mHasAppliedDamageThisAttack = true;
  return true;
}

void SkeletonEnemy::render()
{
  if (mSkeleton) mSkeleton->render();
}

void SkeletonEnemy::shutdown()
{
  delete mSkeleton;
  mSkeleton = nullptr;
}

bool SkeletonEnemy::isActive() const
{
  return mSkeleton != nullptr && mSkeleton->isActive();
}

void SkeletonEnemy::setAutoRespawn(bool shouldRespawn)
{
  mAutoRespawn = shouldRespawn;
}
