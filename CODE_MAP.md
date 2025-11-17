# ZRaw Developer - Code Map

## Project Overview

**ZRaw Developer** is a professional RAW photo editor for Linux built with C++, Qt6, OpenGL, and LibRaw. It uses ACES color science for film-like rendering and supports both GUI and headless (CLI) modes.

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                         main.cpp                            │
│  Entry point - handles GUI vs Headless mode routing         │
└─────────────────┬───────────────────────────────────────────┘
                  │
        ┌─────────┴──────────┐
        │                    │
    GUI Mode           Headless Mode
        │                    │
        ▼                    ▼
┌───────────────┐    ┌──────────────┐
│  MainWindow   │    │  CLI Export  │
│   (Qt GUI)    │    │   Pipeline   │
└───────┬───────┘    └──────┬───────┘
        │                   │
        └───────────┬───────┘
                    ▼
        ┌───────────────────────┐
        │  Processing Pipeline  │
        │                       │
        │  RawProcessor (LibRaw)│
        │         ↓             │
        │  GPUPipeline (OpenGL) │
        │         ↓             │
        │  ImageExporter (TIFF) │
        └───────────────────────┘
```

## Directory Structure

```
src/
├── main.cpp                    # Entry point
├── core/                       # Core processing logic
│   ├── RawProcessor.*         # LibRaw wrapper for RAW loading
│   ├── ImageBuffer.*          # 16-bit image data container
│   ├── CLIHandler.*           # Command-line argument parsing
│   ├── ImageExporter.*        # Export to TIFF/JPEG/PNG
│   └── XMPHandler.*           # Non-destructive XMP sidecar files
├── gpu/                        # GPU acceleration
│   ├── GLContext.*            # OpenGL context management
│   ├── ShaderProgram.*        # Shader compilation/linking
│   └── GPUPipeline.*          # Main GPU processing pipeline
├── adjustments/                # Adjustment algorithms (legacy)
│   ├── ExposureAdjustment.*
│   ├── ContrastAdjustment.*
│   └── SharpnessAdjustment.*
└── ui/                         # Qt user interface
    ├── MainWindow.*           # Main application window
    ├── ImageViewer.*          # OpenGL image display widget
    ├── AdjustmentPanel.*      # Slider controls panel
    ├── ResettableSlider.*     # Custom slider widget
    └── CropOverlay.*          # Crop tool overlay
