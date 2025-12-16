#include "raylib.h"
#include "game.h"

int main()
{
    const int screenWidth = 640;
    const int screenHeight = 480;

    InitWindow(screenWidth, screenHeight, "c-maze");
    SetExitKey(KEY_ESCAPE);
    DisableCursor();
    SetTargetFPS(60);

    Game game;

    while (!WindowShouldClose())
    {
        game.Update();
        game.Draw();
    }

    CloseWindow();
    return 0;
}
