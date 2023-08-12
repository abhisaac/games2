
#include "raylib.h"
#include <vector>

struct ColorPicker {
    static const int N = 8;
    std::vector<Rectangle> rect  {
        {0.f, 0.f, 30.f, 30.f},
        {30.f, 0.f, 30.f, 30.f} ,
        {60.f, 0.f, 30.f, 30.f} ,
        {90.f, 0.f, 30.f, 30.f},
        {120.f, 0.f, 30.f, 30.f},
        {150.f, 0.f, 30.f, 30.f},
        {180.f, 0.f, 30.f, 30.f} ,
        {210.f, 0.f, 30.f, 30.f},
    };
    
    std::vector<Color> color  {
        DARKGRAY,
        ORANGE ,
        PINK ,
        RED ,
        GREEN ,
        SKYBLUE ,
        DARKBLUE ,
        BROWN ,
    };

    struct {
        Rectangle rect;
        Color color;
    } current;
    
    ColorPicker() : current({rect[0], color[0]}) {}
    void draw(){
        for (int i = 0; i < N; ++i)
            DrawRectangleRec(rect[i], color[i]);
        // highlight current rect
        DrawRectangleLinesEx(current.rect, 6.f, GOLD);  
    }
};

struct Stroke {
    std::vector<Vector2> pos;
    Color color;
    
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

int main(void)
{
    ColorPicker cp;

    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "!! Paint");

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------
    std::vector<Stroke> strokes(100);
    // Main game loop

    int s = 0; // stroke counter

    bool isundo = false;
    bool down = false;
    
    Rectangle hover;
    bool showhover = false;
    
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
    
        // TODO: 
        // redo(s)
        // brush picker
        // save .png etc.

        if (IsKeyUp(KEY_Z) || IsKeyUp(KEY_LEFT_CONTROL)) isundo = false;
        
        else if (!isundo && IsKeyDown(KEY_Z) && IsKeyDown(KEY_LEFT_CONTROL)) {
            isundo = true;
            strokes[s-1].pos.clear();
            --s;
            if (s<0) s=0;
        }
        
        Vector2 mousePoint = GetMousePosition();
        //hover
        for (auto& r : cp.rect) {
            if (CheckCollisionPointRec(mousePoint, r)) {
                hover = r;
                showhover = true;
                break;
            }
        }

        //click
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            bool valid = true;
            for (auto& r : cp.rect) {
                if (CheckCollisionPointRec(mousePoint, r)) {
                    valid = false;
                    break;
                }
            }
            if (valid) {
                strokes[s].color = cp.current.color;
                strokes[s].pos.emplace_back(mousePoint);
            }
            down = true;
        }
        else if (down && IsMouseButtonUp(MOUSE_BUTTON_LEFT)) {
            bool select = false;
            int i = 0;
            for (auto& r : cp.rect) {
                if (CheckCollisionPointRec(mousePoint, r)) {
                    cp.current.color = cp.color[i];
                    cp.current.rect = r;
                    select = true;  
                    break;
                }
                ++i;
            }
            if (!select) {
                strokes[s].color = cp.current.color;
                ++s;
            }
            down = false;
        }
            

        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);
            DrawRectangleLines(0, 0, 30, 30, RED);

// debug
            // char ss[3];
            // sprintf(ss,"%d", s);
            // DrawText(ss, 270, 0, 50, RED);
            
            // color picker
            cp.draw();
            // shape picker
      
            if (showhover) {
                DrawRectangleLinesEx(hover, 7.f, GRAY);  
                showhover = false;
            }
            
            // for(int i = 0; i <= s; ++i) {
            for(auto&s : strokes)
                s.draw();
            // }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    CloseWindow();
    return 0;
}