```

## Core Components

### 1. Entry Point (`main.cpp`)

**Purpose**: Application entry point that routes to GUI or headless mode.

**Key Functions**:
- `main()` - Detects mode (GUI vs headless) and creates appropriate Qt application
- `runGUI()` - Launches Qt GUI interface
- `runHeadless()` - Runs CLI export pipeline without GUI

**Flow**:
1. Parse command-line arguments
2. Check for `--headless` flag
3. Create `QApplication` (GUI) or `QCoreApplication` (headless)
4. Route to appropriate execution mode

---

### 2. Core Module (`src/core/`)

#### **RawProcessor** - RAW File Loading
**File**: `RawProcessor.h/cpp`

**Purpose**: Wrapper around LibRaw for loading and initial processing of RAW files.

**Key Methods**:
- `loadRaw(filepath)` - Load RAW file from disk
- `processToRGB()` - Demosaic and convert to 16-bit RGB
- `getImageBuffer()` - Get processed image data
- `getCameraWBTemperature()` - Extract camera white balance
- Metadata getters: `width()`, `height()`, `cameraModel()`, `iso()`, etc.

**Dependencies**: LibRaw library

---

#### **ImageBuffer** - Image Data Container
**File**: `ImageBuffer.h/cpp`

**Purpose**: Stores 16-bit per channel RGB image data for high dynamic range.

**Key Features**:
- 16-bit unsigned integer storage (`uint16_t`)
- 3-channel RGB data
- Conversion to 8-bit for display
- Memory management for large images

---

#### **XMPHandler** - Non-Destructive Editing
**File**: `XMPHandler.h/cpp`

**Purpose**: Save/load adjustment parameters to XMP sidecar files (non-destructive workflow).

**Key Methods**:
- `loadAdjustments(rawFilePath)` - Load adjustments from `.xmp` file
- `saveAdjustments(rawFilePath, adjustments)` - Save adjustments to `.xmp` file
- `xmpExists(rawFilePath)` - Check if XMP sidecar exists

**Adjustments Stored**:
- Exposure, Contrast, Sharpness
- Temperature, Tint
- Highlights, Shadows, Whites, Blacks
- Vibrance, Saturation
- Highlight/Midtone/Shadow Contrast

---

#### **ImageExporter** - File Export
**File**: `ImageExporter.h/cpp`

**Purpose**: Export processed images to TIFF, JPEG, or PNG formats.

**Key Features**:
- TIFF export with 16-bit color depth
- JPEG export with quality control
- PNG export
- Proper color profile embedding

**Dependencies**: libtiff

---

#### **CLIHandler** - Command-Line Interface
**File**: `CLIHandler.h/cpp`

**Purpose**: Parse command-line arguments for both GUI and headless modes.

**Supported Arguments**:
- `--headless` - Run without GUI
- `--input <file>` - Input RAW file
- `--output <file>` - Output file path
- `--format <tiff|jpeg|png>` - Export format
- `--exposure <value>` - Exposure adjustment
- `--contrast <value>` - Contrast adjustment
- `--sharpness <value>` - Sharpness adjustment

---

### 3. GPU Module (`src/gpu/`)

#### **GPUPipeline** - Main Processing Engine
**File**: `GPUPipeline.h/cpp`

**Purpose**: GPU-accelerated image processing using OpenGL shaders.

**Key Features**:
- Real-time processing using fragment shaders
- ACES color science implementation
- HDR output support (PQ, HLG, Full ACES)
- Crop functionality
- Before/After comparison mode

**Processing Pipeline** (in shader):
```
1. White Balance (Bradford chromatic adaptation)
2. Exposure Adjustment (linear space)
3. Tone Recovery (highlights/shadows)
4. ACES Tone Mapping
5. Contrast (log space - perceptual)
6. Color Adjustments (LCH space - vibrance/saturation)
7. Sharpening (edge-aware)
8. Output Transform (SDR/HDR)
```

**Key Methods**:
- `initialize()` - Set up OpenGL context and shaders
- `uploadImage(buffer)` - Upload image to GPU texture
- `setExposure()`, `setContrast()`, etc. - Set adjustment parameters
- `process()` - Execute shader pipeline
- `downloadImage()` - Download processed image from GPU
- `setCrop()` - Set crop region
- `setOutputMode()` - Select SDR/HDR output

**Adjustment Parameters**:
- **Exposure**: -3.0 to +3.0 EV
- **Contrast**: -1.0 to +1.0
- **Temperature**: -100 to +100 (Kelvin shift)
- **Tint**: -100 to +100 (green-magenta)
- **Highlights/Shadows**: -100 to +100 (tone recovery)
- **Whites/Blacks**: -100 to +100 (fine tone control)
- **Vibrance/Saturation**: -100 to +100
- **Sharpness**: 0.0 to 2.0

---

#### **ShaderProgram** - Shader Management
**File**: `ShaderProgram.h/cpp`

**Purpose**: Compile and link OpenGL shaders.

**Key Methods**:
- `compileShader(source, type)` - Compile vertex/fragment shader
- `linkProgram()` - Link shader program
- `use()` - Activate shader program
- `setUniform()` - Set shader uniform variables

---

#### **GLContext** - OpenGL Context
**File**: `GLContext.h/cpp`

**Purpose**: Manage OpenGL context initialization.

---

### 4. UI Module (`src/ui/`)

#### **MainWindow** - Application Window
**File**: `MainWindow.h/cpp`

**Purpose**: Main Qt window that orchestrates the entire application.

**Components**:
- Menu bar (File → Open, Save)
- Image viewer (center)
- Adjustment panel (right sidebar)
- Status bar

**Key Responsibilities**:
- File opening/saving
- Coordinate between UI and processing pipeline
- Load/save XMP adjustments
- Debounced XMP saving (auto-save after adjustments)

**Signal Flow**:
```
User adjusts slider
    ↓
AdjustmentPanel emits signal
    ↓
MainWindow slot receives signal
    ↓
GPUPipeline parameter updated
    ↓
ImageViewer triggers repaint
    ↓
