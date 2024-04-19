#pragma once
#include "raylib.h"
#include "utils.h"
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

enum SHAPE {
    SELECT,
    RECTANGLE,
    CIRCLE,
    _SHAPE_SIZE
} ;


// NE, NW, SE, SW handles
enum DIRECTION {
    NW, NE, SE, SW
} direction;

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
    virtual void resize(Vector2 mousePoint) {

    }
    bool isHovering(Vector2 mousePoint) {
        return CheckCollisionPointRec(mousePoint, bounds);
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

