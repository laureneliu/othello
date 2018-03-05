#include "board.hpp"
#include <iostream>
#define BOARDSIZE 8
const int loc_scores[64] = {20, -3, 11, 8, 8, 11, -3, 20, 
                          -3, -7, -4, 1, 1, -4, -7, -3, 
                          11, -4, 2, 2, 2, 2, -4, 11,
                           8, 1, 2, -3, -3, 2, 1, 8,
                           8, 1, 2, -3, -3, 2, 1, 8, 
                          11, -4, 2, 2, 2, 2, -4, 11, 
                          -3, -7, -4, 1, 1, -4, -7, -3, 
                          20, -3, 11, 8, 8, 11, -3, 20};
/*
 * Make a standard 8x8 othello board and initialize it to the standard setup.
 */
Board::Board() {
    taken.set(3 + 8 * 3);
    taken.set(3 + 8 * 4);
    taken.set(4 + 8 * 3);
    taken.set(4 + 8 * 4);
    black.set(4 + 8 * 3);
    black.set(3 + 8 * 4);
}

/*
 * Destructor for the board.
 */
Board::~Board() {
}

/*
 * Returns a copy of this board.
 */
Board *Board::copy() {
    Board *newBoard = new Board();
    newBoard->black = black;
    newBoard->taken = taken;
    return newBoard;
}

bool Board::occupied(int x, int y) {
    return taken[x + 8*y];
}

bool Board::get(Side side, int x, int y) {
    return occupied(x, y) && (black[x + 8*y] == (side == BLACK));
}

void Board::set(Side side, int x, int y) {
    taken.set(x + 8*y);
    black.set(x + 8*y, side == BLACK);
}

bool Board::onBoard(int x, int y) {
    return(0 <= x && x < 8 && 0 <= y && y < 8);
}


/*
 * Returns true if the game is finished; false otherwise. The game is finished
 * if neither side has a legal move.
 */
bool Board::isDone() {
    return !(hasMoves(BLACK) || hasMoves(WHITE));
}

/*
 * Returns true if there are legal moves for the given side.
 */
bool Board::hasMoves(Side side) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            Move move(i, j);
            if (checkMove(&move, side)) return true;
        }
    }
    return false;
}

/*
 * Returns true if a move is legal for the given side; false otherwise.
 */
bool Board::checkMove(Move *m, Side side) {
    // Passing is only legal if you have no moves.
    if (m == nullptr) return !hasMoves(side);

    int X = m->getX();
    int Y = m->getY();

    // Make sure the square hasn't already been taken.
    if (occupied(X, Y)) return false;

    Side other = (side == BLACK) ? WHITE : BLACK;
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (dy == 0 && dx == 0) continue;

            // Is there a capture in that direction?
            int x = X + dx;
            int y = Y + dy;
            if (onBoard(x, y) && get(other, x, y)) {
                do {
                    x += dx;
                    y += dy;
                } while (onBoard(x, y) && get(other, x, y));

                if (onBoard(x, y) && get(side, x, y)) return true;
            }
        }
    }
    return false;
}

/*
 * Modifies the board to reflect the specified move.
 */
void Board::doMove(Move *m, Side side) {
    // A nullptr move means pass.
    if (m == nullptr) return;

    // Ignore if move is invalid.
    if (!checkMove(m, side)) return;

    int X = m->getX();
    int Y = m->getY();
    Side other = (side == BLACK) ? WHITE : BLACK;
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (dy == 0 && dx == 0) continue;

            int x = X;
            int y = Y;
            do {
                x += dx;
                y += dy;
            } while (onBoard(x, y) && get(other, x, y));

            if (onBoard(x, y) && get(side, x, y)) {
                x = X;
                y = Y;
                x += dx;
                y += dy;
                while (onBoard(x, y) && get(other, x, y)) {
                    set(side, x, y);
                    m->flipped[m->num_flipped] = x + 8 * y;
                    m->num_flipped += 1;
                    x += dx;
                    y += dy;
                }
            }
        }
    }
    set(side, X, Y);
}

void Board::undoMove(Move *m) {
    taken.set(m->getX() + m->getY() * 8, 0);
    black.set(m->getX() + m->getY() * 8, 0);
    for (int i = 0; i < m->num_flipped; ++i) {
        black.flip(m->flipped[i]);
    }
}

/*
 * Current count of given side's stones.
 */
int Board::count(Side side) {
    return (side == BLACK) ? countBlack() : countWhite();
}

/*
 * Current count of black stones.
 */
int Board::countBlack() {
    return black.count();
}

/*
 * Current count of white stones.
 */
int Board::countWhite() {
    return taken.count() - black.count();
}

/*
 * Sets the board state given an 8x8 char array where 'w' indicates a white
 * piece and 'b' indicates a black piece. Mainly for testing purposes.
 */
void Board::setBoard(char data[]) {
    taken.reset();
    black.reset();
    for (int i = 0; i < 64; i++) {
        if (data[i] == 'b') {
            taken.set(i);
            black.set(i);
        } if (data[i] == 'w') {
            taken.set(i);
        }
    }
}

int Board::naiveScore(Side side)
{
    return count(side) - count((side == BLACK) ? WHITE : BLACK);
}
/**
 * returns score of board. Black maximizes and white minimizes
 * TODO
 */
