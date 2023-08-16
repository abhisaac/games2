
#include "raylib.h"
#include <string>
// #include <cstdio>
// #include <cstdlib>
#include <fstream>
#include <cmath>
#include <set>
// #include <sstream>
// #include <iostream>
// #include <iostream>
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
// #include <string>
/*TODO
* Enemies
* player animation run/jump etc
* 
* fix jumping/ collision detection
* disable/enable tools
* edit mode vs game mode
* game pause
* hot reloading (separate logic vs UI)
* undo move/resize etc.
* UUID for objects instead of index in strokes
* clone
* grid 
* texture loading
*/

//helpers
float min(float x, float y) { return x < y ? x : y;}
float max(float x, float y) { return x > y ? x : y;}


enum SHAPE {
    SELECT,
    RECTANGLE,
    CIRCLE,
    _SHAPE_SIZE
} ;

struct Rectangle2 : Rectangle {
    Vector2 bottomRight() {
        return {x + width, y + height};
    }
    Vector2 bottomLeft() {
        return {x, y + height};
    }
    Vector2 topRight() {
        return {x + width, y};
    }
    Vector2 topLeft() {
        return {x, y};
    }
};

struct Shape {
    SHAPE shape;
    Color color;
    Rectangle2 bounds;
    virtual void push(Vector2 pos) {}
    virtual void draw() {}
    virtual void clear() {}
    virtual bool intersects(Vector2 pos) {return false;}
    virtual void move(Vector2 dt) {} 
    virtual char* serialize() { return nullptr;}
    virtual Shape* clone() {return nullptr;}
};


struct ShapePicker {
    static const int N = _SHAPE_SIZE;
    Rectangle2 rect[N]  {
        {0.f, 40.f, 30.f, 30.f},
        {0.f, 70.f, 30.f, 30.f} ,
        {0.f, 100.f, 30.f, 30.f} ,
    };

    const char* shapes_str[4] = {"+", "R", "C"};

    struct {
        Rectangle2 rect;
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
        current.shape = SELECT;
    }
    void draw(){
        for (int i = 0; i < N; ++i)
            DrawText(shapes_str[i], rect[i].x + 10.f, rect[i].y+8.f, 16.f, WHITE);
        // highlight current rect
        DrawRectangleLinesEx(current.rect, 4.f, GOLD);  
    }
};

struct ColorPicker {
    static const int N = 8;
    Rectangle2 rect[N]  {
        {0.f, 0.f,  28.f, 30.f},
        {30.f, 0.f, 28.f, 30.f} ,
        {60.f, 0.f, 28.f, 30.f} ,
        {90.f, 0.f, 28.f, 30.f},
        {120.f, 0.f, 28.f, 30.f},
        {150.f, 0.f, 28.f, 30.f},
        {180.f, 0.f, 28.f, 30.f} ,
        {210.f, 0.f, 28.f, 30.f},
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
        Rectangle2 rect;
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
            DrawRectangleRounded(rect[i], .25f, 2, color[i]);
        // highlight current rect
        DrawRectangleLinesEx(current.rect, 4.f, GOLD);  
    }
};

struct RectShape : public Shape {
    Vector2 start, end;
    
