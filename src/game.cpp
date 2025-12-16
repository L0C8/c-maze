#include "game.h"

#include "raylib.h"
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

    camera.position = { 2.5f, 0.75f, 2.5f };
    camera.target = { 1.0f, 1.0f, 2.5f };
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
    const float mouseSensitivity = 0.15f;

    Vector3 move = {0};
    if (IsKeyDown(KEY_W)) move.x += 1.0f;
    if (IsKeyDown(KEY_S)) move.x -= 1.0f;
    if (IsKeyDown(KEY_D)) move.y += 1.0f;
    if (IsKeyDown(KEY_A)) move.y -= 1.0f;

    float len = std::sqrt(move.x * move.x + move.y * move.y);
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
            DrawModelEx(ceilModel, { tileCenter.x, 1.05f, tileCenter.z }, { 0, 1, 0 }, 0.0f, { 1.0f, 0.1f, 1.0f }, WHITE);

            if (map[z][x] == 1)
            {
                DrawModelEx(wallModel, { tileCenter.x, 0.5f, tileCenter.z }, { 0, 1, 0 }, 0.0f, { 1.0f, 1.0f, 1.0f }, WHITE);
            }
        }
    }

    EndMode3D();

    EndDrawing();
}
