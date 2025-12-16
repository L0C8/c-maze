#include "game.h"

#include "raylib.h"
#include "rlgl.h"
#include "maze_builder.h"
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

    Mesh planeMesh = GenMeshPlane(1.0f, 1.0f, 1, 1);
    wallModel = LoadModelFromMesh(planeMesh);
    SetMaterialTexture(&wallModel.materials[0], MATERIAL_MAP_ALBEDO, wallTex);

    Mesh floorMesh = GenMeshPlane(1.0f, 1.0f, 1, 1);
    floorModel = LoadModelFromMesh(floorMesh);
    SetMaterialTexture(&floorModel.materials[0], MATERIAL_MAP_ALBEDO, floorTex);

    Mesh ceilMesh = GenMeshPlane(1.0f, 1.0f, 1, 1);
    ceilModel = LoadModelFromMesh(ceilMesh);
    SetMaterialTexture(&ceilModel.materials[0], MATERIAL_MAP_ALBEDO, ceilingTex[0]); 

    camera.position = { 1.5f, 0.75f, 1.5f };
    camera.target = { 2.5f, 0.75f, 1.5f };
    camera.up = { 0.0f, 1.0f, 0.0f };
    camera.fovy = 70.0f;
    camera.projection = CAMERA_PERSPECTIVE;
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

    UpdateCameraPro(&camera, move, rot, 0.0f);
}

void Game::Draw()
{
    const float wallHeight = 1.0f;

    BeginDrawing();
    ClearBackground(SKYBLUE);

    BeginMode3D(camera);
    rlDisableBackfaceCulling(); // Draw planes visible from both sides.

    for (int z = 0; z < kMapHeight; z++)
    {
        for (int x = 0; x < kMapWidth; x++)
        {
            Vector3 tileCenter = { static_cast<float>(x), 0.0f, static_cast<float>(z) };

            DrawModelEx(floorModel, { tileCenter.x, 0.0f, tileCenter.z }, { 0, 1, 0 }, 0.0f, { 1.0f, 1.0f, 1.0f }, WHITE);
            
            Texture2D ceilTex = ceilingTex[(x + z) % 4];
            SetMaterialTexture(&ceilModel.materials[0], MATERIAL_MAP_ALBEDO, ceilTex);
            DrawModelEx(ceilModel, { tileCenter.x, 1.0f, tileCenter.z }, { 0, 1, 0 }, 0.0f, { 1.0f, 1.0f, 1.0f }, WHITE);

            if (map[z][x] == 1)
            {
                // Use orientation based on edge direction.
                if (x == 0 || x == kMapWidth - 1)
                {
                    DrawModelEx(wallModel, { tileCenter.x, wallHeight * 0.5f, tileCenter.z }, { 0, 0, 1 }, 90.0f, { wallHeight, 1.0f, 1.0f }, WHITE);
                }
                else if (z == 0 || z == kMapHeight - 1)
                {
                    DrawModelEx(wallModel, { tileCenter.x, wallHeight * 0.5f, tileCenter.z }, { 1, 0, 0 }, 90.0f, { 1.0f, 1.0f, wallHeight }, WHITE);
                }
            }
        }
    }

    rlEnableBackfaceCulling();
    EndMode3D();

    EndDrawing();
}
