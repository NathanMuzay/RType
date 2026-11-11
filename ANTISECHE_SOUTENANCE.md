# Anti-sèche — Soutenance R-Type

> Note de travail interne (français), comme [PLAN_RESEAU.md](PLAN_RESEAU.md) et [trackNet.md](trackNet.md).
> Structurée sur le barème de la midterm : pour chaque critère, le reproche reçu, ce qu'on a fait depuis,
> la phrase à dire, et les questions pièges.
> La **partie III** est entièrement consacrée à la track réseau.

---

## Sommaire

| Partie | Contenu |
|---|---|
| **0** | Le pitch en 60 secondes + le plan de démo |
| **I** | Les notions à maîtriser (vocabulaire, définitions, pourquoi) |
| **II** | Défense critère par critère (les 13 lignes du barème) |
| **III** | **TRACK RÉSEAU** — ce qu'on a implémenté, pourquoi, et les preuves |
| **IV** | Les chiffres à connaître par cœur |
| **V** | Questions pièges du jury + réponses |
| **VI** | Les 12 fichiers à savoir ouvrir en 10 secondes |
| **VII** | Ce qu'on n'a pas fait, et comment le dire |

---
---

# PARTIE 0 — Le pitch et la démo

## Le pitch en 60 secondes

> « On n'a pas fait un jeu, on a fait **un moteur**, et R-Type est l'une des deux données qu'on lui donne.
> Le C++ est intégralement dans `engine/` ; `game/` ne contient que du JSON, du Python et des assets.
> La preuve, c'est `game2/` : **Skyhop**, un deuxième jeu, solo, à un seul bouton, qui tourne sur le même
> moteur sans une ligne de C++ en plus.
>
> Le jeu est en client/serveur : **serveur autoritaire à 60 ticks/s**, protocole **binaire sur UDP**, 20 octets
> d'en-tête, snapshots à 20 Hz **en delta contre la dernière photo acquittée par le client**, et une couche
> d'acquittement qui rend fiables uniquement les messages qui doivent l'être.
>
> Tout ce qu'on affirme est mesuré : on a un générateur de charge, une sonde protocolaire, et des mesures
> sous **`netem`** (perte, délai, duplication, réordonnancement, limitation de débit) avec les vrais binaires. »

## Plan de démo (ordre conseillé)

| # | Ce qu'on montre | Ce que ça coche dans le barème |
|---|---|---|
| 1 | `r-type_server -p 4242` + 3 clients, lobby → room → partie | Networked server, prototype jouable, fluidité |
| 2 | Un joueur tire, meurt, respawn, boss | Prototype, interactions, autorité serveur |
| 3 | Le **chat** dans la room, avec pseudos | Track thème 1, fiabilité visible |
| 4 | On **tue un client** (Ctrl-C) → les autres continuent, message « a quitté » | Error handling, décou­plage réseau |
| 5 | `nc -u 127.0.0.1 4242` puis taper n'importe quoi → le serveur ignore et continue | Error handling, validation |
| 6 | `./scripts/protocol_probe.py 4242 0.4` → client Python qui joue avec 40 % de perte | Protocole documenté + fiabilité |
| 7 | `./scripts/load_test.py 4242 48 10` → 48 parties, 60.0 ticks/s | Plusieurs instances en parallèle |
| 8 | `./skyhop` | **Le moteur est générique** |
| 9 | `mkdocs serve` → la doc, dont `protocol.md` | README + doc protocole |

> **Astuce** : garde `docs/measurements/netem-2026-11-08/report.md` ouvert dans un onglet.
> Un tableau de 7 scénarios tous en PASS, c'est l'argument le plus court de toute la soutenance.

---
---

# PARTIE I — Les notions à maîtriser

## 1. Architecture et POO

| Notion | Définition courte | Chez nous |
|---|---|---|
| **Moteur vs jeu** | Le moteur ne connaît aucun jeu ; le jeu est de la donnée | `engine/` = C++ ; `game/`, `game2/` = JSON + Python + assets |
| **Data-driven** | Le comportement vient de fichiers lus à l'exécution, pas du code compilé | Entités, scènes, HUD, systèmes, réglages réseau : tout en JSON |
| **Couplage / découplage** | A dépend de B si A ne compile pas sans B | 13 bibliothèques CMake séparées, dépendances déclarées une par une |
| **Dépendance privée** | Une lib tierce n'apparaît dans aucun header public | Asio dans `engine/net/internal` uniquement, via pimpl |
| **Interface / classe abstraite** | Classe avec méthodes virtuelles pures, instanciée nulle part | `ISystem`, `IRenderer`, `IAudio`, `IGameSession`, `ISessionFactory`, `IEntityFactory`, `ISparseArray` |
| **Pimpl** | « pointer to implementation » : cacher les membres privés et leurs includes | `UdpTransport::Impl` cache Asio ; `ScriptEngineImpl` cache pybind11 |
| **Injection de dépendance** | On passe la dépendance, on ne la construit pas | `GameServer(port, settings, ISessionFactory&)` |
| **Visiteur** | Un objet qui sait traiter chaque type d'une union | `Packet::visit` + `MessageValidator`, `MessageWriter`, `TokenReader` |
| **Factory** | Un objet dont le métier est de construire | `SimulationFactory` implémente `ISessionFactory` |

**La phrase clé sur le découplage :**
> « `GameServer` ne connaît que deux interfaces : `IGameSession` et `ISessionFactory`. Il échange exactement
> deux choses avec le jeu : des **états de boutons** qui entrent, des **snapshots** qui sortent. C'est
> précisément pour ça que le **mode solo** existe : `LocalClient` branche la simulation directement sur la vue,
> sans réseau. Le réseau est un transport interchangeable, pas une dépendance. »

## 2. ECS

| Notion | Définition | Chez nous |
|---|---|---|
| **Entity** | Un identifiant, pas un objet | `Entity` = `index` + `generation` |
| **Component** | De la donnée, sans comportement | `Transform`, `Velocity`, `Health`, `Weapon`… toutes des **classes** avec membres privés et accesseurs |
| **System** | Du comportement, sans donnée propre | `MovementSystem`, `CollisionSystem`… héritent de `ISystem` |
| **Composition vs héritage** | On combine des composants au lieu de dériver des classes | Un vaisseau respawné **reçoit** `Invulnerable`, puis le **perd** |
| **Sparse array** | Un tableau indexé par l'entité, avec des trous | `SparseArray<Component>` = `vector<optional<Component>>` |
| **Component mask** | Un entier où chaque bit = « possède ce composant » | `ComponentMask` = `uint64_t`, **64 types de composants max** |
| **Handle générationnel** | index + génération, pour détecter un handle périmé | Un index réutilisé change de génération → l'ancien handle est détectablement mort |

**Comment marche une requête (à savoir expliquer) :**
1. Chaque type de composant reçoit un **numéro de bit** à son enregistrement (`m_bits`, une `unordered_map<type_index, size_t>`).
2. Ajouter un composant à une entité, c'est `m_masks[index] |= 1ull << bit`.
3. `registry.view<Transform, Velocity>()` calcule le masque voulu, puis parcourt les masques et garde les entités où `(mask & wanted) == wanted`.

> « On compare **un entier** par entité au lieu d'interroger chaque stockage. C'est là qu'intervient le
> bit-shifting : `1ull << bit` construit le masque, le `&` teste l'appartenance. »

**Le piège que le jury adore** — *« pourquoi `IComponent` n'a pas de destructeur virtuel ? »*
> « C'est une décision documentée (décision 17). Les composants sont stockés **par valeur** dans les
> `SparseArray` et ne sont jamais détruits par un pointeur de base. Un destructeur virtuel n'ajoutait donc
> qu'un **pointeur de vtable** caché dans chaque composant — et ce pointeur désignait le code de **celui qui
> avait créé le composant**. Un composant créé par un plugin pointait dans le `.so` du plugin, et **crashait
> après rechargement du plugin**. `IComponent` est donc une classe à destructeur `protected` non virtuel :
> c'est de la donnée pure, plus petite, et partageable entre le moteur et les plugins. »

## 3. Réseau — le vocabulaire minimum

| Terme | Définition à sortir telle quelle |
|---|---|
| **UDP** | Datagrammes, sans connexion, sans garantie : peut perdre, dupliquer, réordonner, retarder |
| **TCP** | Flux fiable ordonné, au prix du **head-of-line blocking** : un segment perdu bloque tout ce qui suit |
| **Head-of-line blocking** | *La* raison technique de refuser TCP : une donnée fraîche attend derrière une donnée périmée |
| **Protocole binaire** | Les champs sont écrits en octets de taille fixe, pas en texte ni en JSON |
| **Big-endian** | Octet de poids fort en premier — la convention réseau |
| **MTU** | Taille max avant fragmentation IP ; on se limite à **1200 octets** pour ne jamais fragmenter |
| **Serveur autoritaire** | Le serveur calcule la vérité ; le client envoie des intentions et affiche |
| **Snapshot** | Photo complète de l'état du monde à un tick |
| **Delta snapshot** | On n'envoie que la différence avec une photo de référence (*baseline*) |
| **Keyframe** | Un snapshot complet envoyé périodiquement pour resynchroniser — comme une image I en vidéo |
| **ACK / ackBits** | Le dernier paquet reçu + un historique sur 32 bits des 32 précédents |
| **reliableId** | L'identité du **message**, stable à travers ses retransmissions |
| **RTT** | Aller-retour, mesuré depuis les acquittements, lissé en moyenne pondérée |
| **Jitter (gigue)** | La **variation** du délai, pas le délai |
| **Heartbeat** | Paquet périodique qui prouve qu'on est vivant même quand on ne joue pas |
| **Session / token** | Un identifiant aléatoire 32 bits lié à (adresse, port), exigé sur chaque message |
| **netem** | Module Linux `tc` qui dégrade volontairement une interface : délai, perte, duplication, désordre, débit |

## 4. Les trois mots qu'on confond toujours (le jury teste ça)

| Notion | Définition | Exemple chez nous |
|---|---|---|
| **Bit-shifting** | L'opération `<<` / `>>`. **Un outil, pas une économie.** | Fenêtre glissante d'`ackBits`, calcul des masques de composants |
| **Bitfield** | Un entier utilisé comme tableau de booléens | Boutons (6 bits utilisés), `ackBits` (32), `fieldMask` d'`EntityChange` (5) |
| **Bit-packing** | Plusieurs valeurs **distinctes** dans un octet, dont au moins une plus large qu'un bit | `visual` = 1 bit de flash + **7 bits d'angle** ; `type` = 1 bit de direction + 7 bits d'identifiant |
| **Quantification** | Réduire la précision pour réduire la taille | Angle : 360° sur **128 crans** (2,8° de précision) au lieu d'un `float` → **×36** de gain |

> « Un `float` d'angle coûte 4 octets. Chez nous il coûte **7 bits**. C'est à la fois du packing et de la
> quantification, et c'est le meilleur exemple du projet. »

## 5. Build et outillage

| Notion | Chez nous |
|---|---|
| **CMake** | 3.21+, `CMAKE_CXX_STANDARD 20`, extensions désactivées |
| **vcpkg manifest** | `vcpkg.json` + `builtin-baseline` → **versions figées**, installation automatique |
| **`find_package`** | `Threads`, `asio`, `nlohmann_json`, `SFML 3`, `Python3`, `pybind11` |
| **Options CMake** | `ENGINE_BUILD_CLIENT`, `ENGINE_BUILD_SERVER` → on peut compiler le serveur seul (pas de X11/OpenGL requis) |
| **Fonctions CMake** | `engine_add_game(...)`, `engine_add_plugin(...)` → déclarer un jeu = **8 lignes** dans le CMakeLists racine |
| **Plusieurs exécutables** | `r-type_server`, `r-type_client`, `skyhop` |
| **Plugins** | `.so` / `.dll` chargés à l'exécution, listés dans le JSON du jeu, rechargés quand le fichier change |
| **Exceptions** | Tout dérive d'`EngineError`, une classe par module (`NetError`, `EcsError`, `SceneError`…) ; sortie **84** |
| **Doc** | MkDocs Material, 60+ pages, diagrammes **Mermaid** (donc du texte, donc accessible et diffable) |

