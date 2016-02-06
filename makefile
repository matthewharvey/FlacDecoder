CPPFLAGS=-std=c++11 -g -Wall
all: flac_decoder
OBJECTS=FlacDecoder.o FlacSubFrame.o BitReader.o main.o

flac_decoder: $(OBJECTS)
	g++ -o flac_decoder $(OBJECTS)

BitReader.o : BitReader.cpp BitReader.hpp
	g++ $(CPPFLAGS) -c BitReader.cpp

FlacDecoder.o : FlacDecoder.cpp FlacDecoder.hpp FlacSubFrame.hpp BitReader.hpp
	g++ $(CPPFLAGS) -c FlacDecoder.cpp

FlacSubFrame.o: FlacSubFrame.cpp FlacSubFrame.hpp BitReader.hpp
	g++ $(CPPFLAGS) -c FlacSubFrame.cpp

main.o: main.cpp FlacDecoder.hpp
	g++ $(CPPFLAGS) -c main.cpp

clean:
	rm -f $(OBJECTS)