XMP save scheduled (debounced)
```

---

#### **ImageViewer** - OpenGL Display Widget
**File**: `ImageViewer.h/cpp`

**Purpose**: OpenGL widget that displays the processed image.

**Key Features**:
- Real-time display of GPU-processed image
- Zoom and pan controls (mouse wheel + drag)
- Before/After toggle button
- Crop overlay integration
- Aspect-ratio preserving viewport

**Interaction**:
- Mouse wheel: Zoom in/out
- Left-click drag: Pan image
- Double-click: Reset view
- Before/After button: Toggle original vs edited

---

#### **AdjustmentPanel** - Controls Panel
**File**: `AdjustmentPanel.h/cpp`

**Purpose**: Right sidebar with all adjustment sliders.

**Sections**:
1. **Exposure & Tone**
   - Exposure, Highlights, Shadows, Whites, Blacks

2. **Contrast**
   - Overall Contrast
   - Highlight/Midtone/Shadow Contrast

3. **Color**
   - Temperature, Tint
   - Vibrance, Saturation

4. **Sharpness**
   - Sharpness slider

5. **Crop**
   - Crop mode toggle
   - Aspect ratio presets
   - Reset button

**Features**:
- Double-click slider to reset to default
- Real-time value display
- Camera white balance display

---

#### **CropOverlay** - Crop Tool
**File**: `CropOverlay.h/cpp`

**Purpose**: Interactive crop overlay on the image viewer.

**Features**:
- Draggable crop rectangle
- Corner/edge handles for resizing
- Aspect ratio locking
- Grid overlay (rule of thirds)
- Darkened areas outside crop

---

#### **ResettableSlider** - Custom Slider
**File**: `ResettableSlider.h/cpp`

**Purpose**: QSlider subclass that resets to default on double-click.

---

### 5. Adjustments Module (`src/adjustments/`)

**Note**: This module appears to be legacy code. The actual adjustment processing is now done in GPU shaders within `GPUPipeline`.

Files:
- `ExposureAdjustment.*`
- `ContrastAdjustment.*`
- `SharpnessAdjustment.*`

---

## Data Flow

### Opening a RAW File

```
1. User: File → Open
2. MainWindow::openFile()
3. MainWindow::processRawFile(filepath)
4. RawProcessor::loadRaw(filepath)
5. RawProcessor::processToRGB()
6. GPUPipeline::uploadImage(buffer)
7. XMPHandler::loadAdjustments(filepath)
8. AdjustmentPanel::setExposure/Contrast/etc.
9. GPUPipeline::process()
10. ImageViewer::updateDisplay()
```

### Adjusting a Parameter

```
1. User: Moves exposure slider
2. AdjustmentPanel::exposureChanged(value) [signal]
3. MainWindow::onExposureChanged(value) [slot]
4. GPUPipeline::setExposure(value)
5. MainWindow::updateImage()
6. GPUPipeline::process()
7. ImageViewer::update() [triggers paintGL]
8. MainWindow::scheduleXMPSave() [debounced]
9. (After delay) XMPHandler::saveAdjustments()
```

### Exporting an Image

```
1. User: File → Save
2. MainWindow::saveFile()
3. GPUPipeline::downloadImage()
4. ImageExporter::exportImage(buffer, filepath, format)
5. Write TIFF/JPEG/PNG to disk
```

### Headless Mode (CLI)

```
1. Command: ./zraw-developer --headless --input photo.cr2 --output out.tiff
2. main() detects --headless flag
3. runHeadless(options)
4. Create offscreen OpenGL context
5. RawProcessor::loadRaw()
6. RawProcessor::processToRGB()
7. GPUPipeline::uploadImage()
8. GPUPipeline::setExposure/Contrast/etc.
9. GPUPipeline::process()
10. GPUPipeline::downloadImage()
11. ImageExporter::exportImage()
```

---

## Color Science Pipeline

The color processing happens in GPU shaders (`GPUPipeline.cpp`):

### 1. White Balance
- **Method**: Bradford chromatic adaptation
- **Space**: Linear RGB
- **Purpose**: Adjust color temperature and tint

### 2. Exposure
- **Method**: Linear multiplication
- **Space**: Linear RGB
- **Purpose**: Brighten/darken image (physical light adjustment)

### 3. Tone Recovery
- **Method**: Highlight compression, shadow lift
- **Space**: Linear RGB
- **Purpose**: Recover blown highlights, lift shadows

### 4. ACES Tone Mapping
- **Method**: ACES RRT (Reference Rendering Transform)
- **Space**: ACES AP1 → sRGB
- **Purpose**: Film-like highlight rolloff, prevent clipping

### 5. Contrast
- **Method**: Log-space contrast curve
- **Space**: Perceptual (log)
- **Purpose**: Adjust contrast in perceptually uniform way
- **Zones**: Separate controls for highlights, midtones, shadows

### 6. Color Adjustments
- **Method**: LCH color space manipulation
- **Space**: LCH (Lightness, Chroma, Hue)
- **Purpose**: 
  - **Vibrance**: Boost muted colors without oversaturating
  - **Saturation**: Overall color intensity

### 7. Sharpening
- **Method**: Unsharp mask with edge detection
- **Space**: Luminance
- **Purpose**: Enhance detail without amplifying noise

### 8. Output Transform
- **Modes**:
  - **SDR**: sRGB gamma encoding
  - **HDR PQ**: SMPTE ST 2084 (Rec.2020)
  - **HDR HLG**: Hybrid Log-Gamma
  - **Full ACES**: Linear ACES output

---

## Key Technologies

### Libraries
- **Qt6**: GUI framework (Widgets, OpenGL)
- **LibRaw**: RAW file decoding
- **OpenGL 3.3**: GPU acceleration
- **libtiff**: TIFF export

### Build System
- **CMake**: Cross-platform build configuration
- **C++17**: Modern C++ standard

### Color Science
- **ACES**: Academy Color Encoding System
- **Bradford**: Chromatic adaptation transform
- **LCH**: Perceptual color space
- **Rec.2020**: Wide color gamut for HDR

---

## File Formats

### Input
- **RAW files**: CR2, NEF, ARW, RAF, ORF, RW2, etc. (via LibRaw)

### Output
- **TIFF**: 16-bit, uncompressed, with color profile
- **JPEG**: 8-bit, quality adjustable
- **PNG**: 8-bit, lossless

### Sidecar
- **XMP**: XML-based adjustment storage (non-destructive)

---

## Extension Points

### Adding New Adjustments

1. Add parameter to `GPUPipeline.h`:
   ```cpp
   float m_newParameter;
   void setNewParameter(float value);
   ```

2. Add setter in `GPUPipeline.cpp`:
   ```cpp
   void GPUPipeline::setNewParameter(float value) {
       m_newParameter = value;
   }
   ```

3. Add to shader uniforms in `GPUPipeline.cpp`:
   ```cpp
   m_shader->setUniform("u_newParameter", m_newParameter);
   ```

4. Implement in fragment shader (embedded in `GPUPipeline.cpp`)

5. Add UI slider in `AdjustmentPanel.cpp`

6. Connect signal/slot in `MainWindow.cpp`

7. Add to XMP storage in `XMPHandler.h/cpp`

### Adding New Export Formats

1. Add format enum to `ImageExporter.h`
2. Implement export logic in `ImageExporter.cpp`
3. Add to CLI options in `CLIHandler.cpp`
4. Add to save dialog in `MainWindow.cpp`

---

## Performance Considerations

### GPU Processing
- All adjustments run in real-time on GPU
- Single shader pass for efficiency
- 16-bit texture precision for quality

### Memory Management
- Images stored as 16-bit to preserve dynamic range
- Smart pointers (`std::shared_ptr`) for automatic cleanup
- GPU textures released when not needed

### Debounced XMP Saving
- XMP saves are debounced (delayed) to avoid excessive disk writes
- Timer triggers save 500ms after last adjustment

---

## Testing

### Manual Testing
```bash
# Build
mkdir build && cd build
cmake ..
make -j$(nproc)

