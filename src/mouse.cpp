
#include "raylib.h"
#include <unordered_set>

float abs2(float x) {
    return x > 0 ? x : -x;
}
typedef struct MyVector2 {
    float x;                // Vector x component
    float y;                // Vector y component
    bool end;
    bool valid;
    bool start;
    bool operator==(const MyVector2& other) const {
        return abs2(x - other.x) < 5 && abs2(y - other.y) < 5;
    }
} MyVector2;

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    bool grid[screenWidth][screenHeight];

    for (int i = 0; i < screenWidth; ++i) {
      for (int j = 0; j < screenHeight; ++j) {
        grid[i][j] = false;
      }
    }

    InitWindow(screenWidth, screenHeight, "!! Paint");
    float eraserSize = 10.0, penSize = 2.0;
    SetTargetFPS(60);  

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
           
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
               if (auto dt = GetMouseWheelMove()){
                penSize += dt;
              }
              auto [x,y] = GetMousePosition();
              if (x < screenWidth && y < screenHeight && x > -1 && y > -1)
                grid[(int)x][(int)y] = true;
            } else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
              if (auto dt = GetMouseWheelMove()){
                eraserSize += dt;
              }
              auto [x,y] = GetMousePosition();
              for (auto i = -eraserSize; i  < eraserSize ; ++i) {
                for (auto j = -eraserSize; j< eraserSize; ++j) {
                  int x2 = (int)x + i;
                  int y2 = (int)y + j;
                  if (x2 < 0) x2 = 0;
                  if (y2 < 0) y2 = 0;
                  if (x2 > (screenWidth-1)) x2 = screenWidth-1;
                  if (y2 > (screenHeight-1)) y2 = screenHeight-1;
                  grid[x2][y2] = false;
                }
              }
              
              DrawCircle(x, y, eraserSize , GRAY);
            }
            else if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
                memset(grid, 0, screenHeight*screenWidth*sizeof(bool));
            }

             BeginDrawing();

            ClearBackground(BLACK);
            for (int i = 0; i < screenWidth; ++i) {
              for (int j = 0; j < screenHeight; ++j) {
                if (grid[i][j])
                  DrawCircle(i, j, penSize, SKYBLUE);
              }
            }
            EndDrawing();
        //----------------------------------------------------------------------------------
    }
    CloseWindow();        // Close window and OpenGL context

    return 0;
}