#!/usr/bin/env python3
"""Run real server/client integration tests inside a disposable Linux network namespace.

Build scripts/netem_client.cpp against engine_network first, then:
  NETEM_HOST_NS=$(readlink /proc/self/ns/net)
  unshare --user --map-root-user --net python3 scripts/netem_test.py \
    --host-netns "$NETEM_HOST_NS" --server build/netem/bin/r-type_server \
    --client build/netem/bin/netem_client
No host interface is changed. Logs and JSON remain in --output.
"""
import argparse
import datetime
import hashlib
import json
import os
from pathlib import Path
import platform
import subprocess
import time

ROOT = Path(__file__).resolve().parents[1]
CASES = {
    "baseline": [],
    "delay_100ms": ["delay", "100ms"],
    "loss_10pct": ["loss", "10%"],
    "duplicate_10pct": ["duplicate", "10%"],
    "reorder_25pct": ["delay", "50ms", "reorder", "25%", "50%"],
    "combined": ["delay", "80ms", "20ms", "distribution", "normal", "loss", "5%", "duplicate", "2%", "reorder", "25%", "50%"],
    "rate_128kbit": ["delay", "30ms", "rate", "128kbit"],
}


def command(*args, check=True):
    return subprocess.run(args, text=True, stdout=subprocess.PIPE,
                          stderr=subprocess.STDOUT, check=check).stdout


