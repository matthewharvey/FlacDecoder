CPPFLAGS=-std=c++11 -g -Wall
all: flac_decoder

flac_decoder: FlacDecoder.o FlacSubFrame.o main.o
	g++ -o flac_decoder FlacDecoder.o FlacSubFrame.o main.o

FlacDecoder.o : FlacDecoder.cpp FlacDecoder.hpp
	g++ $(CPPFLAGS) -c FlacDecoder.cpp

FlacSubFrame.o: FlacSubFrame.cpp FlacSubFrame.hpp
	g++ $(CPPFLAGS) -c FlacSubFrame.cpp

main.o: main.cpp
	g++ $(CPPFLAGS) -c main.cpp

clean:
	rm -f FlacDecoder.o FlacSubFrame.o main.o
