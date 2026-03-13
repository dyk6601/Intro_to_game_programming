/**
* Author: Derek Kim
* Assignment: Lunar Lander
* Date due: March 14, 2026
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "raylib.h"
#include <cmath>
#include <vector>


constexpr int   SCREEN_W   = 800;
constexpr int   SCREEN_H   = 600;

constexpr float GRAVITY    = 150.0f;   
constexpr float THRUST     = 300.0f;   

constexpr float MAX_FUEL   = 100.0f;
constexpr float FUEL_BURN  = 14.0f;    

constexpr float ARMY_W     = 50.0f;   
constexpr float ARMY_H     = 60.0f;   

constexpr float BALL_SZ    = 64.0f;   

constexpr float BUZZ_W     = 48.0f;    
constexpr float BUZZ_H     = 58.0f;   

constexpr float ARMY_HIT_INSET_X   = 13.0f;  // shrink side
constexpr float ARMY_HIT_INSET_TOP =  8.0f;  // shrink top 

enum GameState { STATE_MENU, STATE_PLAYING, STATE_WON, STATE_LOST };

struct Entity {
    Vector2 pos;            
    Vector2 size;
    bool    winZone;       
    bool    moving;
    float   speed;
    float   lo, hi;         
    int     dir;            
};

static GameState           g_state = STATE_MENU;

static Texture2D           g_texBg;
static Texture2D           g_texBuzz;
static Texture2D           g_texBall;
static Texture2D           g_texArmy;

// Player
static Vector2             g_position;
static Vector2             g_velocity;
static Vector2             g_acceleration;
static float               g_fuel;

static double              g_prevTime;

static std::vector<Entity> g_entities;

static bool g_wantStart   = false;
static bool g_wantRestart = false;


static float Len(Vector2 v) { return sqrtf(v.x * v.x + v.y * v.y); }

static Rectangle PlayerRect() {
    return { g_position.x - BUZZ_W * 0.5f,
             g_position.y - BUZZ_H * 0.5f,
             BUZZ_W, BUZZ_H };
}

static Rectangle EntRect(const Entity& e) {
    return { e.pos.x, e.pos.y, e.size.x, e.size.y };
}

// Inset collision rect for green-army tiles so the hitbox matches the
static Rectangle ArmyHitRect(const Entity& e) {
    return {
        e.pos.x + ARMY_HIT_INSET_X,
        e.pos.y + ARMY_HIT_INSET_TOP,
        e.size.x - ARMY_HIT_INSET_X * 2.0f,
        e.size.y - ARMY_HIT_INSET_TOP * 2.0f
    };
}

static bool AABBOverlap(Rectangle a, Rectangle b) {
    return a.x < b.x + b.width  && a.x + a.width  > b.x &&
           a.y < b.y + b.height && a.y + a.height > b.y;
}

static bool UpperHalfCircleHit(const Entity& ball) {
    float cx = ball.pos.x + ball.size.x * 0.5f;  
    float cy = ball.pos.y + ball.size.y * 0.5f;   
    float r  = ball.size.x * 0.55f;              

    float fx = g_position.x;
    float fy = g_position.y + BUZZ_H * 0.50f;     
    float dx = fx - cx;
    float dy = fy - cy;
    // fy < cy  →  bottom is in the top half (screen +Y is DOWN)
    return (dx * dx + dy * dy <= r * r) && (fy < cy);
}



static void BuildLevel() {
    g_entities.clear();

    float floorY = (float)SCREEN_H - ARMY_H;
    int   cols   = (int)ceilf((float)SCREEN_W / ARMY_W);   

    //landing-zone gaps 
    const int gapA = 3;   
    const int gapB = 11;

    // Floor
    for (int c = 0; c < cols; ++c) {
        bool inGap = (c == gapA || c == gapA + 1 ||
                      c == gapB || c == gapB + 1);
        if (inGap) continue;                      

        g_entities.push_back({
            { c * ARMY_W, floorY }, { ARMY_W, ARMY_H },
            false, false, 0, 0, 0, 1
        });
    }

    // Pixar Ball
    auto addBall = [&](int gapStart) {
        float bx = gapStart * ARMY_W + (2.0f * ARMY_W - BALL_SZ) * 0.5f;
        float by = floorY + (ARMY_H - BALL_SZ);
        g_entities.push_back({
            { bx, by }, { BALL_SZ, BALL_SZ },
            true, false, 0, 0, 0, 1
        });
    };
    addBall(gapA);
    addBall(gapB);

    // Floating army
    for (int i = 0; i < 3; ++i)
        g_entities.push_back({
            { 170.0f + i * ARMY_W, 420.0f }, { ARMY_W, ARMY_H },
            false, false, 0, 0, 0, 1
        });

    for (int i = 0; i < 2; ++i)
        g_entities.push_back({
            { 520.0f + i * ARMY_W, 350.0f }, { ARMY_W, ARMY_H },
            false, false, 0, 0, 0, 1
        });

    g_entities.push_back({
        { 60.0f, 300.0f }, { ARMY_W, ARMY_H },
        false, false, 0, 0, 0, 1
    });

    // Moving green-army 
    g_entities.push_back({
        { 100.0f, 260.0f }, { ARMY_W, ARMY_H },
        false, true, 130.0f, 50.0f, 560.0f, 1
    });
    g_entities.push_back({
        { 600.0f, 160.0f }, { ARMY_W, ARMY_H },
        false, true, 100.0f, 180.0f, 700.0f, -1
    });
}


static void ResetPlayer() {
    g_position     = { SCREEN_W * 0.5f, 60.0f };
    g_velocity     = { 0.0f, 0.0f };
    g_acceleration = { 0.0f, 0.0f };
    g_fuel         = MAX_FUEL;
    (void)0; 
}

static void Initialise() {
    InitWindow(SCREEN_W, SCREEN_H, "Toy Story - Lunar Lander");
    SetTargetFPS(60);

    g_texBg   = LoadTexture("assets/toystory.jpg");
    g_texBuzz = LoadTexture("assets/buzz.png");
    g_texBall = LoadTexture("assets/Pixar_Ball.png");
    g_texArmy = LoadTexture("assets/Greenarmy.png");

    BuildLevel();
    ResetPlayer();

    g_prevTime = GetTime();
    g_state    = STATE_MENU;
}

static void ProcessInput(float dt) {
    //  Menu
    if (g_state == STATE_MENU && IsKeyPressed(KEY_ENTER)) {
        g_wantStart = true;
        return;
    }
    if ((g_state == STATE_WON || g_state == STATE_LOST) &&
        IsKeyPressed(KEY_R)) {
        g_wantRestart = true;
        return;
    }
    if (g_state != STATE_PLAYING) return;

    g_acceleration = { 0.0f, 0.0f };

    bool thrusting = false;

    if (g_fuel > 0.0f) {
        if (IsKeyDown(KEY_UP)    || IsKeyDown(KEY_W)) {
            g_acceleration.y -= THRUST;
            thrusting = true;
        }
        if (IsKeyDown(KEY_LEFT)  || IsKeyDown(KEY_A)) {
            g_acceleration.x -= THRUST;
            thrusting = true;
        }
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
            g_acceleration.x += THRUST;
            thrusting = true;
        }

        float len = Len(g_acceleration);
        if (len > THRUST) {
            g_acceleration.x *= THRUST / len;
            g_acceleration.y *= THRUST / len;
        }

        // Burn fuel
        if (thrusting) {
            g_fuel -= FUEL_BURN * dt;
            if (g_fuel < 0.0f) g_fuel = 0.0f;
        }
    }
}


static void Update(float dt) {
    //  State Transitions
    if (g_state == STATE_MENU && g_wantStart) {
        g_state     = STATE_PLAYING;
        g_wantStart = false;
        return;
    }
    if ((g_state == STATE_WON || g_state == STATE_LOST) && g_wantRestart) {
        ResetPlayer();
        // Reset moving platforms 
        for (auto& e : g_entities) {
            if (e.moving) e.pos.x = e.lo;
        }
        g_state       = STATE_PLAYING;
        g_wantRestart = false;
        return;
    }
    if (g_state != STATE_PLAYING) return;

    //gravity
    g_acceleration.y += GRAVITY;

    //Velocity
    g_velocity.x += g_acceleration.x * dt;
    g_velocity.y += g_acceleration.y * dt;

    g_position.x += g_velocity.x * dt;
    g_position.y += g_velocity.y * dt;

    //  Screen-edge clamp 
    float hw = BUZZ_W * 0.5f, hh = BUZZ_H * 0.5f;
    if (g_position.x < hw)              { g_position.x = hw;              g_velocity.x = 0; }
    if (g_position.x > SCREEN_W - hw)   { g_position.x = SCREEN_W - hw;  g_velocity.x = 0; }
    if (g_position.y < hh)              { g_position.y = hh;              g_velocity.y = 0; }

    // Move platforms 
    for (auto& e : g_entities) {
        if (!e.moving) continue;
        e.pos.x += e.speed * (float)e.dir * dt;
        if (e.pos.x <= e.lo) { e.pos.x = e.lo; e.dir =  1; }
        if (e.pos.x >= e.hi) { e.pos.x = e.hi; e.dir = -1; }
    }

    // Collision detection
    Rectangle pr = PlayerRect();

    for (const auto& e : g_entities) {
        Rectangle er = e.winZone ? EntRect(e) : ArmyHitRect(e);
        if (!AABBOverlap(pr, er)) continue;

        if (e.winZone) {
            // Pixar Ball
            if (UpperHalfCircleHit(e)) {
                g_state = STATE_WON;
                return;
            }

            float oTop = (pr.y + pr.height) - er.y;
            float oBot = (er.y + er.height) - pr.y;
            if (oTop < oBot) {
                g_position.y -= oTop;
                if (g_velocity.y > 0) g_velocity.y = 0;
            } else {
                g_position.y += oBot;
                if (g_velocity.y < 0) g_velocity.y = 0;
            }
            pr = PlayerRect();
            if (AABBOverlap(pr, er)) {
                float oL = (pr.x + pr.width)  - er.x;
                float oR = (er.x + er.width)  - pr.x;
                if (oL < oR) {
                    g_position.x -= oL;
                    if (g_velocity.x > 0) g_velocity.x = 0;
                } else {
                    g_position.x += oR;
                    if (g_velocity.x < 0) g_velocity.x = 0;
                }
                pr = PlayerRect();
            }
        } else {
            // Green Army
            g_state = STATE_LOST;
            return;
        }
    }

    // Floor
    if (g_position.y > SCREEN_H + BUZZ_H) {
        g_state = STATE_LOST;
    }
}

static void Render() {
    BeginDrawing();
    ClearBackground(BLACK);

    // Background 
    DrawTexturePro(g_texBg,
        { 0, 0, (float)g_texBg.width, (float)g_texBg.height },
        { 0, 0, (float)SCREEN_W, (float)SCREEN_H },
        { 0, 0 }, 0.0f, WHITE);

    //  Entities 
    for (const auto& e : g_entities) {
        const Texture2D& tex = e.winZone ? g_texBall : g_texArmy;
        DrawTexturePro(tex,
            { 0, 0, (float)tex.width, (float)tex.height },
            { e.pos.x, e.pos.y, e.size.x, e.size.y },
            { 0, 0 }, 0.0f, WHITE);
    }

    //  Buzz
    DrawTexturePro(g_texBuzz,
        { 0, 0, (float)g_texBuzz.width, (float)g_texBuzz.height },
        { g_position.x, g_position.y, BUZZ_W, BUZZ_H },
        { BUZZ_W * 0.5f, BUZZ_H * 0.5f }, 0.0f, WHITE);

    // Fuel bar 
    if (g_state == STATE_PLAYING) {
        float pct = g_fuel / MAX_FUEL;
        DrawRectangle(10, 10, 204, 24, Color{ 40, 40, 40, 200 });
        Color barCol = (pct > 0.3f) ? Color{ 34, 177, 76, 255 }
                                     : Color{ 230, 30, 30, 255 };
        DrawRectangle(12, 12, (int)(200.0f * pct), 20, barCol);
        DrawRectangleLines(10, 10, 204, 24, WHITE);
        DrawText("FUEL", 220, 13, 20, WHITE);
    }

    // overlays 
    auto Cx = [](const char* txt, int sz) {
        return (SCREEN_W - MeasureText(txt, sz)) / 2;
    };

    switch (g_state) {
    case STATE_MENU: {
        const char* t1 = "TOY STORY: LUNAR LANDER";
        const char* t2 = "Press ENTER to Play";
        const char* t3 = "Arrow Keys / WASD to move";
        const char* t4 = "Land on the Pixar Ball!";
        DrawText(t1, Cx(t1, 30), SCREEN_H / 2 - 70, 30, YELLOW);
        DrawText(t2, Cx(t2, 22), SCREEN_H / 2 - 10, 22, WHITE);
        DrawText(t3, Cx(t3, 16), SCREEN_H / 2 + 25, 16, LIGHTGRAY);
        DrawText(t4, Cx(t4, 18), SCREEN_H / 2 + 50, 18, GOLD);
        break;
    }
    case STATE_WON: {
        DrawRectangle(0, 0, SCREEN_W, SCREEN_H, Color{ 0, 0, 0, 160 });
        const char* w1 = "MISSION COMPLETED!";
        const char* w2 = "Press R to Restart";
        DrawText(w1, Cx(w1, 44), SCREEN_H / 2 - 40, 44, GOLD);
        DrawText(w2, Cx(w2, 22), SCREEN_H / 2 + 20, 22, WHITE);
        break;
    }
    case STATE_LOST: {
        DrawRectangle(0, 0, SCREEN_W, SCREEN_H, Color{ 0, 0, 0, 160 });
        const char* l1 = "MISSION FAILED!";
        const char* l2 = "Press R to Restart";
        DrawText(l1, Cx(l1, 44), SCREEN_H / 2 - 40, 44, RED);
        DrawText(l2, Cx(l2, 22), SCREEN_H / 2 + 20, 22, WHITE);
        break;
    }
    default: break;
    }

    EndDrawing();
}

static void Shutdown() {
    UnloadTexture(g_texBg);
    UnloadTexture(g_texBuzz);
    UnloadTexture(g_texBall);
    UnloadTexture(g_texArmy);
    CloseWindow();
}

int main() {
    Initialise();

    while (!WindowShouldClose()) {
        double now = GetTime();
        float  dt  = (float)(now - g_prevTime);
        g_prevTime = now;
        if (dt > 0.05f) dt = 0.05f;   

        ProcessInput(dt);
        Update(dt);
        Render();
    }

    Shutdown();
    return 0;
}
