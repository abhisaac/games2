
#include "raylib.h"
#include <vector>
int main2(){
  InitWindow(800, 450, "raylib [core] example - basic window");

  int x = 190; //, y = 200;

  while (!WindowShouldClose()){
    if (IsKeyDown(KEY_LEFT)) x-=.1f;
    if (IsKeyDown(KEY_RIGHT)) x+=.1f;
    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawText("Congrats! You created your first window!", x, 200, 20, RED);
    EndDrawing();
  }

  CloseWindow();
  return 0;
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "!! Paint");

    Vector2 ballPosition = { (float)screenWidth/2, (float)screenHeight/2 };

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------
    std::vector<Vector2> pos;
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyDown(KEY_RIGHT)) ballPosition.x += 2.0f;
        if (IsKeyDown(KEY_LEFT)) ballPosition.x -= 2.0f;
        if (IsKeyDown(KEY_UP)) ballPosition.y -= 2.0f;
        if (IsKeyDown(KEY_DOWN)) ballPosition.y += 2.0f;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            // DrawText("move the ball with arrow keys", 10, 10, 20, DARKGRAY);
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) pos.emplace_back(GetMousePosition());
            if (IsMouseButtonUp(MOUSE_BUTTON_LEFT)) pos.clear();
            bool first = true;
            Vector2 last;

            for(auto&p : pos) {
              if (first){first = !first; last = p;continue;}
              
              DrawLineBezier(last, p, 3, MAROON);
              last = p;
              // DrawCircleV(p, 3, MAROON);
            }
              

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}