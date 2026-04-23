#ifndef SKELETON_ENEMY_H
#define SKELETON_ENEMY_H

#include "Entity.h"

class SkeletonEnemy {
private:
  Entity *mSkeleton = nullptr;
  int mAnimationState = 0;

  int mHitPoints = 5;
  bool mIsDead = false;

  Vector2 mBaseSpawnPosition = {0.0f, 0.0f};
  Vector2 mSpawnPosition = {0.0f, 0.0f};

  float mDamageTimer = 0.0f;
  float mDeathTimer = 0.0f;
  float mAttackTimer = 0.0f;
  float mAttackCooldownTimer = 0.0f;

  bool mHasAppliedDamageThisAttack = false;
  bool mDidFinishAttackThisFrame = false;
  bool mAutoRespawn = true;

  std::map<Direction, std::vector<int>> mIdleAnimationAtlas;
  std::map<Direction, std::vector<int>> mMoveAnimationAtlas;
  std::map<Direction, std::vector<int>> mAttackAnimationAtlas;
  std::map<Direction, std::vector<int>> mTakeDamageAnimationAtlas;
  std::map<Direction, std::vector<int>> mDeathAnimationAtlas;

  static constexpr int HIT_POINTS = 5;
  static constexpr int SPEED = 145;
  static constexpr int SKELETON_IDLE = 0;
  static constexpr int SKELETON_MOVING = 1;
  static constexpr int SKELETON_ATTACKING = 2;
  static constexpr int SKELETON_TAKING_DAMAGE = 3;
  static constexpr int SKELETON_DYING = 4;
  static constexpr float ATTACK_RANGE = 105.0f;
  static constexpr float DETECTION_RANGE = 720.0f;
  static constexpr float DAMAGE_ANIM_DURATION = 0.22f;
  static constexpr float DEATH_ANIM_DURATION = 0.72f;
  static constexpr float ATTACK_DURATION = 0.66f;
  static constexpr float ATTACK_COOLDOWN = 0.45f;
  static constexpr float ATTACK_HITBOX_X_PADDING = 28.0f;
  static constexpr float ATTACK_HITBOX_Y_PADDING = 20.0f;
  static constexpr float ATTACK_HITBOX_FORWARD_OFFSET = 26.0f;

  void setAnimation(int newState);
  bool isPlayerInAttackHitbox(Entity *player) const;

public:
  SkeletonEnemy();
  ~SkeletonEnemy();

  void initialise(Vector2 origin);
  void initialise(Vector2 origin, Vector2 spawnPosition);
  void update(float deltaTime, Entity *player, Map *map);
  void render();
  void shutdown();

  bool processBulletHit(Entity *bullet);
  bool shouldDamagePlayer(Entity *player);
  bool isActive() const;
  void setAutoRespawn(bool shouldRespawn);
};

#endif