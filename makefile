tube: tube.o main.o
	g++ -g -Wall tube.o main.o -o tube

main.o: main.cpp tube.h
	g++ -c -g -Wall main.cpp

tube.o: tube.cpp tube.h
	g++ -c -g -Wall tube.cpp

clean:
	rm -rf *.o tube