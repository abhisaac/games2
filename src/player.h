#pragma once
#include "raylib.h"
#include "shapes.h"

enum class Dir {
    right,
    left
};


struct Bullet {
  Vector2 pos;
  bool valid;
  float size = 5.f; 
  Dir dir;
  float runningTime = 0.f;
  Bullet() {
    
    valid = false;
  }
  void draw() {
    if (valid)
      DrawCircle(pos.x, pos.y, size, ORANGE);
  }
  void update(float dT) {
    if (!valid) return;
    if (dir == Dir::right)
        pos.x += 400.f*dT;
    else
        pos.x -= 400.f*dT;
    if (pos.x > screenWidth || pos.x < 0.f) valid = false;

  }
};

std::vector<Bullet> bullets(100);
unsigned int b_idx = 0;


struct Player {
    Color color;
    Rectangle2 pos;
    Texture2D hero;
    Rectangle heroRec;

    float screenWidth;
    float screenHeight;
    
    Sound jumpSound;
    Dir dir;
    
    float gravity = 300.f;
    float speedy = 0.0f;
    float speedx = 1.f;
    bool jump = false;

    int frame = 0;
    const int SPRITE_FRAMES = 5;
    
    float updateTime = 1.0/10.0;
    float runningTime = 0.0;
    float dT;

    void reset () {
         gravity = 300.f;
         speedy = 0.0f;
         speedx = 1.f;
         jump = false;
        pos.x = 130;
        pos.y = 130;
    }
    Player(float w, float h) : color(ORANGE), screenWidth(w), screenHeight(h) {
        hero  =  LoadTexture("assets/sprite.png");
        heroRec.width = hero.width/SPRITE_FRAMES;
        heroRec.height = hero.height;
        heroRec.x = 0;
        heroRec.y = 0;
        pos = {130, 130, heroRec.width, heroRec.height};
        reset();
        jumpSound = LoadSound("assets/jump.mp3");
    };
    ~Player() {
        UnloadTexture(hero);
    }
    void update() {
        dT = GetFrameTime();
        
        if (IsKeyDown(KEY_RIGHT)) {
            dir = Dir::right;
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
            dir = Dir::left;
            speedx = -306.f;
            if (heroRec.width > 0) heroRec.width = -heroRec.width;
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
            jump = true;
            PlaySound(jumpSound);
        }

//Fire bullets
        if (IsKeyPressed(KEY_SPACE)) {
            if (b_idx > bullets.size()-1) b_idx = 0;
            bullets[b_idx].valid = true;
            bullets[b_idx].dir = dir;
            bullets[b_idx].pos.x = pos.x;
            if (dir == Dir::right)
                bullets[b_idx].pos.x += pos.width;
           
            bullets[b_idx].pos.y = pos.y + heroRec.height/2.f;
            b_idx++;
        }
        
        for (auto& b : bullets) { b.update(dT); b.draw(); }
        
        
        pos.x += speedx * dT;
        if (pos.x+heroRec.width > screenWidth) pos.x = screenWidth-heroRec.width;
        if (pos.x < 0) pos.x = 0;
    }
    void updateSpeed(bool collide, float newy) {
        if (!collide) {
            pos.y += speedy * dT;
            if (pos.y+heroRec.height> screenHeight) pos.y = screenHeight-heroRec.height;
            if (pos.y < 0) pos.y = 0.f;
            speedy += gravity * dT;
        } else {
            jump = false;
            pos.y = newy;
            speedy = 0.f;
        }
            
        speedx *= .8f;
        if (speedx < 0.001f) speedx = 0;
    }
    void draw() {
        DrawTextureRec(hero,heroRec,{pos.x, pos.y},WHITE);
    }
};