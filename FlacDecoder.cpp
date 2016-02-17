#include <cassert>
#include <cstring>
#include <cmath>

#include "FlacDecoder.hpp"
#include "FlacSubFrame.hpp"

FlacDecoder::FlacDecoder(std::istream& in, std::ostream& out)
:
m_in(in),
m_out(out)
{
}

void FlacDecoder::parse_flac_stream()
{
    assert(parse_header_data() == 0);
    assert(parse_metadata_blocks() == 0);
    while (m_in.eof() != true)
    {
        //parse_frame_fake();
        parse_frame();
    }
}

int FlacDecoder::parse_header_data()
{
    char entry_data_expected[] = "fLaC";
    char entry_data[4];
    m_in.read(entry_data, 4);
    for (unsigned int i = 0; i < strlen(entry_data_expected); ++i)
    {
        if (entry_data[i] != entry_data_expected[i])
        {
            std::cerr << "Stream did not begin with fLaC as expected, but with " << entry_data << std::endl;
            return -1;
        }
    }

    return 0;
}

int FlacDecoder::parse_metadata_blocks()
{
    bool is_last_metadata_block = false;
    do
    {
        assert(parse_metadata_block(&is_last_metadata_block) == 0);
    } while (is_last_metadata_block == false);
    return 0;
}

int FlacDecoder::parse_metadata_block(bool* is_last_metadata_block)
{
    uint8 header_data[4];
    m_in.read((char*)header_data, 4);
    *is_last_metadata_block = ((header_data[0] >> 7) == 1);
    EMetadataType metadata_type = (EMetadataType)(header_data[0] & 0x7F);
    unsigned int length = u8touint(header_data[3], header_data[2], header_data[1]);
    return parse_metadata_block_data(metadata_type, length);
}

int FlacDecoder::parse_metadata_block_data(EMetadataType metadata_type, unsigned int length)
{
    uint8* metadata_block_data = new uint8[length];
    std::cerr << "length is " << length << std::endl;
    m_in.read((char*)metadata_block_data, length);
    switch (metadata_type)
    {
    case STREAMINFO:
        std::cerr << "Encountering a Streaminfo block" << std::endl;
        assert(length == 34);
        read_stream_info(metadata_block_data);
        assert(check_stream_info() == 0);
        break;
    case PADDING:
        std::cerr << "Encountering a Padding block" << std::endl;
        break;
    case APPLICATION:
        std::cerr << "Encountering a Application block" << std::endl;
        break;
    case SEEKTABLE:
        std::cerr << "Encountering a Seektable block" << std::endl;
        break;
    case VORBIS_COMMENT:
        std::cerr << "Encountering a Comment block" << std::endl;
        break;
    case CUESHEET:
        std::cerr << "Encountering a Cuesheet block" << std::endl;
        break;
    case PICTURE:
        std::cerr << "Encountering a Picture block" << std::endl;
        break;
    default:
        std::cerr << "Encountering a Unknown block " << (int)metadata_type << std::endl;
        break;
    }
    delete [] metadata_block_data;
    return 0;
}

void FlacDecoder::read_stream_info(uint8* data)
{
    m_minBlockSize = u8touint(data[1], data[0]);
    m_maxBlockSize = u8touint(data[3], data[2]);
    m_minFrameSize = u8touint(data[6], data[5], data[4]);
    m_maxFrameSize = u8touint(data[9], data[8], data[7]);
    m_sampleRate = (data[12] >> 4) | (data[11] << 4) | (data[10] << 12);
    m_numChannels = (data[12]) & 0x07;
    m_bitsPerSample = ((data[13] >> 4) | (data[12] & 0x10));
    //Encoded in the file is bits per sample - 1, so I add the one back
    m_bitsPerSample += 1;
    m_numSamples = (data[17]) | (data[16] << 8) | (data[15] << 16) | (data[14] << 24) | (data[13] << 28);
    m_CRCLow64 = data[25] | (data[24] << 8) | (data[23] << 16) | (data[22] << 24) |
                ((uint64)data[21] << 32) | ((uint64)data[20] << 40) | ((uint64)data[19] << 48) | ((uint64)data[18] << 56);
    m_CRCHigh64 = data[33] | (data[32] << 8) | (data[31] << 16) | (data[30] << 24) |
                ((uint64)data[29] << 32) | ((uint64)data[28] << 40) | ((uint64)data[27] << 48) | ((uint64)data[26] << 56);
}

