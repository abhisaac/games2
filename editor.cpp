
#include "raylib.h"
#include <string>
// #include <cstdio>
// #include <cstdlib>
#include <fstream>
#include <cmath>
// #include <sstream>
// #include <iostream>
// #include <iostream>
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
// #include <string>
/*TODO
* resize tool
* undo move/resize etc.
* UUID for objects instead of index in strokes
* clone
* grid 
* player physics
* texture loading
*/

//helpers
float min(float x, float y) { return x < y ? x : y;}
float max(float x, float y) { return x > y ? x : y;}


enum SHAPE {
    RECTANGLE,
    CIRCLE,
    SELECT,
    // TRIANGLE,
    // LINE
    _SHAPE_SIZE
} ;



struct ShapePicker {
    static const int N = _SHAPE_SIZE;
    Rectangle rect[N]  {
        {0.f, 40.f, 30.f, 30.f},
        {0.f, 70.f, 30.f, 30.f} ,
        {0.f, 100.f, 30.f, 30.f} ,
        // {0.f, 130.f, 30.f, 30.f},
    };

    const char* shapes_str[4] = {"R", "C", "+", "L"};

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
    Rectangle rect[N]  {
        {0.f, 0.f, 30.f, 30.f},
        {30.f, 0.f, 30.f, 30.f} ,
        {60.f, 0.f, 30.f, 30.f} ,
        {90.f, 0.f, 30.f, 30.f},
        {120.f, 0.f, 30.f, 30.f},
        {150.f, 0.f, 30.f, 30.f},
        {180.f, 0.f, 30.f, 30.f} ,
        {210.f, 0.f, 30.f, 30.f},
    };
    
    Color color[N]  {
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
    SHAPE shape;
    Color color;
    virtual void push(Vector2 pos) {}
    virtual void draw() {}
    virtual void clear() {}
    virtual bool intersects(Vector2 pos) {return false;}
    virtual void move(Vector2 dt) {}
    virtual char* serialize() { return nullptr;}
};

struct RectShape : public Shape {
    Vector2 start, end;
    Rectangle bounds;
    RectShape() {shape = RECTANGLE;}
    RectShape(Vector2 pos) : start(pos), end(pos) { shape=RECTANGLE;}
    void draw() {
        DrawRectangle(bounds.x, bounds.y, bounds.width, bounds.height, color);
    }
    void clear() {
        
    }
    char* serialize() {
        char* out = (char*) malloc(100*sizeof(char));
        sprintf(out, "RECTANGLE %f %f %f %f COLOR %u %u %u %u", 
                    bounds.x, bounds.y, bounds.width, bounds.height,
                    color.r, color.g, color.b, color.a);
        return out;
    }
    void move (Vector2 dt){
        bounds.x += dt.x;
        bounds.y += dt.y;
    }
    void push(Vector2 newpos) {
        end = newpos;
        bounds = {min(start.x,end.x), min(start.y, end.y), abs(end.x-start.x), abs(end.y-start.y)};
    }
    bool intersects(Vector2 pos) {
        return CheckCollisionPointRec(pos, bounds);
    }
};

struct CircleShape : public Shape {
    Vector2 start, end;
    float radius;
    CircleShape() {shape = CIRCLE;}
    CircleShape(Vector2 pos) : start(pos), end(pos) {shape = CIRCLE;}
    void draw() {
        DrawCircle(start.x, start.y, radius, color);
    }
    void clear() {
        
    }
    char* serialize() {
        char* out = (char*) malloc(100*sizeof(char));
        sprintf(out, "CIRCLE %f %f %f COLOR %u %u %u %u", start.x, start.y, radius,
                        color.r, color.g, color.b, color.a);
        return out;
    }
    void move (Vector2 dt){
        start.x += dt.x;
        start.y += dt.y;
    }
    void push(Vector2 newpos) {
        end = newpos;
        radius =  max(abs(end.x-start.x), abs(end.y-start.y));
    }
    bool intersects(Vector2 pos) {
        return CheckCollisionPointCircle(pos, start, radius);
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
    Shape* strokes[100] = {0};
    // Main game loop

    int s = 0; // stroke counter
    
    bool isundo = false;
    bool isredo = false;
    bool down = false;
    bool save = false;

    Rectangle hover;
    bool showhover = false;

    Rectangle rect {350, 10, 40, 10};
    
    Shape* transient = nullptr;

    Shape* selectTransient = nullptr;
    // Vector2 startMousePos;
    // bool select = false;
    
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        if (sp.current.shape == SHAPE::SELECT) SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        else SetMouseCursor(MOUSE_CURSOR_CROSSHAIR);
        //save
        if (IsKeyUp(KEY_S) || IsKeyUp(KEY_LEFT_CONTROL)) save = false;
        if (!save && IsKeyDown(KEY_S) && IsKeyDown(KEY_LEFT_CONTROL)) {
        //    TakeScreenshot("tmp.png"); // TODO: Fix png size; edge pixels bleeding out
            FILE* fid=fopen("test.map", "w");
            for (int i = 0;i < s; ++i){
                fprintf(fid, "%s\n", strokes[i]->serialize());
            }
            
            fclose(fid);
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
                SetMouseCursor(MOUSE_CURSOR_DEFAULT);
                hover = r;
                showhover = true;
                break;
            }
        }
        for (auto& r : sp.rect) {
            if (CheckCollisionPointRec(mousePoint, r)) {
                SetMouseCursor(MOUSE_CURSOR_DEFAULT);
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
        if (wheel == 1) {sp.prev();}
        if (wheel == -1) {sp.next();}
        
        //click
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            bool valid = true;

            // disable clicks on tools while drawing
            for (auto& r : cp.rect) {
                if (CheckCollisionPointRec(mousePoint, r)) {
                    SetMouseCursor(MOUSE_CURSOR_DEFAULT);
                    valid = false;
                    break;
                }
            }
            for (auto& r : sp.rect) {
                if (CheckCollisionPointRec(mousePoint, r)) {
                    SetMouseCursor(MOUSE_CURSOR_DEFAULT);
                    valid = false;
                    break;
                }
            }

            // if (sp.current.shape == SHAPE::SELECT) {
            //      for (int i = 0; i < s; ++i) {
            //         if (strokes[i]->intersects(mousePoint)) {
            //             // move shape with mouse drag
            //         }
            //      }
            // }
           

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
                    case SHAPE::SELECT:
                    {
                        // select = true;
                        if (!selectTransient) {
                            for (int i= s-1; i >=0 ;--i) {
                                if (strokes[i]->intersects(mousePoint)) {
                                    selectTransient = strokes[i];
                                    break;
                                }
                            }
                        } else {
                            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
                            selectTransient->move(GetMouseDelta());
                        }
                        
                    }
                        break;
                    // case SHAPE::LINE:
                    // {
                    //     auto ls =  new LineShape(mousePoint);
                    //     ls->color = cp.current.color;
                    //     transient = ls;
                    // }
                    //     break;
                    
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
                    SetMouseCursor(MOUSE_CURSOR_DEFAULT);
                    cp.updateCurrent(i);
                    select = true;  
                    break;
                }
                ++i;
            }
            i = 0;
            for (auto& r : sp.rect) {
                if (CheckCollisionPointRec(mousePoint, r)) {
                    SetMouseCursor(MOUSE_CURSOR_DEFAULT);
                    sp.current.shape = (SHAPE)i;
                    sp.current.rect = r;
                    select = true;  
                    break;
                }
                ++i;
            }
            if (!select && transient) {
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
            selectTransient = nullptr;
        }
            

        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);
            // Clear
            if (GuiButton({350, 0, 80, 40}, "#0#Clear")) { 
                int i = 0;
                while(strokes[i++]) {
                    delete(strokes[i]);
                    strokes[i] = nullptr;
                }
                s = 0;
            }

