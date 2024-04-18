/* Asteroids:
    Another classic game
*/
/*
TODOs
1. game mechanics
2. explosion animation can be better
*/

#include "raylib.h"
#include <iostream>
#include <vector>
struct GameConstants {
  inline static const float SIZEX = 600.f;
  inline static const float SIZEY = 750.f;
  inline static const float DELTAX = 0.4f;
  inline static const float DELTAY = 3.4f;
};

Vector2 randDelta() {
  return { rand()%2 ? -GameConstants::DELTAX : GameConstants::DELTAX,
           GameConstants::DELTAY};
}
Vector2 randPosition() {
  // respawn at the top 1/8th of the window
  return {(float)rand()*GameConstants::SIZEX/ (float)(RAND_MAX),  0};
}

// SHIP
struct Sprite
{
  Sprite(const char *name)
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
    if (pos.y > GameConstants::SIZEY-70.f) pos.y = GameConstants::SIZEY-70.f;
    if (pos.y < 0.f) pos.y = 0.f;
  }
  ~Sprite()
  {
    UnloadTexture(sp);
  }
  Texture2D sp;
  Vector2 pos;
};


long long cnt = 0L;
Texture2D enemyTex;

// ENEMY
struct Enemy {
  Enemy(Texture2D& sp, Vector2 pos, Color tint) : 
        sp(sp), tint(tint)
  {
    reset();
  }

  void draw()
  {
    rot += 0.5f;
    if (destroyed) {
      offset += 0.4f;  
      size -= 0.02f;
      if (size > 0.f) {
        DrawTextureEx(sp, {pos.x - offset*2.f, pos.y- offset}, -offset, size, tint);
        DrawTextureEx(sp, {pos.x - offset, pos.y}, offset, size, tint);
        DrawTextureEx(sp, {pos.x + offset, pos.y}, -offset, size, tint);  
      } else {
        reset();
      }
      
    } else {
      DrawTexturePro(sp, { 0.0f, 0.0f, (float)sp.width, (float)sp.height }, 
        { pos.x, pos.y, (float)sp.width, (float)sp.height }, 
        {sp.width / 2.f, sp.height / 2.f}, dir ? -rot : rot, tint);
    }
  }

  void reset() {
    // sp = enemyTex[rand()%5];  
    tint = {(unsigned char) (rand() % 255), (unsigned char) (rand() % 255), (unsigned char) (rand() % 255), 255};
    pos = randPosition();
    delta = randDelta(); 
    offset = 0.f;
    destroyed = false;
    size = 1.f;
    rot = 0.0f;
    dir = rand()  %2 ;
  }
  
  void update() {
    pos.x += delta.x;
    pos.y += delta.y;

    if (pos.x > GameConstants::SIZEX || pos.x < 0.f 
        || pos.y > GameConstants::SIZEY || pos.y < 0.f) 
    {
      reset();
    } 
  }

  bool dir;
  Texture2D sp;
  Vector2 pos;
  Vector2 delta;
  bool destroyed;
  float offset;
  float size;
  float rot;
  Color tint;
};


struct Bullet {
  Vector2 pos;
  bool valid;
  float size = 5.f; 
  Bullet() {
    valid = false;
  }
  void draw() {
    if (valid)
      DrawCircle(pos.x, pos.y, size, ORANGE);
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
  
  enemyTex = LoadTexture("assets/e1.png");
  // enemyTex[1] = LoadTexture("assets/e2.png");
  // enemyTex[2] = LoadTexture("assets/e3.png");
  // enemyTex[3] = LoadTexture("assets/e4.png");
  // enemyTex[4] = LoadTexture("assets/e5.png");
  SetTargetFPS(60);
  
  // Enemies
  int N = 4;
  std::vector<Enemy> badguys;
  for (int i = 0; i< N; ++i) {
    Color color {(unsigned char) (rand() % 255), (unsigned char) (rand() % 255), (unsigned char) (rand() % 255), 255};
    badguys.emplace_back(enemyTex, randPosition(), color);
  }
    
    

  // Bullets
  const int B = 500;
  Bullet bullets[B];
 
  
  {
    Sprite ship("assets/fighter.png");
    bool gameover = false;
    while (!WindowShouldClose())
    {
      
      BeginDrawing();
        ClearBackground(GRAY);
        if (gameover) {
          DrawText("GAME OVER", GameConstants::SIZEX/3 - 20, GameConstants::SIZEY/2, 40, WHITE);
          DrawText("Press S to start", GameConstants::SIZEX/3 - 20, GameConstants::SIZEY/2 + 50, 20, WHITE);
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

          if (!e.destroyed && 
          CheckCollisionCircleRec({e.pos.x, e.pos.y}, e.sp.width/2, 
                {ship.pos.x, ship.pos.y, (float)ship.sp.width, (float)ship.sp.height}))
          {
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
            bullets[i].pos = {ship.pos.x + ship.sp.width/2.f, ship.pos.y};
            bullets[i].valid = true;
          }
        }
        
          for(int i =0 ;i<500;++i) {
            bullets[i].draw();
            bullets[i].update();
            if (!bullets[i].valid) continue;
            for (auto& e : badguys) {
              if (CheckCollisionPointCircle(bullets[i].pos, {e.pos.x, e.pos.y}, e.sp.width/2))
              {    ++cnt;
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
  // UnloadTexture(enemyTex[1]);
  // UnloadTexture(enemyTex[2]);
  // UnloadTexture(enemyTex[3]);
  // UnloadTexture(enemyTex[4]);
  CloseWindow();
  return 0;
}
