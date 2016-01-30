#include <iostream>

#include "FlacDecoder.hpp"

int main()
{
    FlacDecoder decoder(std::cin, std::cout);
    decoder.parse_flac_stream();
    return 0;
}
