#include "player.hpp"
#include <iostream>
#include <limits>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <algorithm> 
#include <vector>
#include <chrono>
#include <fstream>


#define BOARDSIZE 8
#define NTHREADS 6
#define MAX_TRANSPOSITION_SIZE 20000
#define OPENING false


    
typedef std::chrono::high_resolution_clock Time;
typedef std::chrono::milliseconds ms;
typedef std::chrono::duration<float> fsec;

/*
 * Constructor for the player; initialize everything here. The side your AI is
 * on (BLACK or WHITE) is passed in as "side". The constructor must finish
 * within 30 seconds.
 * 
 * TODO:
 * generate moves and sort do depth 2
 * prune half of the bad moves and go to lower depth
 */
 

Player::Player(Side temp) {
    // Will be set to true in test_minimax.cpp.
    testingMinimax = false;
    side = temp;
    board = Board();
    cache = new DLlist;
    cache->start = nullptr;
    cache->end = nullptr;
    use_obook = true;
    
    if (!OPENING)
    {
        return;
    }
    if (side == BLACK)
    {
        string board;
        int x;
        ifstream myfile ("Team 436_BLACK_OPENING_BOOK.txt");
        while (!myfile.eof())
        {
            myfile >> board >> x;
            opening_book.insert({board, x});
        }
    }
    else
    {
        string board;
        int x;
        ifstream myfile ("Team 436_WHITE_OPENING_BOOK.txt");
        while (!myfile.eof())
        {
            myfile >> board >> x;
            opening_book.insert({board, x});
        }
    }
}

/*
 * Destructor for the player
 */
Player::~Player() {
    while(length(cache) > 0)
    {
        string to_erase = pop(cache);
        delete trans_table[to_erase];
        trans_table.erase(to_erase);
    }
    trans_table.erase(trans_table.begin(), trans_table.end());
    delete cache;
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
    if (board.checkMove(opponentsMove, (side == BLACK) ? WHITE : BLACK))
    {
        board.doMove(opponentsMove, (side == BLACK) ? WHITE : BLACK);
    }
    if (use_obook)
    {
        string board_string = board.toString();
        if (opening_book.find(board_string) != opening_book.end())
        {
            int m = opening_book[board_string];
            Move *opening_move = new Move(m % 8, m / 8);
            board.doMove(opening_move, side);
            cerr << "using book" << endl;
            return opening_move;
        }
        else
        {
            use_obook = false;
        }
    }
    vector<Move*> moves = board.generateMoves(side);
    if (moves.size() == 0)
    {
        return nullptr;
    }
    if (moves.size() == 1)
    {
        board.doMove(moves[0], side);
        return moves[0];
    }
    
    auto t0 = Time::now();
    auto t1 = Time::now();
    fsec fs = t1 - t0;
    ms d = std::chrono::duration_cast<ms>(fs);
    int depth = 1;
    while (d.count() < msLeft / (64 - board.count() / 2))
    {
        depth += 1;
        AlphaBetaMoveMultithread(moves, &board, depth);
        if (moves[0]->score == std::numeric_limits<double>::max() ||
            depth > (64 - board.count()))
        {
            cerr << moves[0]->score << endl;
            break;
        }
        auto t1 = Time::now();
        fs = t1 - t0;
        d = std::chrono::duration_cast<ms>(fs);
    }
    cerr << side << "Depth reached " << depth <<  ". Time spent " << d.count() << endl;
    board.doMove(moves[0], side);
    for (uint i = 1; i < moves.size(); ++i)
    {
        delete moves[i];
    }
    return moves[0];
}

