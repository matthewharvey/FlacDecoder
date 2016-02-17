CPPFLAGS=-std=c++11 -g -Wall
all: flac_decoder BitReaderTest/test
DECODER_OBJECTS=FlacDecoder.o FlacSubFrame.o FlacResidual.o BitReader.o main.o
BRTEST_OBJECTS=BitReader.o BitReaderTest/test.o

flac_decoder: $(DECODER_OBJECTS)
	g++ -o flac_decoder $(DECODER_OBJECTS)

BitReaderTest/test: $(BRTEST_OBJECTS)
	g++ -o BitReaderTest/test $(BRTEST_OBJECTS)

BitReader.o : BitReader.cpp BitReader.hpp
	g++ $(CPPFLAGS) -c BitReader.cpp

FlacDecoder.o : FlacDecoder.cpp FlacDecoder.hpp FlacSubFrame.hpp BitReader.hpp
	g++ $(CPPFLAGS) -c FlacDecoder.cpp

FlacSubFrame.o: FlacSubFrame.cpp FlacSubFrame.hpp FlacResidual.hpp BitReader.hpp
	g++ $(CPPFLAGS) -c FlacSubFrame.cpp

FlacResidual.o: FlacResidual.cpp FlacResidual.hpp BitReader.hpp

main.o: main.cpp FlacDecoder.hpp
	g++ $(CPPFLAGS) -c main.cpp

clean:
	rm -f $(DECODER_OBJECTS) $(BRTEST_OBJECTS)
