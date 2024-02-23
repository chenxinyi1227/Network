#src=main.c doubleLinkList.c doubleLinkListQueue.c balanceBinarySearchTree.c
src=demoManyServer.c threadPool.c
s:${src}
	gcc ${src} -o server -lpthread

1:demoClient1.c
	gcc demoClient1.c -o client1

2:demoClient2.c
	gcc demoClient2.c -o client2

all:s 1 2
clean:
	@rm -rf server client1 client2
