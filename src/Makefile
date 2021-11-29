# the compiler: gcc for C program, define as g++ for C++
CC = g++

all: serverC.out serverS.out serverT.out serverP.out clientA.out clientB.out

serverC.out: central.cpp
	@ $(CC) -o serverC central.cpp

serverS.out: serverS.cpp
	@ $(CC) -o serverS serverS.cpp

serverT.out: serverT.cpp
	@ $(CC) -o serverT serverT.cpp

serverP.out: serverP.cpp
	@ $(CC) -o serverP serverP.cpp

clientA.out: clientA.cpp
	@ $(CC) -o clientA clientA.cpp

clientB.out: clientB.cpp
	@ $(CC) -o clientB clientB.cpp

clean:
	@ $(RM) serverC serverT serverS serverP clientA clientB
