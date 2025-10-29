# Changelog

All notable changes to ZRaw Developer will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.2.2] - 2025-10-29

### Added
- **Whites and Blacks adjustments** - Precise control over highlight and shadow brightness
  - Whites: Adjusts bright tones (above 18% gray) without color shifts
  - Blacks: Adjusts dark tones (below 18% gray) without color shifts
- **Double-click to reset sliders** - All adjustment sliders now reset to default value on double-click
- **ResettableSlider component** - Custom slider widget with built-in reset functionality

### Changed
- **Improved tone mapping for SDR displays** - Replaced aggressive ACES tone mapping with gentler curve
  - Images now display brighter and more natural on SDR monitors
  - Only compresses highlights above 1.0 to prevent clipping
  - Preserves midtones and shadows without darkening
- **Wider adjustment panel** - Increased panel width range (150-300px) for better label spacing
- **Reduced minimum window size** - ImageViewer minimum size reduced to 400x300 for better flexibility
- **Set minimum window size** - Window now enforces 650x400 minimum to prevent UI overlap

### Fixed
- **Dark image display issue** - Fixed HDR tone mapping being applied incorrectly to SDR output
- **Segmentation fault on startup** - Fixed null pointer crashes from removed temperature/tint sliders
- **Viewport overlap** - Fixed image viewport rendering over adjustment panel when window narrowed
- **XMP compatibility** - Temperature and tint now default to 0 in XMP files (sliders removed from UI)

### Removed
- Temperature and Tint sliders from Basic section (replaced with Whites and Blacks)

## [0.2.1] - Previous Release

### Added
- Professional color science pipeline with ACES workflow
- HDR output support (PQ and HLG encoding)
- Full ACES RRT/ODT implementation
- Bradford chromatic adaptation for white balance
- LCH color space for saturation (halo-free)
- Adaptive gamut mapping
- Edge-aware RAW sharpening (deconvolution-based)
- Advanced adjustment sliders:
  - White Balance (Temperature, Tint)
  - Tone Recovery (Highlights, Shadows)
  - Local Contrast (Highlights, Midtones, Shadows)
  - Color (Vibrance, Saturation)
- XMP sidecar support with debounced auto-save
- X-Trans sensor support (DHT demosaicing)
- GPU-accelerated processing pipeline

### Changed
- Reorganized UI to match professional workflow
- Improved sharpening algorithm (no halos)
- Better luminance calculations (Rec.2020)
- Log-space contrast for perceptual uniformity

### Fixed
- XMP file formatting for compatibility
- Excessive XMP writes with debouncing
- Image zoom now fills viewport properly

## [0.1.0] - Initial Development

### Added
- Basic RAW file loading with LibRaw
- OpenGL GPU pipeline
- Qt6 GUI framework
- Basic adjustments (Exposure, Contrast, Sharpness)
- Real-time preview

---

## Version Numbering

- **Major (X.0.0)**: Breaking changes, major features
- **Minor (0.X.0)**: New features, backwards compatible
- **Patch (0.0.X)**: Bug fixes, minor improvements
