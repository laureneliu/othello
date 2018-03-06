#ifndef __COMMON_H__
#define __COMMON_H__

enum Side { 
    WHITE, BLACK
};

class Move {
   
public:
    int x, y;
    int flipped[21];
    int num_flipped;
    Move(int x, int y) {
        this->x = x;
        this->y = y; 
        this->num_flipped = 0;

    }
    ~Move() {

    }

    int getX() { return x; }
    int getY() { return y; }

    void setX(int x) { this->x = x; }
    void setY(int y) { this->y = y; }
};

#endif
