#include "Scene.h"
#include "SkeletonEnemy.h"

#ifndef LEVELC_H
#define LEVELC_H

constexpr int LEVEL_C_WIDTH = 21, LEVEL_C_HEIGHT = 5;

class LevelC : public Scene {
private:
	SkeletonEnemy *mSkeletonEnemy = nullptr;
	Entity *mPrincess = nullptr;

	bool mHasGun = false;
	bool mIsShooting = false;
	bool mIsDashing = false;
	bool mIsReloading = false;
	int mBulletsRemaining = 0;
	float mReloadTimer = 0.0f;
	std::map<Direction, std::vector<int>> mNormalWalkAnimationAtlas;
	std::map<Direction, std::vector<int>> mDashAnimationAtlas;
	std::map<Direction, std::vector<int>> mGunDashAnimationAtlas;
	std::map<Direction, std::vector<int>> mWalkGunAnimationAtlas;
	std::map<Direction, std::vector<int>> mGunAnimationAtlas;
	std::map<Direction, std::vector<int>> mGunDeathAnimationAtlas;

	bool mIsLevelTransitioning = false;
	float mLevelTransitionTimer = 0.0f;
	bool mIsLifeLossTransitioning = false;
	float mLifeLossTransitionTimer = 0.0f;
	bool mIsPlayerDying = false;
	float mPlayerDeathAnimTimer = 0.0f;
	int mPendingLifeLossSceneID = NO_SCENE;
	float mBulletFireCooldown = 0.0f;
	Sound mLevelClearSound = {};
	std::vector<Entity *> mBullets;
	std::vector<float> mBulletLifetimes;
	int mTutorialStep = 0;

	unsigned int mLevelData[LEVEL_C_WIDTH * LEVEL_C_HEIGHT] = {
			5,  27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 47, 48, 6,
			16, 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 1,  1,  1,  1,  1,  1,  1,  14,
			16, 1,  1,  1, 1, 1,  1,  1,  1,  1,  1,  1,  1,  1, 1,  1,  1,  1,  1,  1, 14,
			16, 1,  1,  1,  1,  1,  1,  1,  1,  1, 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  14,
			17, 3,  3,  3,  3,  3, 3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  18};

public:
	static constexpr float TILE_DIMENSION = 75.0f, ACCELERATION_OF_GRAVITY = 0.0f;
	static constexpr int PLAYER_NORMAL_SPEED = 200;
	static constexpr int PLAYER_DASH_SPEED = 430;
	static constexpr int MAX_BULLETS = 30;
	static constexpr float BULLET_SPEED = 900.0f;
	static constexpr float BULLET_LIFETIME = 1.2f;
	static constexpr float BULLET_FIRE_INTERVAL = 0.09f;
	static constexpr float RELOAD_DURATION = 1.0f;
	static constexpr float BULLET_SCALE = 42.0f;
	static constexpr float LIFE_LOSS_DELAY = 0.30f;
	static constexpr float PLAYER_DEATH_ANIM_DURATION = 0.95f;

	LevelC();
	LevelC(Vector2 origin, const char *bgHexCode);
	~LevelC();

	void initialise() override;
	void update(float deltaTime) override;
	void render() override;
	void shutdown() override;
};

#endif