            // Load
            if (GuiButton({350+80, 0, 80, 40}, "#5#Load")) { 
                //clear
                int i = 0;
                while(strokes[i++]) {
                    delete(strokes[i]);
                    strokes[i] = nullptr;
                }
                s = 0;

                //load file to strokes
                std::ifstream ifs("test.map"); //, "r");
                // FILE* fid = fopen("temp.map", "r");
                // size_t n = 100;
                // char* line = (char*)malloc(sizeof(char) * n);
                std::string line;
                while(std::getline(ifs, line)) {
                    // std::string type, data;
                    char shape[25], data[70];
                    sscanf(line.c_str(), "%s %[^\n]", shape, data );
                    // std::stringstream ss(line);
                    // line >> type >> data;
                    if (strcmp(shape, "RECTANGLE") == 0) {
                        Rectangle r;
                        Color c;
                        sscanf(data, "%f %f %f %f COLOR %hhd %hhd %hhd %hhd",
                                    &r.x, &r.y, &r.width, &r.height,
                                    &c.r, &c.g, &c.b, &c.a);
                        auto tmp = new RectShape();
                        tmp->color = c;
                        tmp->bounds = r;
                        strokes[s++] = tmp;
                        // std::cout << "RECT" << std::endl;
                    } else if (strcmp(shape,"CIRCLE")==0) {
                        Vector2 center;
                        float radius;
                        Color c;
                        sscanf(data, "%f %f %f COLOR %hhd %hhd %hhd %hhd",
                                    &center.x, &center.y, &radius,
                                    &c.r, &c.g, &c.b, &c.a);
                        auto tmp = new CircleShape();
                        tmp->color = c;
                        tmp->radius = radius;
                        tmp->start = center;
                        strokes[s++] = tmp;
                        // std::cout << "CIRC" << std::endl;
                    }
                }

            }
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