double Board::score(Side side)
{
    Move* possible;
    double white_count = 0;
    double black_count = 0;
    for (int i = 0; i < BOARDSIZE; ++i)
    {
        for (int j = 0; j < BOARDSIZE; ++j)
        {
            possible = new Move(i, j);
            if (checkMove(possible, BLACK))
            {
                ++black_count;
            }
            if (checkMove(possible, WHITE))
            {
                ++white_count;
            }
            delete possible;
        }
    }
    double move_diff_val;
    if (black_count + white_count != 0)
    {
        if (side == BLACK)
        {
            move_diff_val = 100 * (black_count - white_count) 
                                   / (black_count + white_count);
        }
        else
        {  
            move_diff_val = 100 * (white_count - black_count) 
                                   / (black_count + white_count);
        }
    }
    else
    {
        move_diff_val = 0;
    }
    
    double white_move_score = 0;
    double black_move_score = 0;
    for (int i = 0; i < BOARDSIZE * BOARDSIZE; ++i)
    {
        if (taken[i])
        {
            if (black[i])
            {
                black_move_score += loc_scores[i];
            }
            else
            {
                white_move_score += loc_scores[i];
            }
        }
    }
    
    double mob_diff_val;
    if (black_move_score + white_move_score != 0)
    {
        if (side == BLACK)
        {
            mob_diff_val = 100 * (black_move_score - white_move_score) 
                                      / (black_move_score + white_move_score);
        }
        else
        {
            mob_diff_val = 100 * (white_move_score - black_move_score) 
                                      / (black_move_score + white_move_score);
        }
    }
    else
    {
        mob_diff_val = 0;
    }
    
    double piece_diff_val;
    if (side == BLACK)
    {
        piece_diff_val = 100 * (double) (count(BLACK) - count(WHITE))
                                    / (double) (count(BLACK) + count(WHITE));
    }
    else
    {
        piece_diff_val = 100 * (double) (count(WHITE) - count(BLACK))
                                    / (double) (count(BLACK) + count(WHITE));
    }
    
    double black_corners = 0;
    double white_corners = 0;
    double black_corner_closeness = 0;
    double white_corner_closeness = 0;
    if(taken[0])
    {
        if (black[0])
        {
            ++black_corners;
        }
        else
        {
            ++white_corners;
        }
    }
    else
    {
        if (taken[1])
        {
            if (black[1]) ++black_corner_closeness;
            else ++white_corner_closeness;
        }
        if (taken[8])
        {
            if (black[8]) ++black_corner_closeness;
            else ++white_corner_closeness;
        }
        if (taken[9])
        {
            if (black[9]) ++black_corner_closeness;
            else ++white_corner_closeness;
        }
    }
    if(taken[7])
    {
        if (black[7])
        {
            ++black_corners;
        }
        else
        {
            ++white_corners;
        }
    }
    else
    {
        if (taken[6])
        {
            if (black[6]) ++black_corner_closeness;
            else ++white_corner_closeness;
        }
        if (taken[14])
        {
            if (black[14]) ++black_corner_closeness;
            else ++white_corner_closeness;
        }
        if (taken[15])
        {
            if (black[15]) ++black_corner_closeness;
            else ++white_corner_closeness;
        }
    }
    if(taken[56])
    {
        if (black[56])
        {
            ++black_corners;
        }
        else
        {
            ++white_corners;
        }
    }
    else
    {
        if (taken[48])
        {
            if (black[48]) ++black_corner_closeness;
            else ++white_corner_closeness;
        }
        if (taken[49])
        {
            if (black[49]) ++black_corner_closeness;
            else ++white_corner_closeness;
        }
        if (taken[57])
        {
            if (black[57]) ++black_corner_closeness;
            else ++white_corner_closeness;
        }
    }
    if(taken[63])
    {
        if (black[63])
        {
            ++black_corners;
        }
        else
        {
            ++white_corners;
        }
    }
    else
    {
        if (taken[54])
        {
            if (black[54]) ++black_corner_closeness;
            else ++white_corner_closeness;
        }
        if (taken[55])
        {
            if (black[55]) ++black_corner_closeness;
            else ++white_corner_closeness;
        }
        if (taken[62])
        {
            if (black[62]) ++black_corner_closeness;
            else ++white_corner_closeness;
        }
    }
    double cc_val;
    if (white_corner_closeness + black_corner_closeness != 0)
    {
        if (side == BLACK)
        {
            cc_val = 100 * (white_corner_closeness - black_corner_closeness)
                         / (black_corner_closeness + white_corner_closeness);
        }
        else
        {
            cc_val = 100 * (black_corner_closeness - white_corner_closeness)
                         / (black_corner_closeness + white_corner_closeness);
        }
    }
    else
    {
        cc_val = 0;
    }
    
    double corner_diff_val;
    if (black_corners + white_corners != 0)
    {
        if (side == BLACK)
        {
            corner_diff_val = 100 * (black_corners - white_corners) 
                                         / (black_corners + white_corners);
        }
        else
        {
            corner_diff_val = 100 * (white_corners - black_corners) 
                                         / (black_corners + white_corners);
        }
    }
    else
    {
        corner_diff_val = 0;
    }
    
    return piece_diff_val / 10 + 
           (mob_diff_val + move_diff_val) + 
           2 * cc_val + 10 * corner_diff_val;
}

/**
 * prints the current state of the board for debugging purposes
 */
void Board::print()
{
    std::cerr << std::endl;
    for (int i = 0; i < 8; ++i)
    {
        for (int j = 0; j < 8; ++j)
        {
            if (get(BLACK, j, i))
                std::cerr << 'b';
            else if (get(WHITE, j, i))
                std::cerr << 'w';
            else
                std::cerr << '_';
        }
        std::cerr << std::endl;
    }
}

char *Board::toString()
{
    char *rep = new char[129];
    for (int i = 0; i < 64; ++i)
    {
        rep[i] = taken[i] + '0';
    }
    for (int i = 64; i < 128; ++i)
    {
        rep[i] = black[i - 64] + '0';
    }
    rep[128] = '\0';
    return rep;
}