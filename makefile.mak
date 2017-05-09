all: projCode

projCode: projCode.cpp
	g++ -o projCode projCode.cpp
clean:
	rm -f projCode *~ core

