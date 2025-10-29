# Changelog

All notable changes to ZRaw Developer will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

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
