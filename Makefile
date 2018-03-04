CC          = g++
CFLAGS      = -Wall -pedantic -ggdb --std=c++11 -pthread
OBJS        = player.o board.o
PLAYERNAME  = meow

all: $(PLAYERNAME) testgame

$(PLAYERNAME): $(OBJS) wrapper.o
	$(CC) -o $@ $^ -pthread

testgame: testgame.o
	$(CC) -o $@ $^ -pthread

testminimax: $(OBJS) testminimax.o
	$(CC) -o $@ $^

%.o: %.cpp
	$(CC) -c $(CFLAGS) -x c++ $< -o $@

java:
	make -C java/

cleanjava:
	make -C java/ clean

clean:
	rm -f *.o $(PLAYERNAME) testgame testminimax

.PHONY: java testminimax
