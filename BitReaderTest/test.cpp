#include <iostream>
#include "../BitReader.hpp"
#include <cstdint>

struct expected_info
{
    uint8_t num;
    uint8_t bits;
};

expected_info expected[] = 
{
{18, 6}, //010010
{5,  4}, //0101
{16, 7}, //0010000
{20, 5}, //10100
{144,8}, //10010000
{15, 4}, //1111
{12, 4}, //1100
{10, 6}, //001010
{121,7}, //1111001
{4,  3}, //100
{2,  3}, //010
{1,  1}, //1
{1,  5}, //00001
{0,  1}, //0
{112,7}, //1110000
{14, 5}, //01110
{98, 7}, //1100010
{54, 6}, //110110
{1,  1}, //1
{0,  1}, //0
{1,  1}, //1
{0,  1}, //0
{1,  1}, //1
{0,  1}, //0
{1,  1}, //1
{0,  1}, //1
{1,  1}, //1
{0,  1}, //0
{1,  1}, //1
{0,  1}, //0
{1,  1}, //1
{0,  1}, //0
{1,  1}  //1
};
//full vector = 01001001 01001000 01010010 01000011 11110000 10101111 00110001 01000010 11100000 11101100 01011011 01010101 01010101
//full hexvec=0x49 48 52 43 F0 AF 31 42 E0 EC 5B 55 55

int main()
{
    BitReader br(std::cin);
    for (int i = 0; i < 1; ++i)
    {
        for (unsigned int j = 0; j < (sizeof(expected) / sizeof(expected[0])); ++j)
        {
            if (br.ReadInt(expected[j].bits) != (uint32_t)expected[j].num)
            {
                std::cerr << "Failed to read " << (int)expected[j].num << " from " << (int)expected[j].bits << " bits." << std::endl;
                exit(1);
            }
        }
        /*for (unsigned int j = 0; j < (12*8); ++j)
        {
            std::cerr << (int)br.ReadBits(1);
        }*/
    }
}
