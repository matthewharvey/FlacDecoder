#pragma once

#include <istream>
#include <cstdint>

class BitReader
{
public:
    BitReader(std::istream& stream);
    void ReadBits(uint8_t* buffer, unsigned int bits);
    uint8_t ReadBits(unsigned int bits);
    void ReadByteBlock(char* buffer, unsigned int length);
    unsigned int GetBytesRead();
private:
    using uint8=uint8_t;

    void ReadNextByte();
    uint8 ReadBitsLessThanByte(unsigned int bits);
    std::istream& m_stream;
    uint8 m_currentByte;
    uint8 m_bitsLeft;
    uint8 m_bitsLeftMask;

    unsigned int m_bytesRead;
};
