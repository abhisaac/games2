/* Asteroids:
    Another classic game
*/
/*
TODOs
1. 
2. texture
3. 
*/

#include "raylib.h"
#include <iostream>
#include <vector>
struct GameConstants {
  inline static const float SIZEX = 800.f;
  inline static const float SIZEY = 450.f;
  inline static const float DELTA = 1.4f;
};

Vector2 randDelta() {
  return { rand()%2 ? -GameConstants::DELTA : GameConstants::DELTA,
           GameConstants::DELTA};
  // return {(float)rand()*GameConstants::DELTA/ (float)(RAND_MAX) - GameConstants::DELTA, 
  //         (float)rand()*GameConstants::DELTA/ (float)(RAND_MAX) - GameConstants::DELTA};
}
Vector2 randPosition() {
  // respawn at the top 1/8th of the window
  return {(float)rand()*GameConstants::SIZEX/ (float)(RAND_MAX),  
          (float)rand()*GameConstants::SIZEY/ (float)(8.0f * RAND_MAX)};
}

struct Sprite
{
  Sprite(char *name)
  {
    sp = LoadTexture(name);
    pos = {(GameConstants::SIZEX/2.f), (GameConstants::SIZEY/2.f)};
  }
  void draw()
  {
    DrawTexture(sp, pos.x, pos.y, WHITE);
  }
  void update() {
    const float updateBy = 5.0f;
    if (IsKeyDown(KEY_RIGHT)) pos.x += updateBy;
    if (IsKeyDown(KEY_LEFT)) pos.x -= updateBy;
    if (IsKeyDown(KEY_UP)) pos.y -= updateBy;
    if (IsKeyDown(KEY_DOWN)) pos.y += updateBy;
    
    if (pos.x > GameConstants::SIZEX) pos.x = 0.f;
    if (pos.x < 0.f) pos.x = GameConstants::SIZEX;
    if (pos.y > GameConstants::SIZEY) pos.y = 0.f;
    if (pos.y < 0.f) pos.y = GameConstants::SIZEY;
  }
  ~Sprite()
  {
    UnloadTexture(sp);
  }
  Texture2D sp;
  Vector2 pos;
};
long long cnt = 0L;
struct Enemy {
  Enemy(Texture2D& sp, Vector2 pos) : 
        sp(sp), pos(pos), delta(randDelta()), destroyed(false), offset(0.f),
        size(1.f)
  {
    // puts("EE");
  }
  void draw()
  {
    if (destroyed) {
      offset += 1.4f;
      size -= 0.02f;
      // DrawCircle(pos.x - offset*2.f, pos.y - offset, size, GREEN);
      // DrawCircl e(pos.x - offset, pos.y + offset, size, GREEN);
      // DrawCircle(pos.x + offset, pos.y, size, GREEN);
      if (size > 0.f) {
        DrawTextureEx(sp, {pos.x - offset*2.f, pos.y- offset}, -offset, size, WHITE);
      DrawTextureEx(sp, {pos.x - offset, pos.y}, offset, size, WHITE);
      DrawTextureEx(sp, {pos.x + offset, pos.y}, -offset, size, WHITE);  
      } else {
        reset();
      }
      
    } else {
      DrawTexture(sp, pos.x, pos.y, WHITE);
    }
  }
  void reset() {
    pos = randPosition();
          delta = randDelta(); 
    offset = 0.f;
    destroyed = false;
    size = 1.f;
  }
  void update() {
    pos.x += delta.x;
    pos.y += delta.y;

    if (pos.x > GameConstants::SIZEX || pos.x < 0.f 
        || pos.y > GameConstants::SIZEY || pos.y < 0.f) {
          reset();
          // ++cnt;
        } 
  }
  Texture2D sp;
  Vector2 pos;
  Vector2 delta;
  bool destroyed;
  float offset;
  float size;
};

 int b_idx = 0;
struct Bullet {
  Vector2 pos;
  bool valid;
  Bullet() {
    valid = false;
  }
  void draw() {
    if (valid)
      DrawCircle(pos.x, pos.y, 3.1f, ORANGE);
  }
  void update() {
    if (!valid) return;
    pos.y -= 4.f;
    if (pos.y < 0.f) valid = false;;
  }
};

int main()
{
  srand(time(0));
  long long int elapsed = 0;
  
  InitWindow(GameConstants::SIZEX, GameConstants::SIZEY, "!! Asteroids");
  Texture2D enemyTex = LoadTexture("e.png");
  SetTargetFPS(60);
  
  // Enemies
  int N = 4;
  std::vector<Enemy> badguys;
  for (int i = 0; i< N; ++i) 
    badguys.emplace_back(enemyTex, randPosition());
    

  // Bullets
  const int B = 500;
  Bullet bullets[B];
 
  
  {
    Sprite ship("x.png");
    bool gameover = false;
    while (!WindowShouldClose())
    {
      
      BeginDrawing();
        ClearBackground(BLACK);
        if (gameover) {
          DrawText("GAME OVER", GameConstants::SIZEX/2 - 20, GameConstants::SIZEY/2, 20, WHITE);
          DrawText("Press S to start", GameConstants::SIZEX/2 - 20, GameConstants::SIZEX/2 + 10, 10, WHITE);
          if (IsKeyDown(KEY_S)) {
              gameover = false;
              cnt = 0;
          }
            
        } else {

        // score
          char s[14];
          sprintf(s, "SCORE: %lld", cnt);
          DrawText(s, 10, 10, 20, WHITE); 
        // ship
          ship.draw();
          ship.update();

        //collision check
        for (auto& e : badguys) {
          e.draw();
          e.update();
          if (!e.destroyed && CheckCollisionRecs({e.pos.x, e.pos.y, 24.f, 24.f},
                                 {ship.pos.x, ship.pos.y, 70.f, 70.f})) {
            gameover = true;
          }
        }

        // bullet
        if (IsKeyDown(KEY_SPACE) || elapsed) {
          ++elapsed;
          if (elapsed > 4) {
            elapsed = 0;
            int i = 0;
            while (i < 500) {
              if (!bullets[i].valid) break;
              ++i;
            } 
            bullets[i].pos = {ship.pos.x + 40.f, ship.pos.y};
            bullets[i].valid = true;
          }
        }
        // TODO: disable bullet after collision
          for(int i =0 ;i<500;++i) {
            bullets[i].draw();
            bullets[i].update();
            if (!bullets[i].valid) continue;
            for (auto& e : badguys) {
              if (CheckCollisionPointRec(bullets[i].pos, 
                {e.pos.x, e.pos.y, 80.f, 80.f})) {
                  ++cnt;
                  // e.reset();
                  e.destroyed = true;
                  bullets[i].valid = false;
              }
            }
          }
        

        }
        

      EndDrawing();
    }
  }
  UnloadTexture(enemyTex);
  CloseWindow();
  return 0;
}
