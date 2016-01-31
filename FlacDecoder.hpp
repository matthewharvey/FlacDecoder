#pragma once

#include<iostream>
#include <istream>
#include <ostream>
#include <cstdint>


class FlacDecoder
{
public:
    FlacDecoder(std::istream& in, std::ostream& out);
    void parse_flac_stream();
private:
    using uint8=uint8_t;
    using uint16=uint16_t;
    using uint32=uint32_t;
    using uint64=uint64_t;
    using int8=int8_t;
    enum EMetadataType
    {
        STREAMINFO = 0,
        PADDING = 1,
        APPLICATION = 2,
        SEEKTABLE = 3,
        VORBIS_COMMENT = 4,
        CUESHEET = 5,
        PICTURE = 6
    };

    //global parsing stuff
    int parse_header_data();
    int parse_metadata_blocks();
    int parse_metadata_block(bool* is_last_metadata_block);
    int parse_metadata_block_data(EMetadataType metadata_type, unsigned int length);
    void read_stream_info(uint8* data);
    int check_stream_info();
    std::istream& m_in;
    std::ostream& m_out;
    uint16_t m_minBlockSize;
    uint16_t m_maxBlockSize;
    uint32_t m_minFrameSize;
    uint32_t m_maxFrameSize;
    uint32_t m_sampleRate;
    uint8_t m_numChannels;
    uint8_t m_bitsPerSample;
    uint64 m_numSamples;
    uint64 m_CRCLow64;
    uint64_t m_CRCHigh64;

    //per frame stuff

public:
    enum EBlockingStrategy
    {
        FIXED_BLOCKSIZE = 0,
        VARIABLE_BLOCKSIZE = 1
    };

    enum EBlockSizeStrategy
    {
        FIXED_192 = 0x01,
        POW_OF_2_MINUS_2 = 0x2,
        GET_8_BIT_FROM_END = 0x6,
        GET_16_BIT_FROM_END = 0x7,
        POW_OF_2_MINUS_8 = 0x8
    };

    enum ESampleRateStrategy
    {
        USE_GLOBAL_HEADER = 0x0,
        FIXED_88200 = 0x1,
        FIXED_176400 = 0x2,
        FIXED_192000 = 0x3,
        FIXED_8000   = 0x4,
        FIXED_16000  = 0x5,
        FIXED_22050  = 0x6,
        FIXED_24000  = 0x7,
        FIXED_32000  = 0x8,
        FIXED_44100  = 0x9,
        FIXED_48000  = 0xA,
        FIXED_96000  = 0xB,
        USE_8_BIT_FROM_END = 0xC,
        USE_16_BIT_FROM_END = 0xD,
        USE_16_BIT_FROM_END_TIMES_TEN = 0xE
    };

    enum EChannelAssignment
    {
        INDEPENDENT = 0,
        LEFT_AND_SIDE = 0x8,
        RIGHT_AND_SIDE = 0x9,
        MID_AND_SIDE = 0xA,
        RESERVED_LOW = 0xB,
        RESERVED_HIGH = 0xF
    };

    enum ESampleSizeStrategy
    {
        USE_GLOBAL_HEADER_SIZE = 0x0,
        FIXED_8_BITS = 0x1,
        FIXED_12_BITS = 0x2,
        FIXED_16_BITS = 0x4,
        FIXED_20_BITS = 0x5,
        FIXED_24_BITS = 0x6
    };

    struct SFrameInformation
    {
        EBlockingStrategy blocking_strategy;
        uint32 block_size; //in inter-channel samples
        ESampleRateStrategy sample_rate_strategy;
        uint32 sample_rate;
        EChannelAssignment channel_assignment;
        uint8 num_channels;
        ESampleSizeStrategy sample_size_strategy;
        uint8 sample_size_bits;
        uint8 crc;
        uint16 data_crc;
    };

private:
    void parse_frame();
    void parse_frame_fake();
    void parse_frame_header(SFrameInformation* frame_info);
    void parse_frame_data(const SFrameInformation& frame_info);
    void parse_frame_footer(SFrameInformation* frame_info);
    void translate_block_size_strategy(uint8 block_size_strategy, bool* need_to_read_block_size_from_bottom, uint32* block_size);
    void translate_sample_rate_strategy(uint8 sample_rate_strategy, bool* need_to_read_sample_rate_from_bottom, uint32* sample_rate);
    void translate_channel_assignment_strategy(uint8 channel_assignment_strategy, EChannelAssignment* channel_assignment, uint8* num_channels);
    void translate_sample_size_strategy(ESampleSizeStrategy sample_size_strategy, uint8* sample_size_bits);
    void read_sample_or_frame_number_string(EBlockingStrategy blocking_strategy);
    int get_total_bytes_from_first_byte(uint8 first_byte);

    //other stuff
    uint32 u8touint(uint8 a, uint8 b, uint8 c = 0, uint8 d = 0);
};
