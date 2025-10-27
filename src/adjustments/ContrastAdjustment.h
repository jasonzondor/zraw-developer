#pragma once

namespace zraw {

/**
 * Contrast adjustment
 * Range: -1.0 to +1.0
 */
class ContrastAdjustment {
public:
    ContrastAdjustment();
    
    void setValue(float value);
    float value() const { return m_value; }
    
    float minValue() const { return -1.0f; }
    float maxValue() const { return 1.0f; }
    float defaultValue() const { return 0.0f; }

private:
    float m_value;
};

} // namespace zraw
