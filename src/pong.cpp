/* Pong */
#include "raylib.h"
#include <iostream>
// #include <vector>

struct GameConstants {
  inline static const float SIZEX = 600.f;
  inline static const float SIZEY = 750.f;
  inline static const float DELTAX = 0.4f;
  inline static const float DELTAY = 3.4f;
};

enum class Dir {
    up,
    down
};
void updateBar(Rectangle& r, float updateBy, Dir dir) {
    if (dir == Dir::up) {
        r.y -= updateBy;
        if (r.y < 0.f) r.y = 0.f;
    } else {
        r.y += updateBy;
        if (r.y + r.height > GameConstants::SIZEY) {
            r.y -= updateBy;
        }
    }
}

int main()
{
  srand(time(0));
  long long int elapsed = 0;
  
  InitWindow(GameConstants::SIZEX, GameConstants::SIZEY, "!! Pong");
  
  SetTargetFPS(60);

// Pongs
  Rectangle one,two;

  one.x = 0.f;
  one.width = 30.f;
  one.height = 180.f;
  one.y = GameConstants::SIZEY/2.f - one.height/2.f; 

  
  two.width = 30.f;
  two.x = GameConstants::SIZEX-two.width;
  two.height = 180.f;
  two.y = GameConstants::SIZEY/2.f - two.height/2.f; 

// Ball
  float cradius = 10.f;
  float cstart_offset = 14.f;
  Vector2 c {one.x + one.width + cradius/2.f + cstart_offset, one.y + one.height/2.f};
  Vector2 cvec {5.f, -2.f};
  
  const float updateBy = 5.0f;
  {
    InitAudioDevice();
    
// Load sounds
    Sound backgroundSound = LoadSound("assets/loop.wav");
    Sound explosionSound = LoadSound("assets/explode.wav");

    PlaySound(backgroundSound);

    bool gameover = false;
    long long cntx = 0, cnty = 0;
    while (!WindowShouldClose())
    {
      if (!IsSoundPlaying(backgroundSound)) {
        PlaySound(backgroundSound);
      }
      BeginDrawing();
      ClearBackground(GRAY);
        if (gameover) {
          DrawText("GAME OVER", GameConstants::SIZEX/3 - 20, GameConstants::SIZEY/2, 40, WHITE);
          DrawText("Press R to Restart", GameConstants::SIZEX/3 - 20, GameConstants::SIZEY/2 + 50, 20, WHITE);
          if (IsKeyDown(KEY_R)) {
              gameover = false;
              cntx = 0;
              cnty = 0;

                c = {one.x + one.width + cradius/2.f + cstart_offset, one.y + one.height/2.f};
                cvec = {5.f, -2.f};
          }
            
        } else {
            // score
          char s[14];
          sprintf(s, "SCORE: X = %lld, Y = %lld", cntx, cnty);
          DrawText(s, 10, 10, 20, WHITE); 
            c.x += cvec.x;
            c.y += cvec.y;

            if (IsKeyDown(KEY_W)) updateBar(one, updateBy, Dir::up);
            if (IsKeyDown(KEY_S)) updateBar(one, updateBy, Dir::down);
            if (IsKeyDown(KEY_UP))   updateBar(two, updateBy, Dir::up);
            if (IsKeyDown(KEY_DOWN)) updateBar(two, updateBy, Dir::down);

            DrawRectangleRec(one, WHITE);
            DrawRectangleRec(two, WHITE);

            DrawCircle(c.x, c.y, cradius, RED);

            if (CheckCollisionCircleRec(c, cradius, one)) {
                cvec.x = -cvec.x;
                c.x += 3.f;
                ++cntx;
            } else if (CheckCollisionCircleRec(c, cradius, two)) {
                cvec.x = -cvec.x;
                c.x -= 3.f;
                ++cnty;
            }
            if (c.y < 0.f || c.y > GameConstants::SIZEY) {
                cvec.y = -cvec.y;
            }
            if (c.x < 0.f || c.x > GameConstants::SIZEX) {
                gameover = true;
            }
        }
      EndDrawing();
    }
    UnloadSound(backgroundSound);

    UnloadSound(explosionSound);
  }
  
  CloseWindow();
  return 0;
}
