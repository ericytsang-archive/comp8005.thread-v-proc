
CC = g++ -Wall -W -Wextra -pedantic -g
LIBS = -lgmp -lpthread



clean:
	rm -f *.o *.out



# executables
all: main.o FindFactorsTask.o
	$(CC) -o ./main.out main.o FindFactorsTask.o $(LIBS)

FindFactorsTaskTest: FindFactorsTaskTest.o FindFactorsTask.o Number.o
	$(CC) -o ./FindFactorsTaskTest.out FindFactorsTaskTest.o FindFactorsTask.o Number.o $(LIBS)

NumberTest: NumberTest.o Number.o
	$(CC) -o ./NumberTest.out NumberTest.o Number.o $(LIBS)

# files
main.o: main.cpp
	$(CC) -c main.cpp

FindFactorsTaskTest.o: FindFactorsTaskTest.cpp
	$(CC) -c FindFactorsTaskTest.cpp

FindFactorsTask.o: FindFactorsTask.cpp
	$(CC) -c FindFactorsTask.cpp

Number.o: Number.cpp
	$(CC) -c Number.cpp