int FlacDecoder::check_stream_info()
{
    return 0;
}

void FlacDecoder::parse_frame_fake()
{
    //just clear the buffer
    uint8 temp_frame_data[64];
    m_in.read((char*)temp_frame_data, 64);
}

void FlacDecoder::parse_frame()
{
    SFrameInformation frame_info;
    memset(&frame_info, 0, sizeof(frame_info));
    parse_frame_header(&frame_info);
    parse_frame_data(frame_info);
    parse_frame_footer(&frame_info);
}

void FlacDecoder::parse_frame_header(SFrameInformation* frame_info)
{
    uint8 initial_data[4];
    m_in.read((char*)initial_data, 4);
    uint16 sync_code = (initial_data[0] << 6) | (initial_data[1] >> 2);
    if (sync_code != 0x3FFE)
    {
        std::cerr << "sync code is " << sync_code << " instead of 0x3ffe" << std::endl;
        exit(-1);
    }
    bool reserved_bit = ((initial_data[1] & 0x02) == 1);
    if (reserved_bit == true)
    {
        std::cerr << "reserved bit is 1 instead of 0" << std::endl;
    }
    frame_info->blocking_strategy = (EBlockingStrategy)(initial_data[1] & 0x01);
    {
        bool need_to_read_block_size_from_bottom = false;
        uint8 block_size_strategy = (initial_data[2] >> 4);
        translate_block_size_strategy(block_size_strategy, &need_to_read_block_size_from_bottom, &(frame_info->block_size));

        bool need_to_read_sample_rate_from_bottom = false;
        uint8 sample_rate_strategy = (initial_data[2] & 0x0F);
        translate_sample_rate_strategy(sample_rate_strategy, &need_to_read_sample_rate_from_bottom, &(frame_info->sample_rate));

        uint8 channel_assignment_strategy = (initial_data[3] >> 4);
        translate_channel_assignment_strategy(channel_assignment_strategy, &(frame_info->channel_assignment), &(frame_info->num_channels));

        ESampleSizeStrategy sample_size_strategy = (ESampleSizeStrategy)((initial_data[3] & 0x0F) >> 1);
        translate_sample_size_strategy(sample_size_strategy, &(frame_info->sample_size_bits));
        
        bool reserved_bit_2 = ((initial_data[3] & 0x01) == 1);
        if (reserved_bit_2 == true)
        {
            std::cerr << "Reserved bit is not 0" << std::endl;
        }

        read_sample_or_frame_number_string(frame_info->blocking_strategy);

        if (need_to_read_block_size_from_bottom == true)
        {
            std::cerr << "Reading block size from bottom of header" << std::endl;
            if (frame_info->block_size == 16)
            {
                uint8 block_size_data[2];
                m_in.read((char*)block_size_data, 2);
                frame_info->block_size = (block_size_data[0] << 8) | (block_size_data[1]);
            }
            else if (frame_info->block_size == 8)
            {
                m_in.read((char*)&(frame_info->block_size), 1);
            }
            else
            {
                std::cerr << "What!? " << frame_info->block_size << "!? How is that block_size possible?" << std::endl;
            }
            frame_info->block_size += 1;
        }
        
        if (need_to_read_sample_rate_from_bottom == true)
        {
            std::cerr << "Reading sample rate from bottom of header" << std::endl;
            if (frame_info->sample_rate == 8)
            {
                m_in.read((char*)&(frame_info->sample_rate), 1);
            }
            else if ((frame_info->sample_rate == 16) || (frame_info->sample_rate == 160))
            {
                bool need_multiply_by_ten = (frame_info->sample_rate == 160);
                uint8 sample_rate_data[2];
                m_in.read((char*)sample_rate_data, 2);
                frame_info->sample_rate = (sample_rate_data[0] << 8) | (sample_rate_data[1]);
                if (need_multiply_by_ten == true)
                {
                    frame_info->sample_rate *= 10;
                }
            }
            else
            {
                std::cerr << "What!? " << frame_info->sample_rate << "!? How is that sample_rate possible?" << std::endl;
            }
        }

        m_in.read((char*)&(frame_info->crc), 1);
    }
}

