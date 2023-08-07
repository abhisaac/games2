
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
    
    struct TargetHitCircle {
        Vector2 pos;
        int valid;
        void draw() {
            if (valid)
                DrawCircle(pos.x, pos.y, 4.0f, WHITE);
        }
        void update() {
            pos.y +=0.4f;
        }
    };

    struct Bullet {
        Vector2 pos;
        bool valid;
        // int hit_idx;
        Bullet () {
            reset();
        }
        void reset() {
            pos = {W/2.f, H/1.f};
            valid = false;
            // hit_idx = 0;
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
            pos.y +=0.4f;
        }
        void draw() {
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
        // hits[h_idx++].pos = targets.front().pos;
        // hits[h_idx].valid = words[sample].size();
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
            // bullets[b_pos].hit_idx = h_idx;
            if (w.size() == targets.front().y) {
                targets.pop_front();
            } 
        }
    }
    void update () {
        for (auto& t : targets) t.update();
        // for (auto& h : hits) h.update();
        
        for (int i = 0; i < 100; ++i) {
            auto& b = bullets[i];
            if (b.valid) {
                b.pos = Vector2MoveTowards(b.pos, targets.front().pos, 29.f);
                DrawCircle(b.pos.x, b.pos.y, 6.f, RED);
                if (CheckCollisionPointCircle(b.pos, targets.front().pos, 4.0f)) {
                    bullets[b_pos--].valid = false;
                }
            } else {
                b.reset();
            }
        }
    }
    void draw() {
        for (auto& t : targets) t.draw();
        // for (auto& h : hits) h.draw();
    }

    // TargetHitCircle hits[100];
    int h_idx = 0;
    std::vector<std::string> words;
    std::deque<Target> targets;
    // Vector2 pos; 
    int b_pos;
    Bullet bullets[100];
    
};

int main(){
  srand(time(0));
  InitWindow(W, H, "!! Ztype");
  SetTargetFPS(60);
  Game g;
  float pos = 1.1f;
  long long int C = 0;
  while (!WindowShouldClose()){
    
    BeginDrawing();
    if (++C % 99 == 0) g.addTarget();
    g.processInput();
    ClearBackground(BLACK);
    g.update();
    g.draw();
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
