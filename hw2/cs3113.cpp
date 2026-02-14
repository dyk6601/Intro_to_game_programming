#include "cs3113.h"
#include <cmath>

// Global Constants
const int SCREEN_WIDTH  = 1280;
const int SCREEN_HEIGHT = 720;
const int FPS           = 60;

const char SUN_FILEPATH[]    = "assets/sun.png";
const char MOON_FILEPATH[]   = "assets/moon.png";
const char GROUND_FILEPATH[] = "assets/ground.png";
const char MARIO_FILEPATH[]  = "assets/mario.png";
const char LUCARIO_FILEPATH[] = "assets/lucario.png";

// Day/Night cycle 
const float DAY_NIGHT_ORBIT_RADIUS = 400.0f;
const float DAY_NIGHT_ORBIT_SPEED = 0.3f;
const Color DAY_SKY = { 135, 206, 235, 255 };
const Color NIGHT_SKY = { 10, 14, 39, 255 };

// Sun 
const Vector2 SUN_BASE_SIZE = { 200.0f, 200.0f };
const float SUN_ROTATION_SPEED = 30.0f;

// Moon 
const Vector2 MOON_BASE_SIZE = { 180.0f, 180.0f };
const float MOON_ROTATION_SPEED = 25.0f;

// Ground 
const float GROUND_BASE_Y = SCREEN_HEIGHT - 50.0f;
const float GROUND_BOB_SPEED = 3.5f;
const float GROUND_BOB_AMPLITUDE = 40.0f;

// Mario 
const Vector2 MARIO_BASE_SIZE = { 100.0f, 100.0f };
const float MARIO_PULSE_AMPLITUDE = 80.0f;  

// Lucario
const Vector2 LUCARIO_SIZE = { 140.0f, 140.0f };
const float LUCARIO_SPEED = 900.0f;     
const float LUCARIO_SPIN_SPEED = 360.0f; 
const float LUCARIO_EDGE_PADDING = 10.0f;   // To prevent lucario from going off screen

// Global Variables
AppStatus gAppStatus = RUNNING;

Vector2 gOrbitCenter = { SCREEN_WIDTH - 300.0f, 300.0f };
Vector2 gMarioPosition = { 200.0f, SCREEN_HEIGHT - 150.0f };

// Textures
Texture2D gSunTexture;
Texture2D gMoonTexture;
Texture2D gGroundTexture;
Texture2D gMarioTexture;
Texture2D gLucarioTexture;


float gDayNightAngle = 0.0f;


Vector2 gSunPosition = { 0.0f, 0.0f };
float gSunRotation = 0.0f;


Vector2 gMoonPosition = { 0.0f, 0.0f };
float gMoonRotation = 0.0f;


float gGroundBobTime = 0.0f;
float gGroundY = GROUND_BASE_Y;
float gGroundRotation = 0.0f;


Vector2 gMarioScale = MARIO_BASE_SIZE;

Vector2 gLucarioPosition = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
Vector2 gLucarioVelocity = { LUCARIO_SPEED, LUCARIO_SPEED };
float gLucarioRotation = 0.0f;

// Delta time
float gPreviousTicks = 0.0f;

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "HW2 - Solar System");
    
    gSunTexture = LoadTexture(SUN_FILEPATH);
    gMoonTexture = LoadTexture(MOON_FILEPATH);
    gGroundTexture = LoadTexture(GROUND_FILEPATH);
    gMarioTexture = LoadTexture(MARIO_FILEPATH);
    gLucarioTexture = LoadTexture(LUCARIO_FILEPATH);
    
    SetTargetFPS(FPS);
}

void processInput() 
{
    if (WindowShouldClose()) gAppStatus = TERMINATED;
}

void update() 
{
    // Calculate delta time
    float ticks = (float)GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks = ticks;
    
    // Day/night 
    gDayNightAngle += DAY_NIGHT_ORBIT_SPEED * deltaTime;
    
    gSunPosition = {
        gOrbitCenter.x + DAY_NIGHT_ORBIT_RADIUS * cos(gDayNightAngle - PI/2),
        gOrbitCenter.y + DAY_NIGHT_ORBIT_RADIUS * sin(gDayNightAngle - PI/2)
    };
    
    // Moon opposite sun
    gMoonPosition = {
        gOrbitCenter.x + DAY_NIGHT_ORBIT_RADIUS * cos(gDayNightAngle + PI/2),
        gOrbitCenter.y + DAY_NIGHT_ORBIT_RADIUS * sin(gDayNightAngle + PI/2)
    };
    
    gSunRotation += SUN_ROTATION_SPEED * deltaTime;
    gMoonRotation += MOON_ROTATION_SPEED * deltaTime;
    
    // Ground
    gGroundBobTime += GROUND_BOB_SPEED * deltaTime;
    float groundWave = sin(gGroundBobTime);
    gGroundY = GROUND_BASE_Y + GROUND_BOB_AMPLITUDE * groundWave;
        gGroundRotation = 0.0f;
    
    // Mario pulse (synced with ground bob)
    float marioPulseOffset = MARIO_PULSE_AMPLITUDE * sin(gGroundBobTime);
    float scaledSize = MARIO_BASE_SIZE.x + marioPulseOffset;
    if (scaledSize < 20.0f) scaledSize = 20.0f;
    gMarioScale = { scaledSize, scaledSize };

    // Lucario fast bounce (translation + rotation using deltaTime)
    gLucarioRotation += LUCARIO_SPIN_SPEED * deltaTime;
    gLucarioPosition.x += gLucarioVelocity.x * deltaTime;
    gLucarioPosition.y += gLucarioVelocity.y * deltaTime;

    float halfW = LUCARIO_SIZE.x / 2.0f + LUCARIO_EDGE_PADDING;
    float halfH = LUCARIO_SIZE.y / 2.0f + LUCARIO_EDGE_PADDING;

    // Simple edge bounce to keep him on-screen
    if (gLucarioPosition.x < halfW) {
        gLucarioPosition.x = halfW;
        gLucarioVelocity.x = fabsf(gLucarioVelocity.x);
    }
    else if (gLucarioPosition.x > SCREEN_WIDTH - halfW) {
        gLucarioPosition.x = SCREEN_WIDTH - halfW;
        gLucarioVelocity.x = -fabsf(gLucarioVelocity.x);
    }

    if (gLucarioPosition.y < halfH) {
        gLucarioPosition.y = halfH;
        gLucarioVelocity.y = fabsf(gLucarioVelocity.y);
    }
    else if (gLucarioPosition.y > SCREEN_HEIGHT - halfH) {
        gLucarioPosition.y = SCREEN_HEIGHT - halfH;
        gLucarioVelocity.y = -fabsf(gLucarioVelocity.y);
    }
}

