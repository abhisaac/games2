
#include "raylib.h"
#include <vector>
#include <set>
#include <iostream>

enum SHAPE {
    RECTANGLE,
    CIRCLE,
    TRIANGLE,
    LINE
} ;

struct ShapePicker {
    static const int N = 4;
    std::vector<Rectangle> rect  {
        {0.f, 40.f, 30.f, 30.f},
        {0.f, 70.f, 30.f, 30.f} ,
        {0.f, 100.f, 30.f, 30.f} ,
        {0.f, 130.f, 30.f, 30.f},
    };

    const char* shapes_str[N] = {"R", "C", "T", "L"};

    struct {
        Rectangle rect;
        SHAPE shape;
    } current;
    int idx = 0;
    void prev() {
        if (idx == 0) idx = N-1;
        else --idx;
        updateCurrent(idx);
    }
    void next(){
        updateCurrent( (idx+1)%N);
    }
    void updateCurrent(int i) {
        idx = i;
        current.rect = rect[i];
        current.shape = (SHAPE)i;
    }
    ShapePicker()  {
        current.rect = rect[0];
        current.shape = RECTANGLE;
    }
    void draw(){
        for (int i = 0; i < N; ++i)
            DrawText(shapes_str[i], rect[i].x + 4.f, rect[i].y+4.f, 20.f, BLACK);
        // highlight current rect
        DrawRectangleLinesEx(current.rect, 6.f, GOLD);  
    }
};

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

    int idx = 0;
    void updateCurrent(int i) { 
        idx = i; 
        current.color = color[i];
        current.rect = rect[i];
    }
    
    void next() {
        updateCurrent((idx+1)%N);
    }
    ColorPicker() : current({rect[0], color[0]}) {}
    void draw(){
        for (int i = 0; i < N; ++i)
            DrawRectangleRec(rect[i], color[i]);
        // highlight current rect
        DrawRectangleLinesEx(current.rect, 6.f, GOLD);  
    }
};

struct Shape {
    Color color;
    virtual void push(Vector2 pos) {}
    virtual void draw() {}
    virtual void clear() {}
};

struct LineShape : public Shape {
    std::vector<Vector2> pos;
    LineShape(Vector2 newpos) {pos.emplace_back(newpos);}
    void draw() {
        bool first = true;
        Vector2 last;
        for (auto& p : pos) {
            if (first){first = !first; last = p;continue;}
            DrawLineBezier(last, p, 3, color);
            last = p;
        }
    }

    void push(Vector2 newpos) {
        pos.emplace_back(newpos);
    }

    void clear() {
        pos.clear();
    }
};

struct RectShape : public Shape {
    Vector2 start, end;
    RectShape(Vector2 pos) : start(pos), end(pos) {}
    void draw() {
        DrawRectangle(std::min(start.x,end.x), std::min(start.y, end.y), abs(end.x-start.x), abs(end.y-start.y), color);
    }
    void clear() {
        
    }
    void push(Vector2 newpos) {
        end = newpos;
    }
};

struct CircleShape : public Shape {
    Vector2 start, end;
    CircleShape(Vector2 pos) : start(pos), end(pos) {}
    void draw() {
        DrawCircle(start.x, start.y, std::max(abs(end.x-start.x), abs(end.y-start.y)), color);
    }
    void clear() {
        
    }
    void push(Vector2 newpos) {
        end = newpos;
    }
};

// TODO: implement triangle
struct TriangleShape : public Shape {
    Vector2 start, end;
    TriangleShape(Vector2 pos) : start(pos), end(pos) {}
    void draw() {
        DrawRectangle(start.x, start.y, end.x-start.x, end.y-start.y, color);
    }
    void clear() {
        
    }
    void push(Vector2 newpos) {
        end = newpos;
    }
};

