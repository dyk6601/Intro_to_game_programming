#include "Entity.h"

Entity::Entity() : mPosition {0.0f, 0.0f}, mMovement {0.0f, 0.0f},
                   mVelocity {0.0f, 0.0f}, mAcceleration {0.0f, 0.0f},
                   mScale {DEFAULT_SIZE, DEFAULT_SIZE},
                   mColliderDimensions {DEFAULT_SIZE, DEFAULT_SIZE},
                   mColliderOffset {0.0f, 0.0f},
                   mTexture {0}, mTextureType {SINGLE}, mAngle {0.0f},
                   mSpriteSheetDimensions {}, mDirection {RIGHT},
                   mAnimationAtlas {{}}, mAnimationIndices {}, mFrameSpeed {0},
                   mEntityType {NONE} { }

Entity::Entity(Vector2 position, Vector2 scale, const char *textureFilepath,
    EntityType entityType) : mPosition {position}, mVelocity {0.0f, 0.0f},
    mAcceleration {0.0f, 0.0f}, mScale {scale}, mMovement {0.0f, 0.0f},
    mColliderDimensions {scale}, mColliderOffset {0.0f, 0.0f},
    mColliderRadius {scale.x / 2.0f},
    mTexture {LoadTexture(textureFilepath)},
    mTextureType {SINGLE}, mDirection {RIGHT}, mAnimationAtlas {{}},
    mAnimationIndices {}, mFrameSpeed {0}, mSpeed {DEFAULT_SPEED},
    mAngle {0.0f}, mEntityType {entityType} { }

Entity::Entity(Vector2 position, Vector2 scale, const char *textureFilepath,
        TextureType textureType, Vector2 spriteSheetDimensions, std::map<Direction,
        std::vector<int>> animationAtlas, EntityType entityType) :
        mPosition {position}, mVelocity {0.0f, 0.0f},
        mAcceleration {0.0f, 0.0f}, mMovement { 0.0f, 0.0f }, mScale {scale},
        mColliderDimensions {scale}, mColliderOffset {0.0f, 0.0f},
        mColliderRadius {scale.x / 2.0f},
        mTexture {LoadTexture(textureFilepath)},
        mTextureType {ATLAS}, mSpriteSheetDimensions {spriteSheetDimensions},
        mAnimationAtlas {animationAtlas}, mDirection {RIGHT},
        mAnimationIndices {animationAtlas.at(RIGHT)},
        mFrameSpeed {DEFAULT_FRAME_SPEED}, mAngle { 0.0f },
        mSpeed { DEFAULT_SPEED }, mEntityType {entityType} { }

Entity::~Entity() { UnloadTexture(mTexture); };

Vector2 Entity::getColliderCenter() const
{
    return {mPosition.x + mColliderOffset.x, mPosition.y + mColliderOffset.y};
}

Vector2 Entity::getColliderHalfExtents() const
{
    if (mColliderType == CIRCLE) return {mColliderRadius, mColliderRadius};

    return {mColliderDimensions.x / 2.0f, mColliderDimensions.y / 2.0f};
}

void Entity::checkCollisionY(Entity *collidableEntities, int collisionCheckCount)
{
    for (int i = 0; i < collisionCheckCount; i++)
    {
        Entity *collidableEntity = &collidableEntities[i];

        if (isColliding(collidableEntity))
        {
            if (collidableEntity->mEntityType == BLOCK)
                mTouchedBlock = true;
            else if (collidableEntity->mEntityType == WINNING_PLATFORM)
                mTouchedWinningPlatform = true;

            Vector2 myCenter = getColliderCenter();
            Vector2 otherCenter = collidableEntity->getColliderCenter();
            Vector2 myHalf = getColliderHalfExtents();
            Vector2 otherHalf = collidableEntity->getColliderHalfExtents();

            float myY = myCenter.y;
            float otherY = otherCenter.y;
            float yDistance = fabs(myY - otherY);
            float yOverlap  = fabs(yDistance - myHalf.y - otherHalf.y);

            if (mVelocity.y > 0)
            {
                mPosition.y -= yOverlap;
                mVelocity.y  = 0;
                mIsCollidingBottom = true;
            } else if (mVelocity.y < 0)
            {
                mPosition.y += yOverlap;
                mVelocity.y  = 0;
                mIsCollidingTop = true;

                if (collidableEntity->mEntityType == BLOCK)
                    collidableEntity->deactivate();
            }
        }
    }
}

void Entity::checkCollisionX(Entity *collidableEntities, int collisionCheckCount)
{
    for (int i = 0; i < collisionCheckCount; i++)
    {
        Entity *collidableEntity = &collidableEntities[i];

        if (isColliding(collidableEntity))
        {
            if (collidableEntity->mEntityType == BLOCK)
                mTouchedBlock = true;
            else if (collidableEntity->mEntityType == WINNING_PLATFORM)
                mTouchedWinningPlatform = true;

            Vector2 myCenter = getColliderCenter();
            Vector2 otherCenter = collidableEntity->getColliderCenter();
            Vector2 myHalf = getColliderHalfExtents();
            Vector2 otherHalf = collidableEntity->getColliderHalfExtents();

            float myY = myCenter.y;
            float otherY = otherCenter.y;
            float yDistance = fabs(myY - otherY);
            float yOverlap  = fabs(yDistance - myHalf.y - otherHalf.y);

            if (yOverlap < Y_COLLISION_THRESHOLD) continue;

            float myX = myCenter.x;
            float otherX = otherCenter.x;
            float xDistance = fabs(myX - otherX);
            float xOverlap  = fabs(xDistance - myHalf.x - otherHalf.x);

            if (mVelocity.x > 0) {
                mPosition.x     -= xOverlap;
                mVelocity.x      = 0;
                mIsCollidingRight = true;
            } else if (mVelocity.x < 0) {
                mPosition.x    += xOverlap;
                mVelocity.x     = 0;
                mIsCollidingLeft = true;
            }
        }
    }
}

