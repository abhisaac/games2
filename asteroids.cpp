/* Asteroids:
    Another classic game
*/
#include "raylib.h"
int main(){
  InitWindow(800, 450, "!! Asteroids");
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

