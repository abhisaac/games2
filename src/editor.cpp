
#include "raylib.h"
#include <string>
// #include <cstdio>
// #include <cstdlib>
#include <fstream>
#include <cmath>
#include <set>
#include <cassert>
// #include <sstream>
#include <iostream>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "player.h"
// #include <string>
/*TODO
* Enemies
* fix jumping/ collision detection
* disable/enable tools
* 
* game pause
* hot reloading (separate logic vs UI)
* undo move/resize etc.
* UUID for objects instead of index in objs

*/
enum class mode {
    game,
    edit
};
mode g_mode;
// const float threshold = 4.f;
bool isApproxEqual(Vector2 a, Vector2 b, float threshold) {
    return (abs(a.x-b.x) < threshold) && (abs(a.y-b.y) < threshold);
}
bool isResizeDrag(Vector2 mouse, Rectangle2 r, float threshold = 5.f) {
    bool res = false;
    if      (isApproxEqual(mouse, r.bottomLeft(), threshold))    { direction = SW; SetMouseCursor(MOUSE_CURSOR_RESIZE_NESW); res=true;}
    else if (isApproxEqual(mouse, r.bottomRight(), threshold))   { direction = SE; SetMouseCursor(MOUSE_CURSOR_RESIZE_NWSE); res=true;}
    else if (isApproxEqual(mouse, r.topLeft(), threshold))       { direction = NW; SetMouseCursor(MOUSE_CURSOR_RESIZE_NWSE); res=true;}
    else if (isApproxEqual(mouse, r.topRight(), threshold))      { direction = NE; SetMouseCursor(MOUSE_CURSOR_RESIZE_NESW); res=true;}
    return res;
}

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
        if (g_mode != mode::edit) return;
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
        if (g_mode != mode::edit) return;
        for (int i = 0; i < N; ++i)
            DrawRectangleRounded(rect[i], .25f, 2, color[i]);
        // highlight current rect
        DrawRectangleLinesEx(current.rect, 4.f, GOLD);  
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


void clearMap(Shape** objs) {
    for (int i = 0; i < SN; ++i) {
        if (objs[i]) {
            delete(objs[i]);
            objs[i] = nullptr;
        }
        
    }
}
char mapfilename[200] = "test.map";
void loadMap(Shape* objs[]) {
    //load file to objs
    std::ifstream ifs(mapfilename); //, "r");

    std::string line;
    while(std::getline(ifs, line)) {

        char shape[25], data[70];
        sscanf(line.c_str(), "%s %[^\n]", shape, data );

        if (strcmp(shape, "RECTANGLE") == 0) {
            Rectangle2 r;
            Color c;
            sscanf(data, "%f %f %f %f COLOR %hhd %hhd %hhd %hhd",
                        &r.x, &r.y, &r.width, &r.height,
                        &c.r, &c.g, &c.b, &c.a);
            auto* tmp = new RectShape();
            tmp->color = c;
            tmp->bounds = r;

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

            objs[getNextIdx()] = tmp;
            
        }
    }
}
void savemap(char* filename, Shape** objs) {

    FILE* fid=fopen(filename, "w");
    for (int i = 0;i < SN; ++i){
        if (objs[i])
            fprintf(fid, "%s\n", objs[i]->serialize());
    }
    
    fclose(fid);
}

