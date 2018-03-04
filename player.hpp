#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <iostream>
#include "common.hpp"
#include "board.hpp"
using namespace std;


class Player {

public:
    Player(Side side);
    ~Player();

    Move *doMove(Move *opponentsMove, int msLeft);
    Move *doNaiveMove(Move *opponentsMove);
    Move *AlphaBetaMove(Move *opponentsMove, int msLeft);
    double AlphaBetaEval(int depth, double alpha, double beta, bool maximizing);

    // Flag to tell if the player is running within the test_minimax context
    bool testingMinimax;
    int starting_depth;
    Side side;
    Board board;
};

#endif