int main(void)
{
    ColorPicker cp;
    ShapePicker sp;

    const int screenWidth = 1200;
    const int screenHeight = 800;

    InitWindow(screenWidth, screenHeight, "!! Paint");

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------
    std::vector<Shape*> strokes(100);
    // Main game loop

    int s = 0; // stroke counter
    
    bool isundo = false;
    bool isredo = false;
    bool down = false;
    bool save = false;

    Rectangle hover;
    bool showhover = false;
    
    Shape* transient = nullptr;
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        //save
        if (IsKeyUp(KEY_S) || IsKeyUp(KEY_LEFT_CONTROL)) save = false;
        if (!save && IsKeyDown(KEY_S) && IsKeyDown(KEY_LEFT_CONTROL)) {
           TakeScreenshot("tmp.png"); // TODO: Fix png size; edge pixels bleeding out
            save = true;
        }

        //redo
        if (IsKeyUp(KEY_Y) || IsKeyUp(KEY_LEFT_CONTROL)) isredo = false;
        else if (!isredo && IsKeyDown(KEY_Y) && IsKeyDown(KEY_LEFT_CONTROL)) {
            isredo = true;
            if (strokes[s])
                ++s;
        }

        //undo
        if (IsKeyUp(KEY_Z) || IsKeyUp(KEY_LEFT_CONTROL)) isundo = false;
        else if (!isundo && IsKeyDown(KEY_Z) && IsKeyDown(KEY_LEFT_CONTROL)) {
            isundo = true;
            --s;
            if (s<0) s=0;
        }
        
        Vector2 mousePoint = GetMousePosition();
        //hover on tools
        for (auto& r : cp.rect) {
            if (CheckCollisionPointRec(mousePoint, r)) {
                hover = r;
                showhover = true;
                break;
            }
        }
        for (auto& r : sp.rect) {
            if (CheckCollisionPointRec(mousePoint, r)) {
                hover = r;
                showhover = true;
                break;
            }
        }

        // cycle through color picker (right click)
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            cp.next();
        }

        // cycle through shape picker (mouse wheel)
        auto wheel = GetMouseWheelMove();
        if (wheel == -1) {sp.prev();}
        if (wheel == 1) {sp.next();}
        
        //click
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            bool valid = true;

            // disable clicks on tools while drawing
            for (auto& r : cp.rect) {
                if (CheckCollisionPointRec(mousePoint, r)) {
                    valid = false;
                    break;
                }
            }
            for (auto& r : sp.rect) {
                if (CheckCollisionPointRec(mousePoint, r)) {
                    valid = false;
                    break;
                }
            }

            if (valid) {
                if (!transient) {
                    // TODO: refactor this below
                    switch (sp.current.shape)
                    {
                    case SHAPE::RECTANGLE:
                    {
                        auto ls =  new RectShape(mousePoint);
                        ls->color = cp.current.color;
                        transient = ls;
                    }
                        break;
                    case SHAPE::CIRCLE:
                    {
                        auto ls =  new CircleShape(mousePoint);
                        ls->color = cp.current.color;
                        transient = ls;
                    }
                        break;
                    case SHAPE::TRIANGLE:
                    {
                        auto ls =  new TriangleShape(mousePoint);
                        ls->color = cp.current.color;
                        transient = ls;
                    }
                        break;
                    case SHAPE::LINE:
                    {
                        auto ls =  new LineShape(mousePoint);
                        ls->color = cp.current.color;
                        transient = ls;
                    }
                        break;
                    
                    default:
                        break;
                    }
                } else {
                    transient->push(mousePoint);
                }
            }
            down = true;
        }
        else if (down && IsMouseButtonUp(MOUSE_BUTTON_LEFT)) {
            bool select = false;
            int i = 0;
            for (auto& r : cp.rect) {
                if (CheckCollisionPointRec(mousePoint, r)) {
                    cp.updateCurrent(i);
                    select = true;  
                    break;
                }
                ++i;
            }
            i = 0;
            for (auto& r : sp.rect) {
                if (CheckCollisionPointRec(mousePoint, r)) {
                    sp.current.shape = (SHAPE)i;
                    sp.current.rect = r;
                    select = true;  
                    break;
                }
                ++i;
            }
            if (!select) {
                //commit transaction

                // cleanup redo stack
                int i = s;
                while(strokes[i++]) {
                    delete(strokes[i]);
                    strokes[i] = nullptr;
                }

                //commit
                strokes[s] = transient;
                transient = nullptr;
                ++s;
            }
            down = false;
        }
            

        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);
            DrawRectangleLines(0, 0, 30, 30, RED);

            if (0) {
                // debug shape counter
                char ss[3];
                sprintf(ss,"%d", s);
                DrawText(ss, 270, 0, 50, RED);
            }
            
            //tools
            cp.draw();
            sp.draw();
      
            if (showhover) {
                DrawRectangleLinesEx(hover, 7.f, GRAY);  
                showhover = false;
            }

            for(int i = 0; i < s; ++i) {
                strokes[i]->draw();
            }
            if (transient)   transient->draw();

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    CloseWindow();
    return 0;
}