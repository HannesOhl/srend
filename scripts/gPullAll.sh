#!/bin/bash
# this script is probably only useful for myself. I use it to pull main repo
# and submodules simultaneously

#!/bin/bash
set -e

eval "$(ssh-agent -s)"
ssh-add ~/.ssh/id_ed25519

echo "Pulling main repo..."
git pull --recurse-submodules

echo "Updating submodules..."
git submodule update --init --recursive --remote

echo "Done."
