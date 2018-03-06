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
    // board->setBoard(boardData);

    // Initialize player as the white player, and set testing_minimax flag.
    Player *player = new Player(WHITE);
    Player *other = new Player(BLACK);
    // player->testingMinimax = true;


    /**
     * TODO: Write code to set your player's internal board state to the
     * example state.
     */
    player->board = *board;
    other->board = *board;

    // Get player's move and check if it's right.
    Move *p_move = nullptr;
    Move *o_move = nullptr;
    while(!board->isDone())
    {
        p_move = player->doMove(o_move, 0);
        o_move = other->doMove(p_move, 0);
    }
    return 0;
        
        
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
