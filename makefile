all: tcp_client tcp_server

tcp_client: tcp_client.cpp tcp_client.h 
	g++ tcp_client.cpp tcp_client.h -o tcp_client

tcp_server: tcp_server.cpp tcp_server.h
	g++ tcp_server.cpp tcp_server.h -o tcp_server

clean:
	rm -f tcp_client tcp_server