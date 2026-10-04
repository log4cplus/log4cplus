#!/usr/bin/env python3
"""Check #588 restart recovery with real separate processes and controlled time."""
import datetime
import json
import os
from pathlib import Path
import select
import subprocess
import sys
import tempfile

REPO_ROOT = Path(__file__).resolve().parents[2]
BUILD = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else REPO_ROOT / "build/issue588"
HERE = BUILD / "repro"
CONFIG = """log4cplus.rootLogger=INFO, COMMON
log4cplus.logger.Hope=ALL
log4cplus.appender.COMMON=log4cplus::TimeBasedRollingFileAppender
log4cplus.appender.COMMON.File=${AppData}/Log/gw.log
log4cplus.appender.COMMON.FilenamePattern=${AppData}/Log/gw_%d{yyyy_MM_dd}.log
log4cplus.appender.COMMON.AsyncAppend={async_append}
log4cplus.appender.COMMON.CreateDirs=true
log4cplus.appender.COMMON.MaxHistory=30
log4cplus.appender.COMMON.CleanHistoryOnStart=false
log4cplus.appender.COMMON.RollOnClose={roll_on_close}
log4cplus.appender.COMMON.layout=log4cplus::PatternLayout
log4cplus.appender.COMMON.layout.ConversionPattern=%D{{%Y/%m/%d %H:%M:%S}} [%-5t] %-5p: %m [%M in %l]%n
log4cplus.appender.CONSOLE.filters.1=log4cplus::spi::LogLevelRangeFilter
log4cplus.appender.CONSOLE.filters.1.LogLevelMin=DEBUG
log4cplus.appender.CONSOLE.filters.1.LogLevelMax=FATAL
log4cplus.appender.CONSOLE.filters.1.AcceptOnMatch=true
"""


def epoch(day, hour=12):
    return int(datetime.datetime(2025, 11, day, hour, tzinfo=datetime.timezone.utc).timestamp())


def files(directory):
    return {p.name: p.read_text() for p in sorted((directory / "Log").glob("*.log"))}


def stamp_files(directory, timestamp):
    # LD_PRELOAD changes user-space time, but kernel file timestamps still use
    # real time. Match the simulated write time before the next process starts.
    for path in (directory / "Log").glob("*.log"):
        os.utime(path, (timestamp, timestamp))


class Process:
    def __init__(self, directory, initial_epoch):
        environment = dict(os.environ, TZ="UTC", AppData=str(directory),
                           ISSUE588_EPOCH=str(initial_epoch),
                           LD_PRELOAD=str(HERE / "fakeclock.so"))
        self.process = subprocess.Popen(
            [str(HERE / "repro"), str(directory / "repro.properties")],
            env=environment, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
            stderr=subprocess.PIPE, text=True, bufsize=1)
        self.output = []
        self.read("READY")

    def read(self, expected):
        if not select.select([self.process.stdout], [], [], 20)[0]:
            self.process.kill()
            raise RuntimeError(f"Timed out waiting for {expected}")
        line = self.process.stdout.readline().rstrip()
        self.output.append(line)
        if line != expected:
            self.process.kill()
            raise RuntimeError(f"Expected {expected}, got {line}")

    def log(self, timestamp, message):
        self.process.stdin.write(f"{timestamp} {message}\n")
        self.process.stdin.flush()
        self.read(f"LOGGED {message}")

    def close(self):
        self.process.stdin.close()
        self.process.stdin = None
        out, err = self.process.communicate(timeout=20)
        if self.process.returncode != 0:
            raise RuntimeError(f"Process exit {self.process.returncode}: {err}")
        return {"exit_code": self.process.returncode,
                "stdout": "\n".join(self.output) + "\n" + out, "stderr": err}


