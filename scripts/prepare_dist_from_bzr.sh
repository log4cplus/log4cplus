#!/bin/bash

set -e

THIS_SCRIPT=`basename "$0"`

function usage
{
    
    echo "$THIS_SCRIPT <BZR_URL>"
}

BZR_URL="$1"
if [[ -z "$BZR_URL" ]] ; then
    usage
    exit 1
fi

if [[ -z "$2" ]] ; then
    BZR_URL=${BZR_URL%/}
    BZR_BASE_URL=${BZR_URL%/*}
    BZR_BRANCH_NAME=${BZR_URL#$BZR_BASE_URL}
    BZR_BRANCH_NAME=${BZR_BRANCH_NAME#/}
    SRC_DIR=$BZR_BRANCH_NAME
else
    SRC_DIR="$2"
fi

DEST_DIR="$PWD"

TMPDIR=${TMPDIR:-${TMP:-${TEMP}}}
export TMPDIR
TMP_DIR=`mktemp -d`
pushd "$TMP_DIR"

bzr export --per-file-timestamps -v "$SRC_DIR" "$BZR_URL"
bzr version-info "$BZR_URL" >"$SRC_DIR/REVISION"

pushd "$SRC_DIR"
$SHELL ./scripts/fix-timestamps.sh
popd

7za a -t7z "$DEST_DIR/$SRC_DIR".7z "$SRC_DIR" >/dev/null \
& 7za a -tzip "$DEST_DIR/$SRC_DIR".zip "$SRC_DIR" >/dev/null

TAR_FILE="$SRC_DIR".tar
bsdtar -cvf "$TAR_FILE" "$SRC_DIR"

xz -e -c "$TAR_FILE" >"$DEST_DIR/$TAR_FILE".xz \
& bzip2 -9 -c "$TAR_FILE" >"$DEST_DIR/$TAR_FILE".bz2 \
& gzip -9 -c "$TAR_FILE" >"$DEST_DIR/$TAR_FILE".gz

echo waiting...
wait
echo done waiting

rm -rf "$SRC_DIR"
rm -f "$TAR_FILE"
popd
rmdir "$TMP_DIR"



