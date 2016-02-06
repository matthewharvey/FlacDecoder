#include <cassert>

#include "FlacSubFrame.hpp"

FlacSubFrame::FlacSubFrame(BitReader& br, const FlacDecoder::SFrameInformation& frame_info)
: m_br(br),
m_frameInfo(frame_info),
m_processed(false)
{
    m_bytesPerSample = ((frame_info.sample_size_bits % 8) == 0) ? (frame_info.sample_size_bits / 8) : ((frame_info.sample_size_bits / 8) + 1);
    m_output_samples = (uint8*)malloc(sizeof(uint8) * frame_info.block_size);
}

FlacSubFrame::~FlacSubFrame()
{
    free(m_output_samples);
}

void FlacSubFrame::process()
{
    assert(m_processed == false);
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
    std::cerr << "Encountered subframe with type " << translateSubTypeToString(m_subFrameType) << std::endl;
    m_processed = true;
}

uint8_t* FlacSubFrame::getData()
{
    assert(m_processed == true);
    return m_output_samples;
}

void FlacSubFrame::parse_header()
{
    uint8 first_bit = m_br.ReadBits(1);
    if (first_bit != 0)
    {
        std::cerr << "Sub Framing error. First bit was 1" << std::endl;
        exit(-1);
    }

    uint8 frame_type_info = m_br.ReadBits(6); //((first_byte >> 1) & 0x3F);
    translate_to_type(frame_type_info);

    /*
     * Now comes something called 'wasted bits per sample, which I don't
     * understand very well. For now, I'm going to simply error out if
     * it's not 0
     */
    uint8 wastedBitsFirstBit = m_br.ReadBits(1);
    if (wastedBitsFirstBit != 0)
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

const char* FlacSubFrame::translateSubTypeToString(EType type)
{
    const char* retval = nullptr;
    switch (type)
    {
    case SUBFRAME_CONSTANT:
        retval = "Constant";
        break;
    case SUBFRAME_VERBATIM:
        retval = "Verbatim";
        break;
    case SUBFRAME_FIXED:
        retval = "Fixed";
        break;
    case SUBFRAME_LPC:
        retval = "LPC";
        break;
    default:
    case SUBFRAME_RESERVED:
        retval = "Reserved";
        break;
    }

    return retval;
}

void FlacSubFrame::parse_constant_subframe()
{
    uint8 sample_data[4];
    //m_in.read((char*)sample_data, m_bytesPerSample);
    for (unsigned int i = 0; i < m_bytesPerSample; ++i)
    {
        sample_data[i] = m_br.ReadBits(8);
    }

    for (unsigned int i = 0; i < m_frameInfo.block_size; ++i)
    {
        for (unsigned int j = 0; j < m_bytesPerSample; ++j)
        {
            m_output_samples[(i + j)] = sample_data[j];
        }
    }
}

void FlacSubFrame::parse_verbatim_subframe()
{
    m_br.ReadByteBlock((char*)m_output_samples, m_bytesPerSample * m_frameInfo.block_size);
}

void FlacSubFrame::parse_fixed_subframe()
{
}

void FlacSubFrame::parse_lpc_subframe()
{
    uint8* warmUpSamples = (uint8*)malloc(sizeof(uint8)*m_subFrameOrder);
    m_br.ReadByteBlock((char*)warmUpSamples, m_subFrameOrder * m_bytesPerSample);
}
