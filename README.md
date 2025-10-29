# ZRaw Developer

A RAW photo processor with **color science** and GPU acceleration.

Built for photographers who appreciate **accurate color**, **professional workflows**, and **real-time performance**.

## ✨ Highlights

- 🎨 **ACES Color Science** - Hollywood-standard color pipeline
- 🚀 **GPU-Accelerated** - Real-time editing with OpenGL compute shaders
- 🎬 **HDR Output** - PQ (HDR10) and HLG (broadcast) encoding support
- 🔬 **Perceptually Accurate** - LCH color space, Bradford adaptation, log-space contrast
- 📸 **Wide Format Support** - CR2, NEF, ARW, DNG, RAF (X-Trans), ORF, RW2, and more
- 💾 **Non-Destructive** - XMP sidecar files for all adjustments
- ⚡ **High Performance** - 16-bit processing pipeline, multi-threaded RAW loading

## 🎨 Professional Color Science

ZRaw Developer implements **industry-standard color science** matching professional workflows:

### Color Pipeline
```
RAW Demosaic → White Balance (Bradford) → Exposure (Linear) 
→ Tone Recovery (Gaussian) → Contrast (Log Space) 
→ Local Contrast (Zone-based) → Color (LCH Space) 
→ ACES Tone Mapping → Gamut Mapping → Sharpening (Edge-aware) 
→ Output Transform (SDR/HDR)
```

### Key Features

**White Balance**
- Bradford chromatic adaptation (industry standard)
- Accurate color temperature and tint control
- No color shifts or artifacts

**Tone Mapping**
- ACES RRT (Reference Rendering Transform)
- Smooth highlight rolloff (no clipping)
- Film-like rendering

**Color Adjustments**
- LCH color space (perceptually uniform, no hue shifts)
- Vibrance (smart saturation for muted colors)
- Saturation (linear chroma adjustment)

**Sharpening**
- Edge-aware deconvolution (halo-free)
- Variance-based detail detection
- Luminance-only (preserves color)

**HDR Output**
- PQ (Perceptual Quantizer) for HDR10/Dolby Vision
- HLG (Hybrid Log-Gamma) for broadcast
- Full ACES workflow (AP0 → RRT → ODT)

## 📋 Adjustment Controls

### White Balance
- **Temperature** - Warm/cool color balance (-100 to +100)
- **Tint** - Green/magenta balance (-100 to +100)

### Tone
- **Exposure** - Overall brightness (-3 to +3 stops)
- **Highlights** - Recover blown highlights (-100 to +100)
- **Shadows** - Lift shadow detail (-100 to +100)

### Contrast
- **Contrast** - Global midpoint contrast (-1 to +1)
- **Highlight Contrast** - Contrast in bright areas (-100 to +100)
- **Midtone Contrast** - Contrast in mid-tones (-100 to +100)
- **Shadow Contrast** - Contrast in dark areas (-100 to +100)

### Color
- **Vibrance** - Smart saturation boost (-100 to +100)
- **Saturation** - Linear saturation (-100 to +100)

### Detail
- **Sharpness** - Edge enhancement (0 to 2.0)

## Architecture

- **C++17** - Maximum performance
- **LibRaw** - Industry-standard RAW processing
- **OpenGL 3.3+** - GPU compute via GLSL shaders
- **Qt6** - Cross-platform GUI framework

## Dependencies

### Ubuntu/Debian
```bash
sudo apt install build-essential cmake
sudo apt install qt6-base-dev qt6-opengl-dev libgl1-mesa-dev
sudo apt install libraw-dev
sudo apt install pkg-config
```

### Fedora
```bash
sudo dnf install gcc-c++ cmake
sudo dnf install qt6-qtbase-devel mesa-libGL-devel
sudo dnf install LibRaw-devel
sudo dnf install pkgconfig
```

### Arch Linux
```bash
sudo pacman -S base-devel cmake
sudo pacman -S qt6-base mesa
sudo pacman -S libraw
sudo pacman -S pkgconf
```

## Building

```bash
# Create build directory
mkdir build && cd build

# Configure
cmake ..

# Build
make -j$(nproc)

# Install (optional)
sudo make install
```

## Usage

### GUI Mode
```bash
# Open application
./zraw-developer

# Open with file
./zraw-developer /path/to/image.cr2
```

### Command-Line Mode (for photo managers)
```bash
# Process image with adjustments
./zraw-developer --input image.cr2 --output output.tiff \
  --exposure 0.5 --contrast 0.2 --sharpness 1.0
```

## Performance

- Real-time preview updates on GPU
- Optimized for multi-core CPUs (`-march=native`)
- Efficient 16-bit processing pipeline
- Minimal memory allocations during editing

## 🗺️ Roadmap

### ✅ Phase 1: Fundamentals (Complete)
- [x] RAW loading with LibRaw (all major formats)
- [x] GPU-accelerated processing pipeline
- [x] Basic adjustments (Exposure, Contrast, Sharpness)
- [x] Qt6 GUI with real-time preview
- [x] XMP sidecar support

