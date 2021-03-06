#ifndef __PLAYER_H__
#define __PLAYER_H__
#define NDEBUG

#include <iostream>
#include <thread>
#include "common.hpp"
#include "board.hpp"
#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <typeinfo>
#include <string>
#include <assert.h>

using namespace std;

struct Node
{
    string key;
    Node *prev;
    Node *next;
};

struct Datum
{
    string board_state;
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
    // when multithreading, be sure to use protection
    mutex m_cv, m_cache;
    condition_variable cv;
    
    // yay for not recomputing thigns
    unordered_map<string, Datum*> trans_table;
    unordered_map<string, int> opening_book;
    bool use_obook;
    DLlist *cache;

public:
    Player(Side side);
    ~Player();

    Move *doMove(Move *opponentsMove, int msLeft);
    void GenerateOpeningBook(Board *b, int depth, int moves);
    
    // fancy multithreading things
    void AlphaBetaRecursiveMultithreadInitial(Board *b, vector<Move*> moves, int depth);
    double AlphaBetaRecursiveMultithread(Board *b, int depth, double alpha, double beta, bool maximizing);
    double AlphaBetaRecursiveMultithread(Board *b, vector<Move*> moves, int depth, double alpha, double beta, bool maximizing);
    double CachedAlphaBetaRecursiveMultithread(Board *b, int depth, double alpha, double beta, bool maximizing);
    
    // normal multithreading things
    void AlphaBetaRecursiveMultithreadEval(Move *possible, Board *b, int depth, 
        double alpha, double beta, bool maximizing, int id, queue<int> &completed);
    void AlphaBetaMoveMultithread(vector<Move*> &moves, Board* b, int starting_depth);
    void AlphaBetaEvalThread(Move *possible_move,
        Board *b, int depth, double alpha, double beta, bool maximizing,
        int id, queue<int> &completed);
    
    // sorting for efficiency TM
    void AlphaBetaSort(vector<Move*> &moves, Board *b, int depth, Side side, bool maximizing);
    
    // normal eval
    double AlphaBetaEval(Board *b, int depth, double alpha, double beta, bool maximizing);
    
    // using the cache to avoid recomputing things
    double CacheEval(Board *b, int depth, double alpha, double beta, bool maximizing);

    // Flag to tell if the player is running within the test_minimax context
    bool testingMinimax;
    Side side;
    Board board;
    
    // a doubly linked list for the cache
    /**
     * inserts a node with value key at the front of the list
     * @param list the list to be inserted in
     * @param key the key to insert
     * @return the node that was just inserted
     */
    Node *insert(DLlist *list, string key)
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

    /**
     * moves a node to the front of a list
     * @param list the list that the node is in
     * @param node the node to move
     */
    void to_front(DLlist *list, Node *node)
    {
        if (list->start == node) return;
        if (list->end == node)
        {
            list->end = node->prev;
        }
        if (node->prev != nullptr)
        {
            node->prev->next = node->next;
        }
        if (node->next != nullptr)
        {
            node->next->prev = node->prev;
        }
        node->next = list->start;
        list->start->prev = node;
        node->prev = nullptr;
        list->start = node;
    }

    /**
     * pops the end of the list
     * @param list the list to pop
     * @return the key of the list
     */
    string pop(DLlist *list)
    {
        if (list->end == nullptr)
        {
            return nullptr;
        }
        string key = list->end->key;
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
            temp->next = nullptr;
            temp->prev = nullptr;
            delete temp;
        }
        return key;
    }
    /**
     * gets the length of a DLlist
     * @param list the list
     * @return the length
     */
    uint length(DLlist *list)
    {
        uint i = 0;
        Node *temp = list->start;
        while(temp != nullptr)
        {
            temp = temp->next;
            ++i;
        }
        return i;
    }
    
    /**
     * checks to see if a list is correctly linked (for debugging purposes)
     * @param list the list to check
     * @return whether or not it's correctly linked
     */
    bool is_correct(DLlist *list)
    {
        Node *temp = list->start;
        while(temp->next != nullptr)
        {
            temp = temp->next;
            if (temp->prev != nullptr)
            {
                assert(temp->prev->next == temp);
            }
            if (temp->next != nullptr)
            {
                assert(temp->next->prev = temp);
            }
        }
        while(temp->prev != nullptr)
        {
            temp = temp->prev;
        }
        return temp == list->start;
    }
};

#endif