def report(output, data):
    (output / "results.json").write_text(json.dumps(data, indent=2) + "\n")
    lines = ["# Linux netem measurements", "", f"UTC date: {data['utc']}", "",
             "Two headless clients use the production C++ `ClientConnection`, the real codec and the real",
             "transport. A fresh server is started for each scenario. One client creates a room, the second",
             "joins, the game starts, then both fire during the measurement. The create, join and start",
             "commands are sent only once by the test: any repetition is done by the reliability layer.", "",
             "netem is applied to `lo` in a disposable network namespace, in both directions.",
             "A 100 ms delay affects each way, so it adds about 200 ms to the round trip.",
             "The configured rate is shared by every packet on that interface.", "",
             "| Scenario | Verdict | Median RTT A/B (ms) | Snapshots/s A/B | Received A/B (KiB/s) | Max gap A/B (s) |",
             "|---|---|---|---|---|---|"]
    for case in data["cases"]:
        r = case["result"]
        clients = r.get("clients", [])
        def values(key, scale=1):
            return " / ".join(f"{p[key] / scale:.2f}" for p in clients) or "-"
        lines.append(f"| {case['name']} | {'PASS' if r.get('passed') else 'FAIL'} | "
                     f"{values('ping_median_ms')} | {values('snapshots_per_second')} | "
                     f"{values('rx_bytes_per_second', 1024)} | {values('max_snapshot_gap_seconds')} |")
        if r.get("error"):
            lines.extend(["", f"Error in {case['name']}: `{r['error']}`", ""])
    lines.extend(["", "## Criteria and limits", "",
                  "PASS means: both clients join and start, each one receives at least 10 snapshots during",
                  "the measurement, no applied tick goes backwards, no duplicate `ROOM_JOINED` or `GAME_STARTED`",
                  "reaches the application, and snapshots never stop for 5 seconds.",
                  "An exception or a process timeout fails the scenario.", "",
                  "The RTT is the heartbeat one, sampled every second; a value can be reused if the next answer",
                  "is lost. It does not measure the delay between a key press and its display. The rate counts",
                  "application bytes, protocol header included, without UDP, IP or Ethernet headers. The snapshot",
                  "rate counts the states actually delivered by the client, after filtering and delta reconstruction.", "",
                  "One pass per scenario, two clients, one room: these runs prove neither behaviour under maximum",
                  "load nor the delivery of every game event. netem percentages are random parameters, not measured",
                  "application loss rates. See the *.qdisc.txt, *.server.log, *.client.log files and results.json.", ""])
    (output / "report.md").write_text("\n".join(lines))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--server", type=Path, required=True)
    parser.add_argument("--client", type=Path, required=True)
    parser.add_argument("--seconds", type=float, default=20)
    parser.add_argument("--output", type=Path, default=ROOT / "docs/measurements/netem-2026-11-08")
    parser.add_argument("--case", choices=CASES, action="append")
    parser.add_argument("--host-netns", required=True, help="readlink /proc/self/ns/net BEFORE unshare")
    args = parser.parse_args()
    if os.readlink("/proc/self/ns/net") == args.host_netns:
        parser.error("Run inside unshare --user --map-root-user --net; host network must remain untouched")
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    server, client = args.server.resolve(), args.client.resolve()
    data = {"utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
            "kernel": platform.platform(), "seconds_per_case": args.seconds,
            "commit": command("git", "rev-parse", "HEAD").strip(),
            "git_status": command("git", "status", "--short"),
            "server_sha256": hashlib.sha256(server.read_bytes()).hexdigest(),
            "client_sha256": hashlib.sha256(client.read_bytes()).hexdigest(),
            "config_sha256": hashlib.sha256((ROOT / "game/data/rtype_game.json").read_bytes()).hexdigest(),
            "tc_version": command("tc", "-V").strip(), "cases": []}
    command("ip", "link", "set", "lo", "up")
    try:
        for index, name in enumerate(args.case or CASES):
            command("tc", "qdisc", "del", "dev", "lo", "root", check=False)
            netem = ["tc", "qdisc", "add", "dev", "lo", "root", "netem", "limit", "1000", *CASES[name]]
            command(*netem)
            port = str(44440 + index)
            argv = [str(client), str(ROOT / "game/data/rtype_game.json"), port, str(args.seconds)]
            print(f"START {name}: {' '.join(netem)}", flush=True)
            with (output / f"{name}.server.log").open("w") as log:
                process = subprocess.Popen([str(server), "-p", port], cwd=ROOT,
                                           stdout=log, stderr=subprocess.STDOUT,
                                           env={**os.environ, "PYTHONDONTWRITEBYTECODE": "1"})
                try:
                    # Startup grace; setup includes its own CONNECT retry/timeout handling.
                    time.sleep(1)
                    if process.poll() is not None:
                        raise RuntimeError(f"server exited: {process.returncode}; see {name}.server.log")
                    probe = subprocess.run(argv, cwd=ROOT, text=True, stdout=subprocess.PIPE,
                                           stderr=subprocess.STDOUT, timeout=args.seconds + 25)
                    text = probe.stdout
                    (output / f"{name}.client.log").write_text(text)
                    records = [line[13:] for line in text.splitlines() if line.startswith("NETEM_RESULT ")]
                    result = json.loads(records[-1]) if records else {"passed": False, "error": "no probe result"}
                    if probe.returncode:
                        result["passed"] = False
                    if process.poll() is not None:
                        result.update(passed=False, error="server exited during test")
                except (RuntimeError, subprocess.TimeoutExpired) as error:
                    result = {"passed": False, "error": str(error)}
                finally:
                    process.terminate()
                    try:
                        process.wait(timeout=5)
                    except subprocess.TimeoutExpired:
                        process.kill()
                        process.wait()
            stats = command("tc", "-s", "qdisc", "show", "dev", "lo")
            (output / f"{name}.qdisc.txt").write_text(stats)
            data["cases"].append({"name": name, "netem_command": netem,
                                  "client_command": argv, "qdisc": stats, "result": result})
            report(output, data)
            print(f"DONE {name}: {'PASS' if result.get('passed') else 'FAIL'}", flush=True)
    finally:
        command("tc", "qdisc", "del", "dev", "lo", "root", check=False)
    return 0 if all(case["result"].get("passed") for case in data["cases"]) else 1


if __name__ == "__main__":
    raise SystemExit(main())
