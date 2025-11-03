# ZRaw Developer

A free, open-source RAW photo editor with simplicity in mind. The philosophy is: do one thing, and do it well.

## ✨ Why ZRaw Developer?

- 🎨 **Color Science** - Uses ACES color science
- 🚀 **Fast & Responsive** - Real-time editing using GPU acceleration
- 📸 **Your Camera, Supported** - Works with Canon, Nikon, Sony, Fujifilm, Olympus, Panasonic, and more
- 💾 **Non-Destructive** - Your original photos are never modified
- 💰 **Completely Free** - No subscriptions, no watermarks, no limitations
- 🐧 **Linux Native** - Built specifically for Linux photographers
- ⚡ **Professional Features** - HDR support, tone controls, and more

## 🎨 What Makes the Colors Special?

ZRaw Developer uses **ACES** (Academy Color Encoding System). This means:

- **Natural-looking highlights** - Bright areas roll off smoothly instead of clipping to white
- **Accurate colors** - Skin tones, skies, and foliage look true to life
- **No weird color shifts** - Adjusting brightness won't make colors look wrong
- **Film-like rendering** - Your photos have that "cinematic" quality

### What You Can Adjust

**Exposure & Tone**
- Brighten or darken your entire photo
- Recover blown-out highlights (like bright skies)
- Lift details from dark shadows
- Adjust whites and blacks for fine control

**Contrast**
- Overall contrast for punch
- Separate control for highlights, midtones, and shadows
- Works in perceptual space (what your eye sees)

**Color**
- **Vibrance** - Boost muted colors without oversaturating (great for landscapes)
- **Saturation** - Overall color intensity control

**Sharpness**
- Edge-aware sharpening that won't create halos
- Only sharpens details, not noise

**HDR Support** (if you have an HDR monitor)
- Export photos that take advantage of HDR displays
- Perfect for viewing on modern TVs and monitors

## 🛠️ Built With

- **C++** - For maximum speed and performance
- **LibRaw** - Industry-standard library for reading RAW files
- **OpenGL** - Uses your graphics card for real-time editing
- **Qt6** - Modern, responsive user interface

## 📦 Build from source

### Step 1: Install Dependencies

**Ubuntu/Debian:**
```bash
sudo apt install build-essential cmake qt6-base-dev qt6-opengl-dev libgl1-mesa-dev libraw-dev pkg-config
```

**Fedora:**
```bash
sudo dnf install gcc-c++ cmake qt6-qtbase-devel mesa-libGL-devel LibRaw-devel pkgconfig
```

**Arch Linux:**
```bash
sudo pacman -S base-devel cmake qt6-base mesa libraw pkgconf
```

### Step 2: Build from Source

```bash
# Clone the repository
git clone https://github.com/jasonzondor/zraw-developer.git
cd zraw-developer

# Build it
mkdir build && cd build
cmake ..
make -j$(nproc)

# Optionally install system-wide
sudo make install
```

## 🚀 Getting Started

**Open the application:**
```bash
./zraw-developer
```

**Or open a specific photo:**
```bash
./zraw-developer /path/to/your/photo.cr2
```

**Tips:**
- Double-click any slider to reset it to default
- Your adjustments are automatically saved to an XMP file next to your photo
- The original RAW file is never modified—you can always start over!

## 🗺️ What's Coming Next?

### ✅ What's Already Working
- [x] Load and edit RAW files from all major camera brands
- [x] Real-time adjustments with instant preview
- [x] Hollywood-grade color science (ACES)
- [x] HDR support for modern displays
- [x] Non-destructive editing with XMP sidecars
- [x] Advanced tone and color controls

### 🚧 Currently Working On
- [x] **Export photos** - Save as TIFF, JPEG, or PNG
- [ ] **Presets** - Save your favorite adjustments and apply them to other photos
- [x] **Before/After view** - Compare your edits side-by-side

### 📅 Planned Features
- [ ] **Tone curves** - Fine-tune your tonal adjustments with curves
- [ ] **HSL controls** - Adjust specific colors (like making skies more blue)
- [ ] **Local adjustments** - Edit just part of your photo with brushes and gradients
- [ ] **Lens corrections** - Automatically fix distortion and vignetting
- [ ] **Noise reduction** - Clean up high-ISO photos
- [ ] **Film looks** - Presets that mimic classic film stocks

### 🎯 Future Ideas
- [ ] Support for Windows and macOS
- [ ] Custom presets and LUTs

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

## 🤝 Want to Help?

ZRaw Developer is built by hobby photographers, for hobby photographers. Contributions are welcome!

**You can help in many ways:**

- 🐛 **Report bugs** - Found something broken? Let us know!
- 💡 **Suggest features** - What would make your workflow better?
- 📝 **Improve docs** - Help make the documentation clearer
- 🎨 **Share presets** - Create and share adjustment presets
- 🧪 **Test features** - Try new features and give feedback
- 💻 **Write code** - If you know C++, check out the issues!

See [CONTRIBUTING.md](CONTRIBUTING.md) for more details on how to get started.

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

**Built with ❤️ by hobby photographers, for hobby photographers.**
