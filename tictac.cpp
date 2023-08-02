
#include "raylib.h"
#include <vector>
#include <iostream>
#include <stack>

// TODO: 
// draw strike through after WIn

struct Board  {
    
    enum Turn {
        FREE, LEFT, RIGHT
    };

    // board elements
    static inline const float ROUNDEDNESS = 0.4f;
    static inline const float SIZE = 80.0f;
    static const int PADDING = 10;
    static inline const float START_X = 50.f, START_Y = 50.f;

    struct Element {
        Rectangle pos;
        Turn turn;
        bool done;
        void reset (Rectangle pos1) {
            pos = pos1; 
            done = false;
            turn = FREE;
        }
    };


    static const int GRID_SIZE = 3;
    Turn currentTurn;
    int remaining = GRID_SIZE*GRID_SIZE;
    bool GAME_OVER = false;
    Turn WINNER = LEFT;
    Element elements[GRID_SIZE][GRID_SIZE];
    
    bool check (Turn turn, int i, int j);
    void checkWinner(int i, int j);
    void initElements() {
        for (int i = 0; i < GRID_SIZE; ++i) {
            for (int j = 0; j < GRID_SIZE; ++j) {

                elements[i][j].reset({START_X + SIZE*i + PADDING*i, 
                                            START_Y + SIZE*j + PADDING*j, SIZE, SIZE});        
            }
        }
    }
    int u_cnt;
    std::pair<int,int> undos[Board::GRID_SIZE*Board::GRID_SIZE];

    Texture2D xt;
    Texture2D ot;

    void reset() {
        u_cnt = -1;
        GAME_OVER = false;
        currentTurn = LEFT;
        initElements();
        remaining = GRID_SIZE*GRID_SIZE;
    }
    Board() {
        xt = LoadTexture("x.png"); 
        ot = LoadTexture("o.png"); 
        reset();
    }
    void undo () {
        if (u_cnt > -1) {
            ++remaining;
            currentTurn = currentTurn == Turn::LEFT ? Turn::RIGHT : Turn::LEFT;
            auto [li, lj] = undos[u_cnt--];
            elements[li][lj].reset({Board::START_X + Board::SIZE*li + Board::PADDING*li, 
                                    Board::START_Y + Board::SIZE*lj + Board::PADDING*lj, Board::SIZE, Board::SIZE});
        }
    }
    void update(int i, int j) {
        --remaining;
        elements[i][j].done = true;
        elements[i][j].turn = currentTurn;
        draw(i, j);
        checkWinner(i, j);
        currentTurn = currentTurn  == LEFT ? RIGHT : LEFT;
    }
    void draw(int i, int j) {
       draw(i, j, elements[i][j].turn);
    }
    void draw(int i, int j, Turn t) {
         int drawoffsetx = Board::START_X + Board::SIZE*i + Board::PADDING*i, 
                            drawoffsety = Board::START_Y + Board::SIZE*j + Board::PADDING*j;
                        
         DrawTexture(t == LEFT ? xt : ot, drawoffsetx, drawoffsety,  WHITE);
    }

    ~Board() {
        UnloadTexture(xt); 
        UnloadTexture(ot);
    }
};


bool Board::check (Turn turn, int i, int j) {
    if (i >= GRID_SIZE || i < 0) return false;
    if (j >= GRID_SIZE || j < 0) return false;
    return elements[i][j].turn == currentTurn;
}
// generic win checker for all grid sizes
void Board::checkWinner(int i, int j) {
    undos[++u_cnt] = {i,j};
    auto turn = currentTurn;
    if ( 
        //  at center, horizontal,vectical lookup
           (check(turn, i-1,j) && check(turn, i+1, j) )
        || (check(turn, i,j-1) && check(turn, i, j+1) )
        //  at center diagonal lookup
        || (check(turn, i-1,j-1) && check(turn, i+1, j+1) )
        || (check(turn, i-1,j+1) && check(turn, i+1, j-1) )
        // corner, horizontal lookup
        || (check(turn, i+1,j) && check(turn, i+2, j) )
        || (check(turn, i-1,j) && check(turn, i-2, j) )
        // corner, vertical lookup
        || (check(turn, i,j-1) && check(turn, i, j-2) )
        || (check(turn, i,j+1) && check(turn, i, j+2))
        // corner, primary diagnal
        || (check(turn, i+1,j+1) && check(turn, i+2, j+2))
        || (check(turn, i-1,j-1) && check(turn, i-2, j-2) )
        // corner, other diagonal
        || (check(turn, i+1,j-1) && check(turn, i+2, j-2) )
        || (check(turn, i-1,j+1) && check(turn, i-2, j+2) )
        ) {
            WINNER = turn;
            GAME_OVER = true;
        }
}


int main(void)
{    
    const int screenWidth = Board::START_X*2 + (Board::SIZE+Board::PADDING)*Board::GRID_SIZE;
    const int screenHeight = Board::START_Y*2 + (Board::SIZE+Board::PADDING)*Board::GRID_SIZE;

    InitWindow(screenWidth, screenHeight, "!!! Tic Tac Toe");

    {
    Board board;

    SetTargetFPS(60);

    bool undo_pressed = false;
    while (!WindowShouldClose())
    {
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        BeginDrawing();

        // Game logic
        ClearBackground(BLACK);
        if (IsKeyUp(KEY_Z) || IsKeyUp(KEY_LEFT_CONTROL))
            undo_pressed = false;

        if (board.GAME_OVER) {
            DrawText(board.WINNER == Board::LEFT ? "LEFT WON" : "RIGHT WON", screenWidth/2 - 50, screenHeight/2, 20, WHITE);
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
                board.reset();
            }
        } else if (board.remaining == 0) {
            DrawText("It's a DRAW", screenWidth/2 - 50, screenHeight/2, 20, WHITE);
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
                    board.reset();
            }
        } else if (!undo_pressed && IsKeyDown(KEY_Z) && IsKeyDown(KEY_LEFT_CONTROL)) {
            undo_pressed = true;
            board.undo();
            
        } else {
            DrawText(board.currentTurn == Board::LEFT ? "Turn: LEFT" : "Turn: RIGHT", 10, 10, 20, WHITE);
        
            for (int i = 0; i < Board::GRID_SIZE; ++i) {
                for (int j = 0; j < Board::GRID_SIZE; ++j) {
                    auto& b = board.elements[i][j];
                    
                    if (CheckCollisionPointRec(GetMousePosition(), b.pos)) {
                        Color c = LIGHTGRAY;
                        if (!b.done) {
                            if (board.currentTurn == Board::LEFT && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                                board.update(i, j);
                            } else if (board.currentTurn == Board::RIGHT && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
                                board.update(i, j);
                            } 
                            else { // draw temp overlay
                                board.draw(i, j, board.currentTurn);
                            }
                        } else {
                            board.draw(i, j);
                        }
                    } else {
                        if (b.done) {
                            board.draw(i, j);
                        } else {
                            DrawRectangleRounded(b.pos, Board::ROUNDEDNESS, 1, DARKGRAY);
                        }
                        
                    } 
                }
            }
        }

        EndDrawing();

    }
    }//Board ends
    
    CloseWindow();
    return 0;
}
