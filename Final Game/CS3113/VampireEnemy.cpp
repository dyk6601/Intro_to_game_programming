#include "VampireEnemy.h"

VampireEnemy::VampireEnemy() {}

VampireEnemy::~VampireEnemy() { shutdown(); }

void VampireEnemy::initialise(Vector2 origin)
{
  initialise(origin, {origin.x + 60.0f, origin.y + 20.0f});
}

void VampireEnemy::initialise(Vector2 origin, Vector2 spawnPosition)
{
  shutdown();

  mAnimationState = VAMPIRE_IDLE;
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
      {DOWN, {0, 1, 2, 3, 4, 5, 6, 7}},
      {LEFT, {0, 1, 2, 3, 4, 5, 6, 7}},
      {UP, {0, 1, 2, 3, 4, 5, 6, 7}},
      {RIGHT, {0, 1, 2, 3, 4, 5, 6, 7}},
  };

  mAttackAnimationAtlas = {
      {DOWN, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}},
      {LEFT, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}},
      {UP, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}},
      {RIGHT, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}},
  };

  mTakeDamageAnimationAtlas = {
      {DOWN, {0, 1, 2, 3, 4}},
      {LEFT, {0, 1, 2, 3, 4}},
      {UP, {0, 1, 2, 3, 4}},
      {RIGHT, {0, 1, 2, 3, 4}},
  };

  mDeathAnimationAtlas = {
      {DOWN, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13}},
      {LEFT, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13}},
      {UP, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13}},
      {RIGHT, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13}},
  };

  mVampire = new Entity(
      mSpawnPosition,
      {150.0f, 150.0f},
      "assets/game/vampire/enemies-vampire_idle.png",
      ATLAS,
      {1, 6},
      mIdleAnimationAtlas,
      PLAYER
  );

  mVampire->setColliderDimensions(
      {mVampire->getScale().x * 0.42f, mVampire->getScale().y * 0.70f});
  mVampire->setAcceleration({0.0f, 0.0f});
  mVampire->setSpeed(SPEED);
  mVampire->setFrameSpeed(12);
  mVampire->setDirection(LEFT);
}

void VampireEnemy::setAnimation(int newState)
{
  if (mVampire == nullptr || mAnimationState == newState) return;

  mAnimationState = newState;

  if (newState == VAMPIRE_IDLE)
  {
    mVampire->setTexture("assets/game/vampire/enemies-vampire_idle.png");
    mVampire->setSpriteSheetDimensions({1, 6});
    mVampire->setAnimationAtlas(mIdleAnimationAtlas);
    mVampire->setFrameSpeed(10);
    mVampire->setAnimateWhenStill(true);
  }
  else if (newState == VAMPIRE_MOVING)
  {
    mVampire->setTexture("assets/game/vampire/enemies-vampire_movement.png");
    mVampire->setSpriteSheetDimensions({1, 8});
    mVampire->setAnimationAtlas(mMoveAnimationAtlas);
    mVampire->setFrameSpeed(12);
    mVampire->setAnimateWhenStill(false);
  }
  else if (newState == VAMPIRE_ATTACKING)
  {
    mVampire->setTexture("assets/game/vampire/enemies-vampire_attack.png");
    mVampire->setSpriteSheetDimensions({1, 16});
    mVampire->setAnimationAtlas(mAttackAnimationAtlas);
    mVampire->setFrameSpeed(15);
    mVampire->setAnimateWhenStill(true);
  }
  else if (newState == VAMPIRE_TAKING_DAMAGE)
  {
    mVampire->setTexture("assets/game/vampire/enemies-vampire_take_damage.png");
    mVampire->setSpriteSheetDimensions({1, 5});
    mVampire->setAnimationAtlas(mTakeDamageAnimationAtlas);
    mVampire->setFrameSpeed(14);
    mVampire->setAnimateWhenStill(true);
  }
  else if (newState == VAMPIRE_DYING)
  {
    mVampire->setTexture("assets/game/vampire/enemies-vampire_death.png");
    mVampire->setSpriteSheetDimensions({1, 14});
    mVampire->setAnimationAtlas(mDeathAnimationAtlas);
    mVampire->setFrameSpeed(14);
    mVampire->setAnimateWhenStill(true);
  }
}

