#include "LevelC.h"

LevelC::LevelC()                                      : Scene { {0.0f}, nullptr   } {}
LevelC::LevelC(Vector2 origin, const char *bgHexCode) : Scene { origin, bgHexCode } {}

LevelC::~LevelC() { shutdown(); }

void LevelC::initialise()
{
	mGameState.nextSceneID = NO_SCENE;
	mIsLevelTransitioning = false;
	mLevelTransitionTimer = 0.0f;

	mGameState.bgm = LoadMusicStream("assets/game/time_for_adventure.mp3");
	SetMusicVolume(mGameState.bgm, 0.33f);
	PlayMusicStream(mGameState.bgm);
	mGameState.jumpSound = LoadSound("assets/game/Jump.wav");
	mLevelClearSound = LoadSound("assets/game/level_up.mp3");

	/*
		----------- MAP -----------
	*/
	mGameState.map = new Map(
		LEVEL_C_WIDTH, LEVEL_C_HEIGHT,   // map grid cols & rows
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
		{mOrigin.x - 1000.0f, mOrigin.y - 200.0f}, // position
		{140.0f * sizeRatio * widthBoost, 140.0f},// scale
		"assets/game/walking.png",                // texture file address
		ATLAS,                                    // single image or atlas?
		{ 3, 4 },                                 // atlas dimensions
		xochitlAnimationAtlas,                    // actual atlas
		PLAYER                                    // entity type
	);

	mGameState.xochitl->setJumpingPower(565.0f);
	mGameState.xochitl->setColliderDimensions({
		mGameState.xochitl->getScale().x / 3.0f,
		mGameState.xochitl->getScale().y / 2.0f
	});
	mGameState.xochitl->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});

	// ----------- red shroom -----------
	std::map<Direction, std::vector<int>> shroomAnimationAtlas = {
		{DOWN,  {0, 1}},
		{LEFT,  {0, 1}},
		{RIGHT, {0, 1}},
		{UP,    {0, 1}},
	};

	mRedShroom = new Entity(
		{mOrigin.x -730.0f, mOrigin.y - 60.0f}, // position
		{110.0f, 70.0f},                         // scale
		"assets/game/red_shroom.png",           // texture
		ATLAS,                                   // animated atlas
		{1, 2},                                  // 1 row, 2 frames
		shroomAnimationAtlas,
		NPC                                      // entity type
	);

	mRedShroom->setColliderDimensions({
		mRedShroom->getScale().x * 0.60f,
		mRedShroom->getScale().y * 0.70f
	});
	mRedShroom->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});
	mRedShroom->setSpeed(170);
	mRedShroom->setFrameSpeed(10);
	mRedShroom->setAIType(FOLLOWER);
	mRedShroom->setAIState(IDLE);
	mRedShroom->setDirection(LEFT);
}

void LevelC::update(float deltaTime)
{
	UpdateMusicStream(mGameState.bgm);

	if (mIsLevelTransitioning)
	{
		mLevelTransitionTimer -= deltaTime;
		if (mLevelTransitionTimer <= 0.0f) mGameState.nextSceneID = WIN_SCENE;
		return;
	}

	mGameState.xochitl->update(
		deltaTime,      // delta time / fixed timestep
		nullptr,        // player
		mGameState.map, // map
		nullptr,        // collidable entities
		0               // col. entity count
	);

	if (mRedShroom)
	{
		mRedShroom->update(
			deltaTime,
			mGameState.xochitl,
			mGameState.map,
			nullptr,
			0
		);

		if (mRedShroom->isCollidingLeft())       mRedShroom->setDirection(RIGHT);
		else if (mRedShroom->isCollidingRight()) mRedShroom->setDirection(LEFT);
	}

	// death trigger
	if (mGameState.xochitl->getPosition().y > END_GAME_THRESHOLD && mGameState.sharedLives)
	{
		(*mGameState.sharedLives)--;

		if (*mGameState.sharedLives <= 0) mGameState.nextSceneID = LOSE_SCENE;
		else mGameState.nextSceneID = LEVEL_C_SCENE;
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
		if (currentTile == 7)
		{
			PlaySound(mLevelClearSound);
			mIsLevelTransitioning = true;
			mLevelTransitionTimer = 0.9f;
		}
	}
}

void LevelC::render()
{
	ClearBackground(ColorFromHex(mBGColourHexCode));

	mGameState.map->render();
	if (mRedShroom) mRedShroom->render();
	mGameState.xochitl->render();

	if (mGameState.sharedLives)
	{
		float uiOffsetX = -520.0f;
		DrawText(TextFormat("Lives: %d", *mGameState.sharedLives), (int)(mGameState.xochitl->getPosition().x + uiOffsetX), 550, 30, WHITE);
	}
	float uiOffsetX = -150.0f;
	DrawText(TextFormat("Levels Press: 1,2,3"), (int)(mGameState.xochitl->getPosition().x + uiOffsetX), 550, 30, GREEN);
    float OffsetX = -150.0f;
	DrawText(TextFormat("What you see is not everything"), (int)(mGameState.xochitl->getPosition().x + OffsetX), 0, 30, BLUE);
}

void LevelC::shutdown()
{
	delete mRedShroom;
	mRedShroom = nullptr;

	delete mGameState.xochitl;
	delete mGameState.map;

	UnloadMusicStream(mGameState.bgm);
	UnloadSound(mGameState.jumpSound);
	UnloadSound(mLevelClearSound);
}
