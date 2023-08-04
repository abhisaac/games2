
#include "raylib.h"
int main(){
  InitWindow(800, 450, "raylib [core] example - basic window");
  SetTargetFPS(60);
  while (!WindowShouldClose()){
    
    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawText("Congrats! You created your first window!", 200, 200, 20, RED);
    EndDrawing();
  }

  CloseWindow();
  return 0;
}

