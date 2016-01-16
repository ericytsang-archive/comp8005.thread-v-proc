
CC = g++ -Wall -W -Wextra -pedantic -g -std=c++11
LIBS = -lgmp -lpthread -pthread



clean:
	rm -f *.o *.out



# executables
Processes-Main: Processes-Main.o FindFactorsTask.o Lock.o Number.o
	$(CC) -o ./Processes-Main.out Processes-Main.o FindFactorsTask.o Lock.o Number.o $(LIBS)

Threads-Main: Threads-Main.o FindFactorsTask.o Lock.o Semaphore.o Number.o
	$(CC) -o ./Threads-Main.out Threads-Main.o FindFactorsTask.o Lock.o Semaphore.o Number.o $(LIBS)

FindFactorsTaskTest: FindFactorsTaskTest.o FindFactorsTask.o Number.o
	$(CC) -o ./FindFactorsTaskTest.out FindFactorsTaskTest.o FindFactorsTask.o Number.o $(LIBS)

NumberTest: NumberTest.o Number.o
	$(CC) -o ./NumberTest.out NumberTest.o Number.o $(LIBS)

# files
Threads-Main.o: Threads-Main.cpp
	$(CC) -c Threads-Main.cpp

Processes-Main.o: Processes-Main.cpp
	$(CC) -c Processes-Main.cpp

FindFactorsTaskTest.o: FindFactorsTaskTest.cpp
	$(CC) -c FindFactorsTaskTest.cpp

FindFactorsTask.o: FindFactorsTask.cpp
	$(CC) -c FindFactorsTask.cpp

Semaphore.o: Semaphore.cpp
	$(CC) -c Semaphore.cpp

Number.o: Number.cpp
	$(CC) -c Number.cpp

Lock.o: Lock.cpp
	$(CC) -c Lock.cpp
