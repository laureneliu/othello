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
#define MAX_TRANSPOSITION_SIZE 50000
#define OPENING true
#define PRUNING_THRESHOLD 5
#define DEBUG false


    
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
            if (DEBUG)
            {
                cerr << "using book" << endl;
            }
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
    int depth = 2;
    // redo this formula
    while (d.count() < msLeft / ((max(50 - board.count(), 1) * pow(1.1, depth))))
    {
        ++depth;
        if (DEBUG)
            cerr << "Searching depth " << depth << ". Time is " << d.count() << ". Bound was " << msLeft / ((max(50 - board.count(), 1) * pow(1.1, depth - 1))) << endl;
        AlphaBetaMoveMultithread(moves, &board, depth);
        if (moves[0]->score == std::numeric_limits<double>::max() ||
            depth > (64 - board.count()))
        {
            break;
        }
        auto t1 = Time::now();
        fs = t1 - t0;
        d = std::chrono::duration_cast<ms>(fs);
    }
    if (DEBUG)
    {
        if (side == BLACK)
        {
            cerr << "BLACK";
        }
        else
        {
            cerr << "WHITE";
        }
        cerr << " " << board.count() << " ";
        cerr << "Depth reached " << depth <<  ". Time spent " << d.count() << ". Time left " << msLeft - d.count() << ". Score " << moves[0]->score << endl;
    }
    board.doMove(moves[0], side);
    for (uint i = 1; i < moves.size(); ++i)
    {
        delete moves[i];
    }
    return moves[0];
}

/*
 *Rewrite...
 */
void Player::GenerateOpeningBook(Board *b, int depth, int num_moves)
{
    // file thing
    if (num_moves == 0) return;
    
    ofstream myfile;
    cout << num_moves << endl;
    if (side == BLACK)
    {
        myfile.open ("Team 436_BLACK_OPENING_BOOK.txt", std::fstream::out | std::fstream::app);
        vector<Move*> moves= b->generateMoves(BLACK);
        AlphaBetaMoveMultithread(moves, b, 3);
        AlphaBetaMoveMultithread(moves, b, depth);
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
                delete b_copy;
            }
            else
            {
                delete possible;
            }
        }
        delete b2;
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
                vector<Move*> moves = b_copy->generateMoves(WHITE);
                AlphaBetaMoveMultithread(moves, b, 3);
                AlphaBetaMoveMultithread(moves, b_copy, depth);
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
        delete b2;
    }
}

