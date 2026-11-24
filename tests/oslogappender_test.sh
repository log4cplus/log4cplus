#!/bin/sh
# SPDX-License-Identifier: Apache-2.0
set -eu

emitter=${1:?expected path to propertyconfig_test}
workdir=$(mktemp -d "${TMPDIR:-/tmp}/oslogappender.XXXXXXXX")
child=
cleanup ()
{
    if [ -n "$child" ]; then
        kill -KILL "$child" 2>/dev/null || :
        wait "$child" 2>/dev/null || :
    fi
    rm -rf "$workdir"
}
trap cleanup EXIT
trap 'exit 1' HUP INT TERM

# Share the polling budget across queries, including time inside log show.
pause_one_second ()
{
    if [ "$remaining" -eq 0 ]; then
        echo "Timed out waiting for $*" >&2
        exit 1
    fi
    sleep 1
    remaining=$((remaining - 1))
}

run_bounded ()
{
    "$@" &
    child=$!
    while kill -0 "$child" 2>/dev/null; do
        pause_one_second "$@"
    done
    child_status=0
    wait "$child" || child_status=$?
    child=
    return "$child_status"
}

subsystem=org.log4cplus.tests
category=oslogappender
token=$(uuidgen)
cat > "$workdir/oslog.properties" <<EOF
log4cplus.rootLogger=WARN, OS
log4cplus.logger.filelogger=OFF
log4cplus.appender.OS=log4cplus::OSLogAppender
log4cplus.appender.OS.Subsystem=$subsystem
log4cplus.appender.OS.Category=$category
log4cplus.appender.OS.Privacy=public
log4cplus.appender.OS.layout=log4cplus::PatternLayout
log4cplus.appender.OS.layout.ConversionPattern=$token %% %p %m
EOF

remaining=30
run_bounded "$emitter" "$workdir/oslog.properties"

expected="$token % WARN Testing...."
predicate="subsystem == \"$subsystem\" AND category == \"$category\""
predicate="$predicate AND messageType == default AND eventMessage == \"$expected\""
remaining=30
while :; do
    run_bounded log show --last 2m --style default --color none --predicate "$predicate" > "$workdir/records"
    # Only event rows start with a date. The header and summary do not count.
    count=$(awk '/^[0-9]/ { n++ } END { print n+0 }' "$workdir/records")
    case $count in
        1) exit 0 ;;
        0) pause_one_second "$predicate" ;;
        *) echo "Expected one record matching $predicate, found $count" >&2
           cat "$workdir/records" >&2
           exit 1 ;;
    esac
done
