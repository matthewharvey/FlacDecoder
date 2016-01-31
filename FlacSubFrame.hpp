#pragma once

#include "FlacDecoder.hpp"


class FlacSubFrame
{
public:
    FlacSubFrame(std::istream& in, std::ostream& out, const FlacDecoder::SFrameInformation& frame_info);
    void process();
private:
    using uint8=uint8_t;
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
    void parse_constant_subframe();
    void parse_verbatim_subframe();
    void parse_fixed_subframe();
    void parse_lpc_subframe();
    std::istream& m_in;
    std::ostream& m_out;
    const FlacDecoder::SFrameInformation& m_frameInfo;
    EType m_subFrameType;
    uint8 m_subFrameOrder;
    uint8 m_wastedBitsPerSample;
    uint8 m_bytesPerSample;
};
