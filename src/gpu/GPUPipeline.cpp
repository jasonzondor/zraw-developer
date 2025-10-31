#include "GPUPipeline.h"
#include <iostream>

namespace zraw {

// Vertex shader for fullscreen quad
static const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}
)";

// Fragment shader with color-science-correct adjustments
static const char* fragmentShaderSource = R"(
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D inputTexture;
uniform float exposure;
uniform float contrast;
uniform float sharpness;
uniform float temperature;
uniform float tint;
uniform float highlights;
uniform float shadows;
uniform float vibrance;
uniform float saturation;
uniform float highlightContrast;
uniform float midtoneContrast;
uniform float shadowContrast;
uniform float whites;
uniform float blacks;
uniform vec2 texelSize;

// Output mode: 0=SDR (default), 1=HDR PQ, 2=HDR HLG, 3=Full ACES
uniform int outputMode;

// ============================================================================
// COLOR SCIENCE FUNCTIONS
// ============================================================================

// Rec.2020 luminance weights (wider gamut, more accurate for modern displays)
float luminance(vec3 color) {
    return dot(color, vec3(0.2627, 0.6780, 0.0593));
}

// Linear to Log encoding (perceptually uniform for contrast adjustments)
vec3 linearToLog(vec3 color) {
    const float epsilon = 0.00001;
    return log2(max(color, epsilon)) / 10.0 + 0.5;
}

// Log to Linear decoding
vec3 logToLinear(vec3 logColor) {
    return exp2((logColor - 0.5) * 10.0);
}

// RGB to XYZ color space (D65 illuminant, sRGB primaries)
vec3 rgbToXYZ(vec3 rgb) {
    mat3 m = mat3(
        0.4124564, 0.3575761, 0.1804375,
        0.2126729, 0.7151522, 0.0721750,
        0.0193339, 0.1191920, 0.9503041
    );
    return m * rgb;
}

// XYZ to RGB color space
vec3 xyzToRGB(vec3 xyz) {
    mat3 m = mat3(
         3.2404542, -1.5371385, -0.4985314,
        -0.9692660,  1.8760108,  0.0415560,
         0.0556434, -0.2040259,  1.0572252
    );
    return m * xyz;
}

// XYZ to Lab color space (D65 illuminant)
vec3 xyzToLab(vec3 xyz) {
    // D65 reference white
    vec3 ref = vec3(0.95047, 1.0, 1.08883);
    xyz = xyz / ref;
    
    // Apply Lab transformation
    vec3 f;
    for (int i = 0; i < 3; i++) {
        if (xyz[i] > 0.008856) {
            f[i] = pow(xyz[i], 1.0/3.0);
        } else {
            f[i] = 7.787 * xyz[i] + 16.0/116.0;
        }
    }
    
    float L = 116.0 * f.y - 16.0;
    float a = 500.0 * (f.x - f.y);
    float b = 200.0 * (f.y - f.z);
    
    return vec3(L, a, b);
}

// Lab to XYZ color space
vec3 labToXYZ(vec3 lab) {
    float fy = (lab.x + 16.0) / 116.0;
    float fx = lab.y / 500.0 + fy;
    float fz = fy - lab.z / 200.0;
    
    vec3 xyz;
    xyz.x = (fx > 0.206897) ? pow(fx, 3.0) : (fx - 16.0/116.0) / 7.787;
    xyz.y = (fy > 0.206897) ? pow(fy, 3.0) : (fy - 16.0/116.0) / 7.787;
    xyz.z = (fz > 0.206897) ? pow(fz, 3.0) : (fz - 16.0/116.0) / 7.787;
    
    // D65 reference white
    vec3 ref = vec3(0.95047, 1.0, 1.08883);
    return xyz * ref;
}

// Lab to LCH (Lightness, Chroma, Hue) - perceptually uniform
vec3 labToLCH(vec3 lab) {
    float C = sqrt(lab.y * lab.y + lab.z * lab.z);
    float H = atan(lab.z, lab.y);
    return vec3(lab.x, C, H);
}

// LCH to Lab
vec3 lchToLab(vec3 lch) {
    float a = lch.y * cos(lch.z);
    float b = lch.y * sin(lch.z);
    return vec3(lch.x, a, b);
}

// Complete RGB to LCH pipeline (for saturation adjustments)
vec3 rgbToLCH(vec3 rgb) {
    return labToLCH(xyzToLab(rgbToXYZ(rgb)));
}

// Complete LCH to RGB pipeline
vec3 lchToRGB(vec3 lch) {
    return xyzToRGB(labToXYZ(lchToLab(lch)));
}

// Bradford chromatic adaptation transform (industry standard)
vec3 bradfordAdaptation(vec3 color, vec3 srcWhite, vec3 dstWhite) {
    // Bradford cone response matrix
    mat3 bradford = mat3(
         0.8951,  0.2664, -0.1614,
        -0.7502,  1.7135,  0.0367,
         0.0389, -0.0685,  1.0296
    );
    mat3 bradfordInv = mat3(
         0.9869929, -0.1470543,  0.1599627,
         0.4323053,  0.5183603,  0.0492912,
        -0.0085287,  0.0400428,  0.9684867
    );
    
    // Convert to cone response domain
    vec3 srcCone = bradford * srcWhite;
    vec3 dstCone = bradford * dstWhite;
    
    // Calculate adaptation matrix
    vec3 scale = dstCone / srcCone;
    mat3 adaptMat = bradfordInv * mat3(
        scale.x, 0.0, 0.0,
        0.0, scale.y, 0.0,
        0.0, 0.0, scale.z
    ) * bradford;
    
    return adaptMat * color;
}