---
---

# PARTIE II — Défense critère par critère

## II.1 — Engine and Architecture (4/6 → viser 6)

**Reproche midterm :** *« Attention à ne pas hardcoder rtype, utiliser plutôt des json. On crée un engine et pas un jeu. »*

**Ce qu'on répond :**

| Ce qui était hardcodé | Ce qui le remplace |
|---|---|
| Rien de R-Type dans le C++ | `engine/` ne contient **aucune** chaîne « rtype », « bydo », « force »… |
| Les réglages réseau | `NetworkSettings::fromGame(DataNode)` lit l'objet `network` de `rtype_game.json` : port, tickRate, snapshotEveryTicks, timeouts, maxClients, maxRooms, longueurs de texte… |
| Le nombre de valeurs transmises | `worldValueCount` et `playerValueCount` viennent du JSON — le snapshot est de taille **variable selon le jeu** |
| La liste et l'ordre des systèmes | Tableau `serverSystems` / `renderSystems` du JSON, créés par nom |
| Les entités | Prefabs JSON = listes de composants ; aucun type d'ennemi n'existe en C++ |
| Les comportements | Python (`game/scripts/*.py`), côté serveur |

**La phrase :**
> « La preuve qu'il n'y a pas de R-Type dans le moteur, c'est **Skyhop** : un jeu solo, à un bouton, avec de
> la gravité et du rebond — des mécaniques que R-Type n'a pas — et il n'a demandé **aucune ligne de C++**.
> Il a même son propre plugin, `wobble`, chargé à l'exécution. »

**À montrer si on vous pousse :** le `CMakeLists.txt` racine. Il fait 50 lignes et ne contient que trois appels : `add_subdirectory(engine)`, puis `engine_add_game(rtype …)` et `engine_add_game(skyhop …)`.

## II.2 — Decoupling and Modularity (2/4 → viser 4)

**Reproches midterm :** *« Utiliser les classes au lieu de structs : IComponents. Fichiers de conf pour chaque jeu. »*

| Reproche | Correction |
|---|---|
| structs → classes | **Tous** les composants sont des `class … : public IComponent` avec membres `m_`, accesseurs const, et constructeur explicite. `SparseArray` le **force** : `static_assert(std::is_base_of_v<IComponent, Component>)` |
| Fichiers de conf par jeu | `game/data/` : `rtype_game.json`, `rtype_client_prefabs.json`, `rtype_server_prefabs.json`, `rtype_settings.json`. `game2/data/` : les équivalents Skyhop |
| Pas de dépendances croisées | 13 cibles CMake, dépendances explicites ; `engine_ecs` ne dépend ni de SFML, ni d'Asio, ni de JSON |

**Les 13 modules (à savoir réciter) :**

```
core      erreurs, temps, géométrie, couleurs, ligne de commande, bibliothèques dynamiques
ecs       Registry, EntityManager, SparseArray, composants, systèmes
physics   AABB, corps, paires de collision
scene     JSON (DataNode), factories de composants et de systèmes, prefabs, plugins
net       UDP + octets : UdpTransport, ByteWriter/ByteReader, Endpoint, Datagram
network   protocole : Packet, Codec, ReliableSender/Receiver, GameServer, ClientConnection
render    SFML graphique : IRenderer, SfmlRenderer, Window, textures, polices
audio     SFML audio : IAudio, SfmlAudio
input     Action, KeyBindings, InputManager
client    GameView, scènes, HUD, OnlineClient, console de dev, métriques
scripting Python embarqué via pybind11
simulation le monde autoritaire d'une room
local     LocalClient : le mode solo
```

**La règle « une lib tierce = un module » (décision 12) :**
SFML → `render`/`audio`/`input` ; Asio → `net` ; nlohmann-json → `scene` ; pybind11 → `scripting`. Chacune en dépendance **privée**.
> « Remplacer SFML touche trois modules et aucun header du reste du moteur. C'est documenté dans
> `docs/guides/engine/replace-a-library.md`. »

## II.3 — Code Factorization (3/4 → viser 4)

| Point | Argument |
|---|---|
| Tout est entité | Un ennemi, un tir, un bouton d'UI, une ligne de chat : des entités |
| La différence se fait par composants | Aucune classe `Enemy`, aucune classe `Bullet` |
| Les trois modes partagent le code | Serveur, client en ligne, solo : **la même** `Simulation` et **la même** `GameView` |
| Les lanceurs sont génériques | `engine/apps/` : 3 fichiers (`ServerMain`, `ClientMain`, `LocalMain`), le nom du jeu est **compilé dedans par CMake** |
| Les messages réseau | 33 messages, **une classe chacun**, tous avec le même triplet `isValid` / `write` / `read` |
| Pas de `switch` recopié | `Packet::visit` + visiteurs : ajouter un message fait **échouer la compilation** partout où il faut le traiter |

## II.4 — Networked and Authoritative Server (4/4 → tenir)

**Ce qui était déjà acquis :** plusieurs clients, pas de bug, le serveur a l'autorité.

**Ce qu'on a ajouté depuis :** rooms multiples (48), chat, pseudos, delta snapshots, fiabilité, et la **preuve de charge**.

**La démonstration d'autorité, en une phrase :**
> « Le client n'envoie **que** `buttons`, un octet. Il n'envoie jamais une position, jamais un dégât, jamais
> un score. Positions, collisions, vies et scores sont calculés serveur. Un client modifié ne peut, au pire,
> que mentir sur ses propres boutons — c'est-à-dire jouer. »

## II.5 — Playable Prototype (3/4 → viser 4)

**Reproche midterm :** *« pas de sons, 1 seul stage. »*

| Reproche | Correction |
|---|---|
| Pas de sons | `MusicSystem`, `AudioSystem`, `SoundCue`, `eventSounds` dans le JSON ; `game/assets/rtype/sounds/` et `music/`. Le serveur envoie `PLAY_SOUND` **en message fiable** |
| 1 seul stage | **3 stages, 3 boss**, vagues, power-ups, la Force, tirs chargés, terrain, intro, fin et crédits |
| — | Difficulté qui s'adapte au nombre de joueurs, mode **versus** à 2 en manches, écrans de fin |

## II.6 — Binary Protocol over UDP (4/4 → tenir)

**Reproche midterm (deux fois dans le commentaire !) :** *« passez en classe pitié. »*

**La réponse :**
> « C'est fait, et à deux endroits. **Un**, chaque message est une classe : `InputMessage`, `SnapshotMessage`,
> `ChatMessage`… 33 classes, membres privés, `isValid(settings)`, `write(ByteWriter&)`, `static read(ByteReader&)`.
> **Deux**, l'en-tête n'est plus lu à coups de `memcpy` sur un `struct` : `Packet` est une classe qui porte un
> `std::variant` des 33 types, et `Codec` écrit champ par champ. Il n'y a **aucun `reinterpret_cast`** d'un
> datagramme vers un struct, ce qui serait de toute façon faux : padding, alignement et endianness. »

Sur l'absence de TCP :
> « **Zéro socket TCP** dans le projet. `grep -r tcp engine/` ne renvoie rien. »

## II.7 — Consistency and Smoothness (2/2 → tenir)

> « Mesuré, pas ressenti : sous `netem`, 20,00 snapshots/s délivrés en référence, **18,15/s à 10 % de perte**,
> et jamais plus de **0,20 s** sans snapshot. Un tick appliqué ne revient jamais en arrière — c'est un
> critère de PASS du test. »

Le mécanisme : `handleSnapshot` refuse tout snapshot qui n'est pas **plus récent** que le dernier appliqué
(`Codec::isNewer`), ce qui neutralise le réordonnancement.

## II.8 — Error Handling and Network Decoupling (3/4 → viser 4)

**Reproches midterm :** *« gestion d'erreurs qui marche mais améliorable (lisibilité). Network pas fully abstract (struct, hardcode…). »*

| Reproche | Correction |
|---|---|
| Lisibilité des erreurs | Une classe d'exception par module, toutes dérivées d'`EngineError`. Chaque message **nomme la classe qui a échoué** : `"ClientConnection: connection lost, the server stopped answering"` |
| struct / hardcode réseau | Tout est classe ; tous les réglages viennent du JSON |
| Abstraction | **5 niveaux** (voir partie III.2) : `UdpTransport` ne connaît pas les types de message, `Codec` ne connaît pas les sessions, `ReliableSender` ne connaît pas le sens des messages, `GameServer` ne connaît pas les sockets |

**Les cas d'erreur à savoir citer :**

| Cas | Comportement |
|---|---|
| Datagramme malformé / tronqué / mauvais magic | `Codec::decode` renvoie `nullopt` → ignoré en silence, aucune réponse, aucune déconnexion |
| `nc -u` qui envoie du texte | Même chemin : ignoré, le serveur continue |
| Client qui disparaît sans dire au revoir | `sessionTimeout` 5 s → `PLAYER_LEFT` avec raison `LEFT_TIMEOUT` ; si c'était le propriétaire, la room est fermée proprement |
| Serveur qui s'arrête | `SERVER_SHUTDOWN` envoyé à tous ; le destructeur du transport attend jusqu'à 200 ms que l'envoi sorte |
| Exception sur le thread d'E/S | Stockée, **relancée sur le thread principal** au prochain `poll()` / `send()` / `checkHealth()` |
| Client qui n'acquitte jamais | Borné par 64 messages en attente par pair **et** par le timeout de 5 s |
| Script Python qui plante | L'erreur est isolée **par entité** : un script cassé ne fait pas tomber le serveur |

## II.9 — Build System (2/3 → viser 3)

| Point | Argument |
|---|---|
| CMake | 3.21+, un `add_library` par module |
| Dépendances selon l'OS | vcpkg résout par triplet ; `ENGINE_BUILD_CLIENT=OFF` permet un serveur sans X11/OpenGL |
| Installation automatique | La première configuration télécharge et compile les dépendances depuis `vcpkg.json` |
| Plusieurs exécutables | 3 binaires + 1 plugin partagé |
| Qualité | `.clang-format` + **GitHub Actions** (`.github/workflows/`) |
| Robustesse | Le CMakeLists racine **répare** le chemin de toolchain produit quand `VCPKG_ROOT` est vide, et donne un message d'erreur explicite sinon |

## II.10 — README (2/3 → viser 3)

**Reproche :** *« Compléter les parties manquantes mais prévues. »* → c'est fait. Le README contient maintenant :
tableau des deux jeux, features, plateformes supportées, dépendances, quick start **Linux et Windows**,
usage et jeu en réseau local, table des contrôles clavier/manette, table « I want to… → read », licence, auteurs.

## II.11 — Protocol Documentation (2/3 → viser 3)

**Reproche :** *« Documentation très très très complète (trop). Format RFC possible, diagramme aussi. »*

**Ce qu'on a fait :** `docs/reference/protocol.md` est maintenant structuré **en 11 sections numérotées façon RFC** :

```
1. Overview          6. Message catalogue (6.1 room entry, 6.2 SNAPSHOT, 6.3 SNAPSHOT_DELTA)
2. Primitive types   7. Session lifecycle
3. Packet header     8. Timing
4. Reliability       9. Worked example       ← octets réels capturés sur le fil
5. Validation rules  10. Writing a client    ← les 7 points obligatoires
                     11. Security considerations
```

Plus : des diagrammes **Mermaid** dans `docs/architecture/`, un schéma ASCII de la fenêtre d'`ackBits`,
et `scripts/protocol_probe.py` **cité comme implémentation de référence** (~300 lignes, un client complet en Python).

