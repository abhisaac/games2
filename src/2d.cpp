
#include "raylib.h"
const int screenWidth = 1200;
const int screenHeight = 800;
#include "editor.h"
#include "player.h"



int main(void)
{
    InitAudioDevice();
    
  
    InitWindow(screenWidth, screenHeight, "!! 2d");
    GuiSetStyle(DEFAULT, TEXT_SIZE, 19.f);
    // GuiSetStyle(DEFAULT, BACKGROUND_COLOR, 0xff0000ff);  
    SetTargetFPS(60);

    Player p(screenWidth, screenHeight);
    Editor editor(screenWidth, screenHeight);
    

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
      BeginDrawing();

        ClearBackground(BLACK);

        if (IsKeyPressed(KEY_R)) {
            p.reset();
        }

        if (!mapedit) {
          p.update();
          bool collide = false;
          float newy = 0.f;
          auto& m = editor.m;
          // TODO: can this collision check be done in map??
          for (int i  = 0;i < m.objs.size(); ++i) {
              if (m.objs[i]) {
                  if (m.objs[i]->shape == RECTANGLE) {
                      RectShape* tmp = (RectShape*)m.objs[i];
                      if (CheckCollisionPointRec({p.pos.x, p.pos.y + p.heroRec.height}, tmp->bounds)) {
                          newy = tmp->bounds.y-p.heroRec.height;
                          collide = true;
                          break;
                      }
                  } else if (m.objs[i]->shape == CIRCLE) {
                      CircleShape* tmp = (CircleShape*)m.objs[i];
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
          
        editor.update();
        p.draw();
      EndDrawing();
        //----------------------------------------------------------------------------------
    }

    CloseWindow();
    return 0;
}
