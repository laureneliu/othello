Group Member Contributions:

Lauren: Multithreading, modifications to Board.cpp

Mark: Alpha Beta Pruning, Opening book, move sorting/prunning

Strategies:

AB prunning is provably more efficient than minimax, so it's faster

We used multithreading with alpha bounds on best move to speed up computations

Opening book also speeds up computation

Move sorting reduces branching factor allowing for deeper search. Heuristic
 is good enough that prunning shouldn't make it worse off.

Iterative deepening allows for more efficient use of time.



