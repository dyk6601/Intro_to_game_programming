#ifndef GORGON_ENEMY_H
#define GORGON_ENEMY_H

#include "Entity.h"

class GorgonEnemy {
private:
  Entity *mGorgon = nullptr;
  Entity *mAttackProjectile = nullptr;
  int mAnimationState = 0;

  int mHitPoints = 20;
  bool mIsDead = false;

  Vector2 mBaseSpawnPosition = {0.0f, 0.0f};
  Vector2 mSpawnPosition = {0.0f, 0.0f};

  float mDamageTimer = 0.0f;
  float mDeathTimer = 0.0f;
  float mAttackTimer = 0.0f;
  float mAttackCooldownTimer = 0.0f;

  bool mDidFinishAttackThisFrame = false;
  bool mAutoRespawn = true;
  bool mHasFiredProjectileThisAttack = false;
  float mProjectileLifetime = 0.0f;

  std::map<Direction, std::vector<int>> mIdleAnimationAtlas;
  std::map<Direction, std::vector<int>> mMoveAnimationAtlas;
  std::map<Direction, std::vector<int>> mAttackAnimationAtlas;
  std::map<Direction, std::vector<int>> mTakeDamageAnimationAtlas;
  std::map<Direction, std::vector<int>> mDeathAnimationAtlas;

  static constexpr int HIT_POINTS = 75;
  static constexpr int SPEED = 165;
  static constexpr int GORGON_IDLE = 0;
  static constexpr int GORGON_MOVING = 1;
  static constexpr int GORGON_ATTACKING = 2;
  static constexpr int GORGON_TAKING_DAMAGE = 3;
  static constexpr int GORGON_DYING = 4;
  
  static constexpr float ATTACK_DETECTION_RANGE = 520.0f;
  static constexpr float DETECTION_RANGE = 2560.0f;
  static constexpr int DEATH_FRAME_SPEED = 1;
  static constexpr float ATTACK_DURATION = 0.90f;
  static constexpr float ATTACK_COOLDOWN = 3.00f;
  static constexpr float PLAYER_TRACKING_Y_OFFSET = 28.0f;
  static constexpr float PROJECTILE_SPEED = 560.0f;
  static constexpr float PROJECTILE_LIFETIME = 1.25f;
  static constexpr float PROJECTILE_SPAWN_TRIGGER_TIME = 0.06f;
  static constexpr float PROJECTILE_FORWARD_OFFSET = 80.0f;
  static constexpr float PROJECTILE_SCALE = 180.0f;
  static constexpr float PROJECTILE_HITBOX_SCALE = 0.08f;

  void setAnimation(int newState);
  void deathAnimationHelper();
  void spawnAttackProjectile();

public:
  GorgonEnemy();
  ~GorgonEnemy();

  void initialise(Vector2 origin);
  void initialise(Vector2 origin, Vector2 spawnPosition);
  void update(float deltaTime, Entity *player, Map *map);
  void render();
  void shutdown();

  bool processBulletHit(Entity *bullet);
  bool shouldDamagePlayer(Entity *player);
  bool isActive() const;
  int getHitPoints() const;
  int getMaxHitPoints() const;
  void setAutoRespawn(bool shouldRespawn);
};

#endif
