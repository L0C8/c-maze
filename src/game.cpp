#include "game.h"

#include "raylib.h"
#include "maze_builder.h"
#include <algorithm>
#include <cmath>

#define SPRITE_SIZE 16

static Texture2D LoadTileTexture(const Image &sheet, int tileIndex)
{
    Rectangle rec = { static_cast<float>(tileIndex * SPRITE_SIZE), 0, SPRITE_SIZE, SPRITE_SIZE };
    Image tile = ImageFromImage(sheet, rec);
    Texture2D tex = LoadTextureFromImage(tile);
    UnloadImage(tile);
    SetTextureFilter(tex, TEXTURE_FILTER_POINT);
    return tex;
}

Game::Game()
{
    GenerateMaze(kMapWidth, kMapHeight, &map[0][0]);

    Image sheet = LoadImage("assets/textures/spritesheet.png");
    wallTex = LoadTileTexture(sheet, 0);
    floorTex = LoadTileTexture(sheet, 1);
    ceilingTex[0] = LoadTileTexture(sheet, 2);
    ceilingTex[1] = LoadTileTexture(sheet, 3);
    ceilingTex[2] = LoadTileTexture(sheet, 4);
    ceilingTex[3] = LoadTileTexture(sheet, 5);
    UnloadImage(sheet);

    Mesh cubeMesh = GenMeshCube(1.0f, 1.0f, 1.0f);

    wallModel = LoadModelFromMesh(cubeMesh);
    SetMaterialTexture(&wallModel.materials[0], MATERIAL_MAP_ALBEDO, wallTex);

    Mesh floorMesh = GenMeshCube(1.0f, 1.0f, 1.0f);
    floorModel = LoadModelFromMesh(floorMesh);
    SetMaterialTexture(&floorModel.materials[0], MATERIAL_MAP_ALBEDO, floorTex);

    Mesh ceilMesh = GenMeshCube(1.0f, 1.0f, 1.0f);
    ceilModel = LoadModelFromMesh(ceilMesh);
    SetMaterialTexture(&ceilModel.materials[0], MATERIAL_MAP_ALBEDO, ceilingTex[0]); 

    camera.position = { 1.5f, 0.75f, 1.5f };
    camera.target = { 2.5f, 0.75f, 1.5f };
    camera.up = { 0.0f, 1.0f, 0.0f };
    camera.fovy = 70.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    jumpBlocks = {
        {3.5f, 0.0f, 3.5f},
        {5.5f, 0.0f, 4.5f},
        {7.5f, 0.0f, 5.5f},
        {9.5f, 0.0f, 6.5f},
        {11.5f, 0.0f, 7.5f},
        {6.5f, 0.0f, 9.5f},
        {8.5f, 0.0f, 10.5f},
    };
}

Game::~Game()
{
    UnloadModel(wallModel);
    UnloadModel(floorModel);
    UnloadModel(ceilModel);
    UnloadTexture(wallTex);
    UnloadTexture(floorTex);
    for (Texture2D &t : ceilingTex) UnloadTexture(t);
}

