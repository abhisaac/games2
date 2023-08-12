
#include "raylib.h"
#include <vector>

struct Rect {
    Rectangle rect;
    Color color;
};

struct Stroke {
    std::vector<Vector2> pos;
    Color color;
    // Stroke(Vector2 pos, Color col) : pos(pos), color (col) {}
    // Stroke () {}
    void draw() {
        bool first = true;
        Vector2 last;
        for (auto& p : pos) {
            if (first){first = !first; last = p;continue;}
            DrawLineBezier(last, p, 3, color);
            last = p;
        }
    }
};

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    Rect rects[8] = {
        {{0.f, 0.f, 30.f, 30.f}, DARKGRAY} ,
        {{30.f, 0.f, 30.f, 30.f}, ORANGE} ,
        {{60.f, 0.f, 30.f, 30.f}, PINK} ,
        {{90.f, 0.f, 30.f, 30.f}, RED },
        {{120.f, 0.f, 30.f, 30.f}, GREEN },
        {{150.f, 0.f, 30.f, 30.f}, SKYBLUE },
        {{180.f, 0.f, 30.f, 30.f}, DARKBLUE} ,
        {{210.f, 0.f, 30.f, 30.f}, BROWN} ,
    };
    bool draw = true;
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "!! Paint");

    Vector2 ballPosition = { (float)screenWidth/2, (float)screenHeight/2 };

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------
    std::vector<Stroke> strokes(100);
    // Main game loop
    Rect current = rects[0];
    int s = 0;
    bool isdrawing = false;
    bool isundo = false;
    bool select = false;
    bool down = false;
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyDown(KEY_RIGHT)) ballPosition.x += 2.0f;
        if (IsKeyDown(KEY_LEFT)) ballPosition.x -= 2.0f;
        if (IsKeyDown(KEY_UP)) ballPosition.y -= 2.0f;
        if (IsKeyDown(KEY_DOWN)) ballPosition.y += 2.0f;
        //----------------------------------------------------------------------------------
            // TODO: each stroke gets loaded into the undo stack
//                     undo(s) [DONE]
//                    redo(s)
//   color picker [DONE]
// brush picker
// save .png etc.
        // Draw
        if (IsKeyUp(KEY_Z) || IsKeyUp(KEY_LEFT_CONTROL)) isundo = false;
        // DrawText("move the ball with arrow keys", 10, 10, 20, DARKGRAY);
        else if (!isundo && IsKeyDown(KEY_Z) && IsKeyDown(KEY_LEFT_CONTROL)) {
            isundo = true;
            strokes[s-1].pos.clear();
            --s;
            if (s<0) s=0;
        }
        // isdrawing = false;
        
            // if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                
                
            // }
            Vector2 mousePoint = GetMousePosition();
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                bool valid = true;
                for (auto& r : rects) {
                     if (CheckCollisionPointRec(mousePoint, r.rect)) {
                        valid = false;
                        break;
                     }
                }
                if (valid) {
                    strokes[s].color = current.color;
                    strokes[s].pos.emplace_back(mousePoint);
                    // isdrawing = true;
                }
                down = true;
            }
            else if (down && IsMouseButtonUp(MOUSE_BUTTON_LEFT)) {
                bool select = false;
                for (auto& r : rects) {
                     if (CheckCollisionPointRec(mousePoint, r.rect)) {
                        current = r;
                        select = true;  
                     }
                }
                if (!select) {
                    strokes[s].color = current.color;
                    ++s;
                    
                }
                down = false;
                // isdrawing = false;
                
                // select = false;
            }
            
if (draw) {
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);
            DrawRectangleLines(0, 0, 30, 30, RED);

// debug
            // char ss[3];
            // sprintf(ss,"%d", s);
            // DrawText(ss, 270, 0, 50, RED);
            
            for (auto& r : rects) {
                DrawRectangleRec(r.rect, r.color);
            }
            // highlight current rect
            DrawRectangleLinesEx(current.rect, 6.f, GOLD);  
            
            // for(int i = 0; i <= s; ++i) {
            for(auto&s : strokes)
                s.draw();
            // }
              

        EndDrawing();
}
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}