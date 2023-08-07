
#include "raylib.h"
#include "raymath.h"
#include <iostream>
#include <random>
#include <fstream>
#include <deque>
#include <queue>
const int W = 600;
const int H = 750;

std::vector<std::string> loadFile(std::string fileName) {
    std::ifstream file(fileName);
    // std::assert(file);

    std::vector<std::string> fileContents;
    std::copy(std::istream_iterator<std::string>(file), 
              std::istream_iterator<std::string>(), 
              std::back_inserter(fileContents));

    return fileContents;
}


struct Game {
    

    struct Bullet {
        Vector2 pos;
        bool valid;
        Bullet () {
            reset();
        }
        void reset() {
            pos = {W/2.f, H/1.f};
            valid = false;
        }
    };

    
    struct Target {
        std::string spaces="                                    ";
        Vector2 pos;
        bool done;
        std::string word;
        int y;
        Target(std::string w) : word(w), y(0) {
            pos.x = (rand()%(W-120)); 
            pos.y = 0.f;
            done = false;
        }
        void update() {
            pos.y +=.7f;
            if (pos.y > H) Game::gameOver = true;
        }
        void draw() {
            DrawCircle(pos.x, pos.y-10.f, 4.0f, WHITE);
            DrawText((spaces.substr(0, y) + word.substr(y)).c_str(), 
                        pos.x, pos.y, 20, GOLD);
        }
    };
    Game() {
        h_idx = 0;
        b_pos = 0;
        readWords();
        reset();
    }
    void readWords() {
        words = loadFile("dict.txt");
    }
    void addTarget() {
       std::random_device rd; // source of randomness
       std::mt19937 rng(rd()); // seed random number engine
       std::uniform_int_distribution<std::size_t> uid(0, words.size()-1);
        std::size_t sample = uid(rng);
        
        targets.emplace_back(words[sample]);
    }
    void reset() {
        addTarget();
    }
    
    void processInput() {
        auto key = GetKeyPressed();
        if (key==0) return;
        auto w = targets.front().word;
        if ((key-65) == w[targets.front().y]-'a') {
            targets.front().y++; // = y;
            bullets[b_pos++].valid = true;
            if (b_pos > 99) b_pos = 0;
            
            if (w.size() == targets.front().y) {
                targets.pop_front();
            } 
        }
    }
    void update () {
        for (auto& t : targets) t.update();
        
        for (int i = 0; i < 100; ++i) {
            auto& b = bullets[i];
            if (b.valid) {
                
                b.pos = Vector2MoveTowards(b.pos, targets.front().pos, 19.f);
                DrawCircle(b.pos.x, b.pos.y, 6.f, RED);
                if (CheckCollisionPointCircle(b.pos, targets.front().pos, 4.0f)) {
                    b.valid = false;
                }
            } 
            
        }
    }
    void draw() {
        DrawText(targets.front().word.c_str(), 10, 10, 20, RED);
        for (auto& t : targets) t.draw();
    }

    
    int h_idx = 0;
    std::vector<std::string> words;
    std::deque<Target> targets;
    
    int b_pos;
    Bullet bullets[100];
    static bool gameOver;
};
bool Game::gameOver = false;
int main(){
  srand(time(0));
  InitWindow(W, H, "!! Ztype");
  SetTargetFPS(60);
  Game g;
  
  long long int C = 0;
  while (!WindowShouldClose()){
    BeginDrawing();
    if (g.gameOver) {
        DrawText("GAME OVER", W/3 - 20, W/2, 40, WHITE);
        DrawText("Press S to start", W/3 - 20, H/2 + 50, 20, WHITE);
        if (IsKeyDown(KEY_S)) {
            g.gameOver = false;
           
            g.targets.clear();
             g.reset();
            C = 0;
        }
    } else {
        
        if (++C % 99 == 0) g.addTarget();
        g.processInput();
        ClearBackground(BLACK);
        g.update();
        g.draw();
        
    }
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
