# Linux netem measurements

UTC date: 2026-11-10T12:39:28.045939+00:00

Two headless clients use the production C++ `ClientConnection`, the real codec and the real
transport. A fresh server is started for each scenario. One client creates a room, the second
joins, the game starts, then both fire during the measurement. The create, join and start
commands are sent only once by the test: any repetition is done by the reliability layer.

netem is applied to `lo` in a disposable network namespace, in both directions.
A 100 ms delay affects each way, so it adds about 200 ms to the round trip.
The configured rate is shared by every packet on that interface.

| Scenario | Verdict | Median RTT A/B (ms) | Snapshots/s A/B | Received A/B (KiB/s) | Max gap A/B (s) |
|---|---|---|---|---|---|
| baseline | PASS | 2.00 / 2.00 | 20.00 / 20.00 | 2.03 / 2.03 | 0.05 / 0.05 |
| delay_100ms | PASS | 202.00 / 202.00 | 20.00 / 20.00 | 4.05 / 4.05 | 0.05 / 0.05 |
| loss_10pct | PASS | 2.00 / 2.00 | 17.85 / 17.90 | 2.01 / 2.02 | 0.20 / 0.15 |
| duplicate_10pct | PASS | 2.00 / 2.00 | 20.00 / 20.00 | 2.25 / 2.25 | 0.05 / 0.05 |
| reorder_25pct | PASS | 102.00 / 103.00 | 18.05 / 18.85 | 2.58 / 2.58 | 0.10 / 0.10 |
| combined | PASS | 148.00 / 112.00 | 17.00 / 17.75 | 2.65 / 2.69 | 0.19 / 0.18 |
| rate_128kbit | PASS | 101.00 / 106.00 | 20.00 / 19.95 | 2.72 / 2.73 | 0.06 / 0.07 |

## Criteria and limits

PASS means: both clients join and start, each one receives at least 10 snapshots during
the measurement, no applied tick goes backwards, no duplicate `ROOM_JOINED` or `GAME_STARTED`
reaches the application, and snapshots never stop for 5 seconds.
An exception or a process timeout fails the scenario.

The RTT is the heartbeat one, sampled every second; a value can be reused if the next answer
is lost. It does not measure the delay between a key press and its display. The rate counts
application bytes, protocol header included, without UDP, IP or Ethernet headers. The snapshot
rate counts the states actually delivered by the client, after filtering and delta reconstruction.

One pass per scenario, two clients, one room: these runs prove neither behaviour under maximum
load nor the delivery of every game event. netem percentages are random parameters, not measured
application loss rates. See the *.qdisc.txt, *.server.log, *.client.log files and results.json.
