all: client server

client: client.cpp board comm global.h
	g++ -o client client.cpp board.o comm.o -O3

server: server.cpp board comm gameServer global.h
	g++ -o server server.cpp board.o comm.o gameServer.o -O3

comm: comm.cpp comm.h global.h board move.h
	g++ -c comm.cpp -O3

board: board.cpp board.h move.h global.h
	g++ -c board.cpp -O3

gameServer: gameServer.cpp gameServer.h board.h move.h global.h
	g++ -c gameServer.cpp -O3

clean:
	rm -f *.o client server
