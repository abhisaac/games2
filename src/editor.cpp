
#include "raylib.h"
#include <string>
// #include <cstdio>
// #include <cstdlib>
#include <fstream>
#include <cmath>
#include <set>
#include <cassert>
// #include <sstream>
// #include <iostream>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
// #include <string>
/*TODO
* Enemies
* fix jumping/ collision detection
* disable/enable tools
* edit mode vs game mode
* game pause
* hot reloading (separate logic vs UI)
* undo move/resize etc.
* UUID for objects instead of index in objs

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



// NE, NW, SE, SW handles
enum DIRECTION {
    NW, NE, SE, SW
} direction;

// const float threshold = 4.f;
bool isApproxEqual(Vector2 a, Vector2 b, float threshold) {
    return (abs(a.x-b.x) < threshold) && (abs(a.y-b.y) < threshold);
}
bool isResizeDrag(Vector2 mouse, Rectangle2 r, float threshold = 4.f) {
    bool res = false;
    if      (isApproxEqual(mouse, r.bottomLeft(), threshold))    { direction = SW; SetMouseCursor(MOUSE_CURSOR_RESIZE_NESW); res=true;}
    else if (isApproxEqual(mouse, r.bottomRight(), threshold))   { direction = SE; SetMouseCursor(MOUSE_CURSOR_RESIZE_NWSE); res=true;}
    else if (isApproxEqual(mouse, r.topLeft(), threshold))       { direction = NW; SetMouseCursor(MOUSE_CURSOR_RESIZE_NWSE); res=true;}
    else if (isApproxEqual(mouse, r.topRight(), threshold))      { direction = NE; SetMouseCursor(MOUSE_CURSOR_RESIZE_NESW); res=true;}
    return res;
}

struct Shape {
    Vector2 start, end;
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
    void resizeInit() {
        switch (direction)
        {
        case NW: start = bounds.bottomRight();
            break;
        case NE: start = bounds.bottomLeft();
            break;
        case SW: start = bounds.topRight();
            break;
        case SE: start = bounds.topLeft();
            break;
        default:
            break;
        }
    }
    virtual void resize(Vector2 mousePoint) {}
    bool isHovering(Vector2 mousePoint) {
        return CheckCollisionPointRec(mousePoint, bounds);
    }
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



Shape* resizeTransient = nullptr;
struct RectShape : public Shape {
    RectShape() {shape = RECTANGLE;}
    RectShape(Vector2 pos) { start = pos; end = pos; shape=RECTANGLE;}
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
    
    void resize(Vector2 mousePoint) {
        end = mousePoint;
        bounds = {min(start.x,end.x), min(start.y, end.y), abs(end.x-start.x), abs(end.y-start.y)};
    }
};


struct CircleShape : public Shape {
    Vector2 center;
    float radius;
    CircleShape() {shape = CIRCLE;}
    CircleShape(Vector2 pos) : center(pos) {shape = CIRCLE;}
    Shape* clone() {
        auto* tmp = new CircleShape;
        tmp->radius = radius;
        tmp->center = center;
        tmp->color = color;
        tmp->setBounds();
        return tmp;
    }
    void draw() {
        DrawCircle(center.x, center.y, radius, color);
    }
    void clear() {
        
    }
    char* serialize() {
        char* out = (char*) malloc(100*sizeof(char));
        sprintf(out, "CIRCLE %f %f %f COLOR %u %u %u %u", center.x, center.y, radius,
                        color.r, color.g, color.b, color.a);
        return out;
    }
    void move (Vector2 dt){
        center.x += dt.x;
        center.y += dt.y;
        setBounds();
    }
    void setBounds(){
        bounds = {center.x-radius, center.y-radius, radius*2.f, radius*2.f};
    }
    void push(Vector2 newpos) {
        // center = newpos;
        radius =  max(abs(center.x-newpos.x), abs(center.y-newpos.y));
        setBounds();
    }
    bool intersects(Vector2 pos) {
        return CheckCollisionPointCircle(pos, center, radius);
    }
    
    void resize(Vector2 mousePoint) {
        end = mousePoint;
        bounds = {min(start.x,end.x), min(start.y, end.y), abs(end.x-start.x), abs(end.y-start.y)};
        radius = min(bounds.width, bounds.height)/2.;
        center = {bounds.x + bounds.width/2.f, bounds.y + bounds.height/2.f};
    }
};

const int screenWidth = 1200;
const int screenHeight = 800;

// Object pool
#define SN 200
Shape* objs[SN] = {0};

int getNextIdx() {
    int i = 0;
    while (i<SN && objs[i] != nullptr) {++i;}
    assert(i < SN);
    return i;
}
struct Player {
    Color color;
    Rectangle2 pos;
    Texture2D hero;
    Rectangle heroRec;
    
    float gravity = 300.f;
    float speedy = 0.0f;
    float speedx = 1.f;
    bool jump;

    int frame = 0;
    const int SPRITE_FRAMES = 5;
    
    float updateTime = 1.0/10.0;
    float runningTime = 0.0;

    void reset () {
         gravity = 300.f;
         speedy = 0.0f;
         speedx = 1.f;
         jump = true;
        pos.x = 130;
        pos.y = 130;
    }
    Player() : color(ORANGE) {
        hero  =  LoadTexture("assets/sprite.png");
        heroRec.width = hero.width/SPRITE_FRAMES;
        heroRec.height = hero.height;
        heroRec.x = 0;
        heroRec.y = 0;
        pos = {130, 130, heroRec.width, heroRec.height};
        reset();
    };
    ~Player() {
        UnloadTexture(hero);
    }
    void update(Shape** objs) {
        auto dT = GetFrameTime();
        
        if (IsKeyDown(KEY_RIGHT)) {
            speedx = 306.f;
            if (heroRec.width < 0) heroRec.width = -heroRec.width;
            runningTime += dT;
            if(runningTime >= updateTime){
                runningTime = 0;
                
                heroRec.x = hero.width/SPRITE_FRAMES * frame;
                frame++;
                if(frame>SPRITE_FRAMES){
                    frame = 0;
                }
            }
        } 
        if (IsKeyDown(KEY_LEFT)) {
            speedx = -306.f;
            if (heroRec.width > 0) heroRec.width = -heroRec.width;
            // heroRec.width = -heroRec.width;
            runningTime += dT;
            if(runningTime >= updateTime){
                runningTime = 0;
                
                heroRec.x = hero.width/SPRITE_FRAMES * frame;
                frame++;
                if(frame>SPRITE_FRAMES){
                    frame = 0;
                }
            }
        } 
        if (!jump && speedy >= 0.f && (IsKeyPressed(KEY_UP))) {
            pos.y -= 2.f;
            speedy = -260.f;
            if (speedy < -260.f)
                speedy = -260.f;
            jump = true;
        }
        
        
        pos.x += speedx * dT;
        if (pos.x+heroRec.width > screenWidth) pos.x = screenWidth-heroRec.width;
        if (pos.x < 0) pos.x = 0;
        bool collide=  false;
        for (int i  = 0;i < SN; ++i) {
            if (objs[i]) {
                if (objs[i]->shape == RECTANGLE) {
                    RectShape* tmp = (RectShape*)objs[i];
                    if (CheckCollisionPointRec({pos.x, pos.y + heroRec.height}, tmp->bounds)) {
                        pos.y = tmp->bounds.y-heroRec.height;
                        collide = true;
                        jump = false;
                        break;
                    }
                } else if (objs[i]->shape == CIRCLE) {
                    CircleShape* tmp = (CircleShape*)objs[i];
                    if (CheckCollisionPointCircle({pos.x, pos.y + heroRec.height}, tmp->center, tmp->radius)) {
                        pos.y = tmp->bounds.y-heroRec.height;
                        collide = true;
                        jump = false;
                        break;
                    }
                }
                
            }
        }
        if (!collide) {
            pos.y += speedy * dT;
            if (pos.y+heroRec.height> screenHeight) pos.y = screenHeight-heroRec.height;
            if (pos.y < 0) pos.y = 0.f;
            speedy += gravity * dT;
        } else {
            speedy = 0.f;
        }
            
        speedx *= .8f;
        if (speedx < 0.001f) speedx = 0;
    }
    void draw() {
        // DrawRectangleRounded (pos, .2f, 2, color);
        DrawTextureRec(hero,heroRec,{pos.x, pos.y},WHITE);
    }
};

void clearStrokes(Shape** objs, int&s) {
    for (int i = 0; i < SN; ++i) {
        if (objs[i]) {
            delete(objs[i]);
            objs[i] = nullptr;
        }
        
    }
    s = 0;
}

void loadStrokes(Shape** objs, int& s) {
    //load file to objs
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
            auto* tmp = new RectShape();
            tmp->color = c;
            tmp->bounds = r;
            // std::cerr << tmp << std::endl;
            objs[getNextIdx()] = tmp;
            
        } else if (strcmp(shape,"CIRCLE")==0) {
            Vector2 center;
            float radius;
            Color c;
            sscanf(data, "%f %f %f COLOR %hhd %hhd %hhd %hhd",
                        &center.x, &center.y, &radius,
                        &c.r, &c.g, &c.b, &c.a);
            auto* tmp = new CircleShape();
            tmp->color = c;
            tmp->radius = radius;
            tmp->center = center;
            tmp->setBounds();
            // std::cerr << tmp << std::endl;
            objs[getNextIdx()] = tmp;
            
        }
    }
}
void savemap(char* filename, Shape** objs, int& s) {

    FILE* fid=fopen(filename, "w");
    for (int i = 0;i < SN; ++i){
        if (objs[i])
            fprintf(fid, "%s\n", objs[i]->serialize());
    }
    
    fclose(fid);
}
int main(void)
{
    
    ColorPicker cp;
    ShapePicker sp;

    InitWindow(screenWidth, screenHeight, "!! Game 1 (editor)");
    SetTargetFPS(60);
    
    
    
    int s = 0; // stroke counter
    
    // avoid multiple key stroke updates with these bools
    bool isundo = false;
    bool isredo = false;
    bool down = false;
    bool save = false;
    bool mapeditcontrol = false;
    bool contextMenu = false;
    bool debug = false;
    bool selectall = false;

    Rectangle2 hover;
    bool showhover = false;

    Rectangle2 rect {350, 10, 40, 10};
    
    Shape* transient = nullptr;

    std::set<int> selectTransients;
    std::set<int> copyTransients;

    Shape* contextTransient = nullptr;
    Shape* multiSelectTransient = nullptr;
    Vector2 contextMenuPosition;
    Shape* hoverTransient = nullptr;

    Player p;
    
    loadStrokes(objs, s);
    char filename[20] = "test.map";
    bool mapedit = false;
    Rectangle2 mapfilenamebox {600, 1, 100, 40};

    bool drawGrid = false;
    bool selected = false;
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        //map name
        GuiTextBox(mapfilenamebox, filename, 90, mapedit);

        //debug
        if (IsKeyPressed(KEY_D)) debug = !debug;
        
        // grid
        if (IsKeyPressed(KEY_G)) {
            drawGrid = !drawGrid;
        }

        // select all
        if (IsKeyUp(KEY_A) || IsKeyUp(KEY_LEFT_CONTROL)) selectall = false;
        if (IsKeyPressed(KEY_A) && IsKeyDown(KEY_LEFT_CONTROL)) {
            selectall = true;
            for(int i = 0; i < SN; ++i) {
                if (objs[i]) selectTransients.insert(i);
            }
        }

        // delete
        if (IsKeyPressed(KEY_DELETE)) {
            for(auto&si : selectTransients) {
                delete objs[si];
                objs[si] = nullptr;
            }
            selectTransients.clear();
        }
        //reset
        if (IsKeyPressed(KEY_R)) {
            p.reset();
            clearStrokes(objs, s);
            loadStrokes(objs, s);
        }
        
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
            savemap(filename, objs, s);
            save = true;
        }
//TODO: fix undo/redo to work with object pool/getNextIdx
        //redo
        if (IsKeyUp(KEY_Y) || IsKeyUp(KEY_LEFT_CONTROL)) isredo = false;
        else if (!isredo && IsKeyDown(KEY_Y) && IsKeyDown(KEY_LEFT_CONTROL)) {
            isredo = true;
            if (objs[s])
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

        bool ishover = false;
        if (hoverTransient && hoverTransient->isHovering(mousePoint)) {
            ishover = true;
        }
        
        if (!ishover && sp.current.shape == SHAPE::SELECT) {
            for (auto& si: objs) {
                if (!si) continue;
                if (si->shape == CIRCLE) {
                    auto* tmp = (CircleShape*)si;
                    if (CheckCollisionPointCircle(mousePoint, tmp->center, tmp->radius)) {
                        ishover = true;
                        hoverTransient = si;
                        break;
                    }
                } else if (si->shape == RECTANGLE) {
                    auto* tmp = (RectShape*)si;
                    if (CheckCollisionPointRec(mousePoint, si->bounds)) {
                        ishover = true;
                        hoverTransient = si;
                        break;
                    }
                }
            }
        }
        
        if (!ishover) hoverTransient = nullptr;

        // resize code
        if (sp.current.shape == SHAPE::SELECT && !contextMenu && !multiSelectTransient) {
             if (resizeTransient) {
                resizeTransient->resize(mousePoint);
             } else if (selectTransients.empty()) {
                for(int i = 0; i < SN; ++i) {
                    if (!objs[i]) continue;

                    if (isResizeDrag(mousePoint, objs[i]->bounds) && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                        resizeTransient = objs[i];
                        resizeTransient->resizeInit();
                        break;
                    }
                }
             }
        }
       
       // context menu
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && sp.current.shape == SELECT) {
            // cp.next();
            contextMenu = true;
            for(int i = 0; i < SN; ++i) {
                if (!objs[i]) continue;
                if (objs[i]->shape == RECTANGLE) {
                    if(CheckCollisionPointRec(mousePoint, objs[i]->bounds)) {
                        contextMenuPosition = mousePoint;
                        contextTransient = objs[i];
                    }
                } else if (objs[i]->shape == CIRCLE) {
                    auto* tmp = (CircleShape*)objs[i];
                    if(CheckCollisionPointCircle(mousePoint, tmp->center, tmp->radius)) {
                        contextMenuPosition = mousePoint;
                        contextTransient = objs[i];
                    }
                }
            }
        }

        // cycle through shape picker (mouse wheel) // TODO: remove this; use it for pan/zoom
        auto wheel = GetMouseWheelMove();
        if (wheel == 1) {sp.prev();}
        if (wheel == -1) {sp.next();}
        
        // shift multi-select
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) ) {
            if (IsKeyDown(KEY_LEFT_SHIFT)) {
                for(int i = 0; i < SN; ++i) if(objs[i] && CheckCollisionPointRec(mousePoint, objs[i]->bounds)) {
                    if (selectTransients.count(i)) {
                        selectTransients.erase(i);
                    } else {
                        selectTransients.insert(i);
                    }
                    
                    selected = true;
                    break;
                }
            } else {
                selected = false;
            }
        }
        else if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !selected) {
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
                    validClick = false;
                    break;
                }
            }
            for (auto& r : sp.rect) {
                if (CheckCollisionPointRec(mousePoint, r)) {
                    validClick = false;
                    break;
                }
            }
            if (validClick) {
                
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
                        for (int i= SN-1; i >=0 ;--i) {
                            if (!objs[i]) continue;
                            if(CheckCollisionRecs(multiSelectTransient->bounds, objs[i]->bounds)) {
                                selectTransients.insert(i);
                            }
                        }
                    }
                    // TODO: copy is moving original blocks; check this
                    if (!resizeTransient && selectTransients.empty() ) {
                        for (int i= SN-1; i >=0 ;--i) {
                            if (!objs[i]) continue;
                            if (objs[i]->intersects(mousePoint)) {
                                if (IsKeyDown(KEY_LEFT_CONTROL)) {
                                    //copy
                                    if (copyTransients.count(i) == 0) {
                                        auto newNode = objs[i]->clone();
                                        selectTransients.insert(s);
                                        objs[getNextIdx()] = newNode;
                                        copyTransients.insert(i);
                                    }
                                    
                                } else {
                                    //move
                                    selectTransients.insert(i);
                                }
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
                         if (IsKeyDown(KEY_LEFT_CONTROL)) {
                            if (copyTransients.empty()) {
                                for (auto&si : selectTransients) {
                                    if (copyTransients.count(si) == 0) {
                                        auto newNode = objs[si]->clone();
                                        objs[getNextIdx()] = newNode;
                                        copyTransients.insert(si);
                                    }
                                }
                            }
                            
                            for (auto&si : copyTransients)
                                objs[si]->move(GetMouseDelta());
                         } else {
                           for (auto&si : selectTransients)
                                objs[si]->move(GetMouseDelta());
                         }
                        
                    }  
                } break;
                
                default:
                    break;
                }
            }
            down = true;
        }
        // mouse up; commit transactions
        else if (down && IsMouseButtonUp(MOUSE_BUTTON_LEFT)) {
            copyTransients.clear();
            SetMouseCursor(MOUSE_CURSOR_DEFAULT);
            bool toolselect = false;
            int i = 0;
            if (mapeditcontrol) mapeditcontrol = false;
            for (auto& r : cp.rect) {
                if (CheckCollisionPointRec(mousePoint, r)) {
                    cp.updateCurrent(i);
                    toolselect = true;  
                    break;
                }
                ++i;
            }
            i = 0;
            for (auto& r : sp.rect) {
                if (CheckCollisionPointRec(mousePoint, r)) {
                    sp.current.shape = (SHAPE)i;
                    sp.current.rect = r;
                    toolselect = true;  
                    break;
                }
                ++i;
            }
            if (!toolselect && transient) {
                //commit transaction

                // // cleanup redo stack
                // int i = s;
                // while(objs[i++]) {
                //     delete(objs[i]);
                //     objs[i] = nullptr;
                // }

                //commit
                objs[getNextIdx()] = transient;
                transient = nullptr;
            }
            down = false;
            if (!multiSelectTransient)
                selectTransients.clear();
            resizeTransient = nullptr;
            multiSelectTransient = nullptr;
        }
        if (!mapedit)
            p.update(objs);
        // Clear
        if (GuiButton({350, 0, 80, 40}, "#0#Clear")) { 
            //clear
            clearStrokes(objs, s);
        }

        // Load
        if (GuiButton({350+80, 0, 80, 40}, "#5#Load")) { 
            //clear
            clearStrokes(objs, s);
            // load
            loadStrokes(objs, s);
        }

        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(BLACK);
            // DrawLineEx({0, 30.f}, {screenWidth, 30.f}, 0.5, GRAY);
            if (drawGrid) {
                for (int i = 0; i < screenHeight; i+=40)
                    DrawLineEx({0, (float)i}, {screenWidth, (float)i}, 1., GRAY);
                for (int i = 0; i < screenWidth; i+=40)
                    DrawLineEx({(float)i, 0}, {(float)i, screenHeight}, 1., GRAY);
            }
            if (debug) {
                // debug shape counter
                char ss[3];
                int i = 0;
                for (auto&si:objs) if (si) ++i;
                sprintf(ss,"%d", i);
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

            for(int i = 0; i < SN; ++i) {
                if (!objs[i]) continue;
                objs[i]->draw();
                if (debug) {
                    char ss[4];
                    sprintf(ss,"%d", i);
                    // char ss[30];
                    // sprintf(ss,"0x%p", objs[i]);
                    DrawText(ss, objs[i]->bounds.x, objs[i]->bounds.y-30.f, 25, ORANGE);
                }
            }
            if (transient)   transient->draw();
            
            for (auto&si :selectTransients) if (objs[si]) DrawRectangleLinesEx(objs[si]->bounds, 3.f, GOLD);
            if (resizeTransient) DrawRectangleLinesEx(resizeTransient->bounds, 3.f, GOLD);
            else if (multiSelectTransient) DrawRectangleLinesEx(multiSelectTransient->bounds, 3.f, GOLD);
            else if (hoverTransient)  DrawRectangleLinesEx(hoverTransient->bounds, 2.f, WHITE);
            p.draw();
            if (contextMenu) {
                if (contextTransient) {
                    auto x = contextMenuPosition.x+10;
                    auto y = contextMenuPosition.y+20;
                    auto width = 150;
                    auto height = 150;
                    if (x + width + 30 > screenWidth) x-=width-30; //30 is padding for color selector
                    GuiColorPicker({x, y, 150, 150}, "ll", &contextTransient->color);
                    for (auto& si : selectTransients) {
                        objs[si]->color = contextTransient->color;
                    }
                }
            }
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    CloseWindow();
    return 0;
}