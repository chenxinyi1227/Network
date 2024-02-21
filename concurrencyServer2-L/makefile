#src=main.c doubleLinkList.c doubleLinkListQueue.c balanceBinarySearchTree.c
src=manyServer.c threadPool.c
s:${src}
	gcc ${src} -o server -lpthread

1:client1.c
	gcc client1.c -o client1

2:client1.c
	gcc client2.c -o client2

all: s 1 2
clean:
	@rm -rf server client1 client2
