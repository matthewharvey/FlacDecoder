#pragma once

#include "FlacDecoder.hpp"
#include "BitReader.hpp"


class FlacSubFrame
{
public:
    FlacSubFrame(BitReader& br, const FlacDecoder::SFrameInformation& frame_info);
    ~FlacSubFrame();
    void process();
    uint32_t* getData();
private:
    using uint8=uint8_t;
    using uint32=uint32_t;
    enum EType
    {
        SUBFRAME_CONSTANT = 0x00,
        SUBFRAME_VERBATIM = 0x01,
        SUBFRAME_RESERVED = 0x02,
        SUBFRAME_FIXED    = 0x08,
        SUBFRAME_LPC      = 0x10
    };
    void parse_header();
    void translate_to_type(uint8 type_byte);
    const char* translateSubTypeToString(EType type);
    void parse_constant_subframe();
    void parse_verbatim_subframe();
    void parse_fixed_subframe();
    void parse_lpc_subframe();
    void read_and_add_residuals();
    int signed_twos_complement_to_int(uint32 data, uint8 numBits);
    BitReader& m_br;
    const FlacDecoder::SFrameInformation& m_frameInfo;
    bool m_processed;
    EType m_subFrameType;
    uint8 m_subFrameOrder;
    uint8 m_wastedBitsPerSample;
    uint8 m_bytesPerSample;

    uint32* m_output_samples;
};