    RectShape() {shape = RECTANGLE;}
    RectShape(Vector2 pos) : start(pos), end(pos) { shape=RECTANGLE;}
    Shape* clone() {
        auto* tmp = new RectShape;
        tmp->bounds = bounds;
        tmp->color = color;
        return tmp;
    }
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

// NE, NW, SE, SW handles
enum DIRECTION {
    NW, NE, SE, SW
} direction;

const float threshold = 4.f;
bool isApproxEqual(Vector2 a, Vector2 b) {
    return (abs(a.x-b.x) < threshold) && (abs(a.y-b.y) < threshold);
}
bool isResizeDrag(Vector2 mouse, Rectangle2 r) {
    bool res = false;
    if(isApproxEqual(mouse, r.bottomLeft())) { direction = SW; SetMouseCursor(MOUSE_CURSOR_RESIZE_NESW); res=true;}
    else if(isApproxEqual(mouse, r.bottomRight())) { direction = SE; SetMouseCursor(MOUSE_CURSOR_RESIZE_NWSE); res=true;}
    else if(isApproxEqual(mouse, r.topLeft())) { direction = NW;  SetMouseCursor(MOUSE_CURSOR_RESIZE_NWSE); res=true;}
    else if(isApproxEqual(mouse, r.topRight())) { direction = NE; SetMouseCursor(MOUSE_CURSOR_RESIZE_NESW); res=true;}
    return res;
}


struct CircleShape : public Shape {
    Vector2 start, end;
    float radius;
    CircleShape() {shape = CIRCLE;}
    CircleShape(Vector2 pos) : start(pos), end(pos) {shape = CIRCLE;}
    Shape* clone() {
        auto* tmp = new CircleShape;
        tmp->radius = radius;
        tmp->start = start;
        tmp->color = color;
        return tmp;
    }
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

const int screenWidth = 1200;
const int screenHeight = 800;

struct Player {
    Color color;
    Rectangle2 pos;
    float gravity = 300.f;
    float speedy = 0.0f;
    float speedx = 1.f;
    bool jump;
    void reset () {
         gravity = 300.f;
         speedy = 0.0f;
         speedx = 1.f;
         jump = true;
        pos.x = 130;
        pos.y = 130;
    }
    Player() : color(ORANGE), pos({130, 130, 30, 30}) {
        reset();
    };
    void update(Shape** objs) {
        auto time = GetFrameTime();
        
        if (IsKeyDown(KEY_RIGHT)) {
            speedx = 306.f;
        } 
        if (IsKeyDown(KEY_LEFT)) {
            speedx = -306.f;
        } 
        if (!jump && speedy >= 0.f && (IsKeyPressed(KEY_UP))) {
            pos.y -= 2.f;
            speedy = -260.f;
            if (speedy < -260.f)
                speedy = -260.f;
            jump = true;
        }

        pos.x += speedx * time;
        if (pos.x+30.f > screenWidth) pos.x = screenWidth-30.f;
        if (pos.x < 0) pos.x = 0;
        bool collide=  false;
        for (int i  = 0;i < 100; ++i) {
            if (objs[i]) {
                RectShape* tmp = (RectShape*)objs[i];
                if (CheckCollisionPointRec({pos.x, pos.y + 30.f}, tmp->bounds)) {
                    pos.y = tmp->bounds.y-30.f;
                    collide = true;
                    jump = false;
                    break;
                }
            }
        }
        if (!collide) {
            pos.y += speedy * time;
            if (pos.y+30.f > screenHeight) pos.y = screenHeight-30.f;
            if (pos.y < 0) pos.y = 0.f;
            speedy += gravity * time;
        } else {
            speedy = 0.f;
        }
            
        speedx *= .8f;
        if (speedx < 0.001f) speedx = 0;
    }
    void draw() {
        DrawRectangleRounded (pos, .2f, 2, color);
    }
};

void clearStrokes(Shape** strokes, int&s) {
    for (int i = 0; i < s; ++i) {
        delete(strokes[i]);
        strokes[i] = nullptr;
    }
    s = 0;
}

void loadStrokes(Shape** strokes, int& s) {
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
            Rectangle2 r;
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
void savemap(char* filename, Shape** strokes, int& s) {

    FILE* fid=fopen(filename, "w");
    for (int i = 0;i < s; ++i){
        fprintf(fid, "%s\n", strokes[i]->serialize());
    }
    
    fclose(fid);
}
int main(void)
{
    ColorPicker cp;
    ShapePicker sp;

    InitWindow(screenWidth, screenHeight, "!! Game 1 (editor)");
    SetTargetFPS(60);
    
    Shape* strokes[100] = {0};
    int s = 0; // stroke counter
    
    // avoid multiple key stroke updates with these bools
    bool isundo = false;
    bool isredo = false;
    bool down = false;
    bool save = false;
    bool mapeditcontrol = false;
    bool contextMenu = false;
    bool debug = false;

    Rectangle2 hover;
    bool showhover = false;

    Rectangle2 rect {350, 10, 40, 10};
    
    Shape* transient = nullptr;

    std::set<int> selectTransients;// = nullptr;
    std::set<int> copyTransients;
    RectShape* resizeTransient = nullptr;
    Shape* contextTransient = nullptr;
    Shape* multiSelectTransient = nullptr;
    Vector2 contextMenuPosition;

    Player p;
    
    // MouseCursor mc;
    loadStrokes(strokes, s);
    char filename[20] = "test.map";
    bool mapedit = false;
    Rectangle2 mapfilenamebox {600, 1, 100, 40};

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        //map name
        GuiTextBox(mapfilenamebox, filename, 90, mapedit);

        //debug
        if (IsKeyPressed(KEY_D)) debug = !debug;

        //reset
        if (IsKeyPressed(KEY_R)) {
            p.reset();
            clearStrokes(strokes, s);
            loadStrokes(strokes, s);
        }
        // if (sp.current.shape == SHAPE::SELECT) SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        if (!resizeTransient) {
            if (sp.current.shape == RECTANGLE || sp.current.shape == CIRCLE)  
                SetMouseCursor(MOUSE_CURSOR_CROSSHAIR);
            else 
                SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        }
            
        //save
        if (IsKeyUp(KEY_S) || IsKeyUp(KEY_LEFT_CONTROL)) save = false;
        if (!save && IsKeyDown(KEY_S) && IsKeyDown(KEY_LEFT_CONTROL)) {
        //    TakeScreenshot("tmp.png"); // TODO: Fix png size; edge pixels bleeding out
            savemap(filename, strokes, s);
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
        
        // mouse position
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

        // resize code
        if (sp.current.shape == SHAPE::SELECT && !contextMenu && !multiSelectTransient) {
             if (resizeTransient) {
                auto start = resizeTransient->start;
                auto end = mousePoint;
                resizeTransient->bounds = {min(start.x,end.x), min(start.y, end.y), abs(end.x-start.x), abs(end.y-start.y)};
             } else if (selectTransients.empty()) {
                for(int i = 0; i < s; ++i) {
                    if (strokes[i]->shape == RECTANGLE) {
                        // if(CheckCollisionPointRec(mousePoint, strokes[i]->bounds)) {    
                        if (isResizeDrag(mousePoint, strokes[i]->bounds) && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                            resizeTransient = (RectShape*)strokes[i];
                            switch (direction)
                            {
                            case NW:
                                {
                                    resizeTransient->start = resizeTransient->bounds.bottomRight();
                                }
                                break;
                            case NE:
                                {
                                    resizeTransient->start = resizeTransient->bounds.bottomLeft();
                                    
                                }
                                break;
                            case SW:
                                {
                                    resizeTransient->start = resizeTransient->bounds.topRight();
                                    
                                }
                                break;
                            case SE:
                                {
                                    resizeTransient->start = resizeTransient->bounds.topLeft();
                                    
                                }
                                break;
                            default:
                                break;
                            }
                            break;
                        }
                        // }
                    } 
                }
             }
             
        }
       
       // context menu
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && sp.current.shape == SELECT) {
            // cp.next();
            contextMenu = true;
            for(int i = 0; i < s; ++i) {
                if (strokes[i]->shape == RECTANGLE) {
                    if(CheckCollisionPointRec(mousePoint, strokes[i]->bounds)) {
                        contextMenuPosition = mousePoint;
                        contextTransient = strokes[i];
                    }
                        
                }
            }
        }

        // cycle through shape picker (mouse wheel) // TODO: remove this; use it for pan/zoom
        auto wheel = GetMouseWheelMove();
        if (wheel == 1) {sp.prev();}
        if (wheel == -1) {sp.next();}
        
        // left click
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {

            // disable context menu
            if (!CheckCollisionPointRec(mousePoint, 
                {contextMenuPosition.x, contextMenuPosition.y, 180, 150})) {
                        contextMenu = false;
            }

            bool validClick = true;
            
            //enable map filename edit
            if (!mapeditcontrol && CheckCollisionPointRec(mousePoint, mapfilenamebox)) {
                mapedit = !mapedit;
                mapeditcontrol = true;
            }

            // disable starting clicks on tools while drawing
            for (auto& r : cp.rect) {
                if (CheckCollisionPointRec(mousePoint, r)) {
                    // SetMouseCursor(MOUSE_CURSOR_DEFAULT);
                    validClick = false;
                    break;
                }
            }
            for (auto& r : sp.rect) {
                if (CheckCollisionPointRec(mousePoint, r)) {
                    // SetMouseCursor(MOUSE_CURSOR_DEFAULT);
                    validClick = false;
                    break;
                }
            }
            if (validClick) {
                
                // TODO: refactor this below
                switch (sp.current.shape)
                {
                case SHAPE::RECTANGLE:
                {
                    if (!transient) {
                        auto ls =  new RectShape(mousePoint);
                        ls->color = cp.current.color;
                        transient = ls;
                    } else {
                        transient->push(mousePoint);
                    }
                } break;
                    
                case SHAPE::CIRCLE:
                {
                    if (!transient) {
                        auto ls =  new CircleShape(mousePoint);
                        ls->color = cp.current.color;
                        transient = ls;
                    } else {
                        transient->push(mousePoint);
                    }
                } break;
                    
                case SHAPE::SELECT:
                if (!contextMenu){
                    // Move/copy
                    if (multiSelectTransient) {
                        multiSelectTransient->push(mousePoint);
                        selectTransients.clear();
                        for (int i= s-1; i >=0 ;--i) {
                            if(CheckCollisionRecs(multiSelectTransient->bounds, strokes[i]->bounds)) {
                                selectTransients.insert(i);
                            }
                        }
                    }
                    //TODO: fix copy blocks
                    if (!resizeTransient && selectTransients.empty() ) {
                        for (int i= s-1; i >=0 ;--i) {
                            // if(CheckCollisionRecs(multiSelectTransient->bounds, strokes[i]->bounds)) {
                            if (strokes[i]->intersects(mousePoint)) {
                                // if (IsKeyDown(KEY_LEFT_CONTROL)) {
                                //     //copy
                                //     if (copyTransients.count(i) == 0) {
                                //         auto newNode = strokes[i]->clone();
                                //         // selectTransients.insert(s);
                                //         copyTransients.insert(s);
                                //         strokes[s++] = newNode;
                                //         copyTransients.insert(i);
                                        
                                //     }
                                    
                                // } else {
                                    //move
                                    selectTransients.insert(i);
                                // }
                                // break;
                            }
                        }
                        if (selectTransients.empty() && !multiSelectTransient) {
                            selectTransients.clear();
                            copyTransients.clear();
                            multiSelectTransient = new RectShape(mousePoint);
                        }
                    }
                    if (!selectTransients.empty() && !multiSelectTransient){
                        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
                        //  if (IsKeyDown(KEY_LEFT_CONTROL)) {
                        //     for (auto&si : copyTransients)
                        //     strokes[si]->move(GetMouseDelta());
                        //  } else {
                           for (auto&si : selectTransients)
                            strokes[si]->move(GetMouseDelta());
                        //  }
                        
                    }  
                } break;
                
                default:
                    break;
                }
            }
            down = true;
        }
        // mouse left button up; commit transactions
        else if (down && IsMouseButtonUp(MOUSE_BUTTON_LEFT)) {
            SetMouseCursor(MOUSE_CURSOR_DEFAULT);
            bool toolselect = false;
            int i = 0;
            if (mapeditcontrol) mapeditcontrol = false;
            for (auto& r : cp.rect) {
                if (CheckCollisionPointRec(mousePoint, r)) {
                    // SetMouseCursor(MOUSE_CURSOR_DEFAULT);
                    cp.updateCurrent(i);
                    toolselect = true;  
                    break;
                }
                ++i;
            }
            i = 0;
            for (auto& r : sp.rect) {
                if (CheckCollisionPointRec(mousePoint, r)) {
                    // SetMouseCursor(MOUSE_CURSOR_DEFAULT);
                    sp.current.shape = (SHAPE)i;
                    sp.current.rect = r;
                    toolselect = true;  
                    break;
                }
                ++i;
            }
            if (!toolselect && transient) {
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
            if (!multiSelectTransient)
                selectTransients.clear();
            resizeTransient = nullptr;
            multiSelectTransient = nullptr;
        }
        if (!mapedit)
            p.update(strokes);
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(BLACK);
            // Clear
            if (GuiButton({350, 0, 80, 40}, "#0#Clear")) { 
                //clear
                clearStrokes(strokes, s);
            }

            // Load
            if (GuiButton({350+80, 0, 80, 40}, "#5#Load")) { 
                //clear
                clearStrokes(strokes, s);

                // load
                loadStrokes(strokes, s);
            }
            // DrawRectangleLines(0, 0, 30, 30, RED);

            if (debug) {
                // debug shape counter
                char ss[3];
                sprintf(ss,"%d", s);
                DrawText(ss, 270, 0, 50, RED);
            }

            //debug variable
            if (debug) {
                char ss[3];
                sprintf(ss,"%zd", copyTransients.size());
                DrawText(ss, 1070, 0, 50, YELLOW);
            }
            
            //tools
            cp.draw();
            sp.draw();
      
            if (showhover) {
                DrawRectangleLinesEx(hover, 5.f, GRAY);  
                showhover = false;
            }

            for(int i = 0; i < s; ++i) {
                strokes[i]->draw();
                if (debug) {
                    char ss[3];
                    sprintf(ss,"%d", i);
                    DrawText(ss, strokes[i]->bounds.x, strokes[i]->bounds.y-30.f, 25, ORANGE);
                }
            }
            if (transient)   transient->draw();
            if (resizeTransient) DrawRectangleLinesEx(resizeTransient->bounds, 3.f, GOLD);
            for (auto&si :selectTransients) DrawRectangleLinesEx(strokes[si]->bounds, 3.f, GOLD);
            if (multiSelectTransient) DrawRectangleLinesEx(multiSelectTransient->bounds, 3.f, GOLD);
            p.draw();
            if (contextMenu) {
                if (contextTransient) {
                    GuiColorPicker({contextMenuPosition.x, contextMenuPosition.y, 150, 150}, "ll", &contextTransient->color);
                }
            }
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    CloseWindow();
    return 0;
}