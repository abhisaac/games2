
#include "raylib.h"
#include <vector>
#include <iostream>
#include <stack>

// TODO: fix game logic, buggy win condition
// remove globals fix code
// add undo condition

Color TurnColors[] = {
    DARKGRAY, RED, GREEN
};

enum Turn {
    FREE, LEFT, RIGHT
};

struct Element {
    Rectangle pos;
    Turn turn;
    bool done;
};

// board elements
const float ROUNDEDNESS = 0.4f;
const int SIZE = 80;
const int PADDING = 10;
const float START_X = 50.f, START_Y = 50.f;

struct Board  {
    static const int GRID_SIZE = 3;
    Turn currentTurn;
    int remaining = GRID_SIZE*GRID_SIZE;
    bool GAME_OVER = false;
    Turn WINNER = LEFT;
    Element elements[GRID_SIZE][GRID_SIZE];
    
    bool check (Turn turn, int i, int j);
    void checkWinner(Turn turn, int i, int j);
    void initElements() {
        for (int i = 0; i < GRID_SIZE; ++i) {
            for (int j = 0; j < GRID_SIZE; ++j) {
                elements[i][j].pos = {START_X + SIZE*i + PADDING*i , 
                                            START_Y + SIZE*j + PADDING*j, SIZE, SIZE};        
                elements[i][j].done = false;
                elements[i][j].turn = FREE;
            }
        }
    }
    void reset() {
        GAME_OVER = false;
        currentTurn = LEFT;
        initElements();
        remaining = GRID_SIZE*GRID_SIZE;
    }
    Board() {
        reset();
    }
    
};


bool Board::check (Turn turn, int i, int j) {
    if (i >= GRID_SIZE || i < 0) return false;
    if (j >= GRID_SIZE || j < 0) return false;
    return elements[i][j].turn == currentTurn;
}
// generic win checker for all grid sizes
void Board::checkWinner(Turn turn, int i, int j) {
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

    const int screenWidth = START_X*2 + (SIZE+PADDING)*Board::GRID_SIZE;
    const int screenHeight = START_Y*2 + (SIZE+PADDING)*Board::GRID_SIZE;

    InitWindow(screenWidth, screenHeight, "!!! Tic Tac Toe");

    SetTargetFPS(60);

    Texture2D xt = LoadTexture("x.png");   
    Texture2D ot = LoadTexture("o.png");   

    while (!WindowShouldClose())
    {
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        BeginDrawing();

            ClearBackground(BLACK);
            
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
            } else if (IsKeyDown(KEY_Z) && IsKeyDown(KEY_LEFT_CONTROL)) {
                // std::copy(board, board + GRID_SIZE*GRID_SIZE, undostack.top());
                // undostack.pop();
            } else {
                DrawText(board.currentTurn == LEFT ? "Turn: LEFT" : "Turn: RIGHT", 10, 10, 20, WHITE);
            
                for (int i = 0; i < Board::GRID_SIZE; ++i) {
                    for (int j = 0; j < Board::GRID_SIZE; ++j) {
                        auto& b = board.elements[i][j];
                        int drawoffsetx = START_X + SIZE*i + PADDING*i, drawoffsety = START_Y + SIZE*j + PADDING*j;
                        if (CheckCollisionPointRec(GetMousePosition(), b.pos)) {
                            Color c = LIGHTGRAY;
                            if (!b.done) {
                                if (board.currentTurn == LEFT && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                                    --board.remaining;
                                    board.currentTurn = RIGHT;
                                    b.done = true;
                                    b.turn = LEFT;
                                    // undostack.push(board);
                                    DrawTexture(xt, drawoffsetx, drawoffsety,  WHITE);
                                    board.checkWinner(LEFT, i, j);
                                } else if (board.currentTurn == RIGHT && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
                                    --board.remaining;
                                    board.currentTurn = LEFT;
                                    b.done = true;
                                    b.turn = RIGHT;
                                    // undostack.push(board);
                                    DrawTexture(ot, drawoffsetx, drawoffsety,  WHITE);
                                    board.checkWinner(RIGHT, i, j);
                                } else {
                                    // DrawRectangleRounded(b.pos, ROUNDEDNESS, 1, GREEN);
                                    DrawTexture(board.currentTurn == LEFT ? xt : ot, drawoffsetx, drawoffsety,  WHITE);
                                }
                            } else {
                                DrawTexture(b.turn == LEFT ? xt : ot, drawoffsetx, drawoffsety,  WHITE);
                            }

                                
                        } else {
                            if (b.done) {
                                DrawTexture(b.turn == LEFT ? xt : ot, drawoffsetx, drawoffsety,  WHITE);
                            } else {
                                DrawRectangleRounded(b.pos, ROUNDEDNESS, 1, TurnColors[b.turn]);
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
