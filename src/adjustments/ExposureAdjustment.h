#pragma once

namespace zraw {

/**
 * Exposure adjustment
 * Range: -3.0 to +3.0 stops
 */
class ExposureAdjustment {
public:
    ExposureAdjustment();
    
    void setValue(float value);
    float value() const { return m_value; }
    
    float minValue() const { return -3.0f; }
    float maxValue() const { return 3.0f; }
    float defaultValue() const { return 0.0f; }

private:
    float m_value;
};

} // namespace zraw
