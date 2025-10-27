# ZRaw Developer

A high-performance, Linux-first RAW photo developer with GPU acceleration.

## Features

- **GPU-Accelerated Processing** - Real-time editing using OpenGL shaders
- **RAW Format Support** - Supports CR2, NEF, ARW, DNG, RAF, ORF, RW2, and more via LibRaw
- **Basic Adjustments**:
  - Exposure (-3 to +3 stops)
  - Contrast (-1 to +1)
  - Sharpness (0 to 2)
- **Modern Qt6 Interface** - Clean, responsive UI
- **Command-Line Support** - Can be called by external photo managers
- **16-bit Processing Pipeline** - High dynamic range preservation

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

## Roadmap

### Phase 1 (Current)
- [x] Basic RAW loading
- [x] GPU pipeline
- [x] Exposure, contrast, sharpness
- [x] Qt6 GUI
- [ ] Image export (TIFF, JPEG, PNG)
- [ ] CLI interface

### Phase 2
- [ ] White balance
- [ ] Highlights/Shadows
- [ ] Vibrance/Saturation
- [ ] Tone curves
- [ ] Color grading

### Phase 3
- [ ] Lens corrections
- [ ] Noise reduction
- [ ] Local adjustments
- [ ] Batch processing
- [ ] Preset system

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

## License

MIT License - See LICENSE file for details

## Contributing

Contributions welcome! Please see CONTRIBUTING.md for guidelines.

## Credits

- LibRaw - RAW processing library
- Qt6 - GUI framework
- OpenGL - GPU acceleration
