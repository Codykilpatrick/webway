# Changeset-Driven Release Workflow

This project uses **changesets** as the primary method for version control and releases. Every PR that includes user-facing changes **must** include at least one changeset.

## 🔄 Development Workflow

### 1. Making Changes
When you make changes that should be released:

```bash
# Make your code changes
git checkout -b feature/my-awesome-feature

# After making changes, create a changeset
yarn changeset

# Follow the prompts:
# - Select packages that changed
# - Choose version bump type (patch/minor/major)  
# - Write a summary of changes
# - Optionally add detailed description
```

### 2. Changeset Selection Guide

**Major (Breaking Changes):**
- Removing or renaming public APIs
- Changing function signatures
- Removing support for versions/platforms
- Any change that requires users to update their code

**Minor (New Features):**
- Adding new public APIs
- Adding new optional parameters
- New functionality that doesn't break existing code
- Performance improvements

**Patch (Bug Fixes):**
- Bug fixes
- Documentation updates
- Internal refactoring without API changes
- Dependency updates

### 3. Changeset File Example

```markdown
---
"@webway/data-parser": minor
---

Add support for APB22 file format parsing

This adds a new parser for the APB22 format with the following features:
- Binary parser for APB22 format
- Arrow and Protobuf output support  
- Comprehensive test coverage
- Updated documentation

Breaking changes: None
Migration: No action required
```

### 4. Creating Pull Requests

- **Include changeset files** in your PR
- PR validation will **fail** if no changeset is included
- Use `skip-changeset` label only for:
  - Documentation-only changes
  - CI/CD updates
  - Internal tooling
  - Test updates

### 5. Release Process

**Automated Release Flow:**

1. **Developer** creates PR with changeset → PR validation passes
2. **Maintainer** merges PR → Changesets accumulate on main
3. **Weekly/As needed**: Run `yarn release:prepare` → Creates release PR
4. **Maintainer** reviews release PR → Merges when ready
5. **Automation** creates GitHub release and publishes packages

## 🚀 Release Commands

```bash
# Create a changeset (always do this for changes)
yarn changeset

# Check current changeset status
yarn changeset:status

# Check if PR needs changesets (CI validation)
yarn changeset:check

# Consume changesets and create release PR (maintainers)
yarn release:prepare

# Create GitHub release after release PR is merged (automation)
yarn release:publish

# Manual version bump (alternative to release:prepare)
yarn version
```

## 📋 PR Checklist

Before submitting a PR with user-facing changes:

- [ ] Changes are tested
- [ ] Changeset is included (`yarn changeset`)
- [ ] Changeset describes what changed and why
- [ ] Version bump type is appropriate
- [ ] Breaking changes are documented
- [ ] Documentation is updated if needed

## 🎯 Benefits of This Workflow

- **Explicit control** over version bumps
- **Clear documentation** of what changed in each release  
- **Automated release notes** generated from changesets
- **Prevention of accidental releases** without proper documentation
- **Team coordination** on what's included in each release
- **Semantic versioning compliance** with manual oversight

## 🔍 Troubleshooting

**"My PR failed validation":**
- Add a changeset: `yarn changeset`
- Or add `skip-changeset` label if no release needed

**"I made a mistake in my changeset":**
- Edit the `.changeset/*.md` file directly
- Or delete it and run `yarn changeset` again

**"I want to combine multiple changes in one release":**
- Multiple PRs can each add changesets
- All changesets will be consumed together in the next release

**"I need to make a hotfix release":**
- Create changeset with appropriate version (usually patch)
- Run `yarn release:prepare` immediately
- Merge the release PR to publish