// White balance - relative adjustment from camera WB
// temp: relative adjustment (-100 to +100), 0 = neutral (camera WB)
// tint: green-magenta shift (-100 to +100)
vec3 applyWhiteBalance(vec3 color, float temp, float tint) {
    // If neutral, skip
    if (abs(temp) < 0.1 && abs(tint) < 0.1) {
        return color;
    }
    
    // Simple RGB multiplier approach (like Lightroom)
    // This is more intuitive and predictable than Bradford
    float tempFactor = temp / 100.0;
    float tintFactor = tint / 100.0;
    
    // Temperature adjustment
    // Positive = warmer (boost red/yellow, reduce blue)
    // Negative = cooler (boost blue, reduce red/yellow)
    vec3 result = color;
    result.r *= 1.0 + tempFactor * 0.5;      // Red channel
    result.b *= 1.0 - tempFactor * 0.5;      // Blue channel
    
    // Tint adjustment  
    // Positive = more green, Negative = more magenta
    result.g *= 1.0 + tintFactor * 0.3;      // Green channel
    
    return result;
}

// ============================================================================
// PARAMETRIC TONE CURVE FUNCTIONS
// ============================================================================

// Parametric tone curve for highlights/shadows
// Uses a smooth S-curve that adjusts based on the control value
// x: input luminance (0-1)
// control: adjustment amount (-1 to +1)
// center: where the curve pivots (0-1)
// width: how wide the affected range is
float parametricCurve(float x, float control, float center, float width) {
    if (abs(control) < 0.001) return x;
    
    // Normalized distance from center
    float dist = (x - center) / width;
    
    // Smooth falloff using a modified sigmoid
    // This creates a natural S-curve
    float weight = 1.0 / (1.0 + exp(-dist * 6.0));
    
    // Apply the adjustment with smooth blending
    float adjustment = control * weight;
    
    // Use a power function for natural-looking tone adjustment
    // Invert the gamma so positive control brightens
    float gamma = 1.0 / (1.0 + adjustment);
    return pow(x, gamma);
}

// Parametric curve for whites/blacks (affects endpoints)
// This adjusts where the curve clips to white/black
float parametricEndpoint(float x, float control, bool isWhite) {
    if (abs(control) < 0.001) return x;
    
    if (isWhite) {
        // Whites: compress or expand the upper end
        // Positive control = brighter whites (expand)
        // Negative control = darker whites (compress)
        float pivot = 0.18;  // 18% gray
        float scale = 1.0 + control * 0.5;
        return pivot + (x - pivot) * scale;
    } else {
        // Blacks: compress or expand the lower end
        float pivot = 0.18;
        float scale = 1.0 + control * 0.5;
        return pivot + (x - pivot) * scale;
    }
}

// Apply parametric curve to RGB while preserving color ratios
vec3 applyParametricToRGB(vec3 color, float control, float center, float width) {
    float lum = luminance(color);
    if (lum < 0.00001) return color;
    
    // Apply curve to luminance
    float newLum = parametricCurve(lum, control, center, width);
    
    // Scale RGB proportionally
    vec3 newColor = color * (newLum / lum);
    
    // Per-channel soft clipping
    const float threshold = 1.0;
    const float shoulder = 0.2;
    
    for (int i = 0; i < 3; i++) {
        if (newColor[i] > threshold) {
            float excess = newColor[i] - threshold;
            float compressed = threshold + shoulder * (excess / (excess + shoulder));
            newColor[i] = compressed;
        }
    }
    
    return newColor;
}

// ============================================================================
// ACES TONE MAPPING
// ============================================================================

// ACES AP0 to AP1 (ACEScg) conversion
vec3 acesAP0toAP1(vec3 color) {
    mat3 m = mat3(
         1.4514393161, -0.2365107469, -0.2149285693,
        -0.0765537734,  1.1762296998, -0.0996759264,
         0.0083161484, -0.0060324498,  0.9977163014
    );
    return m * color;
}

// ACES AP1 to AP0 conversion
vec3 acesAP1toAP0(vec3 color) {
    mat3 m = mat3(
         0.6954522414,  0.1406786965,  0.1638690622,
         0.0447945634,  0.8596711185,  0.0955343182,
        -0.0055258826,  0.0040252103,  1.0015006723
    );
    return m * color;
}