void Player::GenerateOpeningBook(Board *b, int depth, int num_moves)
{
    // file thing
    if (num_moves == 0) return;
    
    ofstream myfile;
    cout << num_moves << endl;
    if (side == BLACK)
    {
        myfile.open ("Team 436_BLACK_OPENING_BOOK.txt", std::fstream::out | std::fstream::app);
        board = (*b);
        vector<Move*> moves= board.generateMoves(BLACK);
        AlphaBetaMoveMultithread(moves, &board, depth);
        myfile << b->toString() << ' ' << moves[0]->getX() + (8 * moves[0]->getY()) << endl;
        b->doMove(moves[0], side);
        myfile.close();

        for (uint i = 0; i < moves.size(); ++i)
        {
            delete moves[i];
        }
        
        Board *b2 = b->copy();
        for (int j = 0; j < BOARDSIZE * BOARDSIZE; j++)
        {
            Move *possible = new Move(j % 8, j / 8);
            if (b->checkMove(possible, WHITE))
            {
                Board *b_copy = b2->copy();
                b_copy->doMove(possible, WHITE);
                GenerateOpeningBook(b_copy, depth, num_moves - 1);
                delete possible;
            }
            else
            {
                delete possible;
            }
        }
    }
    else
    {
        Board *b2 = b->copy();
        for (int j = 0; j < BOARDSIZE * BOARDSIZE; j++)
        {
            Move *possible = new Move(j / 8, j % 8);
            if (b->checkMove(possible, BLACK))
            {
                Board *b_copy = b2->copy();
                b_copy->doMove(possible, BLACK);
                myfile.open ("Team 436_WHITE_OPENING_BOOK.txt", std::fstream::out | std::fstream::app);
                board = *b_copy;
                vector<Move*> moves= b_copy->generateMoves(WHITE);
                AlphaBetaMoveMultithread(moves, &board, depth);
                myfile << b_copy->toString() << ' ' << moves[0]->getX() + (8 * moves[0]->getY()) << endl;
                myfile.close();
                b_copy->doMove(moves[0], side);
                for (uint i = 0; i < moves.size(); ++i)
                {
                    delete moves[i];
                }
                delete possible;
                GenerateOpeningBook(b_copy, depth, num_moves - 1);
                delete b_copy;
            }
            else
            {
                delete possible;
            }
        }
    }
}

void Player::AlphaBetaSort(vector<Move*> &moves, Board *b, Side side, bool maximizing)
{
    for (uint i = 0; i < moves.size(); ++i)
    {
        Board *b_copy = b->copy();
        b_copy->doMove(moves[i], side);
        moves[i]->score = AlphaBetaEval(b_copy, 2, std::numeric_limits<double>::lowest(),
                            std::numeric_limits<double>::max(), !maximizing);
    }
    if (maximizing)
    {
        sort(moves.begin(), moves.end(), MoveComp<Move>);
    }
    else
    {
        sort(moves.begin(), moves.end(), MoveCompRev<Move>);
    }
}


void Player::AlphaBetaMoveMultithread(vector<Move*> &moves, Board* b, int starting_depth) {
    double alpha;
    if (moves.size() > 0)
    {
        AlphaBetaSort(moves, b, side, true);
        Board* b_copy = b->copy();
        b_copy->doMove(moves[0], side);
        alpha = AlphaBetaEval(b_copy, starting_depth, numeric_limits<double>::lowest(),
                              numeric_limits<double>::max(), false);
        moves[0]->score = alpha;
    }
    int nthread = min(NTHREADS, max((int)moves.size() - 1, 0));
    thread **t = new thread*[nthread];
    for (int i = 0; i < nthread; ++i)
    {
        t[i] = nullptr;
    }
    queue<int> completed;
    for (int i = 0; i < nthread; ++i) {
        completed.push(i);
    }
    for (uint i = 1; i < moves.size(); ++i)
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
                       moves[i],
                       b, starting_depth, alpha,
                          std::numeric_limits<double>::max(), false,
                       done, ref(completed));
    }

    
    for (int i = 0; i < nthread; ++i) {
        if (t[i] != nullptr) {
            t[i]->join();
            delete t[i];
        }
    }

    delete[] t;
    sort(moves.begin(), moves.end(), MoveComp<Move>);
}

void Player::AlphaBetaEvalThread(Move *possible_move,
                            Board *b, int depth, double alpha, double beta, bool maximizing,
                            int id, queue<int> &completed) {
    // Prepare for and do AlphaBetaEval
    Board *board_copy = b->copy();
    board_copy->doMove(possible_move, side);
    possible_move->score = AlphaBetaEval(board_copy, depth, alpha, beta, maximizing);
    delete board_copy;
    
    // Notify parent thread that evaluation complete
    m_cv.lock();
    completed.push(id);
    m_cv.unlock();
    cv.notify_one();

    // All lock guards automatically deleted and mutexes unlocked
}

