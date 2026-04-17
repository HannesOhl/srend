#!/bin/bash
# this script is probably only useful for myself. I use it to push main repo
# and submodules simultaneously

RED='\033[0;31m'
GREEN='\033[0;32m'
CYAN='\033[0;36m'
NC='\033[0m'

set -e

MSG="update"

while [[ $# -gt 0 ]]; do
    case $1 in
        -m|--message)
            MSG="$2"
            shift 2
            ;;
        *)
            echo -e "${RED}Unknown argument: $1${NC}"
            exit 1
            ;;
    esac
done

eval "$(ssh-agent -s)"
ssh-add ~/.ssh/id_ed25519

# push submodules first
echo -e "${CYAN}Pushing submodules...${NC}"

for dir in tools/lalg tools/objToC tools/pngToBin; do
    if [ -d "$dir" ]; then
        echo -e "-- $dir"

        if ! git -C "$dir" symbolic-ref -q HEAD >/dev/null; then
            echo -e "${CYAN}$dir is detached; switching to main${NC}"
            git -C "$dir" switch main
        fi

        if [ -n "$(git -C "$dir" status --porcelain)" ]; then
            git -C "$dir" add -A
            git -C "$dir" commit -m "$MSG"
        else
            echo -e "No changes in $dir."
        fi

        git -C "$dir" pull --rebase origin main
        git -C "$dir" push origin main
    fi
done
# update main repo pointers
echo -e "${CYAN}Updating submodule pointers...${NC}"
git add tools/lalg tools/objToC tools/pngToBin

if ! git diff --cached --quiet; then
    git commit -m "$MSG"
fi

# push main repo
echo -e "${CYAN}Pushing main repo...${NC}"
git push

echo -e "${GREEN}Done.${NC}"
