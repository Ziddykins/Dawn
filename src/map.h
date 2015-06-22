#ifndef MAP_H_INCLUDED
#define MAP_H_INCLUDED
struct Building {
    unsigned int x, y;
};

struct Map {
    char name[100];
    unsigned int max_x, max_y;
    unsigned int cur_x, cur_y;
    unsigned int exitx, exity;
    unsigned int min_level;
    struct Building shop, stable, shrine, gym;
    struct Building cshop, wepshop, armshop, bank;
};
#endif