**La phrase :**
> « L'objectif annoncé en tête du document : *un développeur doit pouvoir écrire un nouveau client, ou un
> nouveau serveur, à partir de cette seule page*. On l'a vérifié : le client Python de `scripts/` a été
> écrit **depuis la doc**. »

## II.12 — Accessibility First Steps (1/3 → viser 3)

**Reproche :** *« pas d'accessibilité. »* C'est le critère où il y a le plus à gagner.

**Ce qu'on a maintenant** — `docs/project/accessibility.md`, organisé **par catégorie de handicap** :

| Catégorie | Ce qui existe réellement dans le jeu |
|---|---|
| Moteur / physique | Remapping clavier **complet** dans l'écran SETTINGS (conflits échangés automatiquement), manette (menus inclus), **dead zone réglable**, **AUTO FIRE** (toggle au lieu de maintenir), tous les menus navigables sans souris, difficulté EASY, 2 s d'invulnérabilité au respawn, Skyhop = **un seul bouton** |
| Visuel / auditif | **TEXT SIZE** 0,8 → 1,3 appliqué à chaud, fenêtre redimensionnable (le monde 1920×1080 est mis à l'échelle), **PLAYER NUMBERS** (`P1`…`P4` au-dessus des vaisseaux → on ne distingue pas les joueurs par la couleur seule), flash sur chaque entité touchée, **équivalent visuel pour chaque son porteur d'information** (arrivée du boss = panneau + barre, power-up = texte HUD), volumes musique/effets séparés, HUD à fort contraste |
| Cognitif | Objectifs simples, état toujours visible, phases explicites (STAGE N / BOSS / VICTORY / GAME OVER), erreurs de lobby **en mots** (« WRONG CODE », « ROOM FULL »), redémarrage automatique |
| Documentation | Markdown + site avec recherche, hiérarchie de titres, **tableaux au lieu d'images de texte**, diagrammes Mermaid (donc lisibles par un lecteur d'écran), glossaire, FAQ |

**Et la règle pour les devs — c'est ça qui fait la différence devant un jury :**
> « Ce n'est pas qu'une liste de features, c'est une **contrainte de conception** inscrite dans la doc :
> le remapping passe par `engine::input::KeyBindings`, donc **le code de jeu lit des actions, jamais des
> touches physiques** ; les options visuelles vivent dans les systèmes client, **jamais dans un composant
> de gameplay** ; et tout nouveau son porteur d'information **doit** avoir un équivalent visuel. »

**Assumer les limites :** palettes daltoniens et désactivation du screen shake = *planned* ;
lecteur d'écran dans les menus = **non prévu, parce que SFML n'expose aucune API d'accessibilité** — et
c'est une meilleure réponse que de promettre.

## II.13 — Self-Contained Project (3/3 → tenir)

vcpkg pour tout, `find_package` partout (`Threads`, `asio`, `nlohmann_json`, `SFML 3`, `Python3`, `pybind11`),
`builtin-baseline` qui **fige les versions**, et la stdlib Python recopiée à côté du serveur par la cible
`python_runtime` → le serveur tourne sans Python installé sur la machine.

---
---

# PARTIE III — LA TRACK RÉSEAU

## III.1 — Le cadrage : ce que le sujet demande

Deux thèmes de la track réseau. On a traité **le thème 2 en entier** et **le thème 1 partiellement**.

### Thème 2 — « Advanced network protocol »

| Puce du sujet | Niveau d'exigence | Statut |
|---|---|---|
| *« various messages **MUST** be sent/received reliably »* | **MUST** — le seul de tout le topic | ✅ **fait** (phase 1) |
| *« data packing: quantization, bit-level packing, adequate data type sizes, space-efficient serialization, structs alignment and padding »* | *should* | ✅ **fait** (phases 2c + préexistant) |
| *« general-purpose compression ; for game-specific techniques: **delta snapshot compression** »* | *you may want to investigate* | ✅ delta **fait** ; compression générique **écartée après analyse** |
| *« mitigation of network errors »* | *should* | ✅ fait + **mesuré sous netem** |

> Les deux exemples que le sujet donne lui-même pour le MUST sont exactement les nôtres :
> *« connection to a game instance »* → `JOIN_ROOM`, `ROOM_JOINED`, `GAME_STARTED` sont fiables ;
> *« a player is dead »* → `PLAYER_KILLED`, `ENTITY_DESTROYED` sont fiables.

### Thème 1 — « Advanced server »

| Puce | Statut |
|---|---|
| *« several different game instances in parallel »* | ✅ **48 rooms, mesuré jusqu'à 96 sans dégradation** |
| *« communication between users: text chat »* | ✅ chat par room + pseudos |
| Identités persistantes / reconnexion | ⬜ non fait — assumé |
| Scoreboard global persistant | ⬜ non fait — assumé |
| Console d'administration serveur | ⬜ non fait (la console de dev F1 existe, mais c'est autre chose) |

## III.2 — L'architecture réseau : 5 niveaux

```
  GameServer / ClientConnection   sessions, tokens, rooms, timers, gameplay  │ ne connaît pas les sockets
  ReliableSender / Receiver       quels paquets sont arrivés, quoi réémettre │ ne connaît pas le sens des messages
  Codec / Packet                  les 33 messages et leur validation         │ ne connaît pas les sessions, ni le temps
  ByteWriter / ByteReader         entiers en big-endian, limites de taille   │ ne sait pas ce que les entiers veulent dire
  UdpTransport (Asio)             endpoints et tampons d'octets              │ ne connaît pas les types de message
```

Les deux premiers niveaux sont dans `engine_network`, les deux derniers dans `engine_net`.
**Asio n'apparaît que dans `engine/net/internal/UdpTransportImpl.hpp`**, derrière un pimpl : aucun fichier de jeu ne l'inclut.

**La phrase :**
> « Chaque niveau sait exactement une chose et ignore tout le reste. C'est ce qui nous a permis d'ajouter la
> couche de fiabilité **sans toucher à la simulation**, et les delta snapshots **sans toucher ni à la
> simulation ni au code d'affichage du client**. »

## III.3 — Le protocole, de mémoire

| Propriété | Valeur |
|---|---|
| Transport | UDP sur IPv4, port 4242 par défaut |
| Boutisme | **Big-endian** sur tous les champs multi-octets |
| En-tête | **20 octets**, identiques sur chaque paquet |
| Datagramme max | **1200 octets**, en-tête compris → jamais de fragmentation IP |
| Version | **9** (`Protocol::VERSION`) |
| Règle de base | **1 datagramme = 1 message.** Pas de flux, pas de framing, pas de fragmentation |

### L'en-tête, offset par offset (à savoir réciter)

| Offset | Taille | Champ | Rôle |
|---|---|---|---|
| 0 | u16 | `magic` | `0x454E` = les lettres ASCII **`EN`**. Autre valeur → on jette |
| 2 | u8 | `version` | 9 |
| 3 | u8 | `type` | **bit 7 = sens** (`0x80` = serveur → client), 7 bits d'identifiant |
| 4 | u32 | `sequence` | Compteur **du paquet**, propre à l'émetteur, **démarre à 1** |
| 8 | u32 | `ack` | Dernière `sequence` reçue du pair ; **0 = je n'ai encore rien reçu** |
| 12 | u32 | `ackBits` | Les 32 paquets précédant `ack` ; bit `i` = paquet `ack - 1 - i` |
| 16 | u16 | `reliableId` | Identité **du message**, stable à travers ses retransmissions ; **0 = non fiable** |
| 18 | u16 | `payloadSize` | Nombre d'octets après l'en-tête |

**Le bit de direction** — argument de sécurité, pas de confort :
> « Un récepteur rejette tout paquet dont le sens ne correspond pas à son rôle. Ça **empêche un client
> d'injecter un message que seul un serveur peut produire** — un faux `PLAYER_KILLED`, par exemple. »

**La comparaison de séquences est modulaire** (`Codec::isNewer`) :
`candidate` est plus récent que `previous` si `(candidate - previous) mod 2³²` est dans `1 .. 2³¹-1`.
Ça gère le **wrap-around** à 2³². Une distance de exactement 2³¹ n'est pas « plus récente » (cas ambigu, tranché explicitement).

### Les messages

**Client → serveur (0x01 … 0x0D)** : `CONNECT`, `INPUT`, `HEARTBEAT`, `DISCONNECT`, `LIST_ROOMS`, `CREATE_ROOM`, `JOIN_ROOM`, `LEAVE_ROOM`, `START_GAME`, `KICK_PLAYER`, `DELETE_ROOM`, `CONSOLE_COMMAND`, `CHAT`

**Serveur → client (0x81 … 0x94)** : `ACCEPT`, `REJECT`, `SNAPSHOT`, `HEARTBEAT_ACK`, `ENTITY_DESTROYED`, `PLAYER_JOINED`, `PLAYER_KILLED`, `PLAYER_LEFT`, `SERVER_SHUTDOWN`, `ROOM_LIST`, `ROOM_JOINED`, `ROOM_REJECTED`, `ROOM_UPDATE`, `GAME_STARTED`, `KICKED`, `ROOM_CLOSED`, `PLAY_SOUND`, `CONSOLE_TEXT`, `SNAPSHOT_DELTA`, `CHAT_MESSAGE`

**Les boutons, un seul octet :** `UP 0x01`, `DOWN 0x02`, `LEFT 0x04`, `RIGHT 0x08`, `FIRE 0x10`, `ACTION 0x20`, `BOMB 0x40`, `0x80` réservé.

**Point de conception à savoir défendre — état, pas évènement :**
> « `INPUT` transporte **l'état** des boutons, pas des appuis et relâchements. Un `INPUT` perdu est corrigé
> par le suivant 33 ms plus tard ; un évènement « relâché » perdu laisserait une touche **coincée pour
> toujours**. C'est de la fiabilité obtenue gratuitement, par la nature de la donnée et non par un mécanisme. »

## III.4 — Phase 0 : une séquence par client (le prérequis)

**Le problème :** `GameServer` numérotait ses paquets sortants avec **un seul compteur partagé entre tous
les clients**. À deux joueurs, le serveur envoie le paquet 1 à A, le 2 à B, le 3 à A… et **A ne voit que des
numéros impairs**.

**Pourquoi c'est bloquant :** ces trous ne sont pas des pertes, mais A n'a aucun moyen de le savoir. Une
couche d'ACK aurait lu chaque numéro manquant comme « perdu, retransmets », et le serveur aurait retransmis
à A **des paquets destinés à B**.

**La correction :** `m_outgoingSequence` dans `ClientSession`, attribué par `sendTo()` depuis la session du destinataire. 4 fichiers, +14/−7 lignes.

**La preuve, et c'est l'exemple à raconter :**

| | Séquences reçues par A |
|---|---|
| Avant | `0, 2, 4, 5, 6, 7, 10, 12, 14, 16, 18…` |
| Après | `0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10…` |

> « Le motif "avant" est **exactement celui que la théorie prédit**, et c'est ça qui valide la mesure : pairs
> là où le serveur alterne entre A et B, et le **bloc contigu `4,5,6,7`** correspond au moment où B ne
> recevait encore rien — entre sa connexion et son entrée dans la room — donc où A consommait les numéros
> d'affilée. »

## III.5 — Phase 1 : la fiabilité (le MUST du sujet)

### Le principe : on ne retransmet que ce qui ne peut pas être reconstruit

| Donnée | Stratégie | Pourquoi |
|---|---|---|
| `INPUT` | État complet ~30×/s | Un paquet perdu est **remplacé** par le suivant |
| `SNAPSHOT` | 20×/s | Pareil : une donnée plus fraîche arrive dans 50 ms |
| Évènements de jeu | **Acquittés et retransmis** | Un ennemi détruit perdu reste affiché pour toujours |
| Actions de lobby | **Acquittés et retransmis** | Un `JOIN_ROOM` perdu bloque le joueur dans le menu |
| Vivacité | Heartbeat 1×/s, timeout 5 s | — |

