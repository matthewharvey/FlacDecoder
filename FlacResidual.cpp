#include "FlacResidual.hpp"

FlacResidual::FlacResidual(BitReader& br, unsigned int blockSize, unsigned int predictorOrder)
: m_br(br),
m_blockSize(blockSize),
m_predictorOrder(predictorOrder)
{
    m_residuals = (uint32_t*)malloc(sizeof(uint32_t) * blockSize);
}

FlacResidual::~FlacResidual()
{
    free(m_residuals);
}

void FlacResidual::Parse()
{
    EResidualType type = (EResidualType)m_br.ReadBits(2);
    if ((type == RESIDUAL_RESERVED1) || (type == RESIDUAL_RESERVED2))
    {
        std::cerr << "Residual type is in the reserved space, ignoring" << std::endl;
    }
    switch (type)
    {
    case RESIDUAL_RICE:
        std::cerr << "Reaching rice partition" << std::endl;
        decode_rice(false);
        break;
    case RESIDUAL_RICE2:
        std::cerr << "Reaching rice2 partition" << std::endl;
        decode_rice(true);
        break;
    default:
        break;
    }
}

void FlacResidual::decode_rice(bool isRice2)
{
    uint8 partitionOrder = m_br.ReadBits(4);
    unsigned int partitionOrderPower = pow(2, partitionOrder);
    //there will be 2^partitionOrder partitions
    for (unsigned int i = 0; i < partitionOrderPower; ++i)
    {
        uint8 encodingParameter = m_br.ReadBits(isRice2 ? 5 : 4);
        bool isUnencodedCase = (((isRice2 == false) && (encodingParameter == 15)) || ((isRice2 == true) && (encodingParameter == 31)));
        uint8 bitsPerSample = 0; //unused unless partition order == 15
        //special case unencoded residuals
        if (isUnencodedCase == true)
        {
            std::cerr << "The rice codes are unencoded" << std::endl;
            bitsPerSample = m_br.ReadBits(5);
        }

        unsigned int numSamples;
        if (partitionOrder == 0)
        {
            numSamples = m_blockSize - m_predictorOrder;
        }
        else if (i != 0)
        {
            numSamples = m_blockSize / partitionOrderPower;
        }
        else
        {
            numSamples = (m_blockSize / partitionOrderPower) - m_predictorOrder;
        }
        
        if (isUnencodedCase)
        {
            for (unsigned int i = 0; i < numSamples; ++i)
            {
                m_residuals[i] = m_br.ReadInt(bitsPerSample);
            }
        }
        else
        {
            for (unsigned int i = 0; i < numSamples; ++i)
            {
                unsigned int numZeros = 0;
                while (m_br.ReadBits(1) == 0)
                {
                    ++numZeros;
                }
                uint32_t sampleValue = numZeros << encodingParameter;
                uint32_t lastBits = m_br.ReadInt(encodingParameter);
                sampleValue |= lastBits;
                m_residuals[i] = sampleValue;
            }
        }
    }
}

uint32_t* FlacResidual::GetResidualData()
{
    return m_residuals;
}
