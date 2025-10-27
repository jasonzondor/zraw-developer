#include "ContrastAdjustment.h"
#include <algorithm>

namespace zraw {

ContrastAdjustment::ContrastAdjustment()
    : m_value(0.0f) {
}

void ContrastAdjustment::setValue(float value) {
    m_value = std::clamp(value, minValue(), maxValue());
}

} // namespace zraw