void FlacDecoder::translate_block_size_strategy(uint8 block_size_strategy, bool* need_to_read_block_size_from_bottom, uint32* block_size)
{
        EBlockSizeStrategy strategy_translated = (EBlockSizeStrategy)block_size_strategy;
        if ((block_size_strategy >= 2) && (block_size_strategy <= 5))
        {
            strategy_translated = POW_OF_2_MINUS_2;
        }
        else if (block_size_strategy >= 8)
        {
            strategy_translated = POW_OF_2_MINUS_8;
        }

        switch (strategy_translated)
        {
        case FIXED_192:
            *need_to_read_block_size_from_bottom = false;
            *block_size = 192;
            break;
        case POW_OF_2_MINUS_2:
            *need_to_read_block_size_from_bottom = false;
            *block_size = 576 * pow(2, block_size_strategy - 2);
            break;
        case GET_8_BIT_FROM_END:
            *need_to_read_block_size_from_bottom = true;
            *block_size = 8;
            break;
        case GET_16_BIT_FROM_END:
            *need_to_read_block_size_from_bottom = true;
            *block_size = 16;
            break;
        case POW_OF_2_MINUS_8:
            *need_to_read_block_size_from_bottom = false;
            *block_size = 256 * pow(2, block_size_strategy - 8);
            break;
        }
}
void FlacDecoder::translate_sample_rate_strategy(uint8 sample_rate_strategy, bool* need_to_read_sample_rate_from_bottom, uint32* sample_rate)
{
    switch (sample_rate_strategy)
    {
    case USE_GLOBAL_HEADER:
        *need_to_read_sample_rate_from_bottom = false;
        *sample_rate = m_sampleRate;
        break;
    case FIXED_88200:
        *need_to_read_sample_rate_from_bottom = false;
        *sample_rate = 88200;
        break;
    case FIXED_176400:
        *need_to_read_sample_rate_from_bottom = false;
        *sample_rate = 176400;
        break;
    case FIXED_192000:
        *need_to_read_sample_rate_from_bottom = false;
        *sample_rate = 192000;
        break;
    case FIXED_8000:
        *need_to_read_sample_rate_from_bottom = false;
        *sample_rate = 8000;
        break;
    case FIXED_16000:
        *need_to_read_sample_rate_from_bottom = false;
        *sample_rate = 16000;
        break;
    case FIXED_22050:
        *need_to_read_sample_rate_from_bottom = false;
        *sample_rate = 22050;
        break;
    case FIXED_24000:
        *need_to_read_sample_rate_from_bottom = false;
        *sample_rate = 24000;
        break;
    case FIXED_32000:
        *need_to_read_sample_rate_from_bottom = false;
        *sample_rate = 32000;
        break;
    case FIXED_44100:
        *need_to_read_sample_rate_from_bottom = false;
        *sample_rate = 44100;
        break;
    case FIXED_48000:
        *need_to_read_sample_rate_from_bottom = false;
        *sample_rate = 48000;
        break;
    case FIXED_96000:
        *need_to_read_sample_rate_from_bottom = false;
        *sample_rate = 96000;
        break;
    case USE_8_BIT_FROM_END:
        *need_to_read_sample_rate_from_bottom = true;
        *sample_rate = 8;
        break;
    case USE_16_BIT_FROM_END:
        *need_to_read_sample_rate_from_bottom = true;
        *sample_rate = 16;
        break;
    case USE_16_BIT_FROM_END_TIMES_TEN:
        *need_to_read_sample_rate_from_bottom = true;
        *sample_rate = 160;
        break;
    }
}