void render()
{
    BeginDrawing();
    
    // Sky color based on sun height
    float sunHeight = (gOrbitCenter.y - gSunPosition.y) / DAY_NIGHT_ORBIT_RADIUS;
    float dayAmount = (sunHeight + 1.0f) / 2.0f;  
    
    Color skyColor = {
        (unsigned char)(NIGHT_SKY.r + (DAY_SKY.r - NIGHT_SKY.r) * dayAmount),
        (unsigned char)(NIGHT_SKY.g + (DAY_SKY.g - NIGHT_SKY.g) * dayAmount),
        (unsigned char)(NIGHT_SKY.b + (DAY_SKY.b - NIGHT_SKY.b) * dayAmount),
        255
    };
    
    ClearBackground(skyColor);
    
    // Sun
    Rectangle sunTextureArea = {
        0.0f, 0.0f,
        (float)gSunTexture.width,
        (float)gSunTexture.height
    };
    
    Rectangle sunDestArea = {
        gSunPosition.x,
        gSunPosition.y,
        SUN_BASE_SIZE.x,
        SUN_BASE_SIZE.y
    };
    
    Vector2 sunOrigin = {
        SUN_BASE_SIZE.x / 2.0f,
        SUN_BASE_SIZE.y / 2.0f
    };
    
    DrawTexturePro(
        gSunTexture,
        sunTextureArea,
        sunDestArea,
        sunOrigin,
        gSunRotation,
        WHITE
    );
    
    // Moon
    Rectangle moonTextureArea = {
        0.0f, 0.0f,
        (float)gMoonTexture.width,
        (float)gMoonTexture.height
    };
    
    Rectangle moonDestArea = {
        gMoonPosition.x,
        gMoonPosition.y,
        MOON_BASE_SIZE.x,
        MOON_BASE_SIZE.y
    };
    
    Vector2 moonOrigin = {
        MOON_BASE_SIZE.x / 2.0f,
        MOON_BASE_SIZE.y / 2.0f
    };
    
    DrawTexturePro(
        gMoonTexture,
        moonTextureArea,
        moonDestArea,
        moonOrigin,
        gMoonRotation,
        WHITE
    );
    
    // Mario
    Rectangle marioTextureArea = {
        0.0f, 0.0f,
        (float)gMarioTexture.width,
        (float)gMarioTexture.height
    };
    
    Rectangle marioDestArea = {
        gMarioPosition.x,
        gMarioPosition.y,
        gMarioScale.x,
        gMarioScale.y
    };
    
    Vector2 marioOrigin = {
        gMarioScale.x / 2.0f,
        gMarioScale.y / 2.0f
    };
    
    DrawTexturePro(
        gMarioTexture,
        marioTextureArea,
        marioDestArea,
        marioOrigin,
        0.0f,
        WHITE
    );

    // Lucario
    Rectangle lucarioTextureArea = {
        0.0f, 0.0f,
        (float)gLucarioTexture.width,
        (float)gLucarioTexture.height
    };
    Rectangle lucarioDestArea = {
        gLucarioPosition.x,
        gLucarioPosition.y,
        LUCARIO_SIZE.x,
        LUCARIO_SIZE.y
    };
    Vector2 lucarioOrigin = { LUCARIO_SIZE.x / 2.0f, LUCARIO_SIZE.y / 2.0f };
    DrawTexturePro(
        gLucarioTexture,
        lucarioTextureArea,
        lucarioDestArea,
        lucarioOrigin,
        gLucarioRotation,
        WHITE
    );
    
    // Ground
    Rectangle groundTextureArea = {
        0.0f, 0.0f,
        (float)gGroundTexture.width,
        (float)gGroundTexture.height
    };
    
    Rectangle groundDestArea = {
        SCREEN_WIDTH / 2.0f,
        gGroundY,
        SCREEN_WIDTH + 100.0f,
        100.0f
    };
    
    Vector2 groundOrigin = {
        (SCREEN_WIDTH + 100.0f) / 2.0f,
        50.0f
    };
    
    DrawTexturePro(
        gGroundTexture,
        groundTextureArea,
        groundDestArea,
        groundOrigin,
        gGroundRotation,
        WHITE
    );
    
    EndDrawing();
}

void shutdown() 
{ 
    UnloadTexture(gSunTexture);
    UnloadTexture(gMoonTexture);
    UnloadTexture(gGroundTexture);
    UnloadTexture(gMarioTexture);
    UnloadTexture(gLucarioTexture);
    
    CloseWindow();
}

int main(void)
{
    initialise();

    while (gAppStatus == RUNNING)
    {
        processInput();
        update();
        render();
    }

    shutdown();

    return 0;
}