> « C'est **exactement** ce que TCP ne sait pas exprimer. TCP livre tout de façon fiable, y compris ce qui a
> déjà expiré. Notre couche applique la fiabilité **message par message**. »

### Les acquittements : zéro paquet dédié

```
   ack     = 43
   ackBits = 0b...00001101
                     ││││
                     │││└─ bit 0 → paquet 42 : reçu
                     ││└── bit 1 → paquet 41 : manquant
                     │└─── bit 2 → paquet 40 : reçu
                     └──── bit 3 → paquet 39 : reçu
```

> « Un paquet acquitte jusqu'à **33 paquets**. Et il n'y a **aucun paquet d'acquittement dédié** : les ACK
> voyagent dans l'en-tête des paquets qu'on envoie de toute façon — les inputs à 30 Hz du client, les
> snapshots à 20 Hz du serveur. Perdre un acquittement est inoffensif : le suivant répète la même information. »

### Pourquoi `reliableId` **en plus** de `sequence` — la question la plus probable du jury

> « Une retransmission part dans un **paquet neuf, avec un nouveau numéro de séquence**. C'est obligatoire,
> sinon la notion de "plus récent" s'effondre. Donc le récepteur **ne peut pas** reconnaître une
> retransmission par sa séquence. `reliableId` est l'identité du **message**, stable à travers toutes ses
> retransmissions, et c'est elle qui permet d'écarter un doublon si l'originale arrive en retard après sa
> retransmission.
>
> Il y a donc **deux déduplications distinctes** : `sequence` écarte les **paquets** en double,
> `reliableId` écarte les **messages** en double. »

### Les trois classes

| Classe | Rôle |
|---|---|
| `ReliableReceiver` | Mémorise les séquences arrivées, produit `ack`/`ackBits`, garde une fenêtre de **256 `reliableId`** pour écarter les doublons |
| `ReliableSender` | Garde les messages non acquittés (**64 max par pair**), estime le RTT, réémet après expiration |
| `PendingMessage` | Un message en attente : le paquet + sa date d'envoi |

Chaque `ClientSession` du serveur en a **une paire**, `ClientConnection` aussi : le mécanisme est **strictement symétrique**.

### Le RTT, obtenu gratuitement

Quand un paquet est acquitté : `RTT = maintenant − date d'envoi`, lissé en moyenne pondérée de poids **1/8**
(`m_roundTrip = (m_roundTrip * 7 + sample) / 8`).
Délai de retransmission : **`max(100 ms, RTT × 1,5)`**.

> « Le serveur a maintenant **un RTT par client, sans un seul paquet supplémentaire**. »

### Le réglage corrigé en cours de route (à raconter — ça montre la méthode)

> « Le plancher de retransmission était à **30 ms**. Mesure : **4 retransmissions inutiles à 0 % de perte**.
> Cause : sur un LAN le RTT est d'environ 1 ms, donc c'est le plancher qui s'applique — mais le client
> **n'acquitte que quand il émet**, c'est-à-dire toutes les 33 ms. Le serveur retransmettait avant que le
> premier acquittement ait pu lui parvenir. Plancher relevé à **100 ms** → **0 retransmission inutile**.
>
> La règle qu'on en retire : **le délai de retransmission doit dépasser la cadence d'émission du pair**,
> pas seulement le RTT. »

### Le défaut corrigé en cours de route : acquitter avant de filtrer

> « Repéré en décodant une capture hexadécimale réelle : l'`ackBits` d'un snapshot valait `00 bf ff ff`, avec
> **un bit éteint au milieu** alors que la liaison était un loopback sans perte possible. Le bit désignait un
> paquet `CONNECT`.
>
> Cause : dans `handleConnect`, l'enregistrement de la réception se faisait **après** le garde anti-rafale de
> 200 ms. Un second `CONNECT` arrivant dans la fenêtre sortait par le `return` **sans jamais être
> comptabilisé** — il apparaissait donc comme perdu dans tous les acquittements suivants.
>
> Le principe qu'on en tire : **un paquet reçu doit être enregistré dans la fenêtre d'acquittement dès son
> arrivée, indépendamment de ce qu'on décide d'en faire ensuite. Accuser réception et traiter sont deux
> opérations distinctes** ; le code les mélangeait. »

| | `ack` | `ackBits` | Lecture |
|---|---|---|---|
| Avant | 25 | `00 bf ff ff` | un trou au milieu ❌ |
| Après | 39 | `ff ff ff ff` | les 32 derniers reçus ✅ |

### La classification des messages (et surtout : pourquoi certains **ne sont pas** fiables)

| Fiables | Non fiables | Pourquoi non fiables |
|---|---|---|
| `CREATE_ROOM`, `JOIN_ROOM`, `LEAVE_ROOM`, `START_GAME`, `KICK_PLAYER`, `DELETE_ROOM`, `CHAT`, `CHAT_MESSAGE`, `ENTITY_DESTROYED`, `PLAYER_JOINED`, `PLAYER_KILLED`, `PLAYER_LEFT`, `PLAY_SOUND`, `GAME_STARTED`, `ROOM_JOINED`, `ROOM_REJECTED`, `ROOM_CLOSED`, `KICKED` | `SNAPSHOT`, `SNAPSHOT_DELTA`, `INPUT`, `HEARTBEAT` | Remplacés en permanence par une version plus fraîche : **retransmettre une donnée périmée est exactement le défaut de TCP qu'on évite** |
| | `ROOM_LIST`, `ROOM_UPDATE` | Réémis périodiquement par le serveur → **auto-réparants** |
| | `CONNECT`, `ACCEPT`, `REJECT` | La connexion a déjà sa propre logique de réessai (1 s × 5) |
| | `DISCONNECT`, `SERVER_SHUTDOWN` | **L'émetteur s'arrête juste après : il ne serait pas là pour retransmettre.** Le timeout prend le relais |

### La preuve

Protocole : deux clients dans la même room, 12 s de jeu + 3 s de drainage. **A subit une perte simulée, B n'en
subit aucune et sert de référence.** Un paquet « perdu » par la sonde n'est ni traité **ni acquitté** : du
point de vue du serveur, c'est indiscernable d'une perte réseau.

**À 60 % de perte :**

| | `ENTITY_DESTROYED` reçus | Verdict |
|---|---|---|
| B (référence, 0 % de perte) | 4 | — |
| A **avec** fiabilité, 60 % de perte | **4** | ✅ identique à la référence |
| A **sans** fiabilité, 60 % de perte | **4 sur 7** | ❌ 3 évènements perdus définitivement |

> « Sur le run avec fiabilité, la sonde a ignoré **233 paquets, dont 12 portaient un message fiable. Les 12
> ont tous fini par arriver.** Un cas est parlant : le message `reliableId=6` a été **perdu cinq fois de
> suite** et a quand même été délivré.
>
> Et à **0 % de perte : 0 retransmission, 0 doublon.** Le mécanisme ne coûte rien quand le réseau va bien. »

### Le coût, honnêtement

> « +10 octets par paquet. On avait d'abord annoncé "moins de 1 %" — c'était **calculé sur le pire cas**.
> **Mesuré sur un run réel : 481 paquets × 10 octets = 4 810 octets sur 69 798, soit 7 %.** La plupart des
> paquets sont petits : un `HEARTBEAT_ACK` fait 24 octets dont 20 d'en-tête.
>
> La leçon vaut pour toute la track : **on mesure, on ne calcule pas à partir du pire cas.** »

### Le piège de méthode (très bon à raconter)

> « Un indicateur nous a induits en erreur : **compter les "doublons" ne mesure pas les retransmissions**.
> Quand l'originale est perdue, la retransmission la **remplace** et n'apparaît jamais comme un doublon.
> Le bon indicateur, c'est **le nombre de paquets fiables volontairement perdus**, comparé à la complétude finale. »

## III.6 — Phase 2 : les delta snapshots

### L'idée, en termes simples

> « Imaginez qu'on décrive une scène au téléphone, 20 fois par seconde. Aujourd'hui on redit **tout** à chaque
> appel, alors que trois choses ont bougé. »

Deux économies **différentes**, et on a fait les deux :

| | Idée | Exemple |
|---|---|---|
| **Delta** | Ne pas répéter ce qui n'a pas changé | « rien n'a bougé côté joueurs » au lieu de relire les 4 slots |
| **Data packing** | Dire chaque chose avec moins d'octets | « +75 » au lieu de « il est à la position 1074 » |

La première réduit **le nombre de choses dites**, la seconde **la longueur de chacune**.

### Le danger, et les deux garde-fous

> « Chaque description s'appuie sur la précédente : **si un maillon casse, le monde du client devient faux et
> le reste pour toujours** — sans planter, sans erreur, silencieusement. C'est exactement la logique d'une
> vidéo : des images de différence, et de temps en temps une image complète qui remet d'aplomb. »

### L'invariant de correction — **à connaître par cœur**

> **On ne diffe JAMAIS contre un snapshot *envoyé*, seulement contre un snapshot *acquitté*.**

La récurrence : un delta est toujours construit sur la baseline confirmée ; donc si le client acquitte le
paquet qui le portait, c'est qu'il avait la baseline et a pu l'appliquer. Le cas de base est le snapshot complet.

Deux garde-fous qui rendent l'invariant robuste :
- l'historique **client** fait **32 entrées**, celui du **serveur 16** → la baseline est toujours présente côté client ;
- `SnapshotHistory::promote()` retient le snapshot de **tick le plus récent** parmi les acquittés → le réordonnancement est neutralisé.

### Comment le serveur sait ce que le client possède : **zéro octet ajouté sur le fil**

> « Le serveur retient quel paquet transportait quel snapshot. Quand l'`ack` de la phase 1 confirme ce paquet,
> le snapshot devient la baseline. La boucle de dépouillement des `ackBits` est **la même** que celle de
> `ReliableSender`. Les delta snapshots sont donc **gratuits** sur le fil : ils réutilisent les
> acquittements de la phase 1. »

### Le montage : le delta ne touche ni au jeu ni au client

> « La simulation continue de produire un `SnapshotMessage` **complet**. Le serveur l'encode en différence
> juste avant l'envoi ; le client **reconstruit un snapshot complet** et le passe au code existant.
> Conséquence : `Simulation`, `OnlineClient::applySnapshot` et `GameView::endSync` **ne sont pas modifiés**.
> En particulier la purge "absent = mort" de `endSync` continue de marcher, puisqu'elle voit toujours un
> snapshot complet. »

### Le format de `SNAPSHOT_DELTA` (0x93)

```
tick           u32
baselineTick   u32
gameState      u8
worldCount     u8   puis, par valeur changée : index u8, valeur u16
playerCount    u8   puis, par slot modifié   : slot u8, PlayerStatus
entityCount    u8   puis, par entité modifiée: EntityChange
removedCount   u8   puis les id disparus, u16
```

`EntityChange` porte un **`fieldMask`** : `DX 0x01`, `DY 0x02`, `POSITION 0x04`, `KIND 0x08`, `VISUAL 0x10`.

### Le choix « listes d'index plutôt que masques de bits » — question de jury probable

> « Les masques étaient notre premier design. On les a rejetés pour une raison concrète : **`Codec::readBody`
> est statique et n'a pas accès aux réglages du jeu** — il ne peut pas savoir combien de bits lire. Un masque
> sur 8 bits aurait **limité le moteur à 8 joueurs et 8 valeurs de monde** ; un masque sur 32 bits aurait
> coûté 6 octets de plus par paquet. La liste d'index est **auto-descriptive**, sans limite, et de taille
> identique en pratique aux taux de changement qu'on a mesurés (3 % de slots, 7 % de valeurs). »

### Ce que la mesure a changé dans la conception — **l'argument le plus fort de la track**

Mesuré sur **1558 transitions d'entités** :

| Observation | Mesuré |
|---|---|
| Entités dont la position change entre deux snapshots | **86 %** |
| Entités dont `kind` change | **0 %** |
| Entités dont `visual` change | **0,3 %** |
| Déplacements qui tiennent dans **un octet signé** | **100 %** |
| Slots joueur modifiés entre deux snapshots | **3 %** |
| Valeurs de monde modifiées | **7 %** |

> « Notre intuition de départ était : "on va sauter les entités qui n'ont pas bougé". **La mesure l'a
> démolie** : dans un shoot'em up horizontal, **86 % des entités bougent à chaque snapshot**. Sauter les
> immobiles ne rapporte que 14 %.
>
> Le gain réel est ailleurs : les **slots joueur et valeurs de monde inchangés** (97 % et 93 %), et
> **l'encodage compact** de ce qui change. Le `dx`/`dy` sur un `int8` au lieu d'un `int16`, c'est 100 % des
> cas ; `kind` n'est plus transmis du tout. »

### Les résultats

| Configuration | Taille moyenne d'un snapshot | Gain |
|---|---|---|
| Sans delta (contrôle) | **223 o** | — |
| Index monde/joueurs, entités complètes (2b) | **130 o** | **×1,7** |
| + `EntityChange` compact (2c) | **105 o** | **×2,1** |

Soit **34 kbit/s → 16 kbit/s** par client. Minimum observé : **33 octets**.

### La vérification

> « **Aller-retour `apply(base, build(base, courant)) == courant`** instrumenté dans le serveur :
> **1218 vérifications consécutives, zéro échec** (614 en 2b, 604 en 2c). L'instrumentation a été retirée ensuite.
>
> Et **sous 40 % de perte simulée**, `reliableId` contigus des deux côtés, et A reçoit les mêmes
> `ENTITY_DESTROYED` que B : **la phase 2 ne dégrade pas la phase 1.** »

### Le filet de sécurité : trois déclencheurs de snapshot complet

| Condition | Pourquoi |
|---|---|
| **Aucune baseline acquittée** | Client qui vient d'arriver ou de changer de room |
| **Baseline vieille de plus d'1 seconde de ticks** | On ne sait plus ce que le client possède |
| **Dernier complet il y a plus de 5 secondes** | **Image-clé périodique, comme un codec vidéo** |

> « Les deux premières sont nécessaires à la correction. La troisième est de la **défense en profondeur** :
> elle borne à 5 secondes la durée de n'importe quelle désynchronisation qu'on n'aurait pas prévue.
> Coût mesuré : environ **6 snapshots complets sur 300 envois**. »

`SnapshotHistory::forget()` est appelé à l'entrée **et** à la sortie de room, **des deux côtés** : sans ça un
client qui change de partie garderait une baseline dont le tick n'existe plus.

### La conséquence architecturale à signaler spontanément

> « `tickRoom` ne diffuse plus le même datagramme à toute la room : **chaque client a sa propre baseline, donc
> un encodage par destinataire**. Négligeable en CPU à 4 joueurs et 20 Hz — on l'a mesuré à 5 µs par room et
> par tick — mais c'est un point de contact direct avec la parallélisation des rooms. On l'a noté. »

## III.7 — Phase 3 : le data packing

**Couverture de la puce « Data packing » du sujet :**

| Technique citée par le sujet | Fait ? | Où |
|---|---|---|
| *« data quantization »* | ✅ | positions arrondies au pixel ; **angle en 128 crans** |
| *« bit-level packing »* | ✅ | `visual` = 1 bit flash + 7 bits d'angle ; `type` = 1 bit sens + 7 bits id |
| *« adequate data types sizes »* | ✅ | `dx`/`dy` en **`int8`** au lieu d'`int16` |
| *« space-efficient serialization »* | ✅ | `fieldMask` : on n'écrit **que** ce qui change |
| *« structs alignment and padding »* | ✅ | **écriture champ par champ, aucun padding compilateur, aucun `reinterpret_cast`** |

> « Sur l'alignement et le padding : notre réponse est qu'on ne mappe **jamais** un datagramme sur un struct.
> C'est écrit dans la doc du protocole comme une obligation pour quiconque écrit un client : *"a receiver must
> never map a datagram onto a struct"*. Le padding et l'ordre des octets dépendent du compilateur et de
> l'architecture ; `ByteWriter`/`ByteReader` écrivent champ par champ en big-endian. »

