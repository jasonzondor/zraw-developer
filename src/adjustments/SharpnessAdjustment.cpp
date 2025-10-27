#include "SharpnessAdjustment.h"
#include <algorithm>

namespace zraw {

SharpnessAdjustment::SharpnessAdjustment()
    : m_value(0.0f) {
}

void SharpnessAdjustment::setValue(float value) {
    m_value = std::clamp(value, minValue(), maxValue());
}

} // namespace zraw