### ✅ Phase 2: Professional Color Science (Complete)
- [x] Bradford chromatic adaptation for white balance
- [x] ACES tone mapping (RRT)
- [x] LCH color space for saturation (no hue shifts)
- [x] Log-space contrast adjustments
- [x] Gaussian highlight/shadow recovery
- [x] Zone-based local contrast
- [x] Adaptive gamut mapping

### ✅ Phase 3: Cinema-Grade Features (Complete)
- [x] Full ACES workflow (AP0 → AP1 → RRT → ODT)
- [x] HDR output support (PQ and HLG encoding)
- [x] Edge-aware RAW sharpening (halo-free)
- [x] Professional UI layout (workflow-optimized)
- [x] Multiple output modes (SDR, HDR PQ, HDR HLG, Full ACES)

### 🚧 Phase 4: Export & Workflow (In Progress)
- [ ] Image export (TIFF 16-bit, JPEG, PNG)
- [ ] ICC profile support for output
- [ ] Batch processing
- [ ] Preset system (save/load adjustment sets)
- [ ] Before/After comparison view

### 📅 Phase 5: Advanced Features (Planned)
- [ ] Tone curve editor
- [ ] HSL/Color grading tools
- [ ] Local adjustments (gradients, brushes)
- [ ] Lens corrections database
- [ ] Noise reduction (luminance and chroma)
- [ ] Chromatic aberration correction
- [ ] OCIO (OpenColorIO) integration
- [ ] 3D LUT support

### 🎯 Phase 6: Professional Tools (Future)
- [ ] CLI interface for automation
- [ ] Plugin system
- [ ] Custom color spaces
- [ ] Film emulation presets
- [ ] Tethered shooting support
- [ ] macOS and Windows builds

## Integration with Photo Managers

ZRaw Developer is designed to be called by external photo management applications:

```bash
# Example integration
photo-manager --edit-with=/usr/bin/zraw-developer photo.cr2
```

The application will:
1. Load the RAW file
2. Display editing interface
3. Save processed image when requested
4. Return control to photo manager

## 🆚 Comparison to Other RAW Processors

| Feature | ZRaw Developer | Lightroom | Capture One | DaVinci Resolve |
|---------|----------------|-----------|-------------|-----------------|
| **Color Science** | ||||
| ACES Workflow | ✅ Full | ❌ Inspired | ❌ Partial | ✅ Full |
| Bradford Adaptation | ✅ | ✅ | ✅ | ✅ |
| LCH Saturation | ✅ | ✅ | ✅ | ✅ |
| Log Contrast | ✅ | ✅ | ✅ | ✅ |
| HDR Output (PQ/HLG) | ✅ | ❌ | ❌ | ✅ |
| **Performance** | ||||
| GPU Acceleration | ✅ | ✅ | ✅ | ✅ |
| Real-time Preview | ✅ | ✅ | ✅ | ✅ |
| **Platform** | ||||
| Linux Native | ✅ | ❌ | ❌ | ✅ |
| Open Source | ✅ | ❌ | ❌ | ❌ |
| **Price** | Free | $10/mo | $300 | $295 |

**ZRaw Developer offers DaVinci Resolve-level color science in a free, open-source package!**

## 🤝 Contributing

Contributions are welcome! We follow a professional development workflow:

- **Branch Protection** - All changes via Pull Requests
- **CI/CD** - Automated testing on every PR
- **Code Review** - Maintainer approval required
- **Conventional Commits** - Semantic versioning

See [CONTRIBUTING.md](CONTRIBUTING.md) for detailed guidelines.

### Areas for Contribution
- Export functionality (TIFF, JPEG with ICC profiles)
- Batch processing
- Lens correction database
- Noise reduction algorithms
- UI/UX improvements

## 📄 License

MIT License - See [LICENSE](LICENSE) file for details.

## 🙏 Credits

**Core Libraries:**
- [LibRaw](https://www.libraw.org/) - RAW processing library
- [Qt6](https://www.qt.io/) - GUI framework
- [OpenGL](https://www.opengl.org/) - GPU acceleration

**Color Science References:**
- [ACES](https://www.oscars.org/science-technology/sci-tech-projects/aces) - Academy Color Encoding System
- [Rec.2020](https://www.itu.int/rec/R-REC-BT.2020) - Wide color gamut standard
- [SMPTE ST 2084](https://ieeexplore.ieee.org/document/7291452) - PQ transfer function

**Inspiration:**
- DaVinci Resolve - Professional color grading
- Lightroom - Workflow design
- Capture One - Color accuracy

## 📚 Documentation

- [GITHUB_SETUP.md](GITHUB_SETUP.md) - Repository setup guide
- [GITHUB_RULESET_SETUP.md](GITHUB_RULESET_SETUP.md) - Branch protection with rulesets
- [CONTRIBUTING.md](CONTRIBUTING.md) - Contribution guidelines
- [CHANGELOG.md](CHANGELOG.md) - Version history

## 🔗 Links

- **Repository**: https://github.com/jasonzondor/zraw-developer
- **Issues**: https://github.com/jasonzondor/zraw-developer/issues
- **Releases**: https://github.com/jasonzondor/zraw-developer/releases

---

**Built with ❤️ for photographers who care about color science.**
