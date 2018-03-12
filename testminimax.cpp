#include <iostream>
#include "common.hpp"
#include "player.hpp"
#include "board.hpp"

// Use this file to test your minimax implementation (2-ply depth, with a
// heuristic of the difference in number of pieces).
int main(int argc, char *argv[]) {

    // Create board with example state. You do not necessarily need to use
    // this, but it's provided for convenience.
    char boardData[64] = {
        ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
        ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
        ' ', 'b', ' ', ' ', ' ', ' ', ' ', ' ',
        'b', 'w', 'b', 'b', 'b', 'b', ' ', ' ',
        ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
        ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
        ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
        ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '
    };
    Board *board = new Board();
    //board->setBoard(boardData);

    // Initialize player as the white player, and set testing_minimax flag.
    Player *player = new Player(BLACK);
    player->GenerateOpeningBook(&player->board, 12, 5);
    delete player;
    player = new Player(WHITE);
    player->GenerateOpeningBook(&player->board, 12, 5);
    delete player;
    return 0;
    // player->testingMinimax = true;


    /**
     * TODO: Write code to set your player's internal board state to the
     * example state.
     */
//    Player *player = new Player(BLACK);
//    Player *other = new Player(WHITE);
//    Board *temp1, *temp2;
//    temp1 = board->copy();
//    temp2 = board->copy();
//    player->board = *temp1;
//    other->board = *temp2;
//
//    Move *p_move = nullptr;
//    Move *o_move = nullptr;
//    for (int i = 0; i < 30; ++i)
//    {
//        cerr << i << endl;
//        if (p_move != nullptr)
//            delete p_move;
//        p_move = player->doMove(o_move, 5000);
//        if (o_move != nullptr)
//            delete o_move;
//        o_move = other->doMove(p_move, 5000);
//    }
//    
//    if (p_move != nullptr)
//        delete p_move;
//    if (o_move != nullptr)
//        delete o_move;
//    delete board;
//    delete player;
//    delete other;
//    delete temp1;
//    delete temp2;
//    return 0;
        
        
//
//    if (move != nullptr && move->x == 1 && move->y == 1) {
//        std::cout << "Correct move: (1, 1)" << std::endl;;
//    } else {
//        std::cout << "Wrong move: got ";
//        if (move == nullptr) {
//            std::cout << "PASS";
//        } else {
//            std::cout << "(" << move->x << ", " << move->y << ")";
//        }
//        std::cout << ", expected (1, 1)" << std::endl;
//    }
//
//    return 0;
}
