#!/bin/bash

DIR="symlink_depth_test"
TARGET="a"
LINK=""

rm -rf "$DIR"
mkdir "$DIR"
cd "$DIR"
touch "$TARGET"
echo "test" > "$TARGET"

depth=0

while true; do
    LINK="link_$depth"
    ln -s "$TARGET" "$LINK" 2>/dev/null
    exec 3<"$LINK" 2>/dev/null
    if [ $? -ne 0 ]; then
        break
    fi
    exec 3<&-
    TARGET="$LINK"
    ((depth++))
done

echo "$depth"
cd ..
rm -rf "$DIR"
