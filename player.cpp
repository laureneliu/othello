#include "player.hpp"
#include <iostream>

#define BOARDSIZE 8

/*
 * Constructor for the player; initialize everything here. The side your AI is
 * on (BLACK or WHITE) is passed in as "side". The constructor must finish
 * within 30 seconds.
 */
 

Player::Player(Side temp) {
    // Will be set to true in test_minimax.cpp.
    testingMinimax = false;
    side = temp;
    board = Board();

    /*
     * TODO: Do any initialization you need to do here (setting up the board,
     * precalculating things, etc.) However, remember that you will only have
     * 30 seconds
     */
}

/*
 * Destructor for the player
 */
Player::~Player() {
}

/*
 * Compute the next move given the opponent's last move. Your AI is
 * expected to keep track of the board on its own. If this is the first move,
 * or if the opponent passed on the last move, then opponentsMove will be
 * nullptr.
 *
 * msLeft represents the time your AI has left for the total game, in
 * milliseconds. doMove() must take no longer than msLeft, or your AI will
 * be disqualified! An msLeft value of -1 indicates no time limit.
 *
 * The move returned must be legal; if there are no valid moves for your side,
 * return nullptr.
 */
Move *Player::doMove(Move *opponentsMove, int msLeft) {
    /*
     * TODO: Implement how moves your AI should play here. You should first
     * process the opponent's opponents move before calculating your own move
     */
     return doNaiveMove(opponentsMove);
    
} 

Move *Player::doNaiveMove(Move *opponentsMove) {
    board.doMove(opponentsMove, (side == BLACK) ? WHITE : BLACK);
    std::cerr << "we did the opps move" << std::endl;
    Move *possible;
    Move *temp;
    int possible_score;
    Move *best = nullptr;
    int best_score;
    for (int i = 0; i < BOARDSIZE; ++i)
    {
        for (int j = 0; j < BOARDSIZE; ++j)
        {
            std::cerr << -1;
            possible = new Move(i, j);
            std::cerr << 0;
            if (board.checkMove(possible, side))
            {
                std::cerr << possible->getX() << ", " << possible->getY() << std::endl;
                // board.print(); 
                std::cerr << 1;
                board.doMove(possible, side);
                // board.print();
                std::cerr << 2;
                possible_score = board.naiveScore(side);
                std::cerr << 3;
                board.undoMove(possible);
                std::cerr << 4;
                // board.print();
                if (best == nullptr)
                {
                    best = possible;
                    best_score = possible_score;
                }
                else if (possible_score > best_score)
                {
                    temp = best;
                    best = possible;
                    best_score = possible_score;
                    delete temp;
                }
                else
                {
                    delete possible;
                }
            }
            else
            {
                delete possible;
            }
        }
    }
    board.doMove(best, side);
    board.print();
    return best;
}