void FlacDecoder::translate_channel_assignment_strategy(uint8 channel_assignment_strategy, EChannelAssignment* channel_assignment, uint8* num_channels)
{
    switch (channel_assignment_strategy)
    {
    case LEFT_AND_SIDE:
    case RIGHT_AND_SIDE:
    case MID_AND_SIDE:
        *channel_assignment = (EChannelAssignment)channel_assignment_strategy;
        *num_channels = 2;
        break;
    default:
        if (channel_assignment_strategy >= RESERVED_LOW)
        {
            std::cerr << "The channel assignment is in the reserved space" << channel_assignment_strategy << std::endl;
        }
        else
        {
            *channel_assignment = INDEPENDENT;
            *num_channels = channel_assignment_strategy + 1;
        }
        break;
    }
}

void FlacDecoder::translate_sample_size_strategy(ESampleSizeStrategy sample_size_strategy, uint8* sample_size_bits)
{
    switch (sample_size_strategy)
    {
    case USE_GLOBAL_HEADER_SIZE:
        *sample_size_bits = m_bitsPerSample;
        break;
    case FIXED_8_BITS:
        *sample_size_bits = 8;
        break;
    case FIXED_12_BITS:
        *sample_size_bits = 12;
        break;
    case FIXED_16_BITS:
        *sample_size_bits = 16;
        break;
    case FIXED_20_BITS:
        *sample_size_bits = 20;
        break;
    case FIXED_24_BITS:
        *sample_size_bits = 24;
        break;
    }
}

void FlacDecoder::read_sample_or_frame_number_string(EBlockingStrategy blocking_strategy)
{
    char frame_number[7];
    uint8 first_byte;
    m_in.read((char*)&first_byte, 1);
    frame_number[0] = first_byte;
    int num_bytes_total = get_total_bytes_from_first_byte(first_byte);
    if (num_bytes_total - 1 > 0)
    {
        m_in.read(frame_number, num_bytes_total - 1);
    }
    //frame_number = frame_number << (8 * (num_bytes_total - 1));
    std::cerr << ((blocking_strategy == FIXED_BLOCKSIZE) ? "Frame" : "Sample") << " number is " << (int)frame_number[0] << std::endl;
}

int FlacDecoder::get_total_bytes_from_first_byte(uint8 first_byte)
{
    /*int num_bytes = 0;
    int mask = 0x80;
    while ((first_byte & mask) == 1)
    {
        mask >>= 1;
        ++num_bytes;
    }

    if (num_bytes == 0)
    {
        num_bytes = 1;
    }

    return num_bytes;*/
    return 1;
}

void FlacDecoder::parse_frame_data(SFrameInformation& frame_info)
{
    BitReader br(m_in);
    for (int i = 0; i < frame_info.num_channels; ++i)
    {
        FlacSubFrame sub_frame(br, frame_info);
        sub_frame.process();
    }
    std::cerr << "Bits left is " << br.GetBitsLeft() << std::endl;
}

void FlacDecoder::parse_frame_footer(SFrameInformation* frame_info)
{
    m_in.read((char*)&(frame_info->data_crc), 2);
    std::cerr << "crc is " << frame_info->data_crc << std::endl;
}

/*
 * Parameters must be fed in in the opposite order of what you meant
 * making this once convenient function a total mess.
 */

uint32_t FlacDecoder::u8touint(uint8 a, uint8 b, uint8 c, uint8 d)
{
    return a | (b << 8) | (c << 16) | (d << 24);
}
