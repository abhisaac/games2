CFLAGS = -g -std=c++17
LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

build/a.out: tictac.cpp
	g++ $(CFLAGS) $^ $(LDFLAGS)

