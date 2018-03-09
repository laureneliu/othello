#ifndef __COMMON_H__
#define __COMMON_H__

enum Side { 
    WHITE, BLACK
};

class Move {
   
public:
    int x, y;
    double score;
    Move(int x, int y) {
        this->x = x;
        this->y = y; 

    }
    ~Move() {

    }

    int getX() { return x; }
    int getY() { return y; }

    void setX(int x) { this->x = x; }
    void setY(int y) { this->y = y; }
    
};


template <typename T> bool MoveComp(const T * const & a, const T * const & b)
{
   return a->score > b->score;
}


template <typename T> bool MoveCompRev(const T * const & a, const T * const & b)
{
   return a->score < b->score;
}

#endif