# Test GUI
./zraw-developer /path/to/photo.cr2

# Test CLI
./zraw-developer --headless --input photo.cr2 --output out.tiff --exposure 1.0
```

### Test Cases
- Different camera RAW formats (Canon, Nikon, Sony, Fuji)
- High contrast scenes (highlight recovery)
- Low light images (shadow lift, noise)
- Portraits (skin tones)
- Landscapes (skies, foliage)
- Export formats (TIFF, JPEG, PNG)
- XMP save/load persistence

---

## Future Architecture Considerations

### Planned Features (from CONTRIBUTING.md)
- **Tone curves**: Add curve editor UI and GPU implementation
- **HSL controls**: Per-color adjustments (e.g., make skies more blue)
- **Local adjustments**: Brush/gradient masks with GPU blending
- **Lens corrections**: Database of lens profiles
- **Noise reduction**: GPU-accelerated denoising
- **Batch processing**: Process multiple files with same adjustments

### Architectural Notes
- Keep GPU pipeline modular for adding new processing stages
- Consider compute shaders for more complex algorithms (noise reduction)
- Maintain separation between UI, processing, and export
- XMP format should remain compatible with other RAW processors

---

## Debugging Tips

### Enable OpenGL Debug Output
Add to `GPUPipeline.cpp`:
```cpp
glEnable(GL_DEBUG_OUTPUT);
glDebugMessageCallback(debugCallback, nullptr);
```

### Print Shader Compilation Errors
Check `ShaderProgram.cpp` for shader error messages

### Verify XMP Files
XMP files are plain XML - can be inspected with text editor

### Check LibRaw Errors
`RawProcessor::lastError()` provides detailed error messages

---

## Contributing Guidelines

See `CONTRIBUTING.md` for:
- Branch strategy (main, feature/*, bugfix/*)
- Commit message format (feat:, fix:, docs:, etc.)
- Code review process
- Color science guidelines

---

## License

MIT License - See `LICENSE` file
