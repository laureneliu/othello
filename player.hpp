#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <iostream>
#include "common.hpp"
#include "board.hpp"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <typeinfo>

using namespace std;

struct Node
{
    char *key;
    Node *prev;
    Node *next;
};

struct Datum
{
    char *board_state;
    int depth;
    double score;
    Node *node;
};

struct DLlist
{
    Node* start;
    Node* end;
};





class Player {
private:
    mutex m_cv, m_best, m_cache;
    condition_variable cv;
    unordered_map<char*, Datum*> trans_table;
    DLlist *cache;

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
    double CacheEval(Board &b, int depth, double alpha, double beta, bool maximizing);

    // Flag to tell if the player is running within the test_minimax context
    bool testingMinimax;
    int starting_depth;
    Side side;
    Board board;
    Node *insert(DLlist *list, char *key)
    {
        Node* to_insert = new Node;
        to_insert->key = key;
        if (list->start == nullptr)
        {
            list->start = to_insert;
            list->end = to_insert;
            to_insert->next = nullptr;
            to_insert->prev = nullptr;
            return to_insert;
        }
        list->start->prev = to_insert;
        to_insert->next = list->start;
        to_insert->prev = nullptr;
        list->start = to_insert;
        return to_insert;
    }

    void to_front(DLlist *list, Node *node)
    {
        return;
        if (list->start == node) return;
        cerr << 10 << endl;
        if (list->end == node)
        {
            list->end = node->prev;
        }
        if (node->prev != nullptr)
        {
            cerr << 10.1 << endl;
            node->prev->next = node->next;
        }
        if (node->next != nullptr)
        {
            cerr << 10.2 << endl;
            cerr << typeid(node->next->prev).name() << ' ' << typeid(node->prev).name() << endl;
            node->next->prev = node->prev;
            cerr << 10.3 << endl;
        }
        cerr << 11 << endl;
        node->next = list->start;
        list->start->prev = node;
        node->prev = nullptr;
        list->start = node;
    }

    char *pop(DLlist *list)
    {
        if (list->end == nullptr)
        {
            return nullptr;
        }
        char *key = list->end->key;
        if (list->start == list->end)
        {
            list->start = nullptr;
            delete list->end;
            list->end = nullptr;
        }
        else
        {
            Node *temp = list->end;
            list->end = list->end->prev;
            list->end->next = nullptr;
            delete temp;
        }
        return key;
    }
};

#endif