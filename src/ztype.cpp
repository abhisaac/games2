
#include "raylib.h"
#include "raymath.h"
#include <iostream>
#include <random>
#include <fstream>
#include <deque>
#include <queue>
#include <set>
#include <string>
#include <iterator>
const int W = 600;
const int H = 750;


std::vector<std::string> loadFile(std::string fileName) {
    std::ifstream file(fileName);
    std::vector<std::string> fileContents{std::istream_iterator<std::string>{file}, std::istream_iterator<std::string>{}};
    return fileContents;
}

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
    
    void setup(float x, float y) {
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

    void draw() {
        if (showExplosion && valid) {
            int cnt = 0;
            for (auto &particle : particles)
            {
                if (particle.size < 0.01f) {
                    ++cnt;
                }
                DrawCircle(particle.position.x, particle.position.y, particle.size, GOLD);
            }
            if (cnt == particles.size()) {valid = false; particles.clear();}
        }
    }
};
Explosion explosion;


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
        std::string spaces{"                                    "};
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
    }
    void reset() {
        addTarget();
        b_pos = 0;
        score = 0;
        level =1;
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
                b_pos++;
                if (b_pos > BULLETS_SIZE-1) b_pos = 0;
                bullets[b_pos].reset();
                bullets[b_pos].valid = true;
                bullets[b_pos].target = {targets[xi].pos.x, targets[xi].pos.y - 10.f};
                
                if (w.size() == targets[xi].y) {
                    score += w.size();
                    ++numWordsDone;
                    // std::cout << numWordsDone << std::endl;
                    if (numWordsDone%levelUpFreq == 0) {
                        // std::cout << " ......... Level up >>>>>>>>> " << std::endl;
                        level += 1;
                        showLevel = true;
                    }

                    explosion.setup(targets[xi].pos.x, targets[xi].pos.y);
                
                    //clear target
                    targets.erase(begin(targets) + xi);
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
        // std::set<std::pair<float,float>> targetcircs;
        for (int i = 0; i < BULLETS_SIZE; ++i) {
            auto& b = bullets[i];
            if (b.valid) {
                b.pos = Vector2MoveTowards(b.pos, b.target, 19.f);
                DrawCircle(b.pos.x, b.pos.y, 6.f, RED);
                // targetcircs.insert({b.target.x, b.target.y});
                if (CheckCollisionPointCircle(b.pos, b.target, 4.0f)) {
                    b.valid = false;
                }
            }
        }
        explosion.update();
        // for(auto& t : targetcircs) DrawCircle(t.first, t.second, 4.0f, WHITE);
        
    }
    void draw() {
        // DrawText(targets.front().word.c_str(), 10, 10, 20, RED); // debug
        for (auto& t : targets) t.draw();
        explosion.draw();
        
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
    int numWordsDone = 0;
    int level = 1;
    bool showLevel = false;
    int levelUpFreq = 5;
};

bool Game::gameOver = false;
bool paused = false;


int main(){
  srand(time(0));
  InitWindow(W, H, "!! Ztype");
  SetTargetFPS(60);
  Game g;
  
  float lastT = 0;
  float lastShowLevelT = 0;
  bool showLevelNotRun = true;
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
            // C = 0;
        } else if (IsKeyDown(KEY_ESCAPE)) {
            break;
        }
    } else {
        char sc[20];
        sprintf(sc, "SCORE %d --- LEVEL %d", g.score, g.level);
        DrawText(sc, 10, 10, 20, WHITE);

        if (g.showLevel) {
            if (showLevelNotRun) {
                lastShowLevelT = GetTime();
                showLevelNotRun = false;
            }
            if (GetTime()-lastShowLevelT < 1.3f) {
                char sc2[20];
                sprintf(sc, "LEVEL %d", g.level);
                DrawText(sc, W/3 - 20, W/2, 40, WHITE);
            } else {
                g.showLevel = false;
                showLevelNotRun = true;
            }
        }

        if (GetTime()-lastT > (2.7f/g.level + rand()%3) ) {
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
