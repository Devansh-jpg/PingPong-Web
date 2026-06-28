/*******************************************************************************************
*
*   PingPong - raylib gamejam web/wasm build (C++)
*
*   Classic Pong: player paddle (right) controlled with UP/DOWN, CPU paddle (left) is AI.
*   Game logic kept in C++ (classes). Only the main loop is restructured for web:
*   on PLATFORM_WEB the browser drives the loop via emscripten_set_main_loop().
*
*   Code licensed under an unmodified zlib/libpng license.
*
********************************************************************************************/

#include <iostream>
#include "raylib.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>      // Emscripten library
#endif

using namespace std;

Color Green = Color{38, 185, 154, 255};
Color Dark_Green = Color{20, 160, 133, 255};
Color Light_Green = Color{129, 204, 184, 255};
Color Yellow = Color{243, 213, 91, 255};

int player_score = 0;
int cpu_score = 0;

const int screen_width = 1280;
const int screen_height = 800;

class Ball {
    public:
    float x, y;
    int speed_x, speed_y;
    int radius;

    void Draw(){
        DrawCircle(x, y, radius, Yellow);
    }

    void Update(){
        x += speed_x;
        y += speed_y;

        // bounce off top and bottom
        if(y + radius >= GetScreenHeight() || y - radius <= 0){
            speed_y *= -1;
        }
        // cpu scores when ball exits right
        if(x + radius >= GetScreenWidth()){
            cpu_score++;
            ResetBall();
        }
        // player scores when ball exits left
        if(x - radius <= 0){
            player_score++;
            ResetBall();
        }
    }

    void ResetBall(){
        x = GetScreenWidth() / 2;
        y = GetScreenHeight() / 2;

        int speed_choices[2] = {-1, 1};
        speed_x *= speed_choices[GetRandomValue(0, 1)];
        speed_y *= speed_choices[GetRandomValue(0, 1)];
    }
};

class Paddle {
    protected:
    void LimitMovement(){
        if(y <= 0) y = 0;
        if(y + height >= GetScreenHeight()) y = GetScreenHeight() - height;
    }

    public:
    float x, y;
    float width, height;
    int speed;

    void Draw(){
        DrawRectangleRounded(Rectangle{x, y, width, height}, 0.8, 0, WHITE);
    }

    void Update(){
        if(IsKeyDown(KEY_UP))   y -= speed;
        if(IsKeyDown(KEY_DOWN)) y += speed;
        LimitMovement();
    }
};

class CpuPaddle: public Paddle {
    public:
    void Update(float ball_y){
        if(y + height / 2 > ball_y) y -= speed;
        if(y + height / 2 <= ball_y) y += speed;
        LimitMovement();
    }
};

Ball ball;
Paddle player;
CpuPaddle cpu;

// Update and draw one frame (called by the desktop loop and by Emscripten on web)
void UpdateDrawFrame(void);

int main(){
    cout << "Starting the game" << endl;
    InitWindow(screen_width, screen_height, "My Pong Game!");

    ball.radius = 25;
    ball.x = screen_width / 2;
    ball.y = screen_height / 2;
    ball.speed_x = 7;
    ball.speed_y = 7;

    // player on the right, controlled by UP/DOWN keys
    player.width = 25;
    player.height = 120;
    player.x = screen_width - player.width - 10;
    player.y = screen_height / 2 - player.height / 2;
    player.speed = 6;

    // cpu on the left, AI-controlled
    cpu.width = 25;
    cpu.height = 120;
    cpu.x = 10;
    cpu.y = screen_height / 2 - cpu.height / 2;
    cpu.speed = 6;

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(60);
    while(WindowShouldClose() == false){
        UpdateDrawFrame();
    }
#endif

    CloseWindow();
    return 0;
}

void UpdateDrawFrame(void){
    BeginDrawing();

    // Update
    ball.Update();
    player.Update();
    cpu.Update(ball.y);

    // Collision detection
    if(CheckCollisionCircleRec({ball.x, ball.y}, ball.radius,
        {player.x, player.y, player.width, player.height})){
        ball.speed_x *= -1;
    }
    if(CheckCollisionCircleRec({ball.x, ball.y}, ball.radius,
        {cpu.x, cpu.y, cpu.width, cpu.height})){
        ball.speed_x *= -1;
    }

    // Draw background
    ClearBackground(Dark_Green);
    DrawRectangle(screen_width / 2, 0, screen_width / 2, screen_height, Green);
    DrawCircle(screen_width / 2, screen_height / 2, 150, Light_Green);
    DrawLine(screen_width / 2, 0, screen_width / 2, screen_height, WHITE);

    // Draw game objects
    ball.Draw();
    cpu.Draw();
    player.Draw();

    // Draw scores
    DrawText(TextFormat("%i", cpu_score),    screen_width / 4 - 20,     20, 80, WHITE);
    DrawText(TextFormat("%i", player_score), 3 * screen_width / 4 - 20, 20, 80, WHITE);

    EndDrawing();
}
