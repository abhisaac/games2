
#include "raylib.h"
#include <vector>
#include <iostream>
#include <stack>

// TODO: 
// remove globals fix code
// draw strike through after WIn

enum Turn {
    FREE, LEFT, RIGHT
};


struct Board  {
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
    void reset() {
        u_cnt = -1;
        GAME_OVER = false;
        currentTurn = LEFT;
        initElements();
        remaining = GRID_SIZE*GRID_SIZE;
    }
    Board() {
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
    
    
    Board board;

    const int screenWidth = Board::START_X*2 + (Board::SIZE+Board::PADDING)*Board::GRID_SIZE;
    const int screenHeight = Board::START_Y*2 + (Board::SIZE+Board::PADDING)*Board::GRID_SIZE;

    InitWindow(screenWidth, screenHeight, "!!! Tic Tac Toe");

    SetTargetFPS(60);

    Texture2D xt = LoadTexture("x.png");   
    Texture2D ot = LoadTexture("o.png");   
    bool undo_pressed = false;
    while (!WindowShouldClose())
    {
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        BeginDrawing();

            ClearBackground(BLACK);
            if (IsKeyUp(KEY_Z) || IsKeyUp(KEY_LEFT_CONTROL))
                undo_pressed = false;

            if (board.GAME_OVER) {
                DrawText(board.WINNER == LEFT ? "LEFT WON" : "RIGHT WON", screenWidth/2 - 50, screenHeight/2, 20, WHITE);
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
                DrawText(board.currentTurn == LEFT ? "Turn: LEFT" : "Turn: RIGHT", 10, 10, 20, WHITE);
            
                for (int i = 0; i < Board::GRID_SIZE; ++i) {
                    for (int j = 0; j < Board::GRID_SIZE; ++j) {
                        auto& b = board.elements[i][j];

                        int drawoffsetx = Board::START_X + Board::SIZE*i + Board::PADDING*i, 
                            drawoffsety = Board::START_Y + Board::SIZE*j + Board::PADDING*j;
                        
                        if (CheckCollisionPointRec(GetMousePosition(), b.pos)) {
                            Color c = LIGHTGRAY;
                            if (!b.done) {
                                if (board.currentTurn == LEFT && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                                    --board.remaining;
                                    
                                    b.done = true;
                                    b.turn = LEFT;
                                    
                                    DrawTexture(xt, drawoffsetx, drawoffsety,  WHITE);
                                    board.checkWinner(i, j);
                                    board.currentTurn = RIGHT;
                                } else if (board.currentTurn == RIGHT && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
                                    --board.remaining;
                                    
                                    b.done = true;
                                    b.turn = RIGHT;
                                    
                                    DrawTexture(ot, drawoffsetx, drawoffsety,  WHITE);
                                    board.checkWinner(i, j);
                                    board.currentTurn = LEFT;
                                } else {
                                    DrawTexture(board.currentTurn == LEFT ? xt : ot, drawoffsetx, drawoffsety,  WHITE);
                                }
                            } else {
                                DrawTexture(b.turn == LEFT ? xt : ot, drawoffsetx, drawoffsety,  WHITE);
                            }

                                
                        } else {
                            if (b.done) {
                                DrawTexture(b.turn == LEFT ? xt : ot, drawoffsetx, drawoffsety,  WHITE);
                            } else {
                                DrawRectangleRounded(b.pos, Board::ROUNDEDNESS, 1, DARKGRAY);
                            }
                            
                        } 
                    }
                }
            }
        EndDrawing();

    }
    UnloadTexture(xt); 
    UnloadTexture(ot);
    CloseWindow();
    return 0;
}
