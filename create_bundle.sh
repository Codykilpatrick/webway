#!/bin/bash
# create-bundle.sh
LAST_SYNC=$(git tag -l "bundle-*" | sort -V | tail -1)
BUNDLE_NAME="$(date +%Y%m%d)-bundle.bundle"

if [ -z "$LAST_SYNC" ]; then
    git bundle create "$BUNDLE_NAME" --all
else
    git bundle create "$BUNDLE_NAME" "$LAST_SYNC..HEAD"
fi

git tag -a "$(date +%Y%m%d)-bundle" -m "Bundle created"