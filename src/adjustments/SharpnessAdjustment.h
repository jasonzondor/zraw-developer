#pragma once

namespace zraw {

/**
 * Sharpness adjustment
 * Range: 0.0 to 2.0
 */
class SharpnessAdjustment {
public:
    SharpnessAdjustment();
    
    void setValue(float value);
    float value() const { return m_value; }
    
    float minValue() const { return 0.0f; }
    float maxValue() const { return 2.0f; }
    float defaultValue() const { return 0.0f; }

private:
    float m_value;
};

} // namespace zraw
