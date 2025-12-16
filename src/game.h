#pragma once

#include "raylib.h"

class Game
{
public:
    Game();
    ~Game();

    void Update();
    void Draw();

    void SetMoveSpeed(float speed) { moveSpeed = speed; }
    float GetMoveSpeed() const { return moveSpeed; }

private:
    static constexpr int kMapWidth = 8;
    static constexpr int kMapHeight = 8;
    int map[kMapHeight][kMapWidth] = {
        {1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,1},
        {1,0,1,0,1,0,0,1},
        {1,0,1,0,1,0,1,1},
        {1,0,0,0,0,0,0,1},
        {1,0,1,0,1,0,0,1},
        {1,0,0,0,0,1,0,1},
        {1,1,1,1,1,1,1,1},
    };

    Texture2D wallTex{};
    Texture2D floorTex{};
    Texture2D ceilingTex[4]{};

    Model wallModel{};
    Model floorModel{};
    Model ceilModel{};

    Camera3D camera{};

    float moveSpeed = 2.5f;
};