// ACES RRT (Reference Rendering Transform) + ODT (Output Device Transform)
// This is a simplified version of the full ACES pipeline
vec3 acesToneMap(vec3 color) {
    // Convert to ACES AP1 (ACEScg working space)
    color = acesAP0toAP1(color);
    
    // RRT Sweeteners - subtle adjustments before tone mapping
    // Glow (bloom in highlights)
    float glow = max(0.0, luminance(color) - 0.5);
    color = color + vec3(glow * 0.1);
    
    // Red modifier (reduce red in highlights)
    float redMod = clamp(color.r / (color.r + color.g + color.b + 0.001), 0.0, 1.0);
    color.r = mix(color.r, color.r * 0.85, redMod * 0.3);
    
    // ACES fitted curve (Krzysztof Narkowicz approximation)
    // This is the heart of ACES - the "S-curve" tone mapping
    vec3 a = color * (color + 0.0245786) - 0.000090537;
    vec3 b = color * (0.983729 * color + 0.4329510) + 0.238081;
    color = a / b;
    
    // Convert back to linear RGB
    color = acesAP1toAP0(color);
    
    return color;
}

// Filmic tone mapping (alternative to ACES, slightly different look)
vec3 filmicToneMap(vec3 color) {
    // Uncharted 2 tone mapping operator
    vec3 x = max(vec3(0.0), color - 0.004);
    return (x * (6.2 * x + 0.5)) / (x * (6.2 * x + 1.7) + 0.06);
}

// ============================================================================
// GAMUT MAPPING
// ============================================================================

// Check if color is within sRGB gamut
bool isInGamut(vec3 color) {
    return color.r >= 0.0 && color.r <= 1.0 &&
           color.g >= 0.0 && color.g <= 1.0 &&
           color.b >= 0.0 && color.b <= 1.0;
}

// Calculate distance from gamut boundary
float gamutDistance(vec3 color) {
    float maxOver = max(max(color.r - 1.0, color.g - 1.0), color.b - 1.0);
    float maxUnder = max(max(-color.r, -color.g), -color.b);
    return max(maxOver, maxUnder);
}

// Soft gamut compression (preserves hue, compresses chroma smoothly)
vec3 softGamutCompress(vec3 color) {
    // Convert to LCH for hue-preserving compression
    vec3 lch = rgbToLCH(color);
    
    // Calculate how far out of gamut we are
    float dist = gamutDistance(color);
    
    if (dist > 0.0) {
        // Compress chroma based on distance from gamut
        // Use smooth compression curve
        float compressionFactor = 1.0 / (1.0 + dist * 2.0);
        lch.y *= compressionFactor;
        
        // Convert back to RGB
        color = lchToRGB(lch);
    }
    
    return color;
}

// Perceptual gamut mapping (ACES-style)
// Compresses out-of-gamut colors while preserving in-gamut colors
vec3 perceptualGamutMap(vec3 color) {
    // Calculate luminance
    float lum = luminance(color);
    
    // Normalize by luminance to get chromaticity
    vec3 chroma = (lum > 0.0001) ? color / lum : vec3(0.0);
    
    // Find maximum channel
    float maxChroma = max(max(chroma.r, chroma.g), chroma.b);
    
    // If out of gamut, compress
    if (maxChroma > 1.0) {
        // Smooth compression using hyperbolic tangent
        float compressed = 1.0 + tanh((maxChroma - 1.0) * 2.0) * 0.2;
        chroma = chroma * (compressed / maxChroma);
    }
    
    // Reconstruct color
    return chroma * lum;
}

// Rec.2020 to sRGB gamut mapping
// For wide gamut sources that need to be displayed on sRGB
vec3 rec2020ToSRGB(vec3 color) {
    // Rec.2020 to XYZ
    mat3 rec2020ToXYZ = mat3(
        0.6369580, 0.1446169, 0.1688810,
        0.2627002, 0.6779981, 0.0593017,
        0.0000000, 0.0280727, 1.0609851
    );
    
    // XYZ to sRGB
    mat3 xyzToSRGB = mat3(
         3.2404542, -1.5371385, -0.4985314,
        -0.9692660,  1.8760108,  0.0415560,
         0.0556434, -0.2040259,  1.0572252
    );
    
    vec3 xyz = rec2020ToXYZ * color;
    vec3 srgb = xyzToSRGB * xyz;
    
    // Apply gamut compression if needed
    if (!isInGamut(srgb)) {
        srgb = perceptualGamutMap(srgb);
    }
    
    return srgb;
}

// Adaptive gamut mapping (combines multiple techniques)
vec3 adaptiveGamutMap(vec3 color) {
    // First check if we're in gamut
    if (isInGamut(color)) {
        return color;
    }
    
    // Calculate how far out of gamut
    float dist = gamutDistance(color);
    
    // For slightly out of gamut colors, use soft compression
    if (dist < 0.2) {
        return softGamutCompress(color);
    }
    
    // For very out of gamut colors, use perceptual mapping
    return perceptualGamutMap(color);
}

// Gamut expansion for HDR displays (future-proofing)
vec3 expandToP3(vec3 color) {
    // sRGB to Display P3 (wider gamut)
    // This is for future HDR/wide gamut display support
    mat3 srgbToP3 = mat3(
        0.8224621, 0.1775380, 0.0000000,
        0.0331941, 0.9668058, 0.0000000,
        0.0170827, 0.0723974, 0.9105199
    );
    return srgbToP3 * color;
}

