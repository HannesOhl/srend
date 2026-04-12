#!/bin/bash
# this script is probably only useful for myself. I use it to push main repo
# and subtrees simultaneously

set -e

eval "$(ssh-agent -s)"
ssh-add ~/.ssh/id_ed25519

git subtree pull --prefix=tools/lalg lalg-remote main --squash
git subtree pull --prefix=tools/objToC objToC-remote main --squash
git subtree pull --prefix=tools/pngToBin pngToBin-remote main --squash
