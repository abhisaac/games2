#include "raylib.h"
#include "raymath.h"

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [2d] example - world space panning");

    Camera2D cam = { 0 };
    cam.zoom = 1;
    cam.offset.x = GetScreenWidth() / 2.0f;
    cam.offset.y = GetScreenHeight() / 2.0f;

    Vector2 prevMousePos = GetMousePosition();

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        float mouseDelta = GetMouseWheelMove();

        float newZoom = cam.zoom + mouseDelta * 0.01f;
        if (newZoom <= 0)
            newZoom = 0.01f;

        cam.zoom = newZoom;

        Vector2 thisPos = GetMousePosition();

        Vector2 delta = Vector2Subtract(prevMousePos, thisPos);
        prevMousePos = thisPos;

        if (IsMouseButtonDown(0))
            cam.target = GetScreenToWorld2D(Vector2Add(cam.offset, delta),cam);

        if (IsKeyPressed(KEY_LEFT))
            cam.rotation += 10;
        else if (IsKeyPressed(KEY_RIGHT))
            cam.rotation -= 10;
            
        BeginDrawing();

        ClearBackground(RAYWHITE);

        BeginMode2D(cam);

        float size = 5000;

        for (float i = -size; i <= size; i += 10)
        {
            DrawLine(i, -size, i, size, GRAY);
            DrawLine(-size, i, size, i, GRAY);
        }

        DrawLine(-size, 0, size, 0, RED);
        DrawLine(0, -size, 0, size, RED);

        Vector2 mapGrid = GetScreenToWorld2D(GetMousePosition(), cam);
        mapGrid.x = floorf(mapGrid.x / 10) * 10.0f;
        mapGrid.y = floorf(mapGrid.y / 10) * 10.0f;

        DrawRectangle(mapGrid.x, mapGrid.y, 10, 10, BLUE);

        EndMode2D();

        DrawText(TextFormat("%4.0f %4.0f", mapGrid.x, mapGrid.y),10, 10, 20, BLACK);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}