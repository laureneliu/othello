#include "player.hpp"
#include <iostream>
#include <limits>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

#define BOARDSIZE 8
#define NTHREADS 4
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
    if (testingMinimax)
    {
        starting_depth = 2;
    }
    else
    {
        starting_depth = 5;
    }

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
     return AlphaBetaMoveMultithread(opponentsMove, msLeft);
    
} 

Move *Player::doNaiveMove(Move *opponentsMove) {
    board.doMove(opponentsMove, (side == BLACK) ? WHITE : BLACK);
    Move *possible;
    Move *temp;
    int possible_score;
    Move *best = nullptr;
    int best_score;
    for (int i = 0; i < BOARDSIZE; ++i)
    {
        for (int j = 0; j < BOARDSIZE; ++j)
        {
            possible = new Move(i, j);
            if (board.checkMove(possible, side))
            {
                board.doMove(possible, side);
                possible_score = board.naiveScore(side);
                board.undoMove(possible);
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
    return best;
}

/**
 * TODO: fix it in the case where it wants to minimize?
 * @param opponentsMove
 * @param msleft
 * @return 
 */
Move *Player::AlphaBetaMove(Move *opponentsMove, int msleft)
{
    board.doMove(opponentsMove, (side == BLACK) ? WHITE : BLACK);
    Move *possible;
    Move *temp;
    double possible_score;
    Move *best = nullptr;
    double best_score;
    for (int i = 0; i < BOARDSIZE * BOARDSIZE; ++i)
    {
        possible = new Move(i / 8, i % 8);
        if (board.checkMove(possible, side))
        {
            board.doMove(possible, side);
            possible_score = AlphaBetaEval(board, starting_depth, 
                    std::numeric_limits<double>::lowest(), 
                    std::numeric_limits<double>::max(), false);

            board.undoMove(possible);
            if (best == nullptr)
            {
                best = possible;
                best_score = possible_score;
            }
            else if (possible_score >= best_score)
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
    board.doMove(best, side);
    return best;
}

Move *Player::AlphaBetaMoveMultithread(Move *opponentsMove, int msleft) {
    thread **t = new thread*[NTHREADS];
    for (int i = 0; i < NTHREADS; ++i)
    {
        t[i] = nullptr;
    }
    queue<int> completed;
    for (int i = 0; i < NTHREADS; ++i) {
        completed.push(i);
    }

    board.doMove(opponentsMove, (side == BLACK) ? WHITE : BLACK);
    Move *possible;
    Move *best_move = new Move(-1, -1);
    double best_score = numeric_limits<double>::lowest();
    for (int i = 0; i < BOARDSIZE * BOARDSIZE; ++i)
    {
        possible = new Move(i / 8, i % 8);
        if (board.checkMove(possible, side))
        {
            // Wait for and get index of free thread
            unique_lock<mutex> lk(m_cv);
            if (completed.empty()) {
                cv.wait(lk, [&completed]{return !completed.empty();});
            }
            int done = completed.front();
            completed.pop();
            lk.unlock();
            if (t[done] != nullptr) {
                t[done]->join();
                delete t[done];
            }
            t[done] = new thread(&Player::AlphaBetaEvalThread, this,
                           possible,
                           ref(board), starting_depth, std::numeric_limits<double>::lowest(), 
                           std::numeric_limits<double>::max(), false,
                           done, ref(completed),
                           ref(best_score), best_move);
        }
        else
        {
            delete possible;
        }
    }

    for (int i = 0; i < NTHREADS; ++i) {
        if (t[i] != nullptr) {
            t[i]->join();
            delete t[i];
        }
    }
    delete[] t;

    // delete everything
    if (best_move->getX() == -1)
    {
        return nullptr;
    }
    board.doMove(best_move, side);
    return best_move;
}

void Player::AlphaBetaEvalThread(Move *possible_move,
                            Board &b, int depth, double alpha, double beta, bool maximizing,
                            int id, queue<int> &completed,
                            double &best_score, Move *best_move) {
    // Prepare for and do AlphaBetaEval
    Board board_copy = *b.copy();
    board_copy.doMove(possible_move, side);
    double score = AlphaBetaEval(board_copy, depth, alpha, beta, maximizing);
    // Set best score
    m_best.lock();
    if (score >= best_score) {
        *best_move = *possible_move;
        delete possible_move;
        best_score = score;
    }
    else {
        delete possible_move;
    }
    m_best.unlock();
    
    // Notify parent thread that evaluation complete
    m_cv.lock();
    completed.push(id);
    m_cv.unlock();
    cv.notify_one();

    // All lock guards automatically deleted and mutexes unlocked
}

double Player::AlphaBetaEval(Board &b, int depth, double alpha, double beta, bool maximizing)
{
    if (b.isDone())
    {
        if (testingMinimax)
        {
            return b.naiveScore(side);
        }
        else if (b.naiveScore(side) > 0)
        {
            return std::numeric_limits<double>::max();
        }
        else
        {
            return std::numeric_limits<double>::lowest();
        }
    }
    else if (depth == 0)
    {
        if (testingMinimax)
        {
            return b.naiveScore(side);
        }
        return b.score(side);
    }
    double value = 0;
    double best_value;
    Move *possible;
    bool played = false;
    Side other = (side == BLACK) ? WHITE : BLACK;
    if (maximizing)
    {
        best_value = std::numeric_limits<double>::lowest();
        for (int i = 0; i < BOARDSIZE * BOARDSIZE; ++i)
        {
            possible = new Move(i / 8, i % 8);
            if (b.checkMove(possible, side))
            {
                played = true;
                b.doMove(possible, side);
                value = AlphaBetaEval(b, depth - 1, alpha, beta, !maximizing);
                best_value = max(best_value, value);
                alpha = max(alpha, best_value);
                b.undoMove(possible);
                if (beta < alpha)
                {
                    delete possible;
                    break;
                }
            }
            delete possible;
        }
        if (played)
        {
            return best_value;
        }
        else
        {
            return AlphaBetaEval(b, depth - 1, alpha, beta, !maximizing);
        }
    }
    else
    {
        best_value = std::numeric_limits<double>::max();
        for (int i = 0; i < BOARDSIZE * BOARDSIZE; ++i)
        {
            possible = new Move(i / 8, i % 8);
            if (b.checkMove(possible, other))
            {
                played = true;
                b.doMove(possible, other);
                value = AlphaBetaEval(b, depth - 1, alpha, beta, !maximizing);
                best_value = min(best_value, value);
                beta = min(beta, best_value);
                b.undoMove(possible);
                if (beta < alpha)
                {
                    delete possible;
                    break;
                }
            }
            delete possible;
        }
        if (played)
        {
            return best_value;
        }
        else
        {
            return AlphaBetaEval(b, depth - 1, alpha, beta, !maximizing);
        }
    }
}