// ============================================================================
// HDR OUTPUT TRANSFORMS (PQ and HLG)
// ============================================================================

// PQ (Perceptual Quantizer) - SMPTE ST 2084
// Used for HDR10, Dolby Vision
// Maps [0, 10000] nits to [0, 1]
vec3 linearToPQ(vec3 linear) {
    // PQ constants
    const float m1 = 0.1593017578125;      // 2610 / 16384
    const float m2 = 78.84375;              // 2523 / 32
    const float c1 = 0.8359375;             // 3424 / 4096
    const float c2 = 18.8515625;            // 2413 / 128
    const float c3 = 18.6875;               // 2392 / 128
    
    // Normalize to 10,000 nits reference
    vec3 Y = linear / 10000.0;
    
    // Apply PQ curve
    vec3 Ym1 = pow(Y, vec3(m1));
    vec3 pq = pow((c1 + c2 * Ym1) / (1.0 + c3 * Ym1), vec3(m2));
    
    return pq;
}

// Inverse PQ (for display)
vec3 pqToLinear(vec3 pq) {
    const float m1 = 0.1593017578125;
    const float m2 = 78.84375;
    const float c1 = 0.8359375;
    const float c2 = 18.8515625;
    const float c3 = 18.6875;
    
    vec3 Ym2 = pow(pq, vec3(1.0 / m2));
    vec3 Y = pow(max(Ym2 - c1, 0.0) / (c2 - c3 * Ym2), vec3(1.0 / m1));
    
    return Y * 10000.0;
}

// HLG (Hybrid Log-Gamma) - ITU-R BT.2100
// Used for broadcast HDR (BBC, NHK)
// Backward compatible with SDR displays
float linearToHLG(float linear) {
    const float a = 0.17883277;
    const float b = 0.28466892;  // 1 - 4a
    const float c = 0.55991073;  // 0.5 - a * ln(4a)
    
    if (linear <= 1.0 / 12.0) {
        return sqrt(3.0 * linear);
    } else {
        return a * log(12.0 * linear - b) + c;
    }
}

vec3 linearToHLG(vec3 linear) {
    return vec3(
        linearToHLG(linear.r),
        linearToHLG(linear.g),
        linearToHLG(linear.b)
    );
}

// Inverse HLG
float hlgToLinear(float hlg) {
    const float a = 0.17883277;
    const float b = 0.28466892;
    const float c = 0.55991073;
    
    if (hlg <= 0.5) {
        return (hlg * hlg) / 3.0;
    } else {
        return (exp((hlg - c) / a) + b) / 12.0;
    }
}

vec3 hlgToLinear(vec3 hlg) {
    return vec3(
        hlgToLinear(hlg.r),
        hlgToLinear(hlg.g),
        hlgToLinear(hlg.b)
    );
}

// ============================================================================
// FULL ACES WORKFLOW (AP0 → AP1 → RRT → ODT)
// ============================================================================

// sRGB to ACES AP0 (wide gamut working space)
vec3 srgbToACES(vec3 color) {
    mat3 m = mat3(
        0.4397010, 0.3829780, 0.1773350,
        0.0897923, 0.8134230, 0.0967616,
        0.0175440, 0.1115440, 0.8707040
    );
    return m * color;
}

// ACES AP0 to sRGB
vec3 acesToSRGB(vec3 color) {
    mat3 m = mat3(
         2.5216537, -1.1368990, -0.3847517,
        -0.2764811,  1.3722640, -0.0957829,
        -0.0153780, -0.1529744,  1.1683520
    );
    return m * color;
}

// Full ACES RRT (Reference Rendering Transform)
// More complete than our simplified version
vec3 acesRRT(vec3 color) {
    // Convert to ACES AP1
    color = acesAP0toAP1(color);
    
    // RRT Sweeteners
    // 1. Glow module (adds bloom to highlights)
    float glowGainIn = luminance(color);
    float glowGain = max(0.0, glowGainIn - 0.5) * 0.12;
    color = color + vec3(glowGain);
    
    // 2. Red modifier (desaturate reds in highlights)
    float redRatio = color.r / (color.r + color.g + color.b + 0.001);
    float redMod = smoothstep(0.5, 1.0, redRatio);
    color.r = mix(color.r, color.r * 0.82, redMod * 0.4);
    
    // 3. Gamut compression (pre-tone mapping)
    float lum = luminance(color);
    vec3 chroma = (lum > 0.0001) ? color / lum : vec3(1.0);
    float maxChroma = max(max(chroma.r, chroma.g), chroma.b);
    if (maxChroma > 1.0) {
        float compress = 1.0 + log(maxChroma) * 0.3;
        chroma = chroma * (compress / maxChroma);
        color = chroma * lum;
    }
    
    // 4. Tone mapping curve (Krzysztof Narkowicz fit)
    vec3 a = color * (color + 0.0245786) - 0.000090537;
    vec3 b = color * (0.983729 * color + 0.4329510) + 0.238081;
    color = a / b;
    
    // Convert back to AP0
    color = acesAP1toAP0(color);
    
    return color;
}