void Game::Update()
{
    const float dt = GetFrameTime();
    const float mouseSensitivity = 0.25f;

    const float playerRadius = 0.25f;
    const float wallHeight = kWallHeight;

    Vector3 move = {0};
    move.x = (IsKeyDown(KEY_W) ? 1.0f : 0.0f) - (IsKeyDown(KEY_S) ? 1.0f : 0.0f);
    move.y = (IsKeyDown(KEY_D) ? 1.0f : 0.0f) - (IsKeyDown(KEY_A) ? 1.0f : 0.0f);

    float len = std::sqrt((move.x * move.x) + (move.y * move.y));
    if (len > 0.0001f)
    {
        move.x /= len;
        move.y /= len;
    }
    move.x *= moveSpeed * dt;
    move.y *= moveSpeed * dt;

    Vector2 mouseDelta = GetMouseDelta();
    Vector3 rot = { mouseDelta.x * mouseSensitivity, mouseDelta.y * mouseSensitivity, 0.0f };

    Vector3 prevPos = camera.position;
    Vector3 prevTarget = camera.target;

    UpdateCameraPro(&camera, move, rot, 0.0f);
    Vector3 viewOffset = { camera.target.x - camera.position.x, camera.target.y - camera.position.y, camera.target.z - camera.position.z };

    auto GroundHeight = [&](const Vector3 &pos) -> float
    {
        float ground = 0.0f;
        for (const auto &b : jumpBlocks)
        {
            if (std::abs(pos.x - b.x) <= 0.5f && std::abs(pos.z - b.z) <= 0.5f)
            {
                ground = std::max(ground, blockHeight);
            }
        }
        return ground;
    };

    const float groundY = GroundHeight(camera.position) + eyeHeight;
    const bool onGround = camera.position.y <= groundY + 0.01f;

    if (onGround)
    {
        camera.position.y = groundY;
        verticalVelocity = 0.0f;
        if (IsKeyPressed(KEY_SPACE))
        {
            verticalVelocity = jumpSpeed;
        }
    }
    else
    {
        verticalVelocity -= gravity * dt;
    }

    camera.position.y += verticalVelocity * dt;
    if (camera.position.y < groundY)
    {
        camera.position.y = groundY;
        verticalVelocity = 0.0f;
    }

    auto Collides = [&](const Vector3 &pos) -> bool
    {
        int minX = static_cast<int>(std::floor(pos.x - playerRadius)) - 1;
        int maxX = static_cast<int>(std::ceil(pos.x + playerRadius)) + 1;
        int minZ = static_cast<int>(std::floor(pos.z - playerRadius)) - 1;
        int maxZ = static_cast<int>(std::ceil(pos.z + playerRadius)) + 1;

        minX = std::max(0, minX);
        minZ = std::max(0, minZ);
        maxX = std::min(kMapWidth - 1, maxX);
        maxZ = std::min(kMapHeight - 1, maxZ);

        for (int z = minZ; z <= maxZ; ++z)
        {
            for (int x = minX; x <= maxX; ++x)
            {
                if (map[z][x] != 1) continue;

                const float wallBaseY = 0.0f;
                const float wallTopY = wallHeight;
                const float minY = wallBaseY;
                const float maxY = wallTopY;

                const float minWX = static_cast<float>(x) - 0.5f;
                const float maxWX = static_cast<float>(x) + 0.5f;
                const float minWZ = static_cast<float>(z) - 0.5f;
                const float maxWZ = static_cast<float>(z) + 0.5f;

                const float closestX = std::clamp(pos.x, minWX, maxWX);
                const float closestY = std::clamp(pos.y, minY, maxY);
                const float closestZ = std::clamp(pos.z, minWZ, maxWZ);

                const float dx = pos.x - closestX;
                const float dy = pos.y - closestY;
                const float dz = pos.z - closestZ;
                const float dist2 = dx * dx + dy * dy + dz * dz;

                if (dist2 < playerRadius * playerRadius)
                {
                    return true;
                }
            }
        }
        return false;
    };

    const Vector3 delta = { camera.position.x - prevPos.x, camera.position.y - prevPos.y, camera.position.z - prevPos.z };
    if (Collides(camera.position))
    {
        Vector3 slidePos = camera.position;
        bool resolved = false;

        // Try keeping Z and sliding X.
        Vector3 tryX = { prevPos.x + delta.x, camera.position.y, prevPos.z };
        if (!Collides(tryX))
        {
            slidePos = tryX;
            resolved = true;
        }

        // Try keeping X and sliding Z.
        if (!resolved)
        {
            Vector3 tryZ = { prevPos.x, camera.position.y, prevPos.z + delta.z };
            if (!Collides(tryZ))
            {
                slidePos = tryZ;
                resolved = true;
            }
        }

        if (resolved)
        {
            camera.position = slidePos;
        }
        else
        {
            camera.position = prevPos;
        }
    }

    // Keep within bounds.
    camera.position.x = std::clamp(camera.position.x, 0.5f, kMapWidth - 1.5f);
    camera.position.z = std::clamp(camera.position.z, 0.5f, kMapHeight - 1.5f);

    camera.target = { camera.position.x + viewOffset.x, camera.position.y + viewOffset.y, camera.position.z + viewOffset.z };
}

void Game::Draw()
{
    const float wallHeight = kWallHeight;
    const float ceilingY = wallHeight;

    BeginDrawing();
    ClearBackground(SKYBLUE);

    BeginMode3D(camera);

    for (int z = 0; z < kMapHeight; z++)
    {
        for (int x = 0; x < kMapWidth; x++)
        {
            Vector3 tileCenter = { static_cast<float>(x), 0.0f, static_cast<float>(z) };

            DrawModelEx(floorModel, { tileCenter.x, -0.05f, tileCenter.z }, { 0, 1, 0 }, 0.0f, { 1.0f, 0.1f, 1.0f }, WHITE);
            
            Texture2D ceilTex = ceilingTex[(x + z) % 4];
            SetMaterialTexture(&ceilModel.materials[0], MATERIAL_MAP_ALBEDO, ceilTex);
            DrawModelEx(ceilModel, { tileCenter.x, ceilingY + 0.05f, tileCenter.z }, { 0, 1, 0 }, 0.0f, { 1.0f, 0.1f, 1.0f }, WHITE);

            if (map[z][x] == 1)
            {
                for (int h = 0; h < static_cast<int>(kWallHeight); ++h)
                {
                    DrawModelEx(wallModel, { tileCenter.x, 0.5f + h, tileCenter.z }, { 0, 1, 0 }, 0.0f, { 1.0f, 1.0f, 1.0f }, WHITE);
                }
            }
        }
    }

    // Jump blocks to leap between.
    for (const auto &b : jumpBlocks)
    {
        DrawModelEx(wallModel, { b.x, blockHeight * 0.5f, b.z }, { 0, 1, 0 }, 0.0f, { 1.0f, blockHeight, 1.0f }, WHITE);
    }

    EndMode3D();

    EndDrawing();
}
