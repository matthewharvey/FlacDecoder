#include <cassert>
#include <cstring>

#include "BitReader.hpp"

BitReader::BitReader(std::istream& stream)
: m_stream(stream),
m_currentByte(0),
m_bitsLeft(0),
m_bitsLeftMask(0),
m_bytesRead(0)
{
}

uint32_t BitReader::ReadInt(unsigned int bits)
{
    assert(bits <= 32);
    uint8 buffer[4];
    unsigned int bitsRead = 0;
    for (int b = 32, i = 0; i < 4; b-=8,++i)
    {
        int bitsToRead = 8 - (b - bits);
        if (bitsToRead > 8)
        {
            buffer[i] = ReadBitsLessThanByte(8);
            bitsRead += 8;
        }
        else if (bitsToRead > 0)
        {
            buffer[i] = ReadBitsLessThanByte((unsigned int)bitsToRead);
            bitsRead += bitsToRead;
        }
        else
        {
            buffer[i] = 0;
        }
    }
    assert(bitsRead == bits);
    uint32_t retval = 0;
    retval |= (buffer[0] << 24);
    retval |= (buffer[1] << 16);
    retval |= (buffer[2] << 8);
    retval |= (buffer[3]);
    return retval;
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

unsigned int BitReader::GetBitsLeft()
{
    return m_bitsLeft;
}

void BitReader::ReadNextByte()
{
    m_currentByte = m_stream.get();
    //m_stream.read((char*)&m_currentByte, 1);
    m_bitsLeft = 8;
    m_bitsLeftMask = 0xFF;
    ++m_bytesRead;
}
