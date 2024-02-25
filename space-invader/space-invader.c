#include <stdio.h>
#include <ncurses.h>
#include <locale.h>
#include <unistd.h>
#include <stdlib.h>

#define LASER_VELOCITY 1
#define ACC_THRESH 30
#define NUM_ALIEN_X 10
#define NUM_ALIEN_Y 5
#define NUM_COVER 4
#define COVER_WIDTH 24
#define COVER_HEIGHT 10

typedef struct {
    int x, y, w, h;
    int glyph;
} character;

character craft = {
    .x = 4,
    .y = 3,
    .w = 15,
    .h = 4,
    .glyph = 0
};
int alienvelx = 2;
int accumulator = 0;
int pauseval = 2; // bad variable name?
int maxx, maxy;
character laser;
character alien[NUM_ALIEN_Y][NUM_ALIEN_X];
character alienLasers[2];
character cover[NUM_COVER];
int coverBlock[NUM_COVER][COVER_HEIGHT][COVER_WIDTH];

char* laserGlyphs[] = {"█", "█", "█"};//"┼", "ϟ"};

char* coverGlyphs[] = {" ", "▀", "▄", "█"};//"┼", "ϟ"};

char* glyphs[7][6] = {
    {
        &"       ▄\n",
        &"      ███\n",
        &"▄█████████████▄\n",
        &"███████████████\n",
        NULL,
    },
    {
        &"   ▀▄      ▄▀\n",
        &"█  ██▀████▀██  █\n",
        &"████████████████\n",
        &" ▀█▀▀▀▀▀▀▀▀▀▀█▀\n",
        &" ▀            ▀\n",
        NULL,
    },
    {
        &"   ▀▄      ▄▀\n",        
        &"  ▄██▀████▀██▄\n",
        &"████████████████\n",
        &"█  █▀▀▀▀▀▀▀▀█  █\n",
        &"    ▀▀    ▀▀\n",
        NULL,
    },
    {
        &"     ▄▄▄▄▄▄\n",
        &"▄██████████████▄\n",
        &"████▄▄████▄▄████\n",
        &"   ▄▀▀▀▄▄▀▀▀▄\n",
        &"    ▀      ▀\n",
        NULL,
    },
    {
        &"     ▄▄▄▄▄▄\n",
        &"▄██████████████▄\n",
        &"████▄▄████▄▄████\n",
        &"  ▄▄▀▀▀▄▄▀▀▀▄▄\n",
        &"▀▀            ▀▀\n",
        NULL,
    },
    {
        &"   ▄▄▄▄\n",
        &" ▄██████▄\n",
        &"███▄██▄███\n",
        &"  ▄▀▄▄▀▄\n",
        &" ▀ ▀  ▀ ▀\n",
        NULL,
    },
    {
        &"   ▄▄▄▄\n",
        &" ▄██████▄\n",
        &"███▄██▄███\n",
        &"  ▄▀  ▀▄\n",
        &"   ▀  ▀\n",
        NULL,
    }
};

int collisonPointRect(character point, character rect) {
    return (point.x >= rect.x &&
        point.y >= rect.y &&
        point.x < rect.x + rect.w &&
        point.y < rect.y + rect.h);
}

int craftMoveRight() {
    craft.x += 2;
    if(craft.x > maxx - craft.w) {
        craft.x = maxx - craft.w;
    }
    return 0;
}

int craftMoveLeft() {
    craft.x -= 2;
    if(craft.x < 0) {
        craft.x = 0;
    }
    return 0;
}

int noLaser(character laser) {
    return laser.x == -1;
}

int laserShoot() {
    if(noLaser(laser)) {
        laser.x = craft.x + 7;
        laser.y = craft.y + 1;
    }
    return 0;
}

int laserDestroy() {
    laser.x = -1;
    return 0;
}

int alienDead(character a) {
    return a.x == -1;
}