void Player::AlphaBetaSort(vector<Move*> &moves, Board *b, int depth, Side side, bool maximizing)
{
    for (uint i = 0; i < moves.size(); ++i)
    {
        Board *b_copy = b->copy();
        b_copy->doMove(moves[i], side);
        moves[i]->score = AlphaBetaEval(b_copy, 2, std::numeric_limits<double>::lowest(),
                            std::numeric_limits<double>::max(), !maximizing);
        delete b_copy;
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
        Board* b_copy = b->copy();
        b_copy->doMove(moves[0], side);
        alpha = AlphaBetaEval(b_copy, starting_depth, numeric_limits<double>::lowest(),
                              numeric_limits<double>::max(), false);
        moves[0]->score = alpha;
        delete b_copy;
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
        return AlphaBetaEval(b, depth - 1, alpha, beta, maximizing);
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
        value = AlphaBetaEval(b, depth - 1, alpha, beta, maximizing);
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
        if (cached_depth < depth)
        {
            value = AlphaBetaEval(b, depth - 1, alpha, beta, maximizing);
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
    if (b->isDone())
    {
        if (b->naiveScore(side) > 0)
        {
            return std::numeric_limits<double>::max();
        }
        else
        {
            return std::numeric_limits<double>::lowest();
        }
    }
    if (depth == 0)
    {
        return b->score(side);
    }
    double value = 0;
    double best_value;
    bool played = false;
    Side other = (side == BLACK) ? WHITE : BLACK;
    if (maximizing)
    {
        best_value = std::numeric_limits<double>::lowest();
        vector<Move*> moves = b->generateMoves(side);
        if (depth > 3)
        {
            AlphaBetaSort(moves, b, depth / 2, side, true);
        }
        if (moves.size() > PRUNING_THRESHOLD)
        {
            for (uint i = PRUNING_THRESHOLD; i < moves.size(); ++i)
            {
                delete moves[i];
            }
            moves.resize(PRUNING_THRESHOLD);
        }
        for (uint i = 0; i < moves.size(); ++i)
        {
            played = true;
            Board *new_board = b->copy();
            new_board->doMove(moves[i], side);
            // value = AlphaBetaEval(b, depth - 1, alpha, beta, !maximizing);
            value = CacheEval(new_board, depth, alpha, beta, !maximizing);
            delete new_board;
            best_value = max(best_value, value);
            alpha = max(alpha, best_value);
            if (beta < alpha)
            {
                break;
            }
        }
        for (uint i = 0; i < moves.size(); ++i)
        {
            delete moves[i];
        }
        if (played)
        {
            return best_value;
        }
        else
        {
            // return AlphaBetaEval(b, depth - 1, alpha, beta, !maximizing);
            return CacheEval(b, depth, alpha, beta, !maximizing);
        }
    }
    else
    {
        best_value = std::numeric_limits<double>::max();
        vector<Move*> moves = b->generateMoves(other);
        if (depth > 3)
        {
           AlphaBetaSort(moves, b, depth / 2, other, false);
        }
        if (moves.size() > PRUNING_THRESHOLD)
        {
            for (uint i = PRUNING_THRESHOLD; i < moves.size(); ++i)
            {
                delete moves[i];
            }
            moves.resize(PRUNING_THRESHOLD);
        }
        for (uint i = 0; i < moves.size(); ++i)
        {
            played = true;
            Board *new_board = b->copy();
            new_board->doMove(moves[i], other);
            // value = AlphaBetaEval(b, depth - 1, alpha, beta, !maximizing);
            value = CacheEval(new_board, depth, alpha, beta, !maximizing);
            delete new_board;
            best_value = min(best_value, value);
            beta = min(beta, best_value);
            if (beta < alpha)
            {
                break;
            }
        }
        for (uint i = 0; i < moves.size(); ++i)
        {
            delete moves[i];
        }
        if (played)
        {
            return best_value;
        }
        else
        {
            // return  AlphaBetaEval(b, depth - 1, alpha, beta, !maximizing);
            return CacheEval(b, depth, alpha, beta, !maximizing);
        }
    }
}

//Probably won't finish in time because lazy

double Player::AlphaBetaRecursiveMultithread(Board *b, int depth, double alpha, double beta, bool maximizing)
{
    if (depth == 0)
    {
        return b->score(side);
    }
    if (b->isDone())
    {
        if (b->naiveScore(side) > 0)
        {
            return std::numeric_limits<double>::max();
        }
        else
        {
            return std::numeric_limits<double>::lowest();
        }
    }
    Side other = (side == BLACK) ? WHITE : BLACK;
    if (maximizing)
    {
        vector<Move*> moves = b->generateMoves(side);
        if (moves.size() == 0)
        {
            return CachedAlphaBetaRecursiveMultithread(b, depth - 1, alpha, beta, !maximizing);
        }
        if (moves.size() == 1)
        {
            Board *b_copy = b->copy();
            b_copy->doMove(moves[0], side);
            double value = CachedAlphaBetaRecursiveMultithread(b_copy, depth - 1, alpha, beta, !maximizing);
            delete b_copy;
            delete moves[0];
            return value;
        }
        AlphaBetaSort(moves, b, 2, side, true);
        Board *b_copy = b->copy();
        b_copy->doMove(moves[0], side);
        double value = CachedAlphaBetaRecursiveMultithread(b, depth - 1, alpha, beta, !maximizing);
        alpha = max(alpha, value);
        vector<Move*> remainder = vector<Move*>(moves.begin() + 1, moves.end());
        double remainder_value = AlphaBetaRecursiveMultithread(b_copy, remainder, depth - 1, alpha, beta, !maximizing);
        delete b_copy;
        for (uint i = 0; i < moves.size(); ++i)
        {
            delete moves[i];
        }
        return max(value, remainder_value);
    }
    else
    {
        vector<Move*> moves = b->generateMoves(other);
        if (moves.size() == 0)
        {
            return CachedAlphaBetaRecursiveMultithread(b, depth - 1, alpha, beta, !maximizing);
        }
        if (moves.size() == 1)
        {
            Board *b_copy = b->copy();
            b_copy->doMove(moves[0], other);
            double value = CachedAlphaBetaRecursiveMultithread(b_copy, depth - 1, alpha, beta, !maximizing);
            delete moves[0];
            delete b_copy;
            return value;
        }
        AlphaBetaSort(moves, b, 2, other, false);
        Board *b_copy = b->copy();
        b_copy->doMove(moves[0], other);
        double value = CachedAlphaBetaRecursiveMultithread(b, depth - 1, alpha, beta, !maximizing);
        beta = min(beta, value);
        vector<Move*> remainder = vector<Move*>(moves.begin() + 1, moves.end());
        double remainder_value = AlphaBetaRecursiveMultithread(b_copy, remainder, depth - 1, alpha, beta, !maximizing);
        delete b_copy;
        for (uint i = 0; i < moves.size(); ++i)
        {
            delete moves[i];
        }
        return min(value, remainder_value);
    }
}

void Player::AlphaBetaRecursiveMultithreadInitial(Board *b, vector<Move*> moves, int depth)
{
    if (moves.size() == 0)
    {
        return;
    }
    else
    {
        Board *b_copy = b->copy();
        b_copy->doMove(moves[0], side);
        moves[0]->score = AlphaBetaRecursiveMultithread(b_copy, depth - 1, numeric_limits<double>::lowest(),
                numeric_limits<double>::max(), false);
        delete b_copy;
        vector<Move*> remainder = vector<Move*>(moves.begin() + 1, moves.end());
        AlphaBetaRecursiveMultithread(b, remainder, depth, moves[0]->score, numeric_limits<double>::max(), true);
        sort(moves.begin(), moves.end(), MoveComp<Move>);
    }
}
double Player::AlphaBetaRecursiveMultithread(Board *b, vector<Move*> moves, int depth, double alpha, double beta, bool maximizing)
{
    if (depth == 0)
    {
        return b->score(side);
    }
    if (b->isDone())
    {
        if (b->naiveScore(side) > 0)
        {
            return std::numeric_limits<double>::max();
        }
        else
        {
            return std::numeric_limits<double>::lowest();
        }
    }
    int nthread = min(NTHREADS, max((int)moves.size(), 0));
    thread **t = new thread*[nthread];
    for (int i = 0; i < nthread; ++i)
    {
        t[i] = nullptr;
    }
    queue<int> completed;
    for (int i = 0; i < nthread; ++i) {
        completed.push(i);
    }
    for (uint i = 0; i < moves.size(); ++i)
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
                       b, depth - 1, alpha, beta, !maximizing,
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
    return moves[0]->score;
}

double Player::CachedAlphaBetaRecursiveMultithread(Board *b, int depth, double alpha, double beta, bool maximizing)
{
    if (depth < 3)
    {
        return AlphaBetaEval(b, depth - 1, alpha, beta, maximizing);
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
        value = AlphaBetaRecursiveMultithread(b, depth - 1, alpha, beta, maximizing);
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
        if (cached_depth < depth)
        {
            value = AlphaBetaRecursiveMultithread(b, depth - 1, alpha, beta, maximizing);
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
