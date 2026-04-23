#include "Effects.h"
#include <cmath>

Effects::Effects(Vector2 origin, float windowWidth, float windowHeight) : mAlpha{SOLID}, 
                     mEffectSpeed{DEFAULT_SPEED},
                     mOverlayWidth{windowWidth}, 
                     mOverlayHeight{windowHeight}, mCurrentEffect{EffectType::NONE},
                     mViewOffset{{}}, mOrigin{origin}, 
                     mMaxWindowDimensions{windowWidth, windowHeight},
                     mFlickerTimer{0.0f}
{
}

void Effects::drawOverlay()
{
    float left = mViewOffset.x - mOverlayWidth  / 2.0f;
    float top  = mOrigin.y - mOverlayHeight / 2.0f; // Adjustment for some reason

    DrawRectangle(left,
                  top,
                  mOverlayWidth,
                  mOverlayHeight,
                  Fade(BLACK, mAlpha));
}

void Effects::update(float deltaTime, Vector2 *viewOffset)
{
    if (viewOffset != nullptr) mViewOffset = *viewOffset;

    float diagonalRatio = mMaxWindowDimensions.y / mMaxWindowDimensions.x;

    switch (mCurrentEffect)
    {
        case EffectType::FADEIN:
            mAlpha -= mEffectSpeed * deltaTime;

            if (mAlpha <= TRANSPARENT)
            {
                mAlpha = TRANSPARENT;
                mCurrentEffect = EffectType::NONE;
            }

            break;
        
        case EffectType::FADEOUT:
            mAlpha += mEffectSpeed * deltaTime;

            if (mAlpha >= SOLID)
            {
                mAlpha = SOLID;
                mCurrentEffect = EffectType::NONE;
            }

            break;

        case EffectType::SHRINK:
            mOverlayHeight -= mEffectSpeed * SIZE_SPEED_MULTIPLIER * deltaTime * diagonalRatio;
            mOverlayWidth  -= mEffectSpeed * SIZE_SPEED_MULTIPLIER * deltaTime;

            if (mOverlayHeight <= 0.0f ||
                mOverlayWidth  <= 0.0f)
            {
                mOverlayHeight = 0.0f;
                mOverlayWidth  = 0.0f;
                mCurrentEffect = EffectType::NONE;
            }
            break;

        case EffectType::GROW:
            mOverlayHeight += mEffectSpeed * SIZE_SPEED_MULTIPLIER * deltaTime * diagonalRatio;
            mOverlayWidth  += mEffectSpeed * SIZE_SPEED_MULTIPLIER * deltaTime;

            if (mOverlayHeight >= mMaxWindowDimensions.y ||
                mOverlayWidth  >= mMaxWindowDimensions.x)
            {
                mOverlayHeight = mMaxWindowDimensions.y;
                mOverlayWidth  = mMaxWindowDimensions.x;
                mCurrentEffect = EffectType::NONE;
            }
            break;

        case EffectType::TORCH_FLICKER:
        {
            mFlickerTimer += deltaTime * TORCH_PULSE_INCREMENT;

            float pulse = sinf(mFlickerTimer / TORCH_PULSE_SPEED) * TORCH_PULSE_STRENGTH;
            mAlpha = TORCH_BASE_ALPHA + pulse;

            mOverlayWidth  = mMaxWindowDimensions.x;
            mOverlayHeight = mMaxWindowDimensions.y;
            break;
        }

        case EffectType::NONE:
        default:
            break;
    }
}

void Effects::start(EffectType effectType)
{
    mCurrentEffect = effectType;

    switch (mCurrentEffect)
    {
        case EffectType::FADEIN:
            mAlpha = SOLID;
            break;

        case EffectType::FADEOUT:
            mAlpha = TRANSPARENT;
            break;

        case EffectType::SHRINK:
            mOverlayHeight = mMaxWindowDimensions.y;
            mOverlayWidth  = mMaxWindowDimensions.x;
            break;

        case EffectType::GROW:
            mOverlayHeight = 0.0f;
            mOverlayWidth  = 0.0f;
            break;

        case EffectType::TORCH_FLICKER:
            mAlpha = TORCH_BASE_ALPHA;
            mOverlayHeight = mMaxWindowDimensions.y;
            mOverlayWidth  = mMaxWindowDimensions.x;
            mFlickerTimer = 0.0f;
            break;

        case EffectType::NONE:
        default:
            break;
    }
}

void Effects::render()
{
    switch (mCurrentEffect)
    {
        case EffectType::FADEIN:
        case EffectType::FADEOUT:
        case EffectType::SHRINK:
        case EffectType::GROW:
        case EffectType::TORCH_FLICKER:
            drawOverlay();
            break;

        case EffectType::NONE:
    
    default:
        break;
    }
}