// ACES ODT (Output Device Transform) for sRGB
vec3 acesODTsRGB(vec3 color) {
    // This is applied after RRT
    // Handles final conversion to display space
    
    // Convert to AP1
    color = acesAP0toAP1(color);
    
    // Apply sRGB rendering primaries transform
    mat3 ap1ToSRGB = mat3(
         1.7050509, -0.6217921, -0.0832588,
        -0.1302597,  1.1408027, -0.0105430,
        -0.0240033, -0.1289687,  1.1529720
    );
    color = ap1ToSRGB * color;
    
    // Apply gamma (sRGB EOTF inverse)
    // Note: We're working in linear, so this is for reference
    // Actual gamma is applied by display
    
    return color;
}

// Complete ACES pipeline (AP0 input → sRGB output)
vec3 fullACESPipeline(vec3 color) {
    // 1. Input: Assume we're in linear sRGB from RAW processing
    //    Convert to ACES AP0 (scene-referred, wide gamut)
    color = srgbToACES(color);
    
    // 2. RRT: Reference Rendering Transform
    //    Scene-referred → output-referred
    color = acesRRT(color);
    
    // 3. ODT: Output Device Transform for sRGB
    //    Output-referred → display-referred
    color = acesODTsRGB(color);
    
    return color;
}