bool Entity::isColliding(const Entity *other) const
{
    if (!other->isActive() || other == this) return false;

    Vector2 myCenter = getColliderCenter();
    Vector2 otherCenter = other->getColliderCenter();

    if (mColliderType == CIRCLE && other->mColliderType == CIRCLE)
    {
        float centerDistance = Vector2Distance(myCenter, otherCenter);
        return centerDistance < (mColliderRadius + other->mColliderRadius);
    }

    if (mColliderType == BOX && other->mColliderType == BOX)
    {
        float xDistance = fabs(myCenter.x - otherCenter.x) -
            ((mColliderDimensions.x + other->getColliderDimensions().x) / 2.0f);
        float yDistance = fabs(myCenter.y - otherCenter.y) -
            ((mColliderDimensions.y + other->getColliderDimensions().y) / 2.0f);

        return (xDistance < 0.0f && yDistance < 0.0f);
    }

    const Entity *circle = (mColliderType == CIRCLE) ? this : other;
    const Entity *box    = (mColliderType == CIRCLE) ? other : this;

    Vector2 circleCenter = circle->getColliderCenter();
    Vector2 boxCenter = box->getColliderCenter();
    Vector2 boxHalf = box->getColliderHalfExtents();

    float minX = boxCenter.x - boxHalf.x;
    float maxX = boxCenter.x + boxHalf.x;
    float minY = boxCenter.y - boxHalf.y;
    float maxY = boxCenter.y + boxHalf.y;

    float closestX = fmaxf(minX, fminf(circleCenter.x, maxX));
    float closestY = fmaxf(minY, fminf(circleCenter.y, maxY));

    float dx = circleCenter.x - closestX;
    float dy = circleCenter.y - closestY;

    return (dx * dx + dy * dy) < (circle->mColliderRadius * circle->mColliderRadius);
}

void Entity::animate(float deltaTime)
{
    mAnimationIndices = mAnimationAtlas.at(mDirection);

    mAnimationTime += deltaTime;
    float framesPerSecond = 1.0f / mFrameSpeed;

    if (mAnimationTime >= framesPerSecond)
    {
        mAnimationTime = 0.0f;

        mCurrentFrameIndex++;
        mCurrentFrameIndex %= mAnimationIndices.size();
    }
}

void Entity::update(float deltaTime, Entity *collidableEntities,
    int collisionCheckCount)
{
    if (mEntityStatus == INACTIVE) return;

    resetColliderFlags();

    if (mEntityType != PLAYER)
        mVelocity.x = mMovement.x * mSpeed;

    mVelocity.x += mAcceleration.x * deltaTime;
    mVelocity.y += mAcceleration.y * deltaTime;

    if (mIsJumping)
    {
        mIsJumping = false;
        mVelocity.y -= mJumpingPower;
    }

    mPosition.y += mVelocity.y * deltaTime;
    checkCollisionY(collidableEntities, collisionCheckCount);

    mPosition.x += mVelocity.x * deltaTime;
    checkCollisionX(collidableEntities, collisionCheckCount);

    if (mTextureType == ATLAS && GetLength(mMovement) != 0 && mIsCollidingBottom)
        animate(deltaTime);
}

void Entity::render()
{
    if(mEntityStatus == INACTIVE) return;

    Rectangle textureArea;

    switch (mTextureType)
    {
        case SINGLE:
            textureArea = {
                0.0f, 0.0f,
                static_cast<float>(mTexture.width),
                static_cast<float>(mTexture.height)
            };
            break;
        case ATLAS:
            textureArea = getUVRectangle(
                &mTexture,
                mAnimationIndices[mCurrentFrameIndex],
                mSpriteSheetDimensions.x,
                mSpriteSheetDimensions.y
            );

        default: break;
    }

    Rectangle destinationArea = {
        mPosition.x,
        mPosition.y,
        static_cast<float>(mScale.x),
        static_cast<float>(mScale.y)
    };

    Vector2 originOffset = {
        static_cast<float>(mScale.x) / 2.0f,
        static_cast<float>(mScale.y) / 2.0f
    };

    DrawTexturePro(
        mTexture,
        textureArea, destinationArea, originOffset,
        mAngle, WHITE
    );

    // displayCollider();
}

void Entity::displayCollider()
{
    if (mColliderType == CIRCLE)
    {
        Vector2 center = getColliderCenter();
        DrawCircleLines(center.x, center.y, mColliderRadius, GREEN);
        return;
    }

    Rectangle colliderBox = {
        mPosition.x + mColliderOffset.x - mColliderDimensions.x / 2.0f,
        mPosition.y + mColliderOffset.y - mColliderDimensions.y / 2.0f,
        mColliderDimensions.x,
        mColliderDimensions.y
    };

    DrawRectangleLines(
        colliderBox.x,
        colliderBox.y,
        colliderBox.width,
        colliderBox.height,
        GREEN
    );
}
