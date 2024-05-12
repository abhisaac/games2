#include "raylib.h"
#include <vector>
const int W = 800;
const int H = 600;
const int offset = 20;
const int w2 = W - 2*offset;
const int NumRows = 5;
const int NumPerRow = 10;

struct Entity {
    Vector2 pos;
    Vector2 size;
    Color color;
    
    bool valid;

    Entity(Vector2 pos, Vector2 size, Color color = BLUE) 
        : pos(pos), size(size), color(color), valid(true)
        {}

    void draw() {
        if (valid)
            DrawRectangleV(pos, size, color);
    }

    Rectangle getRect() {
        return {pos.x , pos.y, size.x, size.y};
    }

    bool checkCollision(Entity& b) {
        return CheckCollisionRecs(b.getRect(), getRect());
    }
};


int main(){
  InitWindow(W, H, "Breakout");
  SetTargetFPS(60);
  
  std::vector<Entity> rects;
  for (auto i = 0; i < NumRows; ++i) {
    for (auto j = 0; j < NumPerRow; ++j) {
        Vector2 pos;
        pos.x = offset*(j+2) + 53*j;
        pos.y = 30 + offset*(i+2) + 20*i;
        Vector2 size = {53, 20};
        rects.emplace_back(pos, size);
    }
  }
  Vector2 pos;
  pos.x = W/2 - 35;
  pos.y = H - offset*5;
  Entity p(pos, {70, 25}, DARKBLUE);

  Entity b({W/2, H/2}, {20, 20}, RED);
  Vector2 b_dir = {200, 200};
  int score = 0;
  InitAudioDevice();
    
// Load sounds
  Sound snd = LoadSound("assets/fire.wav");
  while (!WindowShouldClose()){
    float dt = GetFrameTime();
    BeginDrawing();
    ClearBackground(BLACK);
    char s[14];
    sprintf(s, "SCORE: %lld", score);
    DrawText(s, offset + 10, offset + 10, 20, WHITE); 
    DrawRectangleLines(offset, offset, W-2*offset, H-2*offset, GRAY);
    if (IsKeyDown(KEY_RIGHT)) {
        p.pos.x += 200*dt;
        if (p.pos.x > W-offset-70) p.pos.x -= 200*dt;
    }
    if (IsKeyDown(KEY_LEFT)) {
        p.pos.x -= 200*dt;
        if (p.pos.x < offset) p.pos.x += 200*dt;
    }
    
    for (auto& r : rects) {
        if (r.valid && r.checkCollision(b)) {
            ++score;
            r.valid = false;
            b_dir.y = -b_dir.y;
            PlaySound(snd);
        }
            
        r.draw();
    }
    if (p.checkCollision(b)) {
        PlaySound(snd);
        b_dir.y = -b_dir.y;
    }
    b.pos.y += b_dir.y*dt;
    b.pos.x += b_dir.x*dt;
    if (b.pos.x > (W-2*offset)) b_dir.x = -b_dir.x;
    else if (b.pos.x < offset) b_dir.x = -b_dir.x;
    if (b.pos.y < offset) b_dir.y = -b_dir.y;
    else if (b.pos.y > (H-2*offset)) b_dir.y = -b_dir.y;
    p.draw();
    b.draw();
    EndDrawing();
  }
UnloadSound(snd);
  CloseWindow();
  return 0;
}

