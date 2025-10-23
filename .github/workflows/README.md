# GitHub Actions Workflows

This directory contains automated CI/CD workflows for Shaback.

## Workflows

### 1. CI (`ci.yml`)

**Triggers:**
- Push to any branch
- Pull requests

**Jobs:**
- **Build**: Compiles Shaback on Ubuntu 20.04 and 22.04
- **Static Analysis**: Runs cppcheck for code quality
- **Build Summary**: Aggregates results

**Artifacts:**
- Built binaries (7 days retention)
- Static analysis reports (30 days retention)

### 2. Docker Build (`docker.yml`)

**Triggers:**
- Push to main/master/release branches
- Git tags (v*.*.*)
- Pull requests (build only, no push)
- Manual workflow dispatch

**Jobs:**
- Builds multi-stage Docker image
- Pushes to Docker Hub (flockerdub/shaback)
- Automatic tagging (latest, version tags, branch names)

**Requirements:**
- Docker Hub credentials in repository secrets:
  - `DOCKERHUB_USERNAME`
  - `DOCKERHUB_TOKEN`

### 3. Release (`release.yml`)

**Triggers:**
- Git tags matching `v*.*.*` (e.g., v1.33.0)

**Jobs:**
- Builds release binary
- Creates source and binary tarballs
- Generates SHA256 checksums
- Creates GitHub Release with artifacts

**Artifacts:**
- `shaback-vX.X.X-linux-x86_64.tar.gz` - Binary release
- `shaback-vX.X.X-source.tar.gz` - Source code
- `SHA256SUMS` - Checksums

## Setup Instructions

### For Docker Workflow

1. Create Docker Hub access token at https://hub.docker.com/settings/security
2. Add secrets to GitHub repository:
   - Go to Settings → Secrets and variables → Actions
   - Add `DOCKERHUB_USERNAME` (your Docker Hub username)
   - Add `DOCKERHUB_TOKEN` (your access token)

### Creating a Release

```bash
# Tag a new version
git tag -a v1.34.0 -m "Release version 1.34.0"

# Push the tag
git push origin v1.34.0
```

This will automatically:
- Trigger the Release workflow
- Build binaries and create tarballs
- Create a GitHub Release
- Trigger the Docker workflow to build and push versioned images

## Status Badges

Add these to your main README.md:

```markdown
![CI](https://github.com/workflo/shaback/workflows/CI/badge.svg)
![Docker Build](https://github.com/workflo/shaback/workflows/Docker%20Build/badge.svg)
```

## Local Testing

Test workflow syntax:
```bash
# Install act (https://github.com/nektos/act)
curl https://raw.githubusercontent.com/nektos/act/master/install.sh | sudo bash

# Run CI workflow locally
act -W .github/workflows/ci.yml
```
