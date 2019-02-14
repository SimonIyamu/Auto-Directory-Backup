all: mirr

mirr:	Client.o SynchImplementation.o watchImplementation.o listImplementation.o 
	gcc Client.o SynchImplementation.o listImplementation.o watchImplementation.o -g -o mirr

Client.o: Client.c Synchinterface.h SupplementaryStructs.h 
	gcc -c Client.c

SynchImplementation.o: SynchImplementation.c SupplementaryStructs.h
	gcc -c SynchImplementation.c

listImplementation.o: listImplementation.c listInterface.h listTypes.h
	gcc -c listImplementation.c

watchImplementation.o: watchImplementation.c watchInterface.h
	gcc -c watchImplementation.c -g

clean:
	rm mirr listImplementation.o watchImplementation.o Client.o SynchImplementation.o
