#include "FlacSubFrame.hpp"

FlacSubFrame::FlacSubFrame(std::istream& in, std::ostream& out, const FlacDecoder::SFrameInformation& frame_info)
: m_in(in),
m_out(out),
m_frameInfo(frame_info)
{
    m_bytesPerSample = ((frame_info.sample_size_bits % 8) == 0) ? (frame_info.sample_size_bits / 8) : ((frame_info.sample_size_bits / 8) + 1);
}

void FlacSubFrame::process()
{
    parse_header();
    switch (m_subFrameType)
    {
    case SUBFRAME_CONSTANT:
        parse_constant_subframe();
        break;
    case SUBFRAME_VERBATIM:
        parse_verbatim_subframe();
        break;
    case SUBFRAME_FIXED:
        parse_fixed_subframe();
        break;
    case SUBFRAME_LPC:
        parse_lpc_subframe();
        break;
    case SUBFRAME_RESERVED:
        std::cerr << "Encountered subframe with reserved type. Ignoring" << std::endl;
    }
}

void FlacSubFrame::parse_header()
{
    uint8 first_byte;
    m_in.read((char*)&first_byte, 1);
    if ((first_byte & 0x80) != 0)
    {
        std::cerr << "Sub Framing error. First bit was 1" << std::endl;
        exit(-1);
    }

    uint8 frame_type_info = ((first_byte >> 1) & 0x3F);
    translate_to_type(frame_type_info);

    /*
     * Now comes something called 'wasted bits per sample, which I don't
     * understand very well. For now, I'm going to simply error out if
     * it's not 0
     */
    if (first_byte >> 7 != 0)
    {
        std::cerr << "wasted bits per sample is not 0, which this decoder is not able to handle" << std::endl;
        exit(-1);
    }
    m_wastedBitsPerSample = 0;
}

void FlacSubFrame::translate_to_type(uint8 type_byte) //six bits actually
{
    uint8 order = 0;
    EType type = SUBFRAME_RESERVED;
    if (type_byte == 0)
    {
        type = SUBFRAME_CONSTANT;
    }
    else if (type_byte == 1)
    {
        type = SUBFRAME_VERBATIM;
    }
    else if (((type_byte & 0x04) != 0) && ((type_byte | 0x30) == type_byte))
    {
        order = (type_byte & 0x07);
        if (order <= 4)
        {
            type = SUBFRAME_FIXED;
        }
    }
    else if ((type_byte & 0x20) != 0)
    {
        type = SUBFRAME_LPC;
        order = (type_byte & 0x1F);
        order += 1;
    }

    m_subFrameType = type;
    m_subFrameOrder = order;
}

void FlacSubFrame::parse_constant_subframe()
{
    uint8 sample_data[4];
    m_in.read((char*)sample_data, m_bytesPerSample);
}

void FlacSubFrame::parse_verbatim_subframe()
{
}

void FlacSubFrame::parse_fixed_subframe()
{
}

void FlacSubFrame::parse_lpc_subframe()
{
}
