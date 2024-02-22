server:demoSelectServer.c
	gcc demoSelectServer.c -o server

server_thread:demoSelectThreadServer.c
	gcc demoSelectThreadServer.c -o server_thread

client:demoSelectClient.c
	gcc demoSelectClient.c -o client	

all:server_thread client server

clean:
	rm -f server client server_thread