void main() {
    // ========================================================================
    // PROPER COLOR SCIENCE PIPELINE ORDER
    // ========================================================================
    
    // 1. Sample input (linear RGB from RAW processing)
    vec3 color = texture(inputTexture, TexCoord).rgb;
    
    // 2. White Balance FIRST (in linear space, before exposure)
    //    Camera WB already applied, this is for fine-tuning
    //    Temperature is relative (-100 to +100), 0 = neutral
    if (abs(temperature) > 0.1 || abs(tint) > 0.1) {
        color = applyWhiteBalance(color, temperature, tint);
    }
    
    // 3. Exposure with per-channel highlight compression
    //    Apply exposure, then compress each channel independently
    //    This prevents color shifts from differential clipping
    if (abs(exposure) > 0.01) {
        // Apply exposure
        color *= pow(2.0, exposure);
        
        // Per-channel soft clipping to prevent color shifts
        // This is key: compress R, G, B independently so they don't clip at different rates
        const float threshold = 1.0;
        const float shoulder = 0.2;  // Shoulder width for smooth rolloff
        
        for (int i = 0; i < 3; i++) {
            if (color[i] > threshold) {
                // Smooth shoulder compression
                // Uses a modified Reinhard curve for gentle rolloff
                float excess = color[i] - threshold;
                float compressed = threshold + shoulder * (excess / (excess + shoulder));
                color[i] = compressed;
            }
        }
    }
    
    // 3.5. Whites and Blacks adjustment (parametric curve)
    //      Whites: adjusts the upper tonal range with smooth curve
    //      Blacks: adjusts the lower tonal range with smooth curve
    if (abs(whites) > 0.1) {
        // Whites: parametric curve centered on bright tones
        float control = whites / 100.0;
        color = applyParametricToRGB(color, control, 0.75, 0.3);
    }
    
    if (abs(blacks) > 0.1) {
        // Blacks: parametric curve centered on dark tones
        float control = blacks / 100.0;
        color = applyParametricToRGB(color, control, 0.25, 0.3);
    }
    
    // 4. Highlights and Shadows Recovery (parametric curve)
    //      Highlights: targets very bright areas with narrow curve
    //      Shadows: targets very dark areas with narrow curve
    if (abs(highlights) > 0.1) {
        // Highlights: narrow parametric curve centered on bright areas
        float control = highlights / 100.0;
        color = applyParametricToRGB(color, control, 0.9, 0.15);
    }
    
    if (abs(shadows) > 0.1) {
        // Shadows: very narrow parametric curve centered on dark areas
        // Use tighter width to only affect true shadows
        float control = shadows / 100.0;
        color = applyParametricToRGB(color, control, 0.08, 0.1);
    }
    
    // 5. Global Contrast (in LOG space for perceptual uniformity)
    if (abs(contrast) > 0.01) {
        vec3 logColor = linearToLog(color);
        // Apply contrast around middle gray (0.5 in log space = 18% gray)
        logColor = (logColor - 0.5) * (1.0 + contrast) + 0.5;
        color = logToLinear(logColor);
    }
    
    // 6. Local Contrast / Tone-specific contrast (in log space)
    if (abs(highlightContrast) > 0.1 || abs(midtoneContrast) > 0.1 || abs(shadowContrast) > 0.1) {
        float lum = luminance(color);
        
        // Better zone separation using smooth transitions
        float highlightMask = smoothstep(0.5, 0.9, lum);
        float shadowMask = smoothstep(0.5, 0.1, lum);
        float midtoneMask = 1.0 - highlightMask - shadowMask;
        
        // Apply in log space for better behavior
        vec3 logColor = linearToLog(color);
        float localContrast = 
            (highlightContrast / 100.0) * highlightMask +
            (midtoneContrast / 100.0) * midtoneMask +
            (shadowContrast / 100.0) * shadowMask;
        
        logColor = (logColor - 0.5) * (1.0 + localContrast) + 0.5;
        color = logToLinear(logColor);
    }
    
    // 7. Saturation and Vibrance (in LCH space - perceptually uniform, no hue shifts)
    if (abs(saturation) > 0.1 || abs(vibrance) > 0.1) {
        // Convert to LCH (Lightness, Chroma, Hue)
        vec3 lch = rgbToLCH(max(color, 0.0));
        
        // Saturation: adjust chroma directly
        if (abs(saturation) > 0.1) {
            lch.y *= 1.0 + (saturation / 100.0);
        }
        
        // Vibrance: non-linear chroma boost (affects low-chroma colors more)
        if (abs(vibrance) > 0.1) {
            float vibranceFactor = vibrance / 100.0;
            // Normalize chroma to 0-1 range (typical max chroma ~130)
            float chromaNorm = clamp(lch.y / 130.0, 0.0, 1.0);
            float vibranceMask = 1.0 - chromaNorm; // Boost muted colors more
            lch.y += vibranceFactor * vibranceMask * 50.0;
        }
        
        // Clamp chroma to reasonable range
        lch.y = max(lch.y, 0.0);
        
        // Convert back to RGB
        color = lchToRGB(lch);
    }
    
    // 8. Output Transform (tone mapping + color space conversion)
    //    Apply FIRST to get display-ready image, then sharpen
    
    if (outputMode == 3) {
        // Full ACES workflow (AP0 → RRT → ODT → sRGB)
        color = fullACESPipeline(max(color, 0.0));
        color = adaptiveGamutMap(color);
        
    } else if (outputMode == 1) {
        // HDR PQ (Perceptual Quantizer) for HDR10/Dolby Vision
        color = acesToneMap(max(color, 0.0));
        color = adaptiveGamutMap(color);
        color = linearToPQ(color * 100.0);
        
    } else if (outputMode == 2) {
        // HDR HLG (Hybrid Log-Gamma) for broadcast
        color = acesToneMap(max(color, 0.0));
        color = adaptiveGamutMap(color);
        color = linearToHLG(color);
        
    } else {
        // Default: SDR output with gentle tone curve
        // Simple shoulder to prevent clipping, but much less aggressive than ACES
        color = max(color, 0.0);
        
        // Gentle highlight rolloff (Reinhard-style but softer)
        // Only compress values above 1.0, leave everything below untouched
        float lum = luminance(color);
        if (lum > 1.0) {
            float newLum = 1.0 + (lum - 1.0) / (1.0 + (lum - 1.0));
            color = color * (newLum / lum);
        }
        
        color = adaptiveGamutMap(color);
    }
    
    // 9. RAW Sharpening (Deconvolution-based, halo-free)
    //    Uses high-frequency enhancement in luminance only
    if (sharpness > 0.001) {
        // Convert to luminance for sharpening
        float centerLum = luminance(color);
        
        // Sample 3x3 neighborhood in luminance
        float l00 = luminance(texture(inputTexture, TexCoord + vec2(-1, -1) * texelSize).rgb);
        float l01 = luminance(texture(inputTexture, TexCoord + vec2( 0, -1) * texelSize).rgb);
        float l02 = luminance(texture(inputTexture, TexCoord + vec2( 1, -1) * texelSize).rgb);
        float l10 = luminance(texture(inputTexture, TexCoord + vec2(-1,  0) * texelSize).rgb);
        float l11 = centerLum;
        float l12 = luminance(texture(inputTexture, TexCoord + vec2( 1,  0) * texelSize).rgb);
        float l20 = luminance(texture(inputTexture, TexCoord + vec2(-1,  1) * texelSize).rgb);
        float l21 = luminance(texture(inputTexture, TexCoord + vec2( 0,  1) * texelSize).rgb);
        float l22 = luminance(texture(inputTexture, TexCoord + vec2( 1,  1) * texelSize).rgb);
        
        // Calculate local variance (measure of detail)
        float mean = (l00 + l01 + l02 + l10 + l11 + l12 + l20 + l21 + l22) / 9.0;
        float variance = (
            pow(l00 - mean, 2.0) + pow(l01 - mean, 2.0) + pow(l02 - mean, 2.0) +
            pow(l10 - mean, 2.0) + pow(l11 - mean, 2.0) + pow(l12 - mean, 2.0) +
            pow(l20 - mean, 2.0) + pow(l21 - mean, 2.0) + pow(l22 - mean, 2.0)
        ) / 9.0;
        
        // Detail mask: sharpen high-variance areas (edges/details), not flat areas
        float detailMask = smoothstep(0.0001, 0.001, variance);
        
        // High-pass filter (extracts fine detail)
        // Gaussian blur approximation
        float blurred = (
            l00 * 0.0625 + l01 * 0.125 + l02 * 0.0625 +
            l10 * 0.125  + l11 * 0.25  + l12 * 0.125 +
            l20 * 0.0625 + l21 * 0.125 + l22 * 0.0625
        );
        
        // Extract high-frequency detail
        float detail = l11 - blurred;
        
        // Adaptive sharpening amount based on local contrast
        float localContrast = max(max(abs(l11 - l01), abs(l11 - l21)), 
                                  max(abs(l11 - l10), abs(l11 - l12)));
        float adaptiveAmount = mix(0.5, 1.0, smoothstep(0.01, 0.1, localContrast));
        
        // Protection masks
        float highlightProtection = 1.0 - smoothstep(0.9, 1.0, centerLum);
        float shadowProtection = smoothstep(0.02, 0.1, centerLum);
        
        // Apply sharpening to luminance
        float sharpenAmount = sharpness * 0.5 * detailMask * adaptiveAmount * 
                             highlightProtection * shadowProtection;
        float sharpenedLum = centerLum + detail * sharpenAmount;
        sharpenedLum = max(sharpenedLum, 0.0);
        
        // Reconstruct color with sharpened luminance (preserves hue and saturation)
        if (centerLum > 0.0001) {
            color = color * (sharpenedLum / centerLum);
        }
    }
    
    // 10. Final clamp to valid display range [0, 1]
    //     Should be mostly in-gamut after processing
    color = clamp(color, 0.0, 1.0);
    
    FragColor = vec4(color, 1.0);
}
)";

GPUPipeline::GPUPipeline()
    : m_context(std::make_unique<GLContext>()),
      m_shader(std::make_unique<ShaderProgram>()),
      m_width(0), m_height(0),
      m_exposure(0.0f), m_contrast(0.0f), m_sharpness(0.0f),
      m_temperature(0.0f), m_tint(0.0f),  // 0 = neutral (camera WB)
      m_highlights(0.0f), m_shadows(0.0f),
      m_vibrance(0.0f), m_saturation(0.0f),
      m_highlightContrast(0.0f), m_midtoneContrast(0.0f), m_shadowContrast(0.0f),
      m_whites(0.0f), m_blacks(0.0f),
      m_outputMode(0),  // Default to SDR
      m_bypassAdjustments(false),
      m_vao(0), m_vbo(0) {
}

GPUPipeline::~GPUPipeline() {
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
}

bool GPUPipeline::initialize() {
    initializeOpenGLFunctions();
    
    if (!m_context->initialize()) {
        return false;
    }
    
    if (!createShaders()) {
        return false;
    }
    
    if (!createBuffers()) {
        return false;
    }
    
    return true;
}

bool GPUPipeline::createShaders() {
    if (!m_shader->loadVertexShader(vertexShaderSource)) {
        std::cerr << m_shader->lastError() << std::endl;
        return false;
    }
    
    if (!m_shader->loadFragmentShader(fragmentShaderSource)) {
        std::cerr << m_shader->lastError() << std::endl;
        return false;
    }
    
    if (!m_shader->link()) {
        std::cerr << m_shader->lastError() << std::endl;
        return false;
    }
    
    return true;
}

bool GPUPipeline::createBuffers() {
    // Fullscreen quad vertices
    float vertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // TexCoord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
    
    return true;
}

bool GPUPipeline::uploadImage(std::shared_ptr<ImageBuffer> buffer) {
    if (!buffer || buffer->width() == 0 || buffer->height() == 0) {
        std::cerr << "Invalid image buffer" << std::endl;
        return false;
    }
    
    m_width = buffer->width();
    m_height = buffer->height();
    
    // Create input texture (for processing)
    m_inputTexture = std::make_unique<QOpenGLTexture>(QOpenGLTexture::Target2D);
    m_inputTexture->setFormat(QOpenGLTexture::RGB16_UNorm);
    m_inputTexture->setSize(m_width, m_height);
    m_inputTexture->allocateStorage();
    m_inputTexture->setData(QOpenGLTexture::RGB, QOpenGLTexture::UInt16, buffer->data());
    m_inputTexture->setMinificationFilter(QOpenGLTexture::Linear);
    m_inputTexture->setMagnificationFilter(QOpenGLTexture::Linear);
    m_inputTexture->setWrapMode(QOpenGLTexture::ClampToEdge);
    
    // Create original texture (for before/after comparison)
    // This stores the processed output with zero adjustments
    m_originalTexture = std::make_unique<QOpenGLTexture>(QOpenGLTexture::Target2D);
    m_originalTexture->setFormat(QOpenGLTexture::RGB16_UNorm);
    m_originalTexture->setSize(m_width, m_height);
    m_originalTexture->allocateStorage();
    m_originalTexture->setMinificationFilter(QOpenGLTexture::Linear);
    m_originalTexture->setMagnificationFilter(QOpenGLTexture::Linear);
    m_originalTexture->setWrapMode(QOpenGLTexture::ClampToEdge);
    
    // Create framebuffer for output
    QOpenGLFramebufferObjectFormat format;
    format.setInternalTextureFormat(GL_RGB16);
    m_fbo = std::make_unique<QOpenGLFramebufferObject>(m_width, m_height, format);
    
    // Process once with zero adjustments to create the "original" reference
    bool oldBypass = m_bypassAdjustments;
    m_bypassAdjustments = true;
    process();
    
    // Copy the framebuffer result to original texture
    m_fbo->bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_originalTexture->textureId());
    // Use glCopyTexSubImage2D since we already allocated storage
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, m_width, m_height);
    glBindTexture(GL_TEXTURE_2D, 0);
    m_fbo->release();
    
    m_bypassAdjustments = oldBypass;
    
    std::cout << "Created original texture: " << m_originalTexture->textureId() << std::endl;
    
    return true;
}

