#include "FlacDecoder.hpp"


class FlacSubFrame
{
public:
    FlacSubFrame(const std::istream& in, std::ostream& out, const FlacDecoder::SFrameInformation& frame_info);
    void process();
private:
    const std::istream& m_in;
    const std::ostream& m_out;
    const FlacDecoder::SFrameInformation& m_frameInfo;
};
