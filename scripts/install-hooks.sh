#!/bin/bash
REPO_BASE=$(git rev-parse --show-toplevel)
SCRIPT_DIR="$REPO_BASE/scripts/hooks"
INSTALL_DIR="$REPO_BASE/.git/hooks"

echo "Repository base directory:$REPO_BASE"

mkdir -p "$INSTALL_DIR"
cp -a "${SCRIPT_DIR}"/* "${INSTALL_DIR}"
echo "All webhooks have been copied"
