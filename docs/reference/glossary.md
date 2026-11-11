# Glossary

| Term | Meaning |
|---|---|
| **Action** (button) | Sixth input button. Its effect is decided by the game (R-Type: the Force) |
| **Action** (UI) | Text sent by a button to the driver: `connect`, `play`, `join:3`... |
| **Authoritative server** | The server decides everything; clients only send inputs and display |
| **Avatar** | The entity a player controls (R-Type: `player_ship`), spawned by `Respawn` |
| **Component** | Plain data attached to an entity (`Health`, `Sprite`...) |
| **Console rule** | Room rule that must be ON for console commands (R-Type: `cheats`) |
| **Datagram** | One UDP packet. One protocol message per datagram |
| **Director** | Invisible server entity whose script drives the game (R-Type: `campaign.py`) |
| **Driver** | The client class that feeds the view: `OnlineClient` or `LocalClient` |
| **ECS** | Entity Component System: entities made of components, logic in systems |
| **Entity** | An index plus a generation, identifying one object |
| **Generation** | Counter that invalidates old handles when an entity index is reused |
| **Hook** | Python function called by the engine: `on_spawn`, `on_update`, `on_damage`, `on_death`, `on_contact` |
| **Kind** | Number of `Replicated` telling clients which client prefab draws the entity |
| **Layer** | Collision category. `Collider.hits` lists the layers an entity can touch |
| **Match** | State of the game in a room: waiting, playing, over |
| **Network id** | Id of an entity in snapshots, different from the ECS index |
| **Player entity** | Entity holding a player's lives, score and values. Survives the avatar's death |
| **Player value** | Number per player shared with the HUD (`playerValues`) |
| **Prefab** | Named template of components in JSON |
| **Registry** | The ECS container: entities and component storages |
| **Replicated** | Component marking an entity as sent to clients |
| **Room** | A group of up to 4 players playing one game on the server |
| **Rule** | Option chosen when creating a room (`rules`) |
| **Scene** | Client screen: its entities, systems, music and escape action |
| **Seat / slot** | Player number in a room, 0 to 3 |
| **Simulation** | The authoritative world of one room (or of the solo game) |
| **Snapshot** | Full state sent to clients 20 times per second |
| **Solo mode** | Running the simulation inside the client, without server |
| **System** | Class that updates every entity having some components |
| **Tick** | One simulation step, 1/60 s |
| **Token** | Random 32-bit number identifying a client session |
| **UI store** | Client table of text values read by the HUD (`world.stage`, `pause.visible`...) |
| **Visual byte** | Field of `EntityState` holding the hit flash and the rotation |
| **World value** | Number shared with every client (`worldValues`) |
