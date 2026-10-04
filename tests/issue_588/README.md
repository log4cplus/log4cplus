# Issue #588 restart reproducer

This Linux reproducer checks fixed-file `TimeBasedRollingFileAppender` restarts
using the issue's configuration, with POSIX path separators. It runs separate
processes with `AsyncAppend` enabled and disabled, waits for pending messages,
and shuts down normally. A preload shim changes wall-clock time without changing
monotonic time or the host clock. The runner also stamps file modification times
to match the simulated write times before restarting.

The checks require each completed day to have its own archive after a restart,
and verify rollover in a process that stays running across midnight. Repeated
same-day shutdown with `RollOnClose=true` still replaces an existing archive;
the runner records that remaining issue separately from restart recovery.

Run these commands from the repository root. Use `-std=c++23` for the driver on
`master`, and `-std=c++11` on `2.2.x`:

```sh
cmake -S . -B build/issue588 -G Ninja -DCMAKE_BUILD_TYPE=Debug -DWITH_UNIT_TESTS=OFF -DLOG4CPLUS_BUILD_TESTING=OFF -DLOG4CPLUS_BUILD_LOGGINGSERVER=OFF -DUNICODE=OFF
nice -n 10 cmake --build build/issue588 --target log4cplus -j4
mkdir -p build/issue588/repro
cc -std=c11 -shared -fPIC -O2 -Wall -Wextra tests/issue_588/fakeclock.c -o build/issue588/repro/fakeclock.so
c++ -std=c++11 -g -Wall -Wextra -Iinclude -Ibuild/issue588/include tests/issue_588/repro.cxx -Lbuild/issue588/src -Wl,-rpath,'$ORIGIN/../src' -llog4cplus -ldl -pthread -o build/issue588/repro/repro
python3 tests/issue_588/run.py build/issue588
```

For an alternate build directory, adjust the include, library, and output paths
and pass that directory to `run.py`. Each run stores process output and every
file-content snapshot in a new `repro/results-*/results.json`; the most recent
directory is recorded in `repro/latest-results.txt`.
