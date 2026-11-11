# Linux netem measurements

UTC date: 2026-11-08T14:51:31.821182+00:00

Two headless clients use the production C++ `ClientConnection`, the real codec and the real transport. A fresh
server is started for each scenario. One client creates a room, the second joins, the game starts, then both
fire during the measurement. The create, join and start commands are sent only once by the test: any repetition
is done by the reliability layer of the project.

netem is applied to `lo` in a disposable network namespace, in both directions. A 100 ms delay affects each way,
so it adds about 200 ms to the round trip. The configured rate is shared by every packet on that interface.

| Scenario | Verdict | Median RTT A/B (ms) | Snapshots/s A/B | Received A/B (KiB/s) | Max gap A/B (s) |
|---|---|---|---|---|---|
| baseline | PASS | 2.00 / 2.00 | 20.00 / 20.00 | 2.04 / 2.04 | 0.05 / 0.05 |
| delay_100ms | PASS | 203.00 / 202.00 | 20.00 / 20.00 | 3.76 / 3.76 | 0.05 / 0.05 |
| loss_10pct | PASS | 2.00 / 2.00 | 18.15 / 18.20 | 1.84 / 1.84 | 0.15 / 0.20 |
| duplicate_10pct | PASS | 2.00 / 2.00 | 20.00 / 20.00 | 2.20 / 2.21 | 0.05 / 0.05 |
| reorder_25pct | PASS | 102.00 / 103.00 | 18.65 / 18.90 | 2.66 / 2.66 | 0.10 / 0.10 |
| combined | PASS | 163.00 / 155.00 | 17.15 / 17.25 | 2.66 / 2.69 | 0.19 / 0.15 |
| rate_128kbit | PASS | 88.00 / 86.00 | 20.00 / 20.00 | 2.72 / 2.74 | 0.06 / 0.06 |

## Criteria and limits

PASS means: both clients join and start, each one receives at least 10 snapshots during the measurement, no
applied tick goes backwards, no duplicate `ROOM_JOINED` or `GAME_STARTED` reaches the application, and snapshots
never stop for 5 seconds. An exception or a process timeout fails the scenario.

The RTT is the heartbeat one, sampled every second; a value can be reused if the next answer is lost. It does
not measure the delay between a key press and its display. The rate counts application bytes, protocol header
included, without UDP, IP or Ethernet headers. The snapshot rate counts the states actually delivered by the
client, after filtering and delta reconstruction.

One pass per scenario, two clients, one room: these runs prove neither behaviour under maximum load nor the
delivery of every game event. netem percentages are random parameters, not measured application loss rates.
The raw `*.qdisc.txt`, `*.server.log`, `*.client.log` files and `results.json` are written next to the report by
each run.
