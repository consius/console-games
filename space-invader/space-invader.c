#include <stdio.h>
#include <ncurses.h>
#include <locale.h>
#include <unistd.h>

typedef struct {
    int x, y;
} vec;

int maxx, maxy;

int update() {
    return 0;
}

int render() {
    return 0;
}

int islost() {
    return 0;
}

int main() {
    setlocale(LC_ALL, "");
    WINDOW *win = initscr();
    cbreak();
    noecho();
    curs_set(0);
    timeout(0);
    getmaxyx(win, maxy, maxx);
    char input;
    vec craft, laser;
    vec alien[10][10]; // what is the size here?

    while(!islost()) {
        input = getch();
        while(input != -1) {
            // handle input here
            switch(input) {
                default: {} 
            }
            input = getch();
        }
        update();
        render();
        usleep(33333);
    }


    return 0;
}