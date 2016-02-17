#include <cassert>

#include "FlacSubFrame.hpp"
#include "FlacResidual.hpp"

FlacSubFrame::FlacSubFrame(BitReader& br, const FlacDecoder::SFrameInformation& frame_info)
: m_br(br),
m_frameInfo(frame_info),
m_processed(false)
{
    assert((frame_info.sample_size_bits % 8) == 0);
    m_output_samples = (uint32*)malloc(sizeof(uint32) * frame_info.block_size);
}

FlacSubFrame::~FlacSubFrame()
{
    free(m_output_samples);
}

void FlacSubFrame::process()
{
    assert(m_processed == false);
    parse_header();
    std::cerr << "Encountered subframe with type " << translateSubTypeToString(m_subFrameType) << std::endl;
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
        break;
    }
    m_processed = true;
}

uint32_t* FlacSubFrame::getData()
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

    uint8 frame_type_info = m_br.ReadBits(6);
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
    std::cerr << "type_bits are " << std::hex << (uint16_t)type_byte << std::dec << std::endl;
    if (type_byte == 0)
    {
        type = SUBFRAME_CONSTANT;
    }
    else if (type_byte == 1)
    {
        type = SUBFRAME_VERBATIM;
    }
    else if (((type_byte & 0x08) != 0) && ((type_byte & 0x30) == 0))
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
    uint32 sampleData = m_br.ReadInt(m_frameInfo.sample_size_bits);

    for (unsigned int i = 0; i < m_frameInfo.block_size; ++i)
    {
        m_output_samples[i] = sampleData;
    }
}

void FlacSubFrame::parse_verbatim_subframe()
{
    for (unsigned int i = 0; i < m_frameInfo.block_size; ++i)
    {
        m_output_samples[i] = m_br.ReadInt(m_frameInfo.sample_size_bits);
    }
}

void FlacSubFrame::parse_fixed_subframe()
{
    for (unsigned int i = 0; i < m_subFrameOrder; ++i)
    {
        m_output_samples[i] = m_br.ReadInt(m_frameInfo.sample_size_bits);
    }
    int coefficients[5];
    //This curve fitting is super naieve
    switch (m_subFrameOrder)
    {
    case 0:
        break;
    case 1:
        coefficients[0] = m_output_samples[0];
        break;
    case 2:
        //(i+1)=mi + b
        coefficients[0] = m_output_samples[0];
        coefficients[1] = ((double)m_output_samples[1] - coefficients[0]) / m_output_samples[0];
        break;
    case 3:
        std::cerr << "I assumed that there wouldn't be any of these " << (int)m_subFrameOrder << std::endl;
        assert(false);
        break;
    case 4:
        std::cerr << "I assumed that there wouldn't be any of these " << (int)m_subFrameOrder << std::endl;
        assert(false);
        break;
    }
    //Do a whole bunch of calculations to get the samples
    for (unsigned int i = m_subFrameOrder; i < m_frameInfo.block_size; ++i)
    {
        uint32 value = 0;
        for (unsigned int j = 0; j < m_subFrameOrder; ++j)
        {
            value += coefficients[j] * m_output_samples[(i - (j + 1))];
        }
        m_output_samples[i] = value;
    }
    read_and_add_residuals();
}

void FlacSubFrame::parse_lpc_subframe()
{
    for (unsigned int i = 0; i < m_subFrameOrder; ++i)
    {
        m_output_samples[i] = m_br.ReadInt(m_frameInfo.sample_size_bits);
    }
    uint8 quantizedLinearCoefficientPrecisionInBits = m_br.ReadBits(4);
    if (quantizedLinearCoefficientPrecisionInBits == 15)
    {
        std::cerr << "Linear Coefficient bits is invalid, which means something is probably wrong" << std::endl;
    }
    quantizedLinearCoefficientPrecisionInBits += 1;
    assert(quantizedLinearCoefficientPrecisionInBits == 12); //for Roxanne test clip
    std::cerr << "quantizedLinearCoefficientPrecisionInBits = " << (int)quantizedLinearCoefficientPrecisionInBits << std::endl;
    int quantizedLinearCoefficientShiftInBits = signed_twos_complement_to_int(m_br.ReadBits(5), 5);
    assert(quantizedLinearCoefficientShiftInBits > 0);
    int32_t* coefficients = (int32_t*)malloc(sizeof(int32_t) * m_subFrameOrder);
    for (unsigned int i = 0; i < m_subFrameOrder; ++i)
    {
        uint32_t data = m_br.ReadInt(quantizedLinearCoefficientPrecisionInBits);
        coefficients[i] = signed_twos_complement_to_int(data, quantizedLinearCoefficientPrecisionInBits);
        coefficients[i] <<= quantizedLinearCoefficientShiftInBits;
    }
    //Do a whole bunch of calculations to get the samples
    for (unsigned int i = m_subFrameOrder; i < m_frameInfo.block_size; ++i)
    {
        uint32 value = 0;
        for (unsigned int j = 0; j < m_subFrameOrder; ++j)
        {
            value += coefficients[j] * m_output_samples[(i - (j + 1))];
        }
        m_output_samples[i] = value;
    }
    read_and_add_residuals();
    free(coefficients);
}

void FlacSubFrame::read_and_add_residuals()
{
    FlacResidual residual(m_br, m_frameInfo.block_size, m_subFrameOrder);
    residual.Parse();
    uint32_t* residualData = residual.GetResidualData();
    for (unsigned int i = 0; i < m_frameInfo.block_size; ++i)
    {
        m_output_samples[i] += residualData[i];
    }
}

int FlacSubFrame::signed_twos_complement_to_int(uint32 data, uint8 numBits)
{
    uint32 signedBitMask = 1 << (numBits - 1);
    if (numBits == 4)
        assert(signedBitMask == 0x00000008);
    if ((data & signedBitMask) != 0)
    {
        uint32 upperBitsMask = 0xFFFFFFFF << numBits;
        data |= upperBitsMask;
    }
 
    return (int)data;
}