def scenario(root, name, async_append, roll_on_close, runs):
    directory = root / name
    directory.mkdir()
    (directory / "repro.properties").write_text(CONFIG.replace(
        "{async_append}", str(async_append).lower()).replace(
        "{roll_on_close}", str(roll_on_close).lower()).replace("{{", "{").replace("}}", "}"))
    steps = []
    for messages in runs:
        process = Process(directory, messages[0][0])
        for timestamp, message in messages:
            process.log(timestamp, message)
            stamp_files(directory, timestamp)
            steps.append({"action": "log " + message, "files": files(directory)})
        result = process.close()
        stamp_files(directory, messages[-1][0])
        steps.append({"action": "normal process shutdown", "files": files(directory),
                      "process": result})
    return {"directory": str(directory), "async_append": async_append,
            "roll_on_close": roll_on_close, "steps": steps}


def main():
    root = Path(tempfile.mkdtemp(prefix="results-", dir=HERE))
    report = {}
    for async_append in (True, False):
        suffix = "async" if async_append else "sync"
        report["same_day_" + suffix] = scenario(root, "same_day_" + suffix,
            async_append, True, [[(epoch(14), "first-run")],
                               [(epoch(14, 13), "second-run")]])
        report["daily_restart_" + suffix] = scenario(root, "daily_restart_" + suffix,
            async_append, False, [[(epoch(14), "day-one")],
                                [(epoch(15), "day-two")],
                                [(epoch(16), "day-three")]])
        report["live_midnight_" + suffix] = scenario(root, "live_midnight_" + suffix,
            async_append, False, [[(epoch(14), "day-one"),
                                 (epoch(15), "day-two")]])
        report["restart_then_midnight_" + suffix] = scenario(root, "restart_then_midnight_" + suffix,
            async_append, False, [[(epoch(14), "day-one")],
                                [(epoch(15), "day-two"),
                                 (epoch(16), "day-three")]])
    for suffix in ("async", "sync"):
        same_day = report["same_day_" + suffix]["steps"]
        first_archive = same_day[1]["files"]["gw_2025_11_14.log"]
        second_archive = same_day[3]["files"]["gw_2025_11_14.log"]
        assert "first-run" in first_archive
        assert "first-run" not in second_archive and "second-run" in second_archive
        daily = report["daily_restart_" + suffix]["steps"][-1]["files"]
        assert set(daily) == {"gw.log", "gw_2025_11_14.log", "gw_2025_11_15.log"}
        assert "day-one" in daily["gw_2025_11_14.log"]
        assert "day-two" not in daily["gw_2025_11_14.log"]
        assert "day-two" in daily["gw_2025_11_15.log"]
        assert "day-one" not in daily["gw_2025_11_15.log"]
        assert "day-three" in daily["gw.log"]
        assert "day-one" not in daily["gw.log"] and "day-two" not in daily["gw.log"]
        live = report["live_midnight_" + suffix]["steps"][-1]["files"]
        assert "day-one" in live["gw_2025_11_14.log"] and "day-two" in live["gw.log"]
        restarted = report["restart_then_midnight_" + suffix]["steps"][-1]["files"]
        assert "day-one" in restarted["gw_2025_11_14.log"]
        assert "day-two" in restarted["gw_2025_11_15.log"]
        assert "day-one" not in restarted["gw_2025_11_15.log"]
        assert "day-three" in restarted["gw.log"]
    metadata = {"revision": subprocess.check_output(
        ["git", "rev-parse", "HEAD"], cwd=REPO_ROOT, text=True).strip(),
        "platform": sys.platform, "timezone": "UTC", "unit_test_hooks": "WITH_UNIT_TESTS:BOOL=ON" in (BUILD / "CMakeCache.txt").read_text(),
        "configuration_changes": "POSIX path separators; scenario varies RollOnClose and AsyncAppend",
        "scenarios": report}
    (root / "results.json").write_text(json.dumps(metadata, indent=2) + "\n")
    (HERE / "latest-results.txt").write_text(str(root) + "\n")
    print("Confirmed on async and sync paths:")
    print("  RollOnClose=true: second same-day shutdown replaces the first archive")
    print("  RollOnClose=false: daily restarts preserve separate dated archives")
    print("  Live midnight control: normal rollover works")
    print("  Restart followed by midnight: each day is archived separately")
    print("Results:", root / "results.json")


if __name__ == "__main__":
    main()
