#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <iostream>
#include "common.hpp"
#include "board.hpp"
#include <queue>
#include <mutex>
#include <condition_variable>
using namespace std;


class Player {
private:
    mutex m_cv, m_best;
    condition_variable cv;

public:
    Player(Side side);
    ~Player();

    Move *doMove(Move *opponentsMove, int msLeft);
    Move *doNaiveMove(Move *opponentsMove);
    Move *AlphaBetaMove(Move *opponentsMove, int msLeft);
    Move *AlphaBetaMoveMultithread(Move *opponentsMove, int msleft);
    void AlphaBetaEvalThread(Move *possible_move,
        Board &b, int depth, double alpha, double beta, bool maximizing,
        int id, queue<int> &completed,
        double &best_score, Move *best_move);
    double AlphaBetaEval(Board &b, int depth, double alpha, double beta, bool maximizing);

    // Flag to tell if the player is running within the test_minimax context
    bool testingMinimax;
    int starting_depth;
    Side side;
    Board board;
};

#endif
