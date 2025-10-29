# Contributing to ZRaw Developer

Thank you for your interest in contributing to ZRaw Developer! This document provides guidelines for contributing to the project.

## Development Workflow

### Branch Strategy

We use a simplified Git Flow:

- **`main`** - Production-ready code, protected branch
- **`develop`** - Integration branch for features
- **`feature/*`** - Feature branches
- **`bugfix/*`** - Bug fix branches
- **`hotfix/*`** - Critical fixes for production

### Getting Started

1. **Fork the repository**
   ```bash
   # Fork on GitHub, then clone your fork
   git clone https://github.com/YOUR_USERNAME/zraw-developer.git
   cd zraw-developer
   ```

2. **Add upstream remote**
   ```bash
   git remote add upstream https://github.com/jasonzondor/zraw-developer.git
   ```

3. **Create a feature branch**
   ```bash
   git checkout -b feature/your-feature-name develop
   ```

### Making Changes

1. **Build and test locally**
   ```bash
   mkdir build && cd build
   cmake ..
   make -j$(nproc)
   ./zraw-developer ../path/to/test.raf
   ```

2. **Follow coding standards**
   - Use C++17 features
   - Follow existing code style (4 spaces, no tabs)
   - Comment complex algorithms
   - Keep functions focused and small

3. **Commit your changes**
   ```bash
   git add .
   git commit -m "feat: add new adjustment slider"
   ```

   **Commit message format:**
   - `feat:` - New feature
   - `fix:` - Bug fix
   - `docs:` - Documentation changes
   - `style:` - Code style changes (formatting, etc.)
   - `refactor:` - Code refactoring
   - `perf:` - Performance improvements
   - `test:` - Adding tests
   - `chore:` - Maintenance tasks

### Submitting a Pull Request

1. **Update your branch**
   ```bash
   git fetch upstream
   git rebase upstream/develop
   ```

2. **Push to your fork**
   ```bash
   git push origin feature/your-feature-name
   ```

3. **Create Pull Request**
   - Go to GitHub and create a PR from your branch to `develop`
   - Fill out the PR template
   - Link any related issues
   - Request review

4. **Address feedback**
   - Make requested changes
   - Push updates to your branch
   - PR will update automatically

## Code Review Process

- All PRs require at least one approval
- CI must pass (build + tests)
- No merge conflicts
- Code follows project standards

## Release Process

Releases are created by maintainers:

1. Merge `develop` into `main`
2. Tag the release: `git tag -a v1.0.0 -m "Release v1.0.0"`
3. Push tag: `git push origin v1.0.0`
4. GitHub Actions will create the release automatically

## Areas for Contribution

### High Priority
- [ ] Export functionality (TIFF, JPEG, PNG with ICC profiles)
- [ ] Batch processing support
- [ ] Preset system
- [ ] Lens correction database
- [ ] Noise reduction algorithms

### Medium Priority
- [ ] Tone curve editor
- [ ] Local adjustment tools (gradients, brushes)
- [ ] Histogram display
- [ ] Before/After comparison view
- [ ] Keyboard shortcuts

### Nice to Have
- [ ] Plugin system
- [ ] Custom LUT support
- [ ] GPU compute shader optimization
- [ ] Multi-threaded RAW loading
- [ ] macOS and Windows builds

## Color Science Guidelines

When working on color processing:

1. **Maintain pipeline order:**
   - White Balance → Exposure → Tone Recovery → Contrast → Color → Sharpening → Output

2. **Use proper color spaces:**
   - Linear RGB for physical operations
   - Log space for contrast
   - LCH for saturation (not HSV!)
   - ACES for tone mapping

3. **Test with various images:**
   - High dynamic range scenes
   - Portraits (skin tones)
   - Landscapes (sky, foliage)
   - Low light / high ISO

## Questions?

- Open an issue for discussion
- Join our discussions on GitHub
- Check existing issues and PRs

Thank you for contributing! 🎨
