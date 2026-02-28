/**
* Author: Derek
* Assignment: Pong Clone
* Date due: 02/28/2025
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "raylib.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <string>

constexpr int SCREEN_WIDTH = 1280;
constexpr int SCREEN_HEIGHT = 720;
constexpr float PADDLE_SPEED = 360.0f;
constexpr float BALL_BASE_SPEED = 330.0f;
constexpr float BALL_SPEED_INCREMENT = 22.0f;
constexpr int POINTS_TO_WIN = 5;
constexpr int MAX_BALLS = 5;

enum class AppStatus
{
    Running,
    Terminated
};

enum class GameState
{
    Playing,
    GameOver
};

struct Paddle
{
    Vector2 position{};
    Vector2 size{};
    float speed = PADDLE_SPEED;
};

struct DvdBall
{
    Vector2 position{};
    Vector2 size{};
    Vector2 velocity{};
    int textureIndex = 0; // 0 = white, 1 = blue
    bool isActive = false;
};

AppStatus gAppStatus = AppStatus::Running;
GameState gGameState = GameState::Playing;

Texture2D gDwightTexture{};
Texture2D gPrisonMikeTexture{};
std::array<Texture2D, 2> gDvdTextures{};

Paddle gDwight{};
Paddle gPrisonMike{};
Vector2 gDwightMovement{};
Vector2 gPrisonMikeMovement{};

bool gDwightAIEnabled = false;
float gDwightAIDirection = 1.0f;

std::array<DvdBall, MAX_BALLS> gBalls{};
int gActiveBallCount = 1;

int gDwightScore = 0;
int gPrisonMikeScore = 0;
std::string gWinnerBanner;

float gPreviousTicks = 0.0f;

void initialise();
void shutdown();
void processInput();
void update();
void render();

void setupPaddles();
void setupBalls();
void applyBallCount(int requestedCount);
float computeDeltaTime();
void updatePaddles(float deltaTime);
void updateBalls(float deltaTime);
void clampPaddle(Paddle& paddle);
Rectangle toRect(const Vector2& position, const Vector2& size);
void drawSprite(const Texture2D& texture, const Vector2& position, const Vector2& size);
void drawPaddle(const Paddle& paddle, const Texture2D& texture, Color borderColor);
void resetBall(DvdBall& ball, float horizontalDirection);
void handleScore(DvdBall& ball, float horizontalDirection);
void declareWinner(const std::string& winnerText);
void resetGame();

int main()
{
    initialise();

    while (gAppStatus == AppStatus::Running)
    {
        processInput();
        update();
        render();
    }

    shutdown();
    return 0;
}

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Dwight vs. Prison Mike - DVD Pong");
    SetTargetFPS(60);

    gDwightTexture = LoadTexture("assets/dwight.jpg");
    gPrisonMikeTexture = LoadTexture("assets/prison_mike.jpg");
    gDvdTextures[0] = LoadTexture("assets/dvd_logo_white.png");
    gDvdTextures[1] = LoadTexture("assets/dvd_logo_blue.png");

    setupPaddles();
    setupBalls();
    gPreviousTicks = static_cast<float>(GetTime());
}

void shutdown()
{
    UnloadTexture(gDwightTexture);
    UnloadTexture(gPrisonMikeTexture);
    for (Texture2D& texture : gDvdTextures)
    {
        UnloadTexture(texture);
    }

    CloseWindow();
}

void setupPaddles()
{
    gDwight.size = {140.0f, 230.0f};
    gDwight.position = {120.0f, SCREEN_HEIGHT / 2.0f};

    gPrisonMike.size = {140.0f, 230.0f};
    gPrisonMike.position = {SCREEN_WIDTH - 120.0f, SCREEN_HEIGHT / 2.0f};
}

void setupBalls()
{
    for (int i = 0; i < MAX_BALLS; ++i)
    {
        gBalls[i].size = {110.0f, 80.0f};
        gBalls[i].textureIndex = i % static_cast<int>(gDvdTextures.size());
        gBalls[i].isActive = i < gActiveBallCount;
        if (gBalls[i].isActive)
        {
            const float direction = (i % 2 == 0) ? 1.0f : -1.0f;
            resetBall(gBalls[i], direction);
        }
    }
}

void applyBallCount(int requestedCount)
{
    const int clamped = std::clamp(requestedCount, 1, MAX_BALLS);
    gActiveBallCount = clamped;

    for (int i = 0; i < MAX_BALLS; ++i)
    {
        gBalls[i].isActive = (i < gActiveBallCount) && (gGameState == GameState::Playing);
        if (gBalls[i].isActive)
        {
            const float direction = (i % 2 == 0) ? 1.0f : -1.0f;
            resetBall(gBalls[i], direction);
        }
    }
}

void processInput()
{
    if (WindowShouldClose() || IsKeyPressed(KEY_Q))
    {
        gAppStatus = AppStatus::Terminated;
        return;
    }

    if (gGameState == GameState::GameOver)
    {
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))
        {
            resetGame();
        }
        return;
    }

    gDwightMovement = {0.0f, 0.0f};
    gPrisonMikeMovement = {0.0f, 0.0f};

    if (!gDwightAIEnabled)
    {
        if (IsKeyDown(KEY_W))
        {
            gDwightMovement.y = -1.0f;
        }
        else if (IsKeyDown(KEY_S))
        {
            gDwightMovement.y = 1.0f;
        }
    }

    if (IsKeyDown(KEY_UP))
    {
        gPrisonMikeMovement.y = -1.0f;
    }
    else if (IsKeyDown(KEY_DOWN))
    {
        gPrisonMikeMovement.y = 1.0f;
    }

    if (IsKeyPressed(KEY_T))
    {
        gDwightAIEnabled = !gDwightAIEnabled;
        gDwightAIDirection = 1.0f;
    }

    if (IsKeyPressed(KEY_ONE))
    {
        applyBallCount(1);
    }
    if (IsKeyPressed(KEY_TWO))
    {
        applyBallCount(2);
    }
    if (IsKeyPressed(KEY_THREE))
    {
        applyBallCount(3);
    }
    if (IsKeyPressed(KEY_FOUR))
    {
        applyBallCount(4);
    }
    if (IsKeyPressed(KEY_FIVE))
    {
        applyBallCount(5);
    }
}

float computeDeltaTime()
{
    const float currentTicks = static_cast<float>(GetTime());
    const float deltaTime = currentTicks - gPreviousTicks;
    gPreviousTicks = currentTicks;
    return deltaTime;
}

void update()
{
    const float deltaTime = computeDeltaTime();

    if (gAppStatus != AppStatus::Running || gGameState == GameState::GameOver)
    {
        return;
    }

    updatePaddles(deltaTime);
    updateBalls(deltaTime);
}

void updatePaddles(float deltaTime)
{
    if (gDwightAIEnabled)
    {
        gDwight.position.y += gDwight.speed * gDwightAIDirection * deltaTime;
        const float halfHeight = gDwight.size.y / 2.0f;
        if (gDwight.position.y - halfHeight <= 0.0f)
        {
            gDwight.position.y = halfHeight;
            gDwightAIDirection = 1.0f;
        }
        else if (gDwight.position.y + halfHeight >= static_cast<float>(SCREEN_HEIGHT))
        {
            gDwight.position.y = SCREEN_HEIGHT - halfHeight;
            gDwightAIDirection = -1.0f;
        }
    }
    else
    {
        gDwight.position.y += gDwightMovement.y * gDwight.speed * deltaTime;
        clampPaddle(gDwight);
    }

    gPrisonMike.position.y += gPrisonMikeMovement.y * gPrisonMike.speed * deltaTime;
    clampPaddle(gPrisonMike);
}

void clampPaddle(Paddle& paddle)
{
    const float halfHeight = paddle.size.y / 2.0f;
    paddle.position.y = std::clamp(paddle.position.y, halfHeight, static_cast<float>(SCREEN_HEIGHT) - halfHeight);
}

void updateBalls(float deltaTime)
{
    const Rectangle dwightRect = toRect(gDwight.position, gDwight.size);
    const Rectangle mikeRect = toRect(gPrisonMike.position, gPrisonMike.size);

    for (DvdBall& ball : gBalls)
    {
        if (!ball.isActive)
        {
            continue;
        }



        ball.position.x += ball.velocity.x * deltaTime;
        ball.position.y += ball.velocity.y * deltaTime;

        const float halfWidth = ball.size.x / 2.0f;
        const float halfHeight = ball.size.y / 2.0f;

        if (ball.position.y - halfHeight <= 0.0f)
        {
            ball.position.y = halfHeight;
            ball.velocity.y *= -1.0f;
            // toggle texture on wall collision
            ball.textureIndex = (ball.textureIndex + 1) % static_cast<int>(gDvdTextures.size());
        }
        else if (ball.position.y + halfHeight >= static_cast<float>(SCREEN_HEIGHT))
        {
            ball.position.y = SCREEN_HEIGHT - halfHeight;
            ball.velocity.y *= -1.0f;
            // toggle texture on wall collision
            ball.textureIndex = (ball.textureIndex + 1) % static_cast<int>(gDvdTextures.size());
        }

        const Rectangle ballRect = toRect(ball.position, ball.size);

        if (CheckCollisionRecs(ballRect, dwightRect) && ball.velocity.x < 0.0f)
        {
            ball.position.x = dwightRect.x + dwightRect.width + halfWidth;
            ball.velocity.x = std::fabs(ball.velocity.x) + BALL_SPEED_INCREMENT;
            const float offset = (ball.position.y - gDwight.position.y) / (gDwight.size.y / 2.0f);
            ball.velocity.y += offset * 140.0f;
            // toggle texture on paddle collision
            ball.textureIndex = (ball.textureIndex + 1) % static_cast<int>(gDvdTextures.size());
        }
        else if (CheckCollisionRecs(ballRect, mikeRect) && ball.velocity.x > 0.0f)
        {
            ball.position.x = mikeRect.x - halfWidth;
            ball.velocity.x = -(std::fabs(ball.velocity.x) + BALL_SPEED_INCREMENT);
            const float offset = (ball.position.y - gPrisonMike.position.y) / (gPrisonMike.size.y / 2.0f);
            ball.velocity.y += offset * 140.0f;
            // toggle texture on paddle collision
            ball.textureIndex = (ball.textureIndex + 1) % static_cast<int>(gDvdTextures.size());
        }

        if (ball.position.x + halfWidth < 0.0f)
        {
            ++gPrisonMikeScore;
            handleScore(ball, 1.0f);
        }
        else if (ball.position.x - halfWidth > static_cast<float>(SCREEN_WIDTH))
        {
            ++gDwightScore;
            handleScore(ball, -1.0f);
        }
    }
}

Rectangle toRect(const Vector2& position, const Vector2& size)
{
    return {position.x - size.x / 2.0f, position.y - size.y / 2.0f, size.x, size.y};
}

void drawSprite(const Texture2D& texture, const Vector2& position, const Vector2& size)
{
    const Rectangle source{0.0f, 0.0f, static_cast<float>(texture.width), static_cast<float>(texture.height)};
    const Rectangle destination{position.x - size.x / 2.0f, position.y - size.y / 2.0f, size.x, size.y};
    const Vector2 origin{0.0f, 0.0f};
    DrawTexturePro(texture, source, destination, origin, 0.0f, WHITE);
}

void drawPaddle(const Paddle& paddle, const Texture2D& texture, Color borderColor)
{
    const float pad = 8.0f;
    Rectangle rect{paddle.position.x - paddle.size.x / 2.0f, paddle.position.y - paddle.size.y / 2.0f, paddle.size.x, paddle.size.y};

    // subtle shadow behind the paddle
    Rectangle shadow = rect;
    shadow.x += 6.0f;
    shadow.y += 6.0f;
    DrawRectangleRounded(shadow, 0.28f, 16, Fade(BLACK, 0.18f));

    // rounded background + border
    DrawRectangleRounded(rect, 0.28f, 16, Fade(DARKGRAY, 0.92f));
    DrawRectangleRoundedLines(rect, 0.28f, 16, borderColor);

    // inner area for the texture
    Rectangle inner{rect.x + pad, rect.y + pad, rect.width - pad * 2.0f, rect.height - pad * 2.0f};

    const Rectangle source{0.0f, 0.0f, static_cast<float>(texture.width), static_cast<float>(texture.height)};

    // preserve aspect ratio when drawing the texture inside the paddle
    float texW = static_cast<float>(texture.width);
    float texH = static_cast<float>(texture.height);
    float scale = std::min(inner.width / texW, inner.height / texH);
    float drawW = texW * scale;
    float drawH = texH * scale;
    Rectangle destTex{inner.x + (inner.width - drawW) / 2.0f, inner.y + (inner.height - drawH) / 2.0f, drawW, drawH};
    const Vector2 origin{0.0f, 0.0f};
    DrawTexturePro(texture, source, destTex, origin, 0.0f, WHITE);
}

void resetBall(DvdBall& ball, float horizontalDirection)
{
    ball.position = {SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};
    const float speed = BALL_BASE_SPEED + static_cast<float>(GetRandomValue(-60, 60));
    const float vy = static_cast<float>(GetRandomValue(-160, 160));
    ball.velocity = {horizontalDirection * speed, vy};
    ball.textureIndex = 0;
}

void handleScore(DvdBall& ball, float horizontalDirection)
{
    if (gDwightScore >= POINTS_TO_WIN)
    {
        declareWinner("Dwight wins!");
        ball.isActive = false;
        return;
    }

    if (gPrisonMikeScore >= POINTS_TO_WIN)
    {
        declareWinner("Prison Mike wins!");
        ball.isActive = false;
        return;
    }

    resetBall(ball, horizontalDirection);
}

void declareWinner(const std::string& winnerText)
{
    gGameState = GameState::GameOver;
    gWinnerBanner = winnerText;
    for (DvdBall& ball : gBalls)
    {
        ball.isActive = false;
    }
}

void resetGame()
{
    gGameState = GameState::Playing;
    gDwightScore = 0;
    gPrisonMikeScore = 0;
    setupPaddles();
    setupBalls();
}

void render()
{
    BeginDrawing();
    ClearBackground(DARKBLUE);

    DrawRectangle(SCREEN_WIDTH / 2 - 2, 0, 4, SCREEN_HEIGHT, Fade(RAYWHITE, 0.4f));

    // draw paddles with consistent, aesthetic styling
    drawPaddle(gDwight, gDwightTexture, ORANGE);
    drawPaddle(gPrisonMike, gPrisonMikeTexture, PURPLE);

    for (const DvdBall& ball : gBalls)
    {
        if (!ball.isActive)
        {
            continue;
        }
        const int idx = ball.textureIndex % static_cast<int>(gDvdTextures.size());
        drawSprite(gDvdTextures[idx], ball.position, ball.size);
    }

    const std::string scoreText = "Dwight " + std::to_string(gDwightScore) + " | " +
                                  std::to_string(gPrisonMikeScore) + " Prison Mike";
    DrawText(scoreText.c_str(), SCREEN_WIDTH / 2 - MeasureText(scoreText.c_str(), 32) / 2, 30, 32, RAYWHITE);

    const std::string instructions = "W/S vs Up/Down | T: Dwight AI | 1-5: DVD logos";
    DrawText(instructions.c_str(), SCREEN_WIDTH / 2 - MeasureText(instructions.c_str(), 20) / 2, SCREEN_HEIGHT - 40, 20, LIGHTGRAY);

    if (gDwightAIEnabled)
    {
        DrawText("Dwight AI ACTIVE", 30, 30, 24, GOLD);
    }

    if (gGameState == GameState::GameOver)
    {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.6f));
        const int bannerSize = 48;
        const int textWidth = MeasureText(gWinnerBanner.c_str(), bannerSize);
        DrawText(gWinnerBanner.c_str(), SCREEN_WIDTH / 2 - textWidth / 2, SCREEN_HEIGHT / 2 - 60, bannerSize, WHITE);
        const char* prompt = "Press Enter to play again";
        DrawText(prompt, SCREEN_WIDTH / 2 - MeasureText(prompt, 24) / 2, SCREEN_HEIGHT / 2 + 10, 24, LIGHTGRAY);
    }

    EndDrawing();
}