int laserCollison() {
    if (laser.y < 0) {
        laserDestroy();
        return 0;
    }
    for(int y = 0; y < NUM_ALIEN_Y; ++y) {
        for(int x = 0; x < NUM_ALIEN_X; ++x) {
            if(!alienDead(alien[y][x])) {
                if(collisonPointRect(laser, alien[y][x])) {
                    alien[y][x].x = -1;
                    laserDestroy();
                    return 0;
                }
            }
        }
    }
    return 0;
}

int alienLasersCollison() {
    for(int x = 0; x < 2; ++x) {
        if(alienLasers[x].y > maxy) {
            alienLasers[x].x = -1;
        }
    }
    return 0;
}

int alienMoveDown() {
    for(int y = 0; y < NUM_ALIEN_Y; ++y) {
        for(int x = 0; x < NUM_ALIEN_X; ++x) {
            alien[y][x].y += 2; // deal with magic nums TODO
        }
    }
    return 0;
}

int alienShootLaser(character* alienLaser) {
    for(int x = 0; x < NUM_ALIEN_X; ++x) {
        for(int y = NUM_ALIEN_Y - 1; y >= 0; --y) {
            if(!alienDead(alien[y][x])) {
                if(rand() % 10 == 0) {
                    alienLaser -> x = alien[y][x].x + alien[y][x].w / 2; 
                    alienLaser -> y = alien[y][x].y + alien[y][x].h; 
                    alienLaser -> glyph = rand() % 2 ? 1 : 2;
                    return 0;
                }
                break;
            }
        }
    }
    return 0;
}

int coverHaveIndex(int y, int x, character cover) {
    return (x >= 0 && 
            y >= 0 &&
            x < cover.w &&
            y < cover.h);
}

int destroyCoverElement(int y, int x, character cover) {
    if(coverHaveIndex(y, x, cover)) {
        coverBlock[cover.glyph][y][x] = 0;
    }
    return 0;
}

int collideCover(character laser) {
    for(int n = 0; n < NUM_COVER; ++n) {
        if(collisonPointRect(laser, cover[n])) {
            int ofsetx = abs(laser.x - cover[n].x);
            int ofsety = abs(laser.y - cover[n].y);
            if(coverBlock[n][ofsety][ofsetx]) {
                for(int y = -2; y <= 2; ++y) {
                    for(int x = -2; x <= 2; ++x) {
                        if(y == -2 ||
                           x == -2 ||
                           y == 2 ||
                           x == 2) {
                            if(rand()%2 == 0) {
                                destroyCoverElement(ofsety+y, ofsetx+x, cover[n]);
                            }
                        } else {
                            destroyCoverElement(ofsety+y, ofsetx+x, cover[n]);
                        }
                    }
                }
                return 1;
            }
        }
    }
    return 0;
}

int update() {
    int moveDown = false;
    laserCollison();
    alienLasersCollison();
    //check collison with aliens

    //move aliens.
    accumulator += pauseval;
    if(accumulator >= ACC_THRESH) {
        for(int y = 0; y < NUM_ALIEN_Y; ++y) {
            for(int x = 0; x < NUM_ALIEN_X; ++x) {
                if((alien[y][x].x >= maxx - alien[y][x].w - 2 
                    && alienvelx == 2)
                    || (alien[y][x].x <= 2
                    && alienvelx == -2)
                    && !alienDead(alien[y][x])) {
                    moveDown = true;
                }
            }
        }

        if(moveDown) {
            alienvelx = alienvelx == 2 ? -2 : 2;
            alienMoveDown();
        } else {
            for(int y = 0; y < NUM_ALIEN_Y; ++y) {
                for(int x = 0; x < NUM_ALIEN_X; ++x) {
                    if(!alienDead(alien[y][x])) {
                        alien[y][x].x += alienvelx;
                    }
                }
            }
        }

        for(int y = 0; y < NUM_ALIEN_Y; ++y) {
            for(int x = 0; x < NUM_ALIEN_X; ++x) {
                alien[y][x].glyph += alien[y][x].glyph % 2 ? 1 : -1;
            }
        }
    accumulator = 0;
    }
        
    if(!noLaser(laser)) {
        laser.y -= LASER_VELOCITY;
    }
    for(int x = 0; x < 2; x++) {
        if(noLaser(alienLasers[x])) {
            alienShootLaser(&alienLasers[x]);
        } else {
            alienLasers[x].y += 1;
        }
    }
    
    if(collideCover(laser)) {
        laser.x = -1;
    }
    
    for(int x = 0; x < NUM_COVER; x++) {
        if(collideCover(alienLasers[x])) {
            alienLasers[x].x = -1;
        }
    }
    return 0;
}

