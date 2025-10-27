#include "ExposureAdjustment.h"
#include <algorithm>

namespace zraw {

ExposureAdjustment::ExposureAdjustment()
    : m_value(0.0f) {
}

void ExposureAdjustment::setValue(float value) {
    m_value = std::clamp(value, minValue(), maxValue());
}

} // namespace zraw