void GPUPipeline::setExposure(float exposure) {
    m_exposure = exposure;
}

void GPUPipeline::setContrast(float contrast) {
    m_contrast = contrast;
}

void GPUPipeline::setSharpness(float sharpness) {
    m_sharpness = sharpness;
}

void GPUPipeline::setTemperature(float temperature) {
    m_temperature = temperature;
}

void GPUPipeline::setTint(float tint) {
    m_tint = tint;
}

void GPUPipeline::setHighlights(float highlights) {
    m_highlights = highlights;
}

void GPUPipeline::setShadows(float shadows) {
    m_shadows = shadows;
}

void GPUPipeline::setVibrance(float vibrance) {
    m_vibrance = vibrance;
}

void GPUPipeline::setSaturation(float saturation) {
    m_saturation = saturation;
}

void GPUPipeline::setHighlightContrast(float highlightContrast) {
    m_highlightContrast = highlightContrast;
}

void GPUPipeline::setMidtoneContrast(float midtoneContrast) {
    m_midtoneContrast = midtoneContrast;
}

void GPUPipeline::setShadowContrast(float shadowContrast) {
    m_shadowContrast = shadowContrast;
}

void GPUPipeline::setWhites(float whites) {
    m_whites = whites;
}

void GPUPipeline::setBlacks(float blacks) {
    m_blacks = blacks;
}

void GPUPipeline::setOutputMode(int mode) {
    m_outputMode = mode;
}

void GPUPipeline::setBypassAdjustments(bool bypass) {
    m_bypassAdjustments = bypass;
}

bool GPUPipeline::process() {
    if (!m_inputTexture || !m_fbo) {
        std::cerr << "Pipeline not ready for processing" << std::endl;
        return false;
    }
    
    // Bind framebuffer
    m_fbo->bind();
    
    // Set viewport
    glViewport(0, 0, m_width, m_height);
    
    // Clear
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Use shader
    m_shader->bind();
    
    // Set uniforms (bypass all adjustments if showing "before")
    m_shader->setUniform("inputTexture", 0);
    m_shader->setUniform("exposure", m_bypassAdjustments ? 0.0f : m_exposure);
    m_shader->setUniform("contrast", m_bypassAdjustments ? 0.0f : m_contrast);
    m_shader->setUniform("sharpness", m_bypassAdjustments ? 0.0f : m_sharpness);
    m_shader->setUniform("temperature", m_bypassAdjustments ? 0.0f : m_temperature);
    m_shader->setUniform("tint", m_bypassAdjustments ? 0.0f : m_tint);
    m_shader->setUniform("highlights", m_bypassAdjustments ? 0.0f : m_highlights);
    m_shader->setUniform("shadows", m_bypassAdjustments ? 0.0f : m_shadows);
    m_shader->setUniform("vibrance", m_bypassAdjustments ? 0.0f : m_vibrance);
    m_shader->setUniform("saturation", m_bypassAdjustments ? 0.0f : m_saturation);
    m_shader->setUniform("highlightContrast", m_bypassAdjustments ? 0.0f : m_highlightContrast);
    m_shader->setUniform("midtoneContrast", m_bypassAdjustments ? 0.0f : m_midtoneContrast);
    m_shader->setUniform("shadowContrast", m_bypassAdjustments ? 0.0f : m_shadowContrast);
    m_shader->setUniform("whites", m_bypassAdjustments ? 0.0f : m_whites);
    m_shader->setUniform("blacks", m_bypassAdjustments ? 0.0f : m_blacks);
    m_shader->setUniform("texelSize", 1.0f / m_width, 1.0f / m_height);
    m_shader->setUniform("outputMode", m_outputMode);
    
    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    m_inputTexture->bind();
    
    // Render quad
    renderQuad();
    
    // Cleanup
    m_inputTexture->release();
    m_shader->release();
    m_fbo->release();
    
    return true;
}

void GPUPipeline::renderQuad() {
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

std::shared_ptr<ImageBuffer> GPUPipeline::downloadImage() {
    if (!m_fbo) {
        return nullptr;
    }
    
    auto buffer = std::make_shared<ImageBuffer>(m_width, m_height, 3);
    
    m_fbo->bind();
    glReadPixels(0, 0, m_width, m_height, GL_RGB, GL_UNSIGNED_SHORT, buffer->data());
    m_fbo->release();
    
    return buffer;
}

GLuint GPUPipeline::getOutputTexture() const {
    // Return original texture when showing "before", otherwise return processed output
    if (m_bypassAdjustments && m_originalTexture) {
        GLuint texId = m_originalTexture->textureId();
        std::cout << "Returning original texture: " << texId << std::endl;
        return texId;
    }
    GLuint texId = m_fbo ? m_fbo->texture() : 0;
    std::cout << "Returning processed texture: " << texId << std::endl;
    return texId;
}

} // namespace zraw
