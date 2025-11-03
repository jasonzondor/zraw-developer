# Changelog

All notable changes to ZRaw Developer are documented here.

This helps you see what's new, what's been fixed, and what's changed in each version.

## [Unreleased]

## [0.2.5] - November 3, 2025

### ✨ New Features
- **Image cropping** - Crop your photos with interactive overlay
  - Drag and resize crop area directly on your image
  - Multiple aspect ratio presets (free, 1:1, 4:3, 16:9, etc.)
  - Portrait and landscape orientation options
  - Reset button to remove crop
- **Improved white balance controls** - More accurate color temperature adjustment
  - Shows actual Kelvin temperature (2000K-10000K)
  - Displays your camera's white balance setting
  - More precise tint control using professional scale

### 👍 Improvements
- Better white balance calculation using Robertson's method
- Enhanced tint adjustment based on Darktable's approach

### 🐛 Bug Fixes
- Fixed libtiff dependency issues for better compatibility across Linux distributions

## [0.2.4] - October 31, 2025

### ✨ New Features
- **Temperature and Tint sliders are back!** - Now with improved accuracy
  - Relative adjustments from your camera's white balance
  - Clear labels showing "As Shot", "Warmer", or "Cooler"
  - Saved to XMP files for non-destructive editing

### 👍 Improvements
- **Better tone adjustments** - More natural-looking results
  - New parametric tone curve system for smoother adjustments
  - Prevents color shifts when adjusting whites and blacks
  - Ratio-preserving color scaling for natural tone changes
  - Improved highlight and shadow recovery

## [0.2.3] - October 29, 2025

### ✨ New Features
- **Export your photos!** - Save edited images in multiple formats
  - **16-bit TIFF** - Full quality with all color information preserved
  - **JPEG** - Standard format for sharing (8-bit)
  - **PNG** - Lossless format (8-bit)
  - File dialog clearly shows bit depth for each format
- **Before/After comparison** - Toggle between original and edited
  - Styled button in the image viewer
  - Instantly see your changes
  - Perfect for checking your adjustments

### 👍 Improvements
- Better highlight handling with smooth compression
- Improved exposure adjustment to prevent color shifts
- Enhanced image saving with proper error handling

### 🐛 Bug Fixes
- Fixed data corruption in JPEG/PNG export
- Fixed color shifts during exposure adjustments

## [0.2.2] - October 29, 2025

### ✨ New Features
- **Whites and Blacks controls** - Fine-tune your brightest and darkest tones
  - **Whites**: Adjust bright areas without affecting midtones
  - **Blacks**: Control shadow depth independently
  - Both work without shifting colors (unlike some other editors!)
- **Double-click to reset** - Double-click any slider to instantly reset it to default

### 👍 Improvements
- **Better display on regular monitors** - Photos now look brighter and more natural on standard (non-HDR) displays
  - Fixed overly dark images caused by aggressive tone mapping
  - Highlights are preserved without crushing midtones
- **More room for controls** - Adjustment panel can now be wider for easier reading
- **Better window sizing** - Window won't get too small and cause controls to overlap

### 🐛 Bug Fixes
- **Fixed crash on startup** - No more crashes when opening the app
- **Fixed dark images** - Images display at proper brightness now
- **Fixed overlapping panels** - UI elements stay in their proper places
- **Fixed XMP compatibility** - Adjustment files work correctly

### 🗑️ Removed
- Temperature and Tint sliders (replaced with the more useful Whites and Blacks controls)

## [0.2.1] - Previous Release

### ✨ Major Features Added
- **Hollywood-grade color science** - Full ACES workflow (same as used in movies!)
- **HDR support** - View and export photos for HDR displays
- **Advanced tone controls**:
  - Recover blown-out highlights
  - Lift shadow details
  - Separate contrast for highlights, midtones, and shadows
- **Smart color adjustments**:
  - **Vibrance** - Boost muted colors without oversaturating
  - **Saturation** - Overall color intensity
- **Professional sharpening** - Edge-aware sharpening without halos or artifacts
- **Fujifilm X-Trans support** - Proper demosaicing for Fujifilm cameras
- **Auto-save** - Your adjustments are automatically saved to XMP files
- **GPU acceleration** - Real-time editing powered by your graphics card

### 👍 Improvements
- Reorganized interface for better workflow
- Better sharpening that doesn't create halos
- More accurate color and brightness calculations
- Smoother, more natural-looking contrast adjustments

### 🐛 Bug Fixes
- Fixed XMP file compatibility issues
- Reduced excessive file writes
- Fixed image zoom behavior

## [0.1.0] - Initial Development

### ✨ First Release!
- Load RAW files from all major camera brands
- GPU-accelerated real-time editing
- Modern, responsive interface
- Basic adjustments:
  - Exposure (brightness)
  - Contrast
  - Sharpness
- Instant preview of your changes

---

## About Version Numbers

We use version numbers like `0.2.2` to track changes:
- **First number (0.X.X)**: Major version - big changes or new features
- **Second number (0.2.X)**: Minor version - new features that don't break existing functionality  
- **Third number (0.2.2)**: Patch version - bug fixes and small improvements
