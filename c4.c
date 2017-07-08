/* Connect 4 using Monte Carlo method.
 *
 * Copyright (c) 2017, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Return values: ivalid, empty circle, red or yellow player, or game draw. */
#define CIRCLE_INVALID -1
#define CIRCLE_EMPTY 0
#define CIRCLE_YELLOW 1
#define CIRCLE_RED 2
#define CIRCLE_DRAW 3

/* Our 6x7 board, it's composed of 7 columns (0 to 6 from left to right) and
 * 6 levels (0 to 5 from bottom to top)

 * [.][.][.][.][.][.][.] level 5
 * [.][.][.][.][.][.][.] level 4
 * [.][.][.][R][.][.][.] level 3
 * [.][.][.][R][.][.][.] level 2
 * [.][.][.][R][Y][.][.] level 1
 * [.][.][.][R][Y][.][.] level 0
 *  0  1  2  3  4  5  6
 */
typedef struct board {
    char holes[6*7];
} board;

/* Return what the specified col/level hole contains. */
int c4Get(board *b, int col, int level) {
    if (col < 0 || col > 6 || level < 0 || level > 5) return CIRCLE_INVALID;
    return b->holes[(5*7)+col-(level*7)];
}

/* Set the specified col/level hole to the specified circle value, see
*  the CIRCLE_* macros. */
void c4Set(board *b, int col, int level, int value) {
    if (col < 0 || col > 6 || level < 0 || level > 5) return;
    b->holes[(5*7)+col-(level*7)] = value;
}

/* Clean the board filling it with empty values. */
void c4Clean(board *b) {
    memset(b->holes,CIRCLE_EMPTY,sizeof(b->holes));
}

/* Copy the source board to the destination board. */
void c4Copy(board *dst, board *src) {
    memcpy(dst->holes,src->holes,sizeof(dst->holes));
}

/* Return true if the given colum is all full with circles. */
int c4ColIsFull(board *b, int col) {
    return c4Get(b,col,5) != CIRCLE_EMPTY;
}

/* Drop a circle in the give column. If it's full 0 is returned, otherwise
 * the first empty level is set to the specified value. */
int c4Drop(board *b, int col, int value) {
    if (c4ColIsFull(b,col)) return 0;
    for (int level = 0; level < 6; level++) {
        if (c4Get(b,col,level) == CIRCLE_EMPTY) {
            c4Set(b,col,level,value);
            break;
        }
    }
    return 1;
}

/* Print an ASCII art representation of the board. */
void c4Print(board *b) {
    for (int level = 5; level >= 0; level--) {
        for (int col = 0; col < 7; col++) {
            int color = c4Get(b,col,level);
            char *set = ".YR";
            printf("[%c]",set[color]);
        }
        printf("\n");
    }
    for (int col = 0; col < 7; col++)
        printf(" %d ",col);
    printf("\n");
}

/* Return the winner color, or CIRCLE_EMPTY if there is no winner.
 * If the board is full but there is no winner, CIRCLE_DRAW is returned. */
int c4GetWinner(board *b) {
    int empty = 0;
    for (int level = 5; level >= 0; level--) {
        for (int col = 0; col < 7; col++) {
            int color = c4Get(b,col,level);
            if (color == CIRCLE_EMPTY) {
                empty++;
                continue;
            }

            /* There are four possible valid directions to make
             * circles in a row: */
            struct {
                int col_incr;
                int level_incr;
            } dir[4] = {
                {1,0},
                {0,1},
                {1,1},
                {-1,1}
            };

            /* Check if from this position there is a winning sequence. */
            for (int d = 0; d < 4; d++) {
                /* Seek the initial circle still having this color. */
                int start_col = col;
                int start_level = level;
                while(c4Get(b,start_col-dir[d].col_incr,
                              start_level-dir[d].level_incr) == color)
                {
                    start_col -= dir[d].col_incr;
                    start_level -= dir[d].level_incr;
                }

                int count = 0; /* Count consecutive circles. */
                while(c4Get(b,start_col,start_level) == color) {
                    count++;
                    start_col += dir[d].col_incr;
                    start_level += dir[d].level_incr;
                }
                if (count >= 4) return color;
            }
        }
    }
    return empty ? CIRCLE_EMPTY : CIRCLE_DRAW;
}

/* Simulate a random game. The 'tomove' argument tells the function who
 * is the player to move first. The board does not need to be empty, the random
 * game is continued from the current state. The function returns the winner
 * color, or CIRCLE_DRAW if there is no winner. */
int c4RandomGame(board *b, int tomove) {
    while(1) {
        if (c4Drop(b,rand()%7,tomove)) {
            tomove = (tomove == CIRCLE_YELLOW) ? CIRCLE_RED : CIRCLE_YELLOW;
        }
        int winner = c4GetWinner(b);
        if (winner != CIRCLE_EMPTY) return winner;
    }
}

/* Suggest a move among the (at max) 7 possible. For each move we play random games
 * and store the ratio between won and lost games. Finally the move with the best
 * ratio is returned. If there is no valid moves, -1 is returned. */
int c4SuggestMove(board *b, int tomove) {
    int best = -1;
    double best_ratio = 0;
    int games_per_move = 10000;
    for (int move = 0; move < 7; move++) {
        if (c4ColIsFull(b,move)) continue; /* No valid move. */
        int won = 0, lost = 0;
        for (int j = 0; j < games_per_move; j++) {
            board copy;
            c4Copy(&copy,b);
            c4Drop(&copy,move,tomove);
            /* If this is a winning move there is nothign to search
             * further, just return it. */
            if (c4GetWinner(&copy) == tomove) return move;
            int next = (tomove == CIRCLE_YELLOW) ? CIRCLE_RED : CIRCLE_YELLOW;
            int winner = c4RandomGame(&copy,next);
            if (winner == CIRCLE_YELLOW || winner == CIRCLE_RED) {
                if (winner == tomove) {
                    won++;
                } else {
                    lost++;
                }
            }
        }
        double ratio = (double)won/(lost+1);
        printf("Move %d ratio: %f\n", move, ratio);
        if (ratio > best_ratio || best == -1) {
            best = move;
            best_ratio = ratio;
        }
    }
    return best;
}

/* Play interactively with the user. */
void c4play(void) {
    board b;
    c4Clean(&b);

    while(1) {
        c4Print(&b);
        if (c4GetWinner(&b) != CIRCLE_EMPTY) break;

        int human_move = -1;
        while(human_move == -1) {
            printf("Red, state your move: ");
            fflush(stdout);
            char buf[32];
            fgets(buf,sizeof(buf),stdin);
            human_move = atoi(buf);

            /* Check the validity. */
            if (human_move > 6) human_move = -1;
            if (human_move < 0) human_move = -1;
            if (human_move != -1 && c4ColIsFull(&b,human_move)) human_move = -1;
        }
        c4Drop(&b,human_move,CIRCLE_RED);

        int computer_move = c4SuggestMove(&b,CIRCLE_YELLOW);
        c4Drop(&b,computer_move,CIRCLE_YELLOW);
    }
}

int main(void) {
    srand(time(NULL));
    c4play();
}
