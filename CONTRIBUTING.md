# Contributing to ZRaw Developer

Thank you for your interest in helping make ZRaw Developer better! 

This project is built by hobby photographers who wanted a free, professional-quality RAW editor for Linux. Whether you're a programmer or not, there are many ways you can contribute!

## Ways to Contribute (No Coding Required!)

### 🐛 Report Bugs
Found something that doesn't work? [Open an issue](https://github.com/jasonzondor/zraw-developer/issues) and tell us:
- What you were trying to do
- What happened instead
- What camera/RAW file you were using
- Screenshots if possible

### 💡 Suggest Features
Have an idea for a new feature? We'd love to hear it! Open an issue and describe:
- What feature you'd like to see
- How it would help your photography workflow
- Examples from other software (if applicable)

### 📝 Improve Documentation
Help make the docs clearer:
- Fix typos or confusing explanations
- Add examples or screenshots
- Translate documentation
- Write tutorials or guides

### 🧪 Test New Features
- Try out new features before they're released
- Report what works and what doesn't
- Share your experience with different cameras/RAW files

### 🎨 Share Your Work
- Create and share adjustment presets
- Show before/after examples
- Help other users in discussions

## For Developers

### Branch Strategy

We use a simple Git workflow:

- **`main`** - Stable, released code
- **`feature/*`** - Your new feature branches
- **`bugfix/*`** - Bug fix branches

### Getting Started with Code

**New to open source?** Don't worry! Here's a simple guide:

1. **Fork the repository**
   - Click the "Fork" button on GitHub
   - Clone your fork to your computer:
   ```bash
   git clone https://github.com/YOUR_USERNAME/zraw-developer.git
   cd zraw-developer
   ```

2. **Connect to the main repository**
   ```bash
   git remote add upstream https://github.com/jasonzondor/zraw-developer.git
   ```

3. **Create a branch for your changes**
   ```bash
   git checkout -b feature/describe-your-feature develop
   ```
   
   Examples: `feature/add-jpeg-export`, `bugfix/fix-slider-crash`

### Making Changes

1. **Build and test your changes**
   ```bash
   mkdir build && cd build
   cmake ..
   make -j$(nproc)
   ./zraw-developer /path/to/your/test-photo.cr2
   ```
   
   Test with different RAW files from different cameras if possible!

2. **Keep it simple and clean**
   - Match the existing code style (look at nearby code)
   - Add comments to explain tricky parts
   - Keep functions small and focused
   - Test your changes thoroughly

3. **Commit your changes with a clear message**
   ```bash
   git add .
   git commit -m "feat: add JPEG export functionality"
   ```

   **Commit message prefixes:**
   - `feat:` - New feature (e.g., "feat: add batch processing")
   - `fix:` - Bug fix (e.g., "fix: crash when opening RAF files")
   - `docs:` - Documentation only
   - `style:` - Code formatting (no functional changes)
   - `refactor:` - Code cleanup
   - `perf:` - Performance improvement

### Submitting a Pull Request

1. **Make sure your branch is up to date**
   ```bash
   git fetch upstream
   git rebase upstream/develop
   ```

2. **Push your changes to GitHub**
   ```bash
   git push origin feature/your-feature-name
   ```

3. **Create a Pull Request**
   - Go to GitHub and you'll see a "Compare & pull request" button
   - Write a clear description of what you changed and why
   - Link any related issues (e.g., "Fixes #123")
   - Submit the PR!

4. **Work with reviewers**
   - A maintainer will review your code
   - They might suggest changes—don't worry, this is normal!
   - Make the requested changes and push them
   - Your PR will update automatically

## Code Review Process

Don't be intimidated by code review! It's a normal part of open source development.

- A maintainer will review your code (usually within a few days)
- They'll check that it builds correctly and works as expected
- They might suggest improvements—this helps everyone learn!
- Once approved, your code will be merged

## Release Process

Maintainers handle releases, but here's how it works:

1. Features are merged into `develop`
2. When ready for release, `develop` is merged to `main`
3. A version tag is created (e.g., v0.3.0)
4. GitHub automatically builds and publishes the release

## What Needs Help?

### 🔥 High Priority (Most Needed)
- [ ] **Export functionality** - Save as TIFF, JPEG, PNG with proper color profiles
- [ ] **Batch processing** - Edit multiple photos at once
- [ ] **Preset system** - Save and load adjustment presets
- [ ] **Lens corrections** - Database of lens profiles for auto-correction
- [ ] **Noise reduction** - Clean up high-ISO images

### 📊 Medium Priority
- [ ] **Tone curve editor** - Fine-tune tones with curves
- [ ] **Local adjustments** - Brushes and gradients for selective editing
- [ ] **Histogram** - Visual representation of tones
- [ ] **Before/After view** - Compare original and edited side-by-side
- [ ] **Keyboard shortcuts** - Speed up workflow

### 💡 Nice to Have
- [ ] **Plugin system** - Let users extend functionality
- [ ] **Custom LUTs** - Import color grading LUTs
- [ ] **Performance improvements** - Make it even faster!
- [ ] **Windows and macOS support** - Reach more photographers
- [ ] **Film emulation presets** - Classic film looks

## Color Science Guidelines (For Advanced Contributors)

If you're working on the color processing pipeline:

1. **Maintain the processing order:**
   ```
   White Balance → Exposure → Tone Recovery → Contrast 
   → Color → Sharpening → Output Transform
   ```

2. **Use the right color space for each operation:**
   - **Linear RGB** - For exposure and white balance (physical light)
   - **Log space** - For contrast adjustments (perceptual)
   - **LCH** - For saturation (avoids hue shifts)
   - **ACES** - For tone mapping (film-like rendering)

3. **Test thoroughly with different types of photos:**
   - **High contrast scenes** - Sunsets, backlit subjects
   - **Portraits** - Skin tones must look natural
   - **Landscapes** - Skies and foliage are critical
   - **Low light** - High ISO noise handling
   - **Different cameras** - Canon, Nikon, Sony, Fujifilm, etc.

## Questions or Need Help?

- **Not sure where to start?** Open an issue and ask!
- **Want to discuss an idea?** Use GitHub Discussions
- **Found existing issues?** Comment on them to show interest
- **Stuck on something?** Don't hesitate to ask for help

Remember: Everyone was a beginner once. We're here to help!

Thank you for contributing! 🎨📸
