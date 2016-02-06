#include <cassert>

#include "BitReader.hpp"

BitReader::BitReader(std::istream& stream)
: m_stream(stream),
m_bytesRead(0)
{
    ReadNextByte();
}

void BitReader::ReadBits(uint8* buffer, unsigned int bits)
{
    unsigned int leftover_bits = bits % 8;
    unsigned int bytes = bits / 8;
    if (leftover_bits == m_bitsLeft)
    {
        buffer[0] = ReadBitsLessThanByte(m_bitsLeft);
        ReadByteBlock((char*)&(buffer[1]), bytes);
    }
    else
    {
        buffer[0] = ReadBitsLessThanByte(leftover_bits);
        for (unsigned int i = 1; i <= bytes; ++i)
        {
            buffer[i] = ReadBitsLessThanByte(8);
        }
    }
}

uint8_t BitReader::ReadBits(unsigned int bits)
{
    return ReadBitsLessThanByte(bits);
}

uint8_t BitReader::ReadBitsLessThanByte(unsigned int bits)
{
    assert(bits <= 8);

    uint8 retval = 0;
    if (bits <= m_bitsLeft)
    {
        unsigned int unwanted = m_bitsLeft - bits;
        retval = ((m_bitsLeftMask & m_currentByte) >> unwanted);
        m_bitsLeft -= bits;
        m_bitsLeftMask >>= bits;
    }
    else
    {
        unsigned int remainder = bits - m_bitsLeft;
        retval = (m_currentByte & m_bitsLeftMask) << remainder;
        m_bitsLeft = 0;
        m_bitsLeftMask = 0;
        ReadNextByte();
        retval |= ReadBits(remainder);
    }
    
    return retval;
}

void BitReader::ReadByteBlock(char* buffer, unsigned int length)
{
    assert(m_bitsLeft == 0);
    assert(m_bitsLeftMask == 0);

    m_bytesRead += length;
    m_stream.read(buffer, length);
}

unsigned int BitReader::GetBytesRead()
{
    return m_bytesRead;
}

void BitReader::ReadNextByte()
{
    m_stream.read((char*)&m_currentByte, 1);
    m_bitsLeft = 8;
    m_bitsLeftMask = 0xFF;
    ++m_bytesRead;
}