int renderCover(character cover) {
    for(int y = 0; y < cover.h; y += 2) {
        for(int x = 0; x < cover.w; ++x) {
            int glyph = coverBlock[cover.glyph][y][x] + 
                        coverBlock[cover.glyph][y+1][x] * 2;
            mvprintw(cover.y + y/2, cover.x + x, "%s", coverGlyphs[glyph]);
        }
    }
    return 0;
}

int renderglyph(character glyph) {
    for(int n = 0; glyphs[glyph.glyph][n] != NULL; ++n) {
        mvprintw(glyph.y+n, glyph.x, "%s", glyphs[glyph.glyph][n]);
    }
    return 0;
}
    
int render() {
    clear();
    renderglyph(craft);
    for(int y = 0; y < NUM_ALIEN_Y; ++y) {
        for(int x = 0; x < NUM_ALIEN_X; ++x) {
            if(!alienDead(alien[y][x])) {
                renderglyph(alien[y][x]);
            }
        }
    }

    for(int n = 0; n < NUM_COVER; ++n) {
        renderCover(cover[n]);
    }

    if(!noLaser(laser)) {
        mvprintw(laser.y, laser.x, "█");
    }
    for(int x = 0; x < 2; x++) {
        if(!noLaser(alienLasers[x])) {
            mvprintw(alienLasers[x].y, alienLasers[x].x, "%s", laserGlyphs[alienLasers[x].glyph]);
        }
    }

    return 0;
}

int islost() {
    return 0;
}

int init() {
    srand(69);
    int lineGlyph[5] = {5, 1, 1, 3, 3};
    craft.x = maxx / 2;
    craft.y = maxy - 4;
    laser.x = -1; 
    //create alien
    for(int y = 0; y < NUM_ALIEN_Y; ++y) {
        for(int x = 0; x < NUM_ALIEN_X; ++x) {
            if(lineGlyph[y] == 5) {
                alien[y][x].x = 2 + 19 * x + 3;
                alien[y][x].y = 2 + 6 * y;
                alien[y][x].w = 10;
                alien[y][x].h = 5;
            } else {
                alien[y][x].x = 2 + 19 * x;
                alien[y][x].y = 2 + 6 * y;
                alien[y][x].w = 16;
                alien[y][x].h = 5;
            }
            alien[y][x].glyph = lineGlyph[y];
        }
    }
    for(int n = 0; n < 2; n++) {
        alienLasers[n].x = -1;
    }
    for(int n = 0; n < NUM_COVER; ++n) {
        cover[n].x = (maxx/5) * (n+1) - (COVER_WIDTH/2);
        cover[n].y = maxy - COVER_HEIGHT/2 - 6;
        cover[n].w = COVER_WIDTH;
        cover[n].h = COVER_HEIGHT;
        cover[n].glyph = n;
        for(int y = 0; y < COVER_HEIGHT; ++y) {
            for(int x = 0; x < COVER_WIDTH; ++x) {
                coverBlock[n][y][x] = 1;
            }
        }
    }
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
    init();
    char input;

    if(maxx < 250 || maxy < 50) {
        endwin();
        printf("Terminal too small :(\n");
        return 0;
    }

    while(!islost()) {
        input = getch();
        while(input != -1) {
            // handle input here
            switch(input) {
                case 'a':
                case 'h':
                case 'r':
                    craftMoveLeft();
                    break;

                case 'd':
                case 'l':
                case 't':
                    craftMoveRight();
                    break;

                case ' ':
                    laserShoot();
                
                default: {} 
            }
            input = getch();
        }
        update();
        render();
        usleep(33333);
    }
    endwin();
    return 0;
}
