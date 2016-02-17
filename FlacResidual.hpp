#pragma once

#include <iostream>
#include <cstdint>
#include <cmath>
#include "BitReader.hpp"


class FlacResidual
{
public:
    FlacResidual(BitReader& br, unsigned int blockSize, unsigned int predictorOrder);
    ~FlacResidual();
    void Parse();
    uint32_t* GetResidualData();
private:
    using uint8=uint8_t;
    enum EResidualType
    {
        RESIDUAL_RICE       = 0x0,
        RESIDUAL_RICE2      = 0x1,
        RESIDUAL_RESERVED1  = 0x2,
        RESIDUAL_RESERVED2  = 0x3
    };

    void decode_rice(bool isRice2);

    BitReader& m_br;
    unsigned int m_blockSize;
    unsigned int m_predictorOrder;
    uint32_t* m_residuals;
};
