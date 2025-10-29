# GitHub Rulesets Setup (Modern Approach)

GitHub Rulesets are the modern replacement for classic branch protection rules. They offer better flexibility and organization.

## Setting Up Rulesets for `main` Branch

### Step 1: Navigate to Rulesets

1. Go to your repository on GitHub
2. Click **Settings** → **Rules** → **Rulesets**
3. Click **New ruleset** → **New branch ruleset**

### Step 2: Configure Basic Settings

**Ruleset Name:** `Protect main branch`

**Enforcement status:** `Active`

**Bypass list:** (optional)
- Add yourself if you need emergency access
- Or leave empty for strict enforcement

### Step 3: Target Branches

**Target branches:**
- Click **Add target**
- Select **Include by pattern**
- Pattern: `main`
- Click **Add inclusion pattern**

### Step 4: Configure Rules

Enable the following rules:

#### ✅ **Require a pull request before merging**
- **Required approvals:** `1`
- ✅ Dismiss stale pull request approvals when new commits are pushed
- ✅ Require review from Code Owners (optional, if you have CODEOWNERS file)
- ✅ Require approval of the most recent reviewable push
- ❌ Require conversation resolution before merging (enable this separately)

#### ✅ **Require status checks to pass**
- ✅ Require branches to be up to date before merging
- **Status checks that are required:**
  - Add: `build` (this comes from your CI workflow)
  - Note: This will appear after your first CI run

#### ✅ **Require conversation resolution before merging**
- Ensures all PR comments are resolved

#### ✅ **Block force pushes**
- Prevents rewriting history on main

#### ✅ **Require linear history** (optional but recommended)
- Enforces a clean, linear git history
- Prevents merge commits (only allows squash or rebase)

#### ❌ **Require deployments to succeed** (skip for now)

#### ❌ **Require signed commits** (optional, for extra security)
- Enable if you want to require GPG signatures

### Step 5: Additional Settings (at bottom)

**Restrict creations:** ❌ (allow creating main branch)
**Restrict updates:** ✅ (this is what protects the branch)
**Restrict deletions:** ✅ (prevent accidental deletion)

**Require code scanning results:** ❌ (skip unless you have CodeQL set up)

### Step 6: Save

Click **Create** at the bottom

---

## Setting Up Rulesets for `develop` Branch (Optional)

If you want to protect `develop` as well:

1. Click **New ruleset** → **New branch ruleset**
2. **Ruleset Name:** `Protect develop branch`
3. **Target branches:** `develop`
4. **Rules:** (slightly relaxed)
   - ✅ Require status checks to pass
   - ✅ Block force pushes
   - ❌ Require pull request (optional - you can push directly to develop)
   - ✅ Require linear history

---

## Comparison: Rulesets vs Classic Branch Protection

| Feature | Classic Rules | Rulesets |
|---------|--------------|----------|
| Multiple branch patterns | ❌ One rule per pattern | ✅ Multiple patterns per ruleset |
| Organization-wide rules | ❌ | ✅ |
| Better UI/UX | ❌ | ✅ |
| More granular controls | ❌ | ✅ |
| Insights and reporting | Limited | Better |
| Future-proof | Deprecated path | ✅ Active development |

**Recommendation:** Use Rulesets for all new repositories.

---

## Verifying Your Ruleset

After creating the ruleset:

1. Go to **Settings → Rules → Rulesets**
2. You should see your ruleset listed as **Active**
3. Click on it to view/edit

### Test It:

```bash
# Try to push directly to main (should fail)
git checkout main
echo "test" >> test.txt
git add test.txt
git commit -m "test"
git push origin main
# ❌ Should be rejected!

# Correct way: create PR
git checkout -b test-branch
git push origin test-branch
# ✅ Then create PR on GitHub
```

---

## Ruleset for Tag Protection (Optional)

To protect release tags:

1. Click **New ruleset** → **New tag ruleset**
2. **Ruleset Name:** `Protect release tags`
3. **Target tags:** `v*.*.*` (matches v1.0.0, v0.1.0, etc.)
4. **Rules:**
   - ✅ Block force pushes
   - ✅ Restrict deletions
5. **Create**

This prevents accidental deletion or modification of release tags.

---

## Migration from Classic Rules

If you already have classic branch protection rules:

1. Note down your existing rules
2. Create equivalent rulesets (as above)
3. Test the rulesets work correctly
4. Delete the old classic rules:
   - Go to **Settings → Branches**
   - Click **Delete** on old rules

**Note:** Rulesets and classic rules can coexist, but it's cleaner to use one system.

---

## Quick Reference

### Main Branch Ruleset (Strict):
- ✅ Require PR with 1 approval
- ✅ Require status checks (CI must pass)
- ✅ Require conversation resolution
- ✅ Block force pushes
- ✅ Require linear history
- ✅ Restrict deletions

### Develop Branch Ruleset (Relaxed):
- ✅ Require status checks
- ✅ Block force pushes
- ❌ PR not required (can push directly)

### Tag Ruleset (Release Protection):
- Pattern: `v*.*.*`
- ✅ Block force pushes
- ✅ Restrict deletions

---

## Troubleshooting

### "Status check 'build' not found"
- This appears after your first CI run
- Push a commit to trigger CI, then add the status check

### "Cannot push to main"
- This is correct! Create a PR instead
- Or add yourself to bypass list (not recommended)

### "Ruleset not applying"
- Check **Enforcement status** is **Active**
- Verify branch pattern matches (e.g., `main` not `refs/heads/main`)
- Check you're not in the bypass list

---

## Resources

- [GitHub Rulesets Documentation](https://docs.github.com/en/repositories/configuring-branches-and-merges-in-your-repository/managing-rulesets/about-rulesets)
- [Migrating from Branch Protection](https://docs.github.com/en/repositories/configuring-branches-and-merges-in-your-repository/managing-rulesets/migrating-from-branch-protection-rules-to-rulesets)

---

**You're all set with modern GitHub Rulesets!** 🎉