void VampireEnemy::update(float deltaTime, Entity *player, Map *map)
{
  if (mVampire == nullptr || player == nullptr) return;

  mDidFinishAttackThisFrame = false;

  if (!mVampire->isActive()) return;

  if (mIsDead)
  {
    setAnimation(VAMPIRE_DYING);
    mVampire->setMovement({0.0f, 0.0f});
    mDeathTimer -= deltaTime;
    mVampire->update(deltaTime, player, map, nullptr, 0);

    if (mDeathTimer <= 0.0f)
    {
      if (!mAutoRespawn)
      {
        mVampire->deactivate();
        mVampire->setMovement({0.0f, 0.0f});
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

      mVampire->setPosition(mSpawnPosition);
      mVampire->setDirection(LEFT);
      mVampire->setMovement({0.0f, 0.0f});
      mVampire->activate();
      setAnimation(VAMPIRE_IDLE);
    }

    return;
  }

  if (mAttackCooldownTimer > 0.0f)
    mAttackCooldownTimer -= deltaTime;

  if (mDamageTimer > 0.0f)
  {
    mDamageTimer -= deltaTime;
    setAnimation(VAMPIRE_TAKING_DAMAGE);
    ///mVampire->setMovement({0.0f, 0.0f});
  }
  else
  {
    Vector2 toPlayer = Vector2Subtract(player->getPosition(), mVampire->getPosition());
    float distanceToPlayer = GetLength(toPlayer);

    if (toPlayer.x < 0.0f)      mVampire->setDirection(RIGHT);
    else if (toPlayer.x > 0.0f) mVampire->setDirection(LEFT);

    if (distanceToPlayer <= ATTACK_RANGE)
    {
      if (mAttackTimer <= 0.0f && mAttackCooldownTimer <= 0.0f)
      {
        mAttackTimer = ATTACK_DURATION;
        mHasAppliedDamageThisAttack = false;
        setAnimation(VAMPIRE_ATTACKING);
      }
      else if (mAttackTimer > 0.0f)
      {
        setAnimation(VAMPIRE_ATTACKING);
      }
      else
      {
        setAnimation(VAMPIRE_IDLE);
      }

      mVampire->setMovement({0.0f, 0.0f});
    }
    else if (distanceToPlayer <= DETECTION_RANGE)
    {
      mAttackTimer = 0.0f;
      mHasAppliedDamageThisAttack = false;
      setAnimation(VAMPIRE_MOVING);

      if (GetLength(toPlayer) > 0.0f)
        Normalise(&toPlayer);

      mVampire->setMovement(toPlayer);
    }
    else
    {
      mAttackTimer = 0.0f;
      mHasAppliedDamageThisAttack = false;
      setAnimation(VAMPIRE_IDLE);
      mVampire->setMovement({0.0f, 0.0f});
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
        setAnimation(VAMPIRE_IDLE);
    }
  }

  mVampire->update(deltaTime, player, map, nullptr, 0);
}

bool VampireEnemy::isPlayerInAttackHitbox(Entity *player) const
{
  if (mVampire == nullptr || player == nullptr) return false;

  Vector2 vampirePosition = mVampire->getPosition();
  Vector2 playerPosition = player->getPosition();

  Vector2 vampireCollider = mVampire->getColliderDimensions();
  Vector2 playerCollider = player->getColliderDimensions();

  float forwardOffset = 0.0f;
  if (mVampire->getDirection() == LEFT) forwardOffset = ATTACK_HITBOX_FORWARD_OFFSET;
  else if (mVampire->getDirection() == RIGHT) forwardOffset = -ATTACK_HITBOX_FORWARD_OFFSET;

  float hitboxCenterX = vampirePosition.x + forwardOffset;
  float hitboxCenterY = vampirePosition.y;

  float reachX = ((vampireCollider.x + playerCollider.x) / 2.0f) + ATTACK_HITBOX_X_PADDING;
  float reachY = ((vampireCollider.y + playerCollider.y) / 2.0f) + ATTACK_HITBOX_Y_PADDING;

  float deltaX = fabs(playerPosition.x - hitboxCenterX);
  float deltaY = fabs(playerPosition.y - hitboxCenterY);

  return (deltaX <= reachX && deltaY <= reachY);
}

bool VampireEnemy::processBulletHit(Entity *bullet)
{
  if (mVampire == nullptr || bullet == nullptr) return false;
  if (!mVampire->isActive() || mIsDead) return false;
  if (!bullet->hasCollidedWith(mVampire)) return false;

  mHitPoints--;

  if (mHitPoints <= 0)
  {
    mIsDead = true;
    mDeathTimer = DEATH_ANIM_DURATION;
    setAnimation(VAMPIRE_DYING);
    mVampire->setMovement({0.0f, 0.0f});
  }
  else
  {
    mDamageTimer = DAMAGE_ANIM_DURATION;
    setAnimation(VAMPIRE_TAKING_DAMAGE);
    mVampire->setMovement({0.0f, 0.0f});
  }

  return true;
}

bool VampireEnemy::shouldDamagePlayer(Entity *player)
{
  if (mVampire == nullptr || player == nullptr) return false;
  if (!mVampire->isActive() || mIsDead) return false;

  if (mAnimationState != VAMPIRE_ATTACKING) return false;
  if (mHasAppliedDamageThisAttack) return false;
  if (mAttackTimer <= 0.0f || mAttackTimer > ATTACK_DURATION * 0.13f) return false;
  if (!isPlayerInAttackHitbox(player)) return false;

  mHasAppliedDamageThisAttack = true;
  return true;
}

void VampireEnemy::render()
{
  if (mVampire) mVampire->render();
}

void VampireEnemy::shutdown()
{
  delete mVampire;
  mVampire = nullptr;
}

bool VampireEnemy::isActive() const
{
  return mVampire != nullptr && mVampire->isActive();
}

void VampireEnemy::setAutoRespawn(bool shouldRespawn)
{
  mAutoRespawn = shouldRespawn;
}
