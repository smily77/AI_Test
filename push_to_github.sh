#!/usr/bin/env bash
set -euo pipefail

if [ $# -lt 1 ]; then
  echo "Usage: $0 <github_repo_url> [branch]" >&2
  exit 1
fi

remote_url="$1"
branch="${2:-$(git rev-parse --abbrev-ref HEAD)}"

if ! git rev-parse --git-dir >/dev/null 2>&1; then
  echo "Error: this script must be run inside a Git repository" >&2
  exit 1
fi

if git remote get-url origin >/dev/null 2>&1; then
  git remote set-url origin "$remote_url"
else
  git remote add origin "$remote_url"
fi

git push -u origin "$branch"
