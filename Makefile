CC          = g++
CFLAGS      = -Wall -pedantic -ggdb --std=c++11 -pthread -O3
OBJS        = player.o board.o
PLAYERNAME  = 436

all: $(PLAYERNAME) testgame

$(PLAYERNAME): $(OBJS) wrapper.o
	$(CC) -o $@ $^ -pthread

testgame: testgame.o
	$(CC) -o $@ $^ -pthread

testminimax: $(OBJS) testminimax.o
	$(CC) -pthread -o $@ $^ -O3

%.o: %.cpp
	$(CC) -c $(CFLAGS) -x c++ $< -o $@

java:
	make -C java/

cleanjava:
	make -C java/ clean

clean:
	rm -f *.o $(PLAYERNAME) testgame testminimax

.PHONY: java testminimax
