
#include "raylib.h"
const int W = 800;
const int H = 600;
int main(){
  InitWindow(W, H, "raylib [core] example - basic window");
  SetTargetFPS(60);
  while (!WindowShouldClose()){
    
    BeginDrawing();
    ClearBackground(BLACK);
    // DrawText("Congrats! You created your first window!", 200, 200, 20, RED);
    EndDrawing();
  }

  CloseWindow();
  return 0;
}

