#pragma once

#include "raylib.h"
#include <vector>

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
    static constexpr float kWallHeight = 5.0f;
    static constexpr int kMapWidth = 16;
    static constexpr int kMapHeight = 16;
    int map[kMapHeight][kMapWidth] = {};

    Texture2D wallTex{};
    Texture2D floorTex{};
    Texture2D ceilingTex[4]{};

    Model wallModel{};
    Model floorModel{};
    Model ceilModel{};

    Camera3D camera{};

    float moveSpeed = 2.5f;
    float gravity = 9.8f;
    float jumpSpeed = 4.5f;
    float verticalVelocity = 0.0f;
    const float eyeHeight = 0.75f;

    std::vector<Vector3> jumpBlocks;
    const float blockHeight = 1.25f;
};
