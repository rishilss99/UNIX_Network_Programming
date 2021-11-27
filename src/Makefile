# the compiler: gcc for C program, define as g++ for C++
CC = g++

all: central.out serverS.out serverT.out serverP.out clientA.out clientB.out

central.out: central.cpp
	@ $(CC) -o Central central.cpp

serverS.out: serverS.cpp
	@ $(CC) -o ServerS serverS.cpp

serverT.out: serverT.cpp
	@ $(CC) -o ServerT serverT.cpp

serverP.out: serverP.cpp
	@ $(CC) -o ServerP serverP.cpp

clientA.out: clientA.cpp
	@ $(CC) -o clientA clientA.cpp

clientB.out: clientB.cpp
	@ $(CC) -o clientB clientB.cpp

clean:
	@ $(RM) Central ServerT ServerS ServerP clientA clientB