int main(void)
{
    InitAudioDevice();
    
    ColorPicker cp;
    ShapePicker sp;

    InitWindow(screenWidth, screenHeight, "!! Game 1 (editor)");
    GuiSetStyle(DEFAULT, TEXT_SIZE, 19.f);
    // GuiSetStyle(DEFAULT, BACKGROUND_COLOR, 0xff0000ff);  
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

    std::set<Shape*> selectTransients;
    std::set<Shape*> copyTransients;

    Shape* contextTransient = nullptr;
    Shape* multiSelectTransient = nullptr;
    Vector2 contextMenuPosition;
    Shape* hoverTransient = nullptr;

    Player p(screenWidth, screenHeight);
    
    loadMap(objs);
    
    bool mapedit = false;
    Rectangle2 mapfilenamebox {600, 1, 150, 40};

    bool drawGrid = false;
    bool selected = false;
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        BeginDrawing();

            ClearBackground(BLACK);
            if (IsKeyPressed(KEY_E) && IsKeyDown(KEY_LEFT_ALT)) {
                g_mode = g_mode == mode::edit ? mode::game : mode::edit;
            }
    if (g_mode == mode::edit) {
            //map name
            GuiTextBox(mapfilenamebox, mapfilename, 90, mapedit);

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
                if (objs[i]) selectTransients.insert(objs[i]);
            }
        }

        // delete
        if (IsKeyPressed(KEY_DELETE)) {
            for(auto&si : selectTransients) {
                delete si;
                for (int i = 0; i < SN; ++i) {
                    if (si == objs[i]) {
                        objs[i] = nullptr;
                    }
                }
            }
            selectTransients.clear();
        }
        //reset
        if (IsKeyPressed(KEY_R)) {
            p.reset();
            clearMap(objs);
            std::cout << mapfilename << std::endl;
            loadMap(objs);
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
            savemap(mapfilename, objs);
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

                    if (hoverTransient == objs[i] && isResizeDrag(mousePoint, objs[i]->bounds) && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
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
        // auto wheel = GetMouseWheelMove();
        // if (wheel == 1) {sp.prev();}
        // if (wheel == -1) {sp.next();}
        
        // shift multi-select
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) ) {
            if (IsKeyDown(KEY_LEFT_SHIFT)) {
                for(int i = 0; i < SN; ++i) if(objs[i] && CheckCollisionPointRec(mousePoint, objs[i]->bounds)) {
                    if (selectTransients.count(objs[i])) {
                        selectTransients.erase(objs[i]);
                    } else {
                        selectTransients.insert(objs[i]);
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
                                selectTransients.insert(objs[i]);
                            }
                        }
                    }
                   
                    if (!resizeTransient && selectTransients.empty() ) {
                        for (int i= SN-1; i >=0 ;--i) {
                            if (!objs[i]) continue;
                            if (objs[i]->intersects(mousePoint)) {
                                if (IsKeyDown(KEY_LEFT_CONTROL)) {
                                    //copy
                                    if (copyTransients.count(objs[i]) == 0) {
                                        auto newNode = objs[i]->clone();
                                        copyTransients.insert(newNode);
                                        objs[getNextIdx()] = newNode;
                                        selectTransients.insert(objs[i]);
                                    }
                                    
                                } else {
                                    //move
                                    selectTransients.insert(objs[i]);
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
                                        auto newNode = si->clone();
                                        objs[getNextIdx()] = newNode;
                                        copyTransients.insert(si);
                                    }
                                }
                            }
                            
                            for (auto&si : copyTransients)
                                si->move(GetMouseDelta());
                         } else {
                           for (auto&si : selectTransients)
                                si->move(GetMouseDelta());
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
        
            

        // Clear
        if (GuiButton({350, 0, 80, 40}, "#0#Clear")) { 
            //clear
            clearMap(objs);
        }

        // Load
        if (GuiButton({350+80, 0, 80, 40}, "#5#Load")) { 
            //clear
            clearMap(objs);
            // load
            loadMap(objs);
        }
    }
        //----------------------------------------------------------------------------------
        

            if (!mapedit) {
                p.update();
                bool collide = false;
                float newy = 0.f;
                for (int i  = 0;i < SN; ++i) {
                    if (objs[i]) {
                        if (objs[i]->shape == RECTANGLE) {
                            RectShape* tmp = (RectShape*)objs[i];
                            if (CheckCollisionPointRec({p.pos.x, p.pos.y + p.heroRec.height}, tmp->bounds)) {
                                newy = tmp->bounds.y-p.heroRec.height;
                                collide = true;
                                break;
                            }
                        } else if (objs[i]->shape == CIRCLE) {
                            CircleShape* tmp = (CircleShape*)objs[i];
                            if (CheckCollisionPointCircle({p.pos.x, p.pos.y + p.heroRec.height}, tmp->center, tmp->radius)) {
                                newy = tmp->bounds.y-p.heroRec.height;
                                collide = true;
                                break;
                            }
                        }
                        
                    }
                }

            // TODO: this API can be better
                p.updateSpeed(collide, newy);
            }
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
                    // char ss[4];
                    // sprintf(ss,"%d", i);
                    char ss[30];
                    sprintf(ss,"%d - 0x%p", i, objs[i]);
                    DrawText(ss, objs[i]->bounds.x, objs[i]->bounds.y-30.f, 25, ORANGE);
                }
            }
            if (transient)   transient->draw();
            
            for (auto&si :selectTransients) if (si) DrawRectangleLinesEx(si->bounds, 3.f, GOLD);
            if (resizeTransient) DrawRectangleLinesEx(resizeTransient->bounds, 3.f, GOLD);
            else if (multiSelectTransient) DrawRectangleLinesEx(multiSelectTransient->bounds, 3.f, GOLD);
            else if (hoverTransient)  DrawRectangleLinesEx(hoverTransient->bounds, 2.f, WHITE);
            p.draw();
            if (contextMenu) {
                if (contextTransient) {
                    auto x = contextMenuPosition.x; //+10;
                    auto y = contextMenuPosition.y; //+20;
                    
                    GuiColorPicker({x, y, 150, 150}, "ll", &contextTransient->color);
                    for (auto& si : selectTransients) {
                        si->color = contextTransient->color;
                    }
                }
            }
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    CloseWindow();
    return 0;
}