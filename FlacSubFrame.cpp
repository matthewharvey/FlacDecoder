#include "FlacSubFrame.hpp"

FlacSubFrame::FlacSubFrame(const std::istream& in, std::ostream& out, const FlacDecoder::SFrameInformation& frame_info)
: m_in(in),
m_out(out),
m_frameInfo(frame_info)
{
}

void FlacSubFrame::process()
{
}