**Ce qui n'est pas fait, assumé :** le packing **à cheval sur les frontières d'octet** (un champ de 11 bits
suivi d'un de 6 bits sans alignement), qui demanderait un `BitWriter`/`BitReader` à curseur en bits.
> « Choix assumé : `kind` n'est de toute façon plus transmis depuis les deltas, et réduire `id` imposerait de
> **borner les identifiants réseau**, ce que `Simulation::nextNetworkId()` ne fait pas. **Gain restant estimé :
> environ 1 octet par entité.** »

## III.8 — Phase 4 : la compression générique, écartée (et c'est une réponse, pas un trou)

| Algorithme | Vitesse | Ratio sur nos charges |
|---|---|---|
| RLE | Très rapide | Mauvais : peu de longues suites identiques après le delta |
| Huffman | Moyenne | Bon, mais coûteux à implémenter et à calibrer |
| LZ4 | Très rapide | Moyen |
| zlib | Lente | Le meilleur ratio, mais inadapté à 20 paquets/s/client |

> « **Choix : aucune, pour l'instant, et la raison est une mesure.** La compression générique et le delta
> exploitent **la même redondance**. Une fois que le delta a supprimé les slots joueur répétés, les valeurs de
> monde inchangées et les champs d'entité constants, il **ne reste pas grand-chose à compresser dans
> 105 octets**. L'ordre honnête, c'est delta d'abord, compression ensuite, et seulement si le gain mesuré
> justifie la dépendance. »

## III.9 — Phase 5 : plusieurs instances en parallèle (et le refus mesuré de paralléliser)

### Ce qu'on allait faire, et pourquoi on ne l'a pas fait

Le plan initial : pool de threads, `session.update(dt)` en parallèle, envois regroupés après la jointure.
**Avant d'écrire une ligne, on a mesuré où passe le temps.** 16 rooms en partie, charge réelle, Apple M5 :

| Poste | Par room et par tick |
|---|---|
| Simulation C++ (physique, collisions, ECS) | 8 µs |
| **Scripts Python** | **24 µs (73 % du total)** |
| Encodage réseau (delta + codec + envoi) | 5 µs |
| **Total** | **37 µs** |

Budget d'un tick à 60 Hz : **16 667 µs**. Les 16 rooms consommaient **3,6 %** du budget.

**Deux obstacles rédhibitoires :**

1. **Le GIL de Python.** La logique de jeu est scriptée. Python **sérialise l'exécution du bytecode** entre
   threads : les 24 µs par room resteraient en file indienne quoi qu'on fasse. Pire, `engine/scripting` ne
   gère **pas du tout** le GIL — appeler l'interpréteur depuis un thread de pool sans
   `py::gil_scoped_acquire` est un **comportement indéfini**.
2. **L'arithmétique.** Sur 4 cœurs, au mieux : `16×24 + 16×13/4 = 436 µs` contre 592 µs aujourd'hui.
   **156 µs gagnés sur 16 667, soit 0,9 % du budget**, pour ~300 lignes de code concurrent et un vrai
   risque de corruption.

### Le vrai goulot d'étranglement n'était pas le CPU

> « C'étaient **deux constantes de configuration** : `maxClients: 16` et `maxRooms: 16`. Le serveur refusait le
> 17ᵉ joueur **en tournant à 3,6 % de sa capacité**.
>
> Et le plafond de `maxRooms` n'est pas le CPU non plus : c'est **le message `ROOM_LIST` qui doit tenir dans
> un datagramme de 1200 octets** :
>
> ```
> 20 (en-tête) + 1 (compteur) + N × 22 (une entrée de room)  ≤  1200   →   N ≤ 53
> ```
>
> Au-delà, il faudrait **paginer** la liste des rooms, donc modifier le protocole. Nouvelles limites :
> **`maxRooms: 48`, `maxClients: 192`** (48 × 4), sous le plafond protocolaire avec une marge.
> `ROOM_LIST` au pire : **1077 octets sur 1200**. »

### Les mesures de charge

Outil : `scripts/load_test.py`. Chaque joueur simulé crée sa propre room et lance une partie.
**Rien n'est instrumenté dans le serveur** : la cadence de simulation est déduite du champ `tick` porté par
chaque snapshot — c'est-à-dire **exactement ce qu'un vrai client observe**.

| Parties simultanées | Cadence vue par les clients | Snapshots/client | Débit total |
|---|---|---|---|
| 8 | **60,0** ticks/s | 20,1 /s | 15,1 KiB/s |
| 16 | **60,0** ticks/s | 20,1 /s | 30,6 KiB/s |
| 32 | **60,0** ticks/s | 20,1 /s | 60,2 KiB/s |
| 48 | **60,0** ticks/s | 20,1 /s | 90,3 KiB/s |
| **96** (hors limites, exprès) | **60,0** ticks/s | 20,1 /s | 91,9 KiB/s |

**Aucune dégradation, nulle part. Pas une fraction de tick perdue, sur un seul thread de simulation.**
Extrapolation depuis les 37 µs : **saturation d'un thread vers 450 parties légères**.

### Les limites de la mesure, à assumer **spontanément**

- Les rooms de test ont **un seul joueur** et peu d'entités. Une partie à 4 joueurs avec un boss coûte 3 à 4× plus → **saturation plutôt vers 120 parties lourdes**.
- Générateur de charge et serveur **sur la même machine**, en loopback : pas de latence réseau, pas de coût de carte.
- Le générateur est un **processus Python unique** ; au-delà de ~100 clients, c'est peut-être lui qui limite, pas le serveur.
- Au-delà de 96, c'est le **rate limit des connexions** (32 `CONNECT`/s) qui freine l'établissement des sessions, **pas** la simulation.

### Ce qu'on dit au jury, mot pour mot

> « Le serveur fait tourner **48 parties simultanées à 60 ticks/s exactement**, et on a mesuré jusqu'à **96
> sans aucune dégradation**. On n'a **pas** parallélisé le tick parce qu'on a **mesuré avant** : 73 % du temps
> de simulation est passé dans l'interpréteur Python, dont le GIL sérialise l'exécution. Le gain théorique
> sur 4 cœurs était de **0,9 % du budget d'un tick**, pour 300 lignes de code concurrent et un risque de
> corruption. On a préféré **lever les vraies limites** — qui étaient de configuration et de protocole — et
> **le prouver par la mesure**. »

**Et le rappel qui désarme la question suivante :**
> « À noter : **le serveur est déjà multithreadé**. Il a son thread d'E/S réseau séparé, avec Asio.
> C'est décrit dans `docs/architecture/threading.md`. Ce qui est mono-thread, c'est **la simulation**, et
> c'est volontaire : pas de verrou, et un comportement déterministe pour une suite d'inputs donnée. »

**Perspective à mentionner :** si la logique de jeu passait un jour en C++ plutôt qu'en Python, le GIL
disparaîtrait et la parallélisation redeviendrait pertinente.

## III.10 — Phase 6 : le chat par room et les pseudos

### Pourquoi cette phase

> « Elle coche une case du thème 1 (*"communication between users: text chat"*) **et** elle démontre la
> phase 1 dans le même mouvement : un message de chat perdu est **immédiatement visible**, donc il passe par
> le canal fiable. C'est aussi la première chose de toute la track **qui se voit à l'écran**. »

### Le protocole

```
0x0D  CHAT          client → serveur    token:u32, text:string    FIABLE
0x94  CHAT_MESSAGE  serveur → clients   slot:u8, text:string      FIABLE
```

Deux réglages dans `rtype_game.json` : `chatMaxLength: 64`, `chatIntervalMs: 500`.
La validation du texte réutilise **`TextRules`**, déjà en place pour les noms de rooms (ASCII imprimable 0x20–0x7E).

### L'anti-spam — argument de **sécurité**, pas de confort

> « Sans limite de débit, n'importe quel client peut faire diffuser au serveur un nombre **illimité de
> messages fiables** à **tous** les membres de sa room. C'est un **vecteur d'amplification**, pas une simple
> gêne. Une limite d'un message par session toutes les 500 ms, vérifiée **côté serveur**. »

### Les mesures

| Scénario | Résultat |
|---|---|
| 6 messages échangés à **40 % de perte** | **6/6 délivrés aux deux clients** |
| 10 messages envoyés d'un bloc | **1 diffusé**, les 9 autres écartés |
| 3 messages espacés de 600 ms | **3 diffusés** |

Sur le run à 40 % : 192 paquets ignorés, dont 4 portaient un message fiable, **tous retransmis jusqu'à réception**.

### Le bug que le test a attrapé — à raconter, ça montre la rigueur

> « Premier essai : **zéro message reçu**, des deux côtés. Cause : `GameServer::tokenOf` est un commutateur
> qui extrait le token de chaque message client, et je n'y avais pas ajouté `CHAT`. Il renvoyait `nullopt`,
> qui ne correspond jamais au token de la session, et le serveur jetait **tous** les paquets de chat avant
> même de les traiter.
>
> La règle qu'on en a tirée, écrite dans le journal : **ajouter un message client → serveur demande de
> toucher *quatre* commutateurs, pas trois — `isValid`, `write`, `readBody` et `tokenOf`. Les trois premiers
> font échouer la compilation si on les oublie ; le quatrième échoue en silence.** »

### Un comportement découvert au passage

> « Le client de test anti-spam **n'acquitte rien**. Résultat : le serveur **retransmet indéfiniment** chaque
> message de chat. C'est **correct** — le pair est vivant et ne confirme pas — et c'est **borné** par les
> 64 messages en attente par pair et par le timeout de session de 5 s. C'est documenté dans la section
> sécurité de `protocol.md`. »

### Les pseudos : un choix de propagation à défendre

`ConnectMessage` gagne un champ `name:string`, réglage `playerNameMaxLength: 12`. Le serveur le retient dans la session.

> « **Choix : propager le nom par `ROOM_UPDATE`, pas dans chaque message de chat.** `ROOM_UPDATE` transporte
> désormais, par slot, l'occupation **et** le nom. Deux raisons : le nom coûte **une fois par seconde au lieu
> d'une fois par message**, et les noms apparaissent aussi dans **la liste des joueurs de la room**, pas
> seulement dans le chat. `CHAT_MESSAGE` ne transporte donc que le **slot**. Un client qui n'a pas encore
> reçu de `ROOM_UPDATE` affiche `P1`, `P2` en repli. »

### Les trois corrections d'interface (le protocole était bon, l'UI non)

1. **Le champ de saisie ne se vidait pas.** J'écrivais dans le store (`setValue("field.chatInput", …)`), mais le composant `TextField` est **maître de son contenu et réécrit le store à chaque frame** → ma valeur était écrasée aussitôt. Il fallait vider **le composant**, via `GameView::setFieldText`.
2. **Le chat restait affiché pendant la partie** → retiré.
3. **Pas de pseudo** → d'où l'implémentation du nom de joueur.

**Choix d'UX à défendre :**
> « Pas de saisie de chat **dans la scène de jeu**. Un champ texte actif pendant une partie **capturerait les
> flèches directionnelles**. On lit le chat en jouant, on l'écrit dans la room. »

### Un piège de plus sur les sondes

> « `load_test.py` est resté en `VERSION = 8` alors que le protocole passait à 9 : plus aucune connexion, et
> le serveur répondait `REJECT_VERSION_MISMATCH` sans que rien ne le signale côté test. **Tout bump de
> version impose de mettre à jour les deux scripts de `scripts/`.** »

## III.11 — Les mesures sous `netem` (la preuve finale)

### Le dispositif — et pourquoi il est meilleur que les mesures précédentes

> « Les mesures des phases 1 et 2 utilisaient une sonde Python avec de la **perte simulée dans la sonde**.
> C'est utile mais contestable : on simule la perte **au-dessus** du socket.
>
> Sous `netem`, c'est le **noyau Linux** qui dégrade l'interface, et les clients sont **deux clients
> headless C++ qui utilisent la vraie classe `ClientConnection`, le vrai `Codec` et le vrai `UdpTransport`
> de production** — `scripts/netem_client.cpp`. Un serveur neuf est démarré **pour chaque scénario**.
>
> Et surtout : **les commandes `CREATE_ROOM`, `JOIN_ROOM` et `START_GAME` ne sont envoyées qu'une seule fois
> par le test. Toute répétition est faite par notre couche de fiabilité**, pas par le harnais. C'est
> précisément ce qu'on veut tester. »

`netem` est appliqué sur `lo` **dans un namespace réseau jetable** (`unshare --user --map-root-user --net`)
→ **aucune interface de la machine hôte n'est modifiée**. Dans les deux sens : un `delay 100ms` ajoute donc
**~200 ms au RTT**.

### Les 7 scénarios — tous PASS

| Scénario | Verdict | RTT médian A/B (ms) | Snapshots/s A/B | Reçu A/B (KiB/s) | Trou max A/B (s) |
|---|---|---|---|---|---|
| `baseline` | **PASS** | 2 / 2 | 20,00 / 20,00 | 2,04 / 2,04 | 0,05 / 0,05 |
| `delay_100ms` | **PASS** | 203 / 202 | 20,00 / 20,00 | 3,76 / 3,76 | 0,05 / 0,05 |
| `loss_10pct` | **PASS** | 2 / 2 | 18,15 / 18,20 | 1,84 / 1,84 | 0,15 / 0,20 |
| `duplicate_10pct` | **PASS** | 2 / 2 | 20,00 / 20,00 | 2,20 / 2,21 | 0,05 / 0,05 |
| `reorder_25pct` | **PASS** | 102 / 103 | 18,65 / 18,90 | 2,66 / 2,66 | 0,10 / 0,10 |
| `combined` | **PASS** | 163 / 155 | 17,15 / 17,25 | 2,66 / 2,69 | 0,19 / 0,15 |
| `rate_128kbit` | **PASS** | 88 / 86 | 20,00 / 20,00 | 2,72 / 2,74 | 0,06 / 0,06 |

*(`combined` = `delay 80ms 20ms distribution normal` + `loss 5%` + `duplicate 2%` + `reorder 25% 50%`)*

### Ce que « PASS » veut dire exactement — **sache le réciter, c'est la question suivante du jury**

> « Les deux clients rejoignent **et** lancent ; chacun reçoit **au moins 10 snapshots** pendant la mesure ;
> **aucun tick appliqué ne revient en arrière** ; **aucun `ROOM_JOINED` ni `GAME_STARTED` en double
> n'atteint l'application** ; et **les snapshots ne s'arrêtent jamais pendant 5 secondes**. Une exception ou
> un dépassement de délai du processus fait échouer le scénario. »

### Les limites, à dire **avant** qu'on vous les demande

> « **Une passe par scénario, deux clients, une room.** Ces runs ne prouvent **ni** le comportement en charge
> maximale, **ni** la livraison de chaque évènement de jeu — c'est ce que couvre l'autre série de mesures,
> celle avec perte simulée dans la sonde.
>
> Deux précisions d'honnêteté : les **pourcentages `netem` sont des paramètres aléatoires, pas des taux de
> perte applicative mesurés**. Et le RTT indiqué est **celui du heartbeat**, échantillonné une fois par
> seconde ; une valeur peut être réutilisée si la réponse suivante est perdue. **Ça ne mesure pas le délai
> entre l'appui sur une touche et son affichage.**
>
> Les fichiers bruts `*.qdisc.txt`, `*.server.log`, `*.client.log` et `results.json` sont écrits à côté du
> rapport par chaque run. »

### La lecture intéressante de ces chiffres

| Observation | Interprétation |
|---|---|
| `duplicate_10pct` → **20,00** snapshots/s, inchangé | La déduplication fonctionne : les doublons sont écartés sans coût visible |
| `reorder_25pct` → 18,65, et RTT 102 ms | Le réordonnancement est **absorbé** par `isNewer` ; les snapshots trop vieux sont jetés, d'où la légère baisse |
| `rate_128kbit` → **20,00** snapshots/s à 128 kbit/s | **Notre budget tient dans un lien à 128 kbit/s** — c'est le delta qui rend ça possible |
| `delay_100ms` → débit **plus élevé** (3,76 KiB/s) | Plus de snapshots complets : les baselines expirent (règle « baseline > 1 s de ticks ») |

### Le script de reproduction manuelle

Pour une démo live, `netemTest.md` donne la procédure courte, sur `lo` :

```sh
./build/bin/r-type_server -p 4242 &

sudo tc qdisc add dev lo root netem loss 20%
./scripts/protocol_probe.py 4242          # le verdict s'affiche en dernière ligne
sudo tc qdisc del dev lo root
```

Les trois choses qu'on surveille :
1. **`FAIL`** ou **`unbroken … NO`** → un message de jeu a été perdu définitivement. **C'est le résultat le plus important.**
2. **`duplicates discarded: 0` dans le scénario avec `duplicate 5%`** → suspect, la déduplication devrait en écarter.
3. **`malformed datagram ignored`** ou un crash → c'est un bug.

> ⚠️ Ne **jamais** cumuler la perte `netem` et la perte simulée de la sonde (2ᵉ argument) : ça fausse tout.

## III.12 — Les leçons de méthode (parfait pour une question ouverte)

| Leçon | Origine |
|---|---|
| **On mesure, on ne calcule pas à partir du pire cas.** | Le surcoût d'en-tête annoncé « <1 % », mesuré à **7 %** |
| **Compter les doublons ne mesure pas les retransmissions.** | Quand l'originale est perdue, la retransmission la remplace et n'apparaît jamais comme un doublon |
| **Un test qui échoue doit être suspecté avant le code.** | À 40 % de perte, A recevait 2 `ROOM_JOINED` : c'était **la sonde** qui réessayait `CREATE_ROOM`, et le serveur répondait légitimement |
| **Le délai de retransmission doit dépasser la cadence d'émission du pair**, pas seulement le RTT. | 4 retransmissions inutiles à 0 % de perte avec un plancher à 30 ms |
| **Accuser réception et traiter sont deux opérations distinctes.** | L'`ackBits` qui mentait à cause du garde anti-rafale |
| **Mesurer avant d'optimiser.** | La parallélisation abandonnée : 0,9 % de gain pour 300 lignes de code concurrent |
| **La mesure peut démolir l'intuition de départ.** | 86 % des entités bougent → sauter les immobiles ne rapporte que 14 % |

## III.13 — La sécurité du protocole (section 11 de `protocol.md`)

| Risque | Parade |
|---|---|
| Datagramme malformé ou tronqué | Chaque longueur et chaque compteur est validé **avant usage** ; paquet fautif jeté **en silence** |
| Épuisement mémoire | Datagrammes ≤ 1200 o, files de réception à **256** entrées, messages fiables en attente à **64 par pair** |
| Vol de session | Token aléatoire 32 bits **lié à l'adresse et au port source**, exigé sur chaque message |
| Flood de connexions | **32 `CONNECT`/s** max, et **un `ACCEPT` par client toutes les 200 ms** |
| Flood de chat | **1 message / 500 ms / session** ; texte validé en longueur et en caractères imprimables |
| Retransmission infinie vers un pair muet | Bornée par 64 messages en attente **et** le timeout de 5 s |
| Triche | **Serveur autoritaire** : positions, collisions, vies, scores calculés serveur ; le client ne soumet que des états de boutons |
| Amplification | Aucun message ne produit une réponse plus grosse que lui, **sauf `LIST_ROOMS`**, qui n'est servi qu'à une session établie |

**Et la limite assumée, à dire soi-même :**
> « Le protocole **n'est ni chiffré ni authentifié** au-delà du token de session. Les tokens ne sont
> imprévisibles que dans la mesure où le générateur aléatoire du serveur l'est, et le trafic peut être observé
> et rejoué par quiconque est sur le chemin. **C'est acceptable pour un jeu en LAN et ne le serait pas pour un
> service exposé.** La suite serait DTLS, ou un handshake avec un secret. »

## III.14 — Ce qui n'est pas fait, et comment le formuler

| Technique | Statut, à dire tel quel |
|---|---|
| **Prédiction côté client + réconciliation** | « **Analysée, pas implémentée.** Le prérequis est un **numéro de séquence sur `INPUT`**, que le protocole ne porte pas encore. Le modèle serveur-autoritaire est exactement celui dont elle a besoin : c'est l'étape suivante documentée. » |
| **Interpolation d'entités** | « Analysée, pas implémentée : le client applique directement les positions du snapshot. » |
| **Lag compensation / rollback serveur** | « Hors périmètre. » |
| **Compression générique (LZ4)** | « **Volontairement différée**, et pour une raison mesurée : le delta a déjà consommé la redondance. » |
| **Packing à cheval sur les octets** | « Volontairement différé : gain restant estimé ~1 octet par entité, contre un `BitWriter`/`BitReader` à curseur en bits. » |
| **Identités persistantes / reconnexion** | Non fait |
| **Scoreboard global persistant** | Non fait |
| **Console d'administration serveur** | Non fait (la console de dev F1 est autre chose : des commandes de jeu, réservées au propriétaire de la room et désactivables par la règle `cheats`) |
| **Mesures sous `clumsy` (Windows)** | Non lancées ; `netem` sur Linux, oui |

**Le coût de l'absence de prédiction, assumé :**
> « Sans prédiction, le vaisseau d'un joueur réagit après **un aller-retour**. Sur un LAN — 2 ms de RTT
> mesurés — ce n'est pas perceptible. Sous `delay_100ms`, c'est-à-dire 200 ms de RTT, ça le devient : c'est
> exactement la limite que la prédiction lèverait, et c'est pour ça qu'elle est la première ligne de notre
> roadmap réseau. »

---
---

# PARTIE IV — Les chiffres à connaître par cœur

## Protocole

| Grandeur | Valeur |
|---|---|
| Version du protocole | **9** |
| Magic | **`0x454E`** (`"EN"`) |
| En-tête | **20 octets** |
| Datagramme max | **1200 octets** |
| Port par défaut | **4242** |
| Boutisme | **big-endian** |
| Messages | **13** client→serveur, **20** serveur→client |
| Fenêtre d'`ackBits` | **32 bits** → 1 paquet acquitte **33** paquets |
| Déduplication par `reliableId` | fenêtre de **256** |
| Messages fiables en attente | **64** par pair max |
| Historique de snapshots | **16** serveur / **32** client |

## Cadences

| Grandeur | Valeur |
|---|---|
| Ticks de simulation | **60 /s** (budget **16 667 µs** par tick) |
| Snapshots | **tous les 3 ticks = 20 /s** |
| Inputs | **toutes les 33 ms ≈ 30 /s** |
| Relâchement des boutons | **500 ms** sans input |
| Heartbeat | **1 /s** |
| Timeout de session | **5 s**, des deux côtés |
| Réessai de connexion | **1 s × 5 tentatives** |
| `ROOM_UPDATE` | **1 /s** |
| Retransmission | **`max(100 ms, RTT × 1,5)`** |
| Keyframe | **5 s** max entre deux snapshots complets |
| Baseline périmée | **1 s de ticks** |

## Tailles et débits

| Grandeur | Valeur |
|---|---|
| `INPUT` | **25 octets** |
| `HEARTBEAT` | 28 o · `HEARTBEAT_ACK` 24 o |
| `SNAPSHOT` complet | **102 + 8 × N** octets (N = entités répliquées) |
| `SNAPSHOT_DELTA` | **105 o** en moyenne, **33 o** minimum |
| Snapshot sans delta (contrôle) | **223 o** |
| Gain du delta | **×2,1** (223 → 130 → 105) |
| Descendant par client | **34 kbit/s → 16 kbit/s** |
| Pire cas (130 entités) | ~183 kbit/s par client, ~732 kbit/s à 4 joueurs |
| Surcoût de l'en-tête de fiabilité | **+10 o/paquet = 7 % mesuré** |
| `EntityState` | **8 octets** · `PlayerStatus` **10 octets** + valeurs |

## Capacités

| Grandeur | Valeur |
|---|---|
| Joueurs par room | **4** |
| Rooms | **48** (plafond protocolaire : **53**, à cause de `ROOM_LIST` dans 1200 o) |
| Clients | **192** |
| Entités répliquées max | **130** |
| Types de composants ECS | **64** (masque `uint64_t`) |
| Parties simultanées mesurées | **96 à 60,0 ticks/s** |
| Saturation extrapolée | ~**450** parties légères, ~**120** parties lourdes |

## Coût CPU par room et par tick

| Poste | Temps |
|---|---|
| Simulation C++ | **8 µs** |
| **Python** | **24 µs (73 %)** |
| Encodage réseau | **5 µs** |
| **Total** | **37 µs** sur un budget de **16 667 µs** |

## Les preuves de fiabilité

| Mesure | Résultat |
|---|---|
| Évènements de jeu perdus à **60 %** de perte | **0** (4/4, contre 4/7 sans fiabilité) |
| Pire cas observé | `reliableId=6` perdu **5 fois de suite**, délivré quand même |
| Retransmissions inutiles à 0 % de perte | **0** |
| Aller-retour `build`/`apply` vérifié | **1218 snapshots, 0 échec** |
| Chat à **40 %** de perte | **6/6 délivrés** |
| Scénarios `netem` | **7 / 7 PASS** |

---
---

# PARTIE V — Questions pièges + réponses

## Sur le réseau

**« Pourquoi UDP et pas TCP ? »**
> « Au-delà de l'exigence du sujet, la raison technique est que **la fiabilité n'est pas une propriété qu'on
> veut uniformément**. Une position perdue ne **doit pas** être retransmise : une plus fraîche arrive 50 ms
> plus tard, et relivrer un état périmé c'est exactement le comportement de TCP qui produit du stutter
> visible. Un "ennemi détruit" perdu, lui, est perdu pour toujours. **TCP ne sait pas exprimer cette
> distinction** ; notre couche d'acquittement applique la fiabilité message par message. »

**« Pourquoi ne pas utiliser TCP pour les messages fiables et UDP pour le reste ? »**
> « On a comparé les trois options dans l'étude comparative. Un canal TCP annexe ne coûte aucune ligne de
> code, mais **ramène le head-of-line blocking sur le flux fiable** — exactement ce qu'on fuit — et ajoute
> **une deuxième connexion, deux modes de panne, deux politiques de timeout**. Notre couche fait ~250 lignes
> et nous donne un contrôle fin. »

**« Et si vous envoyiez juste chaque évènement trois fois ? »**
> « C'était l'option 2 de notre comparaison. **Trivial à écrire, mais ça gaspille de la bande passante en
> permanence et ça échoue toujours sur une rafale de pertes.** Chez nous, à 0 % de perte le mécanisme coûte
> **0 retransmission** ; et à 60 %, un message a été retransmis **cinq fois** et est passé. »

**« Comment savez-vous qu'un paquet est perdu ? »**
> « On ne le sait pas directement, et c'est voulu. Le récepteur annonce dans chaque paquet le dernier numéro
> reçu plus l'historique sur 32 bits des 32 précédents. L'émetteur garde ses messages fiables non acquittés ;
> si un message n'est pas confirmé au bout de `max(100 ms, RTT × 1,5)`, il le réémet. **On ne détecte pas la
> perte, on détecte l'absence de confirmation** — c'est plus robuste, parce que ça couvre aussi la perte de
> l'acquittement lui-même. »

**« Que se passe-t-il si un `ack` est perdu ? »**
> « Rien de grave : le paquet suivant répète la même information, puisque `ack`/`ackBits` décrivent **l'état
> cumulé** du récepteur, pas un évènement. Dans le pire cas, l'émetteur retransmet un message qui était déjà
> arrivé — et le récepteur l'écarte grâce à son `reliableId`. »

**« Pourquoi les séquences commencent-elles à 1 et pas à 0 ? »**
> « Pour que `ack = 0` puisse signifier sans ambiguïté **"je n'ai encore rien reçu de toi"**. »

**« Que se passe-t-il au bout de 2³² paquets ? »**
> « `Codec::isNewer` fait une comparaison **modulaire** : `candidate` est plus récent si
> `(candidate − previous) mod 2³²` tombe dans `1 .. 2³¹−1`. Le wrap-around est géré. Le cas ambigu — une
> distance d'exactement 2³¹ — est tranché explicitement : il n'est **pas** considéré comme plus récent.
> À 20 paquets/s, le wrap arrive de toute façon au bout de ~6 ans. »

**« Votre delta snapshot, que se passe-t-il si le client perd la baseline ? »**
> « C'est **impossible par construction**, et c'est l'invariant du design : **on ne diffe jamais contre un
> snapshot envoyé, seulement contre un snapshot acquitté**. Si le client acquitte le paquet, c'est qu'il
> l'avait. Avec deux garde-fous : l'historique client fait 32 entrées contre 16 côté serveur, et on retient
> toujours la baseline de **tick le plus récent** parmi les acquittées, ce qui neutralise le réordonnancement.
>
> Et **si l'invariant était quand même violé** par un bug qu'on n'a pas vu : le client refuse simplement le
> delta dont il n'a pas la baseline (`handleDelta` renvoie `false`), et le snapshot complet envoyé toutes les
> 5 secondes le remet d'aplomb. **La durée de n'importe quelle désynchronisation est bornée à 5 secondes.** »

**« Combien d'octets avez-vous ajoutés pour les delta snapshots ? »**
> « **Zéro sur le fil.** Le serveur réutilise l'`ack` de la couche de fiabilité pour savoir quelle photo le
> client possède. Les deltas sont entièrement gratuits en protocole. »

**« Vous ne faites pas de compression ? »**
> « Pas de compression **générique**, et c'est une décision mesurée, pas un oubli. La compression générique
> et le delta exploitent **la même redondance** : une fois les slots joueur répétés, les valeurs de monde
> inchangées et les champs constants retirés, il ne reste pas grand-chose dans 105 octets. L'ordre honnête
> c'est delta d'abord, compression ensuite, et seulement si le gain mesuré justifie la dépendance.
> On a comparé RLE, Huffman, LZ4 et zlib ; c'est dans l'étude comparative. »

**« Votre serveur est-il multithreadé ? »** *(piège : le sujet demande "multithreaded server")*
> « Oui : **thread principal + thread d'E/S réseau** avec Asio. Ce qui est mono-thread, c'est la
> **simulation** — volontairement, et après mesure. Je peux vous donner les chiffres : 37 µs par room et par
> tick, dont 73 % dans l'interpréteur Python, dont le GIL sérialise de toute façon. Gain théorique de la
> parallélisation sur 4 cœurs : **0,9 % du budget d'un tick**. On a préféré lever les vraies limites — qui
> étaient de configuration et de protocole — et prouver **48 parties à 60,0 ticks/s**, mesurées jusqu'à 96. »

**« Comment empêchez-vous la triche ? »**
> « Le client n'envoie **qu'un octet de boutons**. Jamais une position, jamais un dégât, jamais un score.
> Tout est calculé serveur. Plus : un token de session aléatoire 32 bits lié à (adresse, port) sur chaque
> message, et le **bit de direction** de l'en-tête qui empêche un client d'émettre un message réservé au
> serveur. Au pire, un client modifié peut **mentir sur ses propres boutons** — c'est-à-dire jouer. »

**« Que se passe-t-il si j'envoie des octets au hasard sur votre port ? »**
> « `Codec::decode` renvoie `nullopt` et le datagramme est jeté **en silence** : pas de réponse, pas de
> déconnexion de qui que ce soit. On le teste avec `nc -u`, et `netem_test.py` fait échouer un scénario si le
> serveur lève une exception. **8 conditions** sont validées avant de lire le corps : taille, magic, sens,
> type connu, version, `payloadSize` cohérent, puis le corps champ par champ, et enfin `isValid`. »

**« Pourquoi 1200 octets et pas 1500 ? »**
> « Pour ne **jamais** dépendre du MTU du chemin. 1500 c'est l'MTU Ethernet, mais une encapsulation (VPN,
> PPPoE, tunnel) le réduit, et un datagramme UDP fragmenté est **perdu en entier dès qu'un fragment manque**.
> 1200 passe partout. C'est aussi ce qui nous a donné la limite de 53 rooms dans `ROOM_LIST` — et on préfère
> une limite connue à une fragmentation silencieuse. »

**« Votre protocole est-il extensible ? Comment ajoutez-vous un message ? »**
> « Un octet de type, 7 bits utilisables par sens → **127 codes par sens**, dont 13 et 20 utilisés : il reste
> **114 codes client→serveur et 107 serveur→client**. Pour ajouter un message :
> une classe avec `isValid`/`write`/`read`, une entrée dans l'enum, une dans le `variant` de `Packet`, une
> dans `Codec::readBody` — et, pour un message client, **`tokenOf`**. Les visiteurs font **échouer la
> compilation** partout où il faut le traiter. C'est documenté dans
> `docs/guides/engine/add-a-network-message.md`. Et on a appris à la dure que `tokenOf` est le seul des
> quatre qui échoue **en silence**. »

## Sur l'architecture

**« Montrez-moi où R-Type est hardcodé dans le moteur. »**
> « Nulle part, et c'est vérifiable en direct — **`grep -ril rtype engine/` ne renvoie aucun fichier**, pas
> plus que `bydo`. Les réglages réseau, le nombre de valeurs transmises, la liste et l'ordre des systèmes,
> les prefabs, les scènes, le HUD : tout vient du JSON. La preuve vivante, c'est **Skyhop**. »

**Les trois `grep` à taper en direct** (vérifiés, tous les trois ne renvoient rien) :

```sh
grep -ril rtype engine/    # aucun fichier → le moteur ne connaît pas R-Type
grep -ril bydo  engine/    # aucun fichier → aucun contenu de jeu dans le C++
grep -ril tcp   engine/    # aucun fichier → zéro socket TCP dans le projet
```

**« Pourquoi ECS et pas de l'héritage ? »**
> « Composition plutôt qu'héritage : les comportements se combinent librement et peuvent être **ajoutés ou
> retirés à l'exécution**. L'exemple concret : un vaisseau respawné **reçoit** `Invulnerable` pendant deux
> secondes, puis le **perd**. En héritage il faudrait une classe `InvulnerableShip` et un changement de type
> à chaud. Le coût assumé : **le comportement d'un objet est réparti sur plusieurs systèmes.** »

**« Vos composants sont-ils des classes ? »**
> « Oui, toutes, avec membres privés et accesseurs. Et `SparseArray` le **force** :
> `static_assert(std::is_base_of_v<IComponent, Component>)` — un composant qui ne dérive pas d'`IComponent`
> ne compile pas. »

**« Pourquoi 64 types de composants maximum ? »**
> « Parce que le masque est un `uint64_t`, et qu'une requête compare **un entier** par entité au lieu
> d'interroger chaque stockage. C'est une limite documentée dans les décisions, avec son remède :
> un type de masque plus large. Les deux jeux sont loin de la limite. »

**« Et si on veut remplacer SFML ? »**
> « Trois modules à toucher — `render`, `audio`, `input` — et **aucun header du reste du moteur**, parce que
> SFML est une dépendance **privée** derrière `IRenderer` et `IAudio`. Il y a un guide dédié :
> `docs/guides/engine/replace-a-library.md`. »

**« Vos plugins, à quoi ça sert concrètement ? »**
> « C'est le dernier niveau de modularité du sujet : **étendre le moteur sans le recompiler**. Un système
> livré en `.so`/`.dll`, nommé dans le JSON du jeu, chargé au démarrage et **rechargé quand son fichier
> change**. Le moteur charge une **copie temporaire** de la bibliothèque, pour qu'on puisse recompiler
> l'originale pendant que le jeu tourne. `plugins/wobble` est utilisé par Skyhop. »

---
---

# PARTIE VI — Les fichiers à savoir ouvrir en 10 secondes

| Si on vous demande… | Ouvrez |
|---|---|
| L'en-tête, les types, les constantes | [engine/network/protocol/include/Protocol.hpp](engine/network/protocol/include/Protocol.hpp) |
| L'encodage / le décodage / la validation | [engine/network/protocol/src/Codec.cpp](engine/network/protocol/src/Codec.cpp) |
| La politique de fiabilité (quels messages) | [engine/network/protocol/src/Protocol.cpp](engine/network/protocol/src/Protocol.cpp) |
| La fenêtre d'ACK, le bit-shifting | [engine/network/protocol/src/ReliableReceiver.cpp](engine/network/protocol/src/ReliableReceiver.cpp) |
| Le RTT, la retransmission | [engine/network/protocol/src/ReliableSender.cpp](engine/network/protocol/src/ReliableSender.cpp) |
| L'invariant de baseline acquittée | [engine/network/protocol/src/SnapshotHistory.cpp](engine/network/protocol/src/SnapshotHistory.cpp) |
| `build` / `apply` du delta | [engine/network/protocol/src/SnapshotDiff.cpp](engine/network/protocol/src/SnapshotDiff.cpp) |
| Les trois déclencheurs de snapshot complet | `GameServer::sendSnapshot` — [engine/network/server/src/GameServer.cpp](engine/network/server/src/GameServer.cpp) |
| Le découplage serveur / jeu | [engine/network/server/include/IGameSession.hpp](engine/network/server/include/IGameSession.hpp) |
| Asio caché derrière un pimpl | [engine/net/internal/UdpTransportImpl.hpp](engine/net/internal/UdpTransportImpl.hpp) |
| Les réglages réseau lus depuis le JSON | [engine/network/protocol/include/NetworkSettings.hpp](engine/network/protocol/include/NetworkSettings.hpp) + `game/data/rtype_game.json` |
| L'ECS : masques et sparse arrays | [engine/ecs/include/Registry.hpp](engine/ecs/include/Registry.hpp), [engine/ecs/include/EntityManager.hpp](engine/ecs/include/EntityManager.hpp) |
| **La doc du protocole, format RFC** | [docs/reference/protocol.md](docs/reference/protocol.md) |
| **Les mesures netem** | [docs/measurements/netem-2026-11-08/report.md](docs/measurements/netem-2026-11-08/report.md) |
| **Les justifications chiffrées des choix** | [docs/comparative-study/networking.md](docs/comparative-study/networking.md) |
| Les décisions d'architecture, coût et limites | [docs/architecture/decisions.md](docs/architecture/decisions.md) |
| Le threading et le refus de paralléliser | [docs/architecture/threading.md](docs/architecture/threading.md) |
| L'accessibilité | [docs/project/accessibility.md](docs/project/accessibility.md) |
| Le journal de la track, phase par phase | [trackNet.md](trackNet.md) |

---
---

# PARTIE VII — Hygiène d'oral

## À faire

- **Donner un chiffre à chaque affirmation.** « C'est fluide » ne vaut rien ; « 20,00 snapshots/s délivrés, 18,15 à 10 % de perte » vaut tout.
- **Annoncer les limites soi-même**, avant qu'on vous les demande. « Une passe par scénario », « générateur et serveur sur la même machine », « rooms de test à un seul joueur ».
- **Dire « mesuré » ou « estimé »**, jamais laisser le doute. Les deux sont acceptables, le flou non.
- **Raconter un bug** quand on vous laisse une question ouverte : l'`ackBits` qui mentait, le `tokenOf` oublié, les 4 retransmissions inutiles. Ça prouve qu'on a compris, pas juste copié.
- **Assumer un non-fait avec sa raison et son prérequis** : « pas de prédiction client ; le prérequis est un numéro de séquence sur `INPUT` ».

## À ne pas dire

| ❌ | ✅ |
|---|---|
| « On a fait du bit-shifting pour compresser » | « `ackBits` est un bitfield manipulé par décalage ; la compression, c'est le delta et la quantification » |
| « UDP c'est plus rapide » | « UDP n'a pas de head-of-line blocking, et nous permet d'appliquer la fiabilité message par message » |
| « On n'a pas eu le temps de paralléliser » | « On a mesuré avant : 0,9 % de gain, 73 % du temps dans le GIL Python. On a levé les vraies limites à la place » |
| « Ça marche bien » | « 7 scénarios netem, 7 PASS, et voici le tableau » |
| « Je crois que… » | « Je ne suis pas sûr, c'est dans tel fichier, je peux vérifier » |
| « On a tout documenté » *(c'était le reproche midterm : trop)* | « La doc est organisée en 11 sections format RFC, avec un objectif annoncé : écrire un client depuis cette seule page » |

## Le résumé en 5 phrases, si on vous coupe

1. **Un moteur, pas un jeu** : tout le C++ est dans `engine/`, et Skyhop le prouve.
2. **Serveur autoritaire**, 60 ticks/s, le client n'envoie qu'un octet de boutons.
3. **Protocole binaire UDP**, 20 octets d'en-tête, 1200 octets max, big-endian, 1 datagramme = 1 message.
4. **La fiabilité est sélective** : on retransmet uniquement ce qui ne peut pas être reconstruit par un paquet plus frais — et à 60 % de perte, zéro évènement de jeu perdu.
5. **Tout est mesuré** : delta ×2,1, 48 parties à 60,0 ticks/s, 7 scénarios `netem` en PASS.

---
---

# ANNEXE — À corriger avant la soutenance (2 défauts repérés dans la doc)

Rien de fonctionnel, mais ce sont deux choses qu'un jury qui ouvre un fichier peut voir.

## 1. `docs/reference/protocol.md` — la « Source map » pointe vers des chemins qui n'existent plus

Le module `engine/network/` a été réorganisé en `protocol/`, `server/`, `client/`, `messages/`, mais le
tableau de la section 1 n'a pas suivi. **7 lignes fausses** :

| Ce qui est écrit | Le vrai chemin |
|---|---|
| `engine/network/include/Protocol.hpp` | `engine/network/protocol/include/Protocol.hpp` |
| `engine/network/src/Codec.cpp` | `engine/network/protocol/src/Codec.cpp` |
| `engine/network/src/ReliableReceiver.cpp` | `engine/network/protocol/src/ReliableReceiver.cpp` |
| `engine/network/src/ReliableSender.cpp` | `engine/network/protocol/src/ReliableSender.cpp` |
| `engine/network/src/SnapshotDiff.cpp` | `engine/network/protocol/src/SnapshotDiff.cpp` |
| `engine/network/src/GameServer.cpp` | `engine/network/server/src/GameServer.cpp` |
| `engine/network/src/ClientConnection.cpp` | `engine/network/client/src/ClientConnection.cpp` |

*(La ligne « Big-endian primitives → `engine/net/src/ByteWriter.cpp` » est correcte, elle.)*

## 2. `trackNet.md` — codes de message périmés

Le journal décrit les codes **d'avant** le commit `39d31c4` (« Protocol v9: unique message codes for console,
chat and delta snapshots ») :

| Dans `trackNet.md` | En vrai |
|---|---|
| `0x0C  CHAT` | **`0x0D`** (`0x0C` = `CONSOLE_COMMAND`) |
| `0x93  CHAT_MESSAGE` | **`0x94`** |
| `SNAPSHOT_DELTA (0x92)` | **`0x93`** (`0x92` = `CONSOLE_TEXT`) |

**`docs/reference/protocol.md` est juste**, lui, et c'est ce document-là qui est livré.
Donc : si on vous demande un code de message, citez **la doc, pas le journal**.
