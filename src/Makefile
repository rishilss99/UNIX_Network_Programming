# the compiler: gcc for C program, define as g++ for C++
CC = gcc

all: central.out serverS.out serverT.out serverP.out clientA.out clientB.out

serverC: Central
	@ ./Central

serverT: ServerT
	@ ./ServerT

serverP: ServerP
	@ ./ServerP

serverS: ServerS
	@ ./ServerS

central.out: central.c
	@ $(CC) -o Central central.c

serverS.out: serverS.c
	@ $(CC) -o ServerS serverS.c

serverT.out: serverT.c
	@ $(CC) -o ServerT serverT.c

serverP.out: serverP.c
	@ $(CC) -o ServerP serverP.c

clientA.out: clientA.c
	@ $(CC) -o clientA clientA.c

clientB.out: clientB.c
	@ $(CC) -o clientB clientB.c

clean:
	@ $(RM) Central ServerT ServerS ServerP clientA clientB
