# GitHub Repository Setup Guide

This guide will help you set up branch protection, PR requirements, and releases for ZRaw Developer.

## Step 1: Enable Branch Protection (Using Rulesets - Recommended)

**Note:** GitHub Rulesets are the modern replacement for classic branch protection rules. They're more flexible and future-proof.

### Quick Setup:

1. Go to your repository on GitHub
2. Click **Settings** → **Rules** → **Rulesets**
3. Click **New ruleset** → **New branch ruleset**

### Configure Ruleset for `main`:

**Ruleset Name:** `Protect main branch`
**Enforcement status:** `Active`
**Target branches:** `main`

**Enable these rules:**
- ✅ Require a pull request before merging (1 approval)
- ✅ Require status checks to pass (`build` from CI)
- ✅ Require conversation resolution
- ✅ Block force pushes
- ✅ Require linear history (optional but recommended)
- ✅ Restrict deletions

Click **Create**

### Detailed Instructions:

See **GITHUB_RULESET_SETUP.md** for complete step-by-step instructions.

### Alternative: Classic Branch Protection

If you prefer classic rules (not recommended for new repos):
- Go to **Settings** → **Branches** → **Add rule**
- See GITHUB_RULESET_SETUP.md for comparison

## Step 2: Set Up GitHub Actions

The workflows are already created in `.github/workflows/`:

### CI Workflow (`ci.yml`)
- Runs on every push and PR to `main` and `develop`
- Builds the project
- Ensures code compiles

### Release Workflow (`release.yml`)
- Triggers when you push a version tag (e.g., `v1.0.0`)
- Builds release binaries
- Creates GitHub release with artifacts

**No additional setup needed** - these will run automatically!

## Step 3: Development Workflow

### Daily Development:

```bash
# 1. Create a feature branch from develop
git checkout develop
git pull origin develop
git checkout -b feature/my-new-feature

# 2. Make your changes
# ... edit files ...

# 3. Commit with conventional commit messages
git add .
git commit -m "feat: add tone curve editor"

# 4. Push to your fork (or origin if you have write access)
git push origin feature/my-new-feature

# 5. Create Pull Request on GitHub
# - Target: develop (not main!)
# - Fill out PR template
# - Wait for CI to pass
# - Request review (if applicable)

# 6. After approval, merge PR
# - Use "Squash and merge" for clean history
# - Delete branch after merge
```

### Creating a Release:

```bash
# 1. Ensure develop is stable and tested
git checkout develop
git pull origin develop

# 2. Update CHANGELOG.md
# - Move items from [Unreleased] to [X.Y.Z]
# - Add release date

# 3. Create PR from develop to main
git checkout -b release/v1.0.0
# Update version in CMakeLists.txt if needed
git add CHANGELOG.md CMakeLists.txt
git commit -m "chore: prepare release v1.0.0"
git push origin release/v1.0.0

# 4. Create PR: release/v1.0.0 → main
# - Get approval
# - Merge to main

# 5. Tag the release
git checkout main
git pull origin main
git tag -a v1.0.0 -m "Release v1.0.0"
git push origin v1.0.0

# 6. GitHub Actions will automatically:
# - Build the release
# - Create GitHub release
# - Upload artifacts

# 7. Merge main back to develop
git checkout develop
git merge main
git push origin develop
```

## Step 4: Semantic Versioning

Use [Semantic Versioning](https://semver.org/):

- **v1.0.0** - Major release (breaking changes)
- **v0.1.0** - Minor release (new features)
- **v0.0.1** - Patch release (bug fixes)

### Examples:
- `v0.1.0` - First public release
- `v0.2.0` - Added export functionality
- `v0.2.1` - Fixed XMP bug
- `v1.0.0` - Stable API, production ready

## Step 5: Release Checklist

Before creating a release:

- [ ] All features tested
- [ ] CHANGELOG.md updated
- [ ] Version bumped in CMakeLists.txt
- [ ] README.md updated (if needed)
- [ ] No known critical bugs
- [ ] CI passing on develop
- [ ] Documentation up to date

## Step 6: Managing Contributions

### For Solo Development:
```bash
# You can work directly on develop
git checkout develop
git pull origin develop
# ... make changes ...
git add .
git commit -m "feat: add new feature"
git push origin develop

# For releases, still use the PR workflow to main
```

### For Team Development:
- Always use feature branches
- Require PR reviews
- Use branch protection
- Follow the full workflow above

## Step 7: Quick Commands Reference

```bash
# Create feature branch
git checkout -b feature/name develop

# Create bugfix branch
git checkout -b bugfix/name develop

# Create hotfix branch (from main)
git checkout -b hotfix/name main

# Update branch from upstream
git fetch origin
git rebase origin/develop

# Squash commits before PR
git rebase -i HEAD~3

# Create and push tag
git tag -a v1.0.0 -m "Release v1.0.0"
git push origin v1.0.0

# Delete merged branch
git branch -d feature/name
git push origin --delete feature/name
```

## Step 8: First Release

To create your first release (v0.1.0):

```bash
# 1. Update CHANGELOG.md
# Move all [Unreleased] items to [0.1.0] - 2025-01-XX

# 2. Commit
git add CHANGELOG.md
git commit -m "chore: prepare v0.1.0 release"
git push origin develop

# 3. Create release PR
git checkout -b release/v0.1.0
git push origin release/v0.1.0
# Create PR: release/v0.1.0 → main

# 4. After merge, tag it
git checkout main
git pull origin main
git tag -a v0.1.0 -m "Initial release v0.1.0"
git push origin v0.1.0

# 5. Check GitHub Actions
# Go to Actions tab to see release build

# 6. Verify release
# Go to Releases tab to see your new release!
```

## Troubleshooting

### CI Fails
- Check Actions tab for error logs
- Ensure dependencies are correct in workflow
- Test build locally first

### Can't Push to Main
- This is correct! Main is protected
- Create a PR instead

### Tag Already Exists
```bash
# Delete local tag
git tag -d v1.0.0
# Delete remote tag
git push origin --delete v1.0.0
# Create new tag
git tag -a v1.0.0 -m "Release v1.0.0"
git push origin v1.0.0
```

## Resources

- [GitHub Flow](https://guides.github.com/introduction/flow/)
- [Semantic Versioning](https://semver.org/)
- [Conventional Commits](https://www.conventionalcommits.org/)
- [Keep a Changelog](https://keepachangelog.com/)

---

**You're all set!** 🚀 Your repository now has professional workflows for development and releases.
