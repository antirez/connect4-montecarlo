#include <stdio.h>
#include <string.h>

#define INVALID_CIRCLE -1
#define EMPTY_CIRCLE 0
#define YELLOW_CIRCLE 1
#define RED_CIRCLE 2

typedef struct board {
    char holes[6*7];
} board;

int c4Get(board *b, int col, int level) {
    if (col < 0 || col > 6 || level < 0 || level > 5) return INVALID_CIRCLE;
    return b->holes[(5*7)+col-(level*7)];
}

void c4Set(board *b, int col, int level, int value) {
    if (col < 0 || col > 6 || level < 0 || level > 5) return;
    b->holes[(5*7)+col-(level*7)] = value;
}

void c4Clean(board *b) {
    memset(b->holes,EMPTY_CIRCLE,sizeof(b->holes));
}

void c4Copy(board *dst, board *src) {
    memcpy(dst->holes,src->holes,sizeof(dst->holes));
}

int c4ColIsFull(board *b, int col) {
    return c4Get(b,col,5) != EMPTY_CIRCLE;
}

int c4Drop(board *b, int col, int value) {
    if (c4ColIsFull(b,col)) return 0;
    for (int level = 0; level < 6; level++) {
        if (c4Get(b,col,level) == EMPTY_CIRCLE) {
            c4Set(b,col,level,value);
            break;
        }
    }
    return 1;
}

void c4Print(board *b) {
    for (int level = 5; level >= 0; level--) {
        for (int col = 0; col < 7; col++) {
            int circle = c4Get(b,col,level);
            char *set = ".YR";
            printf("[%c]",set[circle]);
        }
        printf("\n");
    }
}

int main(void) {
    board b;
    c4Clean(&b);
    c4Drop(&b,3,RED_CIRCLE);
    c4Drop(&b,3,RED_CIRCLE);
    c4Drop(&b,4,YELLOW_CIRCLE);
    c4Print(&b);
}
