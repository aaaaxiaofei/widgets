all : PQ

PQ : PQ.o
	g++ -Wall -o PQ PQ.o -std=c++1y
PQ.o: PQ.cpp
	g++ -Wall -o PQ.o -c PQ_test.cpp -std=c++1y

clean:
	rm -f PQ *.o