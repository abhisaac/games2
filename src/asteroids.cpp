/* Asteroids:
    Another classic game
*/
/*
TODOs
1. game mechanics
2. clean up
3. refactor explosion into a common header
4. 
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
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) pos.x += updateBy;
    if (IsKeyDown(KEY_LEFT)|| IsKeyDown(KEY_A)) pos.x -= updateBy;
    if (IsKeyDown(KEY_UP)|| IsKeyDown(KEY_W)) pos.y -= updateBy;
    if (IsKeyDown(KEY_DOWN)|| IsKeyDown(KEY_S)) pos.y += updateBy;
    
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

struct ExplosionParticle
{
    Vector2 position;
    Vector2 velocity;
    float size;
};
bool showExplosion = true;
struct Explosion {
    std::vector<ExplosionParticle> particles;
    bool valid = false;
    const int numParticles = 60;
    const float explosionSpeed = .3f; // Adjust this value for slower/faster explosion
    Color tint;
    
    void setup(float x, float y, Color c) {
        tint = c;
        particles.clear();
        valid = true;
        for (int i = 0; i < numParticles; ++i) {
            ExplosionParticle particle;
            particle.position = {x, y}; //{GetMouseX(), GetMouseY()};
            particle.velocity = {(float)GetRandomValue(-10, 10), (float)GetRandomValue(-10, 10)};
            particle.size = 9.f;
            particles.push_back(particle);
        }
    }

    void update() {
        if (showExplosion && valid) {
            for (auto &particle : particles)
            {
                particle.position.x += particle.velocity.x * explosionSpeed;
                particle.position.y += particle.velocity.y * explosionSpeed;
                particle.size -= (float)GetRandomValue(30, 70)/100;
            }
        }
    }

    bool draw() {
        if (showExplosion && valid) {
            int cnt = 0;
            for (auto &particle : particles)
            {
                if (particle.size < 0.01f) {
                    ++cnt;
                }
                DrawCircle(particle.position.x, particle.position.y, particle.size, tint);
            }
            if (cnt == particles.size()) {valid = false; particles.clear(); return false;}
        }
        return true;
    }
};

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
      // offset += 0.4f;  
      // size -= 0.02f;
      // if (size > 0.f) {
      //   DrawTextureEx(sp, {pos.x - offset*2.f, pos.y- offset}, -offset, size, tint);
      //   DrawTextureEx(sp, {pos.x - offset, pos.y}, offset, size, tint);
      //   DrawTextureEx(sp, {pos.x + offset, pos.y}, -offset, size, tint);  
      // } else {
      //   reset();
      // }
      if (!explosion.draw()) {
        reset();
      }
    } else {
      DrawTexturePro(sp, { 0.0f, 0.0f, (float)sp.width, (float)sp.height }, 
        { pos.x, pos.y, (float)sp.width*fullsize, (float)sp.height*fullsize }, 
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
    fullsize = (rand()%5)*.2f + .6f;
    // std::cout << fullsize << std::endl;
    size = fullsize;
    
    rot = 0.0f;
    dir = rand()  %2 ;
  }
  
  void update() {
    if (destroyed) {
      explosion.update();
      return;
    }
    pos.x += delta.x;
    pos.y += delta.y;

    if (pos.x > GameConstants::SIZEX || pos.x < 0.f 
        || pos.y > GameConstants::SIZEY || pos.y < 0.f) 
    {
      reset();
    } 

  }
  Explosion explosion;
  bool dir;
  Texture2D sp;
  Vector2 pos;
  Vector2 delta;
  bool destroyed;
  float offset;
  float size;
  float fullsize;
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
          DrawText("Press R to Restart", GameConstants::SIZEX/3 - 20, GameConstants::SIZEY/2 + 50, 20, WHITE);
          if (IsKeyDown(KEY_R)) {
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
              if (!e.destroyed && CheckCollisionPointCircle(bullets[i].pos, {e.pos.x, e.pos.y}, e.sp.width/2))
              {    ++cnt;
                  e.destroyed = true;
                  e.explosion.setup(e.pos.x, e.pos.y, e.tint);
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