double Player::CacheEval(Board *b, int depth, double alpha, double beta, bool maximizing)
{
    if (depth < 3)
    {
        return AlphaBetaEval(b, depth - 1, alpha, beta, !maximizing);
    }
    double value;
    string board_state = b->toString();
    bool in_table;
    double cached_score; 
    int cached_depth;
    m_cache.lock();
    if (!trans_table.empty())
    {
        in_table = (trans_table.find(board_state) != trans_table.end());
        if (in_table)
        {
            cached_score = trans_table[board_state]->score;
            cached_depth = trans_table[board_state]->depth;
        }
    }
    else
    {
        in_table = false;
    }
    m_cache.unlock();
    if (!in_table)
    {
        value = AlphaBetaEval(b, depth - 1, alpha, beta, !maximizing);
        m_cache.lock();
        if (trans_table.find(board_state) != trans_table.end())
        {
            Datum *cached = trans_table[board_state];
            cached->score = value;
            cached->depth = depth;
        }
        else
        {
            Datum *board_data = new Datum;
            board_data->depth = depth;
            board_data->score = value;
            board_data->board_state = board_state;
            board_data->node = insert(cache, board_state);
            trans_table.insert({board_state, board_data});
            if (trans_table.size() > MAX_TRANSPOSITION_SIZE)
            {
                string to_erase = pop(cache);
                delete trans_table[to_erase];
                trans_table.erase(to_erase);
            }
        }
        m_cache.unlock();
    }
    else
    {
        if (cached_depth != depth)
        {
            value = AlphaBetaEval(b, depth - 1, alpha, beta, !maximizing);
            m_cache.lock();
            if (trans_table.find(board_state) != trans_table.end())
            {
                Datum *cached = trans_table[board_state];
                cached->score = value;
                cached->depth = depth;
            }
            else
            {
                Datum *board_data = new Datum;
                board_data->depth = depth;
                board_data->score = value;
                board_data->board_state = board_state;
                board_data->node = insert(cache, board_state);
                trans_table.insert({board_state, board_data});
                if (trans_table.size() > MAX_TRANSPOSITION_SIZE)
                {
                    string to_erase = pop(cache);
                    delete trans_table[to_erase];
                    trans_table.erase(to_erase);
                }
            }
            m_cache.unlock();
        }
        else
        {
            m_cache.lock();
            value = cached_score;
            if (trans_table.find(board_state) != trans_table.end())
            {
                to_front(cache, trans_table[board_state]->node);
            }
            m_cache.unlock();
        }
    }
    return value;
}

double Player::AlphaBetaEval(Board *b, int depth, double alpha, double beta, bool maximizing)
{
    if (depth == 0)
    {
        if (testingMinimax)
        {
            return b->naiveScore(side);
        }
        return b->score(side);
    }
    if (b->isDone())
    {
        if (testingMinimax)
        {
            return b->naiveScore(side);
        }
        else if (b->naiveScore(side) > 0)
        {
            return std::numeric_limits<double>::max();
        }
        else
        {
            return std::numeric_limits<double>::lowest();
        }
    }
    double value = 0;
    double best_value;
    bool done = true;
    bool played = false;
    Side other = (side == BLACK) ? WHITE : BLACK;
    if (maximizing)
    {
        best_value = std::numeric_limits<double>::lowest();
        vector<Move*> moves = b->generateMoves(side);
        if (depth > 3)
        {
            AlphaBetaSort(moves, b, side, true);
        }
        for (uint i = 0; i < moves.size(); ++i)
        {
            done = false;
            played = true;
            Board *new_board = b->copy();
            new_board->doMove(moves[i], side);
            // value = AlphaBetaEval(b, depth - 1, alpha, beta, !maximizing);
            value = CacheEval(new_board, depth, alpha, beta, maximizing);
            delete new_board;
            best_value = max(best_value, value);
            alpha = max(alpha, best_value);
            if (beta < alpha)
            {
                delete moves[i];
                break;
            }
            delete moves[i];
        }
        if (played)
        {
            return best_value;
        }
        else
        {
            // return AlphaBetaEval(b, depth - 1, alpha, beta, !maximizing);
            return CacheEval(b, depth, alpha, beta, maximizing);
        }
    }
    else
    {
        best_value = std::numeric_limits<double>::max();
        vector<Move*> moves = b->generateMoves(other);
        if (depth > 3)
        {
           AlphaBetaSort(moves, b, other, false);
        }
        for (uint i = 0; i < moves.size(); ++i)
        {
            played = true;
            done = false;
            Board *new_board = b->copy();
            new_board->doMove(moves[i], other);
            // value = AlphaBetaEval(b, depth - 1, alpha, beta, !maximizing);
            value = CacheEval(new_board, depth, alpha, beta, maximizing);
            delete new_board;
            best_value = min(best_value, value);
            beta = min(beta, best_value);
            if (beta < alpha)
            {
                delete moves[i];
                break;
            }
            if (b->checkMove(moves[i], side))
            {
                done = false;
            }
            delete moves[i];
        }
        if (done)
        {
            if (testingMinimax)
            {
                return b->naiveScore(side);
            }
            else if (b->naiveScore(side) > 0)
            {
                return std::numeric_limits<double>::max();
            }
            else
            {
                return std::numeric_limits<double>::lowest();
            }
        }
        if (played)
        {
            return best_value;
        }
        else
        {
            // return  AlphaBetaEval(b, depth - 1, alpha, beta, !maximizing);
            return CacheEval(b, depth, alpha, beta, maximizing);
        }
    }
}