
#include "raylib.h"
#include "raymath.h"
#include <iostream>
#include <random>
#include <fstream>
#include <deque>
#include <queue>
#include <set>
#include <iterator>
const int W = 600;
const int H = 750;

/*TODOs
1. levels
2. Blast effect for word
3. 
*/

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
        Vector2 target;
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
        Target(std::string w, float posx) : word(w), y(0) {
            pos.x = posx; 
            pos.y = 0.f;
            done = false;
        }
        bool isClose(float x) {
            
            return false;
        }
        void update() {
            pos.y +=.7f;
            if (pos.y > H) Game::gameOver = true;
        }
        void draw() {
            DrawCircle(pos.x, pos.y - 10.f, 4.0f, WHITE);
            DrawText((spaces.substr(0, y) + word.substr(y)).c_str(), 
                        pos.x, pos.y, 20, y>0 ? RED : GOLD);
        }
    };
    Game() {
        
        readWords();
        reset();
    }
    void readWords() {
        words = loadFile("assets/dict.txt");
    }
    void addTarget() {
       static std::random_device rd; // source of randomness
       static std::mt19937 rng(rd()); // seed random number engine
       static std::uniform_int_distribution<std::size_t> uid(0, words.size()-1);

       std::size_t sample = uid(rng);
        
        float posx = rand()%(W-120);
        targets.emplace_back(words[sample], posx);
        // firstwords.emplace_back(words[sample][0]);
    }
    void reset() {
        addTarget();
        b_pos = 0;
        score = 0;
    }
    
    void processInput() {
        auto key = GetKeyPressed();
        if (key==0) return;
        keycount++;
        if (xi == -1) {
            for (int i = 0; auto &t : targets) {
                if (t.word[0]-'a' == (key-65)) {xi = i; break;}
                i++;
            }
        } 
        if (xi != -1) {
            auto w = targets[xi].word;
        
            if ((key-65) == w[targets[xi].y]-'a') {
                targets[xi].y++;
                bullets[b_pos++].reset();
                bullets[b_pos].valid = true;
                bullets[b_pos].target = {targets[xi].pos.x, targets[xi].pos.y - 10.f};
                if (b_pos > BULLETS_SIZE-1) b_pos = 0;
                
                if (w.size() == targets[xi].y) {
                    score += w.size();
                    targets.erase(begin(targets) + xi);
                    // firstwords.erase(begin(firstwords) + xi);
                    xi = -1;
                } 
            } else {
                ++keymisses;
            }
        } else {
            keymisses++;
        }
        
    }
    void update () {
        for (auto& t : targets) t.update();
        std::set<std::pair<float,float>> targetcircs;
        for (int i = 0; i < BULLETS_SIZE; ++i) {
            auto& b = bullets[i];
            if (b.valid) {
                b.pos = Vector2MoveTowards(b.pos, b.target, 19.f);
                DrawCircle(b.pos.x, b.pos.y, 6.f, RED);
                targetcircs.insert({b.target.x, b.target.y});
                if (CheckCollisionPointCircle(b.pos, b.target, 4.0f)) {
                    b.valid = false;
                }
            }
        }
        for(auto& t : targetcircs) DrawCircle(t.first, t.second, 4.0f, WHITE);
        
    }
    void draw() {
        // DrawText(targets.front().word.c_str(), 10, 10, 20, RED); // debug
        for (auto& t : targets) t.draw();
    }

    
    std::vector<std::string> words;
    std::vector<Target> targets;
    // std::vector<char> firstwords;
    int xi = -1;
    unsigned long long keycount = 1;
    unsigned long long keymisses = 1;
    int b_pos;
    static const int BULLETS_SIZE = 100;
    Bullet bullets[BULLETS_SIZE];
    static bool gameOver;
    int score;
};
bool Game::gameOver = false;
int main(){
  srand(time(0));
  InitWindow(W, H, "!! Ztype");
  SetTargetFPS(60);
  Game g;
  
  long long int C = 0;
  float lastT = 0;
  bool paused = false;
  SetExitKey(0);
  while (!WindowShouldClose()){
    BeginDrawing();

    if (!g.gameOver && IsKeyPressed(KEY_ESCAPE)) {
        paused = !paused;
    }

    if (paused) {
        DrawText("PAUSED", W/3 - 20, W/2, 40, WHITE);
        char sc[40];
        sprintf(sc, "Accuracy: %.02f%", 100*(g.keycount-g.keymisses)/(float)g.keycount);
        DrawText(sc, W/3 - 20, H/2 + 50, 20, WHITE);
    }
    else if (g.gameOver) {
        DrawText("GAME OVER", W/3 - 20, W/2, 40, WHITE);
        DrawText("Press S to start", W/3 - 20, H/2 + 50, 20, WHITE);
        if (IsKeyDown(KEY_S)) {
            g.gameOver = false;
            g.targets.clear();
            g.reset();
            C = 0;
        } else if (IsKeyDown(KEY_ESCAPE)) {
            break;
        }
    } else {
        char sc[20];
        sprintf(sc, "SCORE %d", g.score);
        DrawText(sc, 10, 10, 20, WHITE);

        if (GetTime()-lastT > (1.7 + rand()%3) ) {
            g.addTarget();
            lastT = GetTime();
        }
            
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
