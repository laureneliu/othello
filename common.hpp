#ifndef __COMMON_H__
#define __COMMON_H__

enum Side { 
    WHITE, BLACK
};

class Move {
   
public:
    int x, y;
    int *flipped;
    int num_flipped;
    Move(int x, int y) {
        this->x = x;
        this->y = y;
        this->num_flipped = 0;
        this->flipped = new int[100];
    }
    ~Move() {
        delete[] flipped;
    }

    int getX() { return x; }
    int getY() { return y; }

    void setX(int x) { this->x = x; }
    void setY(int y) { this->y = y; }
};

#endif
