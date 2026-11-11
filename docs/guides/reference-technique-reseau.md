# Référence technique détaillée — archive réseau R-Type

**Document historique, conservé pour ses explications détaillées.** Il décrit l'état antérieur au commit `25bc836` : des fichiers ont été déplacés, les types de messages et les valeurs répliquées ont évolué. Il ne doit plus être utilisé comme spécification à jour. Consulter la [fiche de questions sur le réseau actuel](fiche-revision-reseau.md), [Protocol.hpp actuel](../../engine/network/protocol/include/Protocol.hpp) et la [spécification actuelle](../reference/protocol.md) pour vérifier noms, tailles et comportements.

Complément de la [fiche de révision par critères](fiche-revision-reseau.md). Ce document conserve les explications techniques détaillées et les formats binaires pour approfondir une question.

État du code vérifié le **9 novembre 2026**. Cette fiche couvre le réseau du projet, ses dépendances directes et les notions nécessaires pour expliquer la track Advanced Networking. Elle comprend aussi les fichiers que nous n'avons pas encore étudiés ensemble. Les valeurs viennent du code et du JSON actuels ; elles remplacent les anciennes valeurs évoquées au début du cours.

**À retenir dès maintenant :** serveur autoritaire, UDP/IPv4, protocole binaire version 9, **31 types de messages**, en-tête de 20 octets, limite applicative de 1 200 octets, simulation cible à 60 Hz, snapshots à 20 Hz, inputs environ 30 Hz, fiabilité sélective par ACK, snapshots delta, plusieurs salles indépendantes, pseudos et chat textuel par salle.

**Mise à jour du 9 novembre :** vérification sur le commit `b19fb57`. Le passage de la version 8 à la version 9 ajoute un nom dans CONNECT, les noms dans ROOM_UPDATE et les messages CHAT/ChatBroadcast. Les mesures netem du 8 novembre restent des résultats historiques du protocole version 8 ; elles n'ont pas été rejouées pour cette mise à jour et ne valident pas les nouvelles fonctions de chat.

## Sommaire

1. [Vue d'ensemble et parcours des données](#1-vue-densemble-et-parcours-des-données)
2. [Vocabulaire réseau expliqué](#2-vocabulaire-réseau-expliqué)
3. [Les éléments C++ à reconnaître](#3-les-éléments-c-à-reconnaître)
4. [Carte des fichiers](#4-carte-des-fichiers)
5. [Démarrage dans main.cpp](#5-démarrage-dans-maincpp)
6. [Configuration complète](#6-configuration-complète)
7. [Boucle serveur et temps fixe](#7-boucle-serveur-et-temps-fixe)
8. [GameServer : mémoire et fonctions](#8-gameserver--mémoire-et-fonctions)
9. [Connexion et contrôle d'accès](#9-connexion-et-contrôle-daccès)
10. [Salles et simulations indépendantes](#10-salles-et-simulations-indépendantes)
11. [Transport UDP et Asio](#11-transport-udp-et-asio)
12. [Threads, mémoire et arrêt](#12-threads-mémoire-et-arrêt)
13. [Protocole, Packet et Codec](#13-protocole-packet-et-codec)
14. [Catalogue de tous les messages](#14-catalogue-de-tous-les-messages)
15. [Sérialisation et packing](#15-sérialisation-et-packing)
16. [Séquences, ACK et retransmissions](#16-séquences-ack-et-retransmissions)
17. [Snapshots complets](#17-snapshots-complets)
18. [Compression delta et historiques](#18-compression-delta-et-historiques)
19. [Simulation et scripts](#19-simulation-et-scripts)
20. [Client réseau et affichage](#20-client-réseau-et-affichage)
21. [Lag et techniques de synchronisation](#21-lag-et-techniques-de-synchronisation)
22. [Mesures, formules et netem](#22-mesures-formules-et-netem)
23. [Sécurité et limites réelles](#23-sécurité-et-limites-réelles)
24. [Lien avec chaque thème de la track](#24-lien-avec-chaque-thème-de-la-track)
25. [Démonstration et diagnostic](#25-démonstration-et-diagnostic)
26. [Questions de soutenance avec réponses](#26-questions-de-soutenance-avec-réponses)
27. [Pièges et ordre de révision](#27-pièges-et-ordre-de-révision)

## 1. Vue d'ensemble et parcours des données

Le **client** collecte les touches, envoie des commandes, reçoit un état du monde et l'affiche. Le **serveur** décide des positions, collisions, tirs, vies, scores et comportements des ennemis. Une commande du client exprime une intention ; elle ne constitue pas une position imposée au serveur.

```text
Touches du joueur
  → OnlineClient::buttons() : masque de boutons
  → ClientConnection : InputMessage + en-tête, ACK
  → Codec::encode() : tableau d'octets
  → UdpTransport : envoi UDP
  → réseau
  → UdpTransport du serveur : réception et mise en file
  → GameServer::poll() puis receive() : décodage et contrôles
  → handleInput() : retrouver salle et slot
  → Simulation::setInput() : mettre à jour les commandes
  → Simulation::update(dt) : faire avancer le monde
  → Simulation::snapshot() : état complet
  → GameServer::sendSnapshot() : complet ou delta par client
  → réseau
  → ClientConnection : validation, fraîcheur, reconstruction du delta
  → OnlineClient::applySnapshot()
  → GameView : création, modification, suppression des objets visibles
```

Cette architecture est **client-serveur**, pas peer-to-peer : les clients n'envoient pas directement leurs états aux autres clients. Un serveur héberge plusieurs salles et partage entre elles un transport UDP. Chaque salle possède sa propre simulation.

## 2. Vocabulaire réseau expliqué

| Élément | Définition et exemple dans le projet |
|---|---|
| Réseau | Moyen de faire communiquer des processus, sur la même machine ou sur des machines différentes. |
| Hôte / host | Machine ou nom désignant la destination, par exemple `127.0.0.1` ou un nom résolu en IPv4. |
| Adresse IP | Adresse utilisée pour acheminer les données. Elle n'identifie pas à elle seule un joueur. |
| IPv4 | Famille d'adresses utilisée ici, comme `192.168.1.20`. Le transport ne fournit pas IPv6. |
| Port | Nombre qui désigne une extrémité de communication sur un hôte. Le serveur utilise 4242 par défaut. |
| Endpoint | Couple **adresse IP + port**. Exemple : `192.168.1.20:53012`. |
| Socket | Ressource du système d'exploitation utilisée pour envoyer et recevoir. Ce n'est pas une salle ni un client. |
| Bind | Associer une socket à une adresse et à un port locaux. Le constructeur Asio du serveur le fait. |
| `0.0.0.0` | Adresse locale générique permettant d'écouter sur toutes les interfaces IPv4. Ce n'est pas l'adresse à donner aux joueurs distants. |
| Loopback | Réseau interne à la machine. `127.0.0.1` désigne toujours la machine qui exécute le client. |
| Port éphémère | Port local choisi par le système lorsqu'on demande le port 0. Utilisé pour la socket cliente. |
| UDP | Transport par datagrammes. Préserve les limites des messages, sans garantir leur arrivée, leur ordre ou l'absence de duplication. |
| TCP | Transport fiable et ordonné sous forme de flux d'octets. Une perte peut retarder la livraison des octets suivants. Aucun canal TCP applicatif n'est ouvert par le jeu. |
| Datagramme | Unité envoyée par UDP. Dans le projet, un datagramme transporte un message encodé. |
| Paquet / Packet | Dans le code : objet contenant l'en-tête logique et un message typé. Sa représentation en mémoire n'est pas transmise directement. |
| Message | Intention applicative : `INPUT`, `JOIN_ROOM`, `SNAPSHOT`, etc. |
| Protocole | Convention partagée : types, champs, tailles, ordre des octets et règles de traitement. |
| Header / en-tête | Métadonnées placées au début : version, type, séquence, ACK, etc. Ici 20 octets. |
| Payload | Contenu utile après l'en-tête : par exemple token et boutons dans un `INPUT`. |
| Peer | Correspondant avec lequel on échange. Un client a le serveur pour peer ; le serveur en gère plusieurs. |
| Session | État applicatif d'un client connecté : endpoint, token, timers, historiques et salle éventuelle. UDP ne crée pas cette session automatiquement. |
| Token | Identifiant de session 32 bits envoyé par le serveur et répété dans les messages du client. |
| Pseudo / display name | Nom d'affichage transmis à la connexion. Facultatif, non unique et non authentifié ; ce n'est pas un compte. |
| Chat de salle | Messages textuels relayés par le serveur aux seuls membres de la salle, émetteur compris. |
| Slot | Place locale d'un joueur dans une salle : 0 à 3 dans la configuration actuelle. |
| Room / salle | Groupe de clients associé à une partie et à une simulation distincte. |
| Lobby | Interface de découverte et de sélection des salles, avant ou entre les parties. |
| Sérialisation | Transformer les champs d'un message en octets. |
| Désérialisation | Reconstruire les champs à partir des octets, après contrôle du format. |
| Endianness | Ordre des octets d'un entier multi-octets. Ici big-endian : octet le plus significatif d'abord. |
| Padding | Octets ajoutés par le compilateur dans une structure pour son alignement mémoire. Non transmis ici. |
| Packing | Réduire le coût de représentation : petits types, masques, champs conditionnels. |
| Quantification | Remplacer une valeur précise par une représentation plus grossière : positions entières, angle sur 128 pas. |
| Snapshot | Photo de l'état du monde à un tick donné. |
| Delta | Différence entre un état de référence et un nouvel état. |
| Baseline | Snapshot précis servant de référence à un delta. |
| Keyframe | Snapshot complet périodique permettant de repartir d'un état autonome. |
| Tick | Un pas de simulation. Son compteur appartient à une partie, pas à un paquet. |
| Tick rate | Nombre cible de ticks par seconde : 60 ici. |
| FPS | Images affichées par seconde. Indépendant du tick rate et du rythme des snapshots. |
| Latence | Délai de transmission dans un sens. Le projet mesure surtout l'aller-retour. |
| RTT / ping | Temps d'aller-retour d'un échange. 200 ms de RTT ne signifie pas 200 ms dans chaque sens. |
| Jitter | Variation du délai d'arrivée, qui rend la réception irrégulière. |
| Perte | Datagramme qui n'arrive pas à son destinataire ou qui est abandonné avant traitement. |
| Réordonnancement | Paquets reçus dans un ordre différent de celui des envois. |
| Duplication | Plusieurs arrivées d'un même paquet ; une retransmission est un autre phénomène. |
| Séquence | Numéro d'un paquet envoyé, permettant de reconnaître sa fraîcheur et de l'acquitter. |
| ACK | Information indiquant qu'un paquet a été reçu. Ce n'est pas automatiquement une preuve de succès de l'action demandée. |
| ACK bitfield | Masque donnant la réception des 32 paquets précédant l'ACK principal. |
| Retransmission | Nouvel envoi d'un message fiable non acquitté dans le délai prévu. |
| Déduplication | Éviter d'exécuter plusieurs fois un même message reçu plusieurs fois. |
| Timeout | Délai au-delà duquel l'application considère le correspondant comme perdu. |
| Heartbeat | Message périodique de présence, contenant ici un horodatage renvoyé dans la réponse. |
| Débit | Quantité de données transférée par seconde. Différent de la latence. |
| Bande passante | Capacité disponible sur le lien ; un faible débit observé ne prouve pas que le lien est saturé. |
| Congestion | Situation où les données à transmettre dépassent les ressources disponibles, provoquant attente ou pertes. |
| MTU | Taille maximale d'un paquet IP sans fragmentation sur une interface donnée. La limite 1 200 du jeu est une limite applicative prudente, pas la MTU universelle d'Internet. |
| Fragmentation | Découper un paquet IP en fragments. Le jeu ne possède pas de mécanisme applicatif de fragmentation/réassemblage. |
| DNS | Résolution d'un nom en adresse. `UdpTransport::resolve()` peut résoudre un nom vers IPv4. |
| NAT | Traduction d'adresses et de ports, souvent par une box. Aucune traversée automatique n'est implémentée. |
| Pare-feu | Filtre réseau. Il faut autoriser le bon port **UDP**, pas seulement TCP. |
| Autorité serveur | Le serveur calcule la vérité du jeu ; le client soumet ses commandes et affiche les résultats. |
| Simulation distribuée | Plusieurs participants ont des représentations du monde qui ne sont pas instantanément identiques à cause du délai réseau. |
| netem | Outil Linux permettant d'ajouter délai, pertes, doublons, réordonnancement et limitation de débit. |

## 3. Les éléments C++ à reconnaître

| Syntaxe / type | Signification dans ce code |
|---|---|
| `namespace engine::network` | Espace de noms regroupant les éléments du protocole et des sessions. `engine::net` regroupe les primitives de transport. |
| `Class::method()` | Méthode appartenant à une classe. `GameServer::poll()` traite les sessions ; `UdpTransport::poll()` récupère seulement les datagrammes. |
| `const auto settings = ...` | Type déduit par le compilateur, objet ensuite non modifiable par cette variable. |
| `const Packet &packet` | Référence à un paquet existant, sans copie et sans modification par cette référence. |
| `GameRoom &room` | Référence vers la salle réelle, modifiable par la fonction. |
| `std::optional<T>` | Valeur de type `T` ou absence de valeur. Un slot égal à 0 est une valeur présente, pas une absence. |
| `std::nullopt` | Absence de valeur dans un `optional`. |
| `*client`, `client->...` | Accéder à la valeur contenue ou à ses membres ; exige qu'elle existe. |
| `&&` | ET logique avec court-circuit. Si le premier test est faux, le second n'est pas exécuté. |
| `std::uint8_t/16_t/32_t` | Entiers non signés de taille connue : 1, 2 et 4 octets. |
| `std::int8_t/16_t` | Entiers signés : de −128 à 127, et de −32 768 à 32 767. |
| `std::size_t` | Type utilisé pour tailles et indices en mémoire ; sa taille ne doit pas être supposée identique sur toutes les machines. |
| `std::vector<T>` | Tableau dynamique : clients, datagrammes, entités, octets. |
| `std::map<K,V>` | Conteneur ordonné par clé : salles, paquets en attente et historiques. |
| `std::unordered_set<T>` | Ensemble pour tester l'appartenance et détecter les doublons, sans ordre imposé. |
| `std::deque<T>` | File utilisée pour retenir l'ordre d'expiration des identifiants fiables vus. |
| `std::variant<...>` | Objet contenant exactement une des catégories de messages autorisées. |
| `std::span<const uint8_t>` | Vue sur des octets existants ; ne les possède pas et ne les copie pas. |
| `std::unique_ptr<T>` | Propriété exclusive d'un objet ; destruction automatique avec son propriétaire. |
| `std::shared_ptr<T>` | Propriété partagée : conserve le tampon d'envoi jusqu'à la fin du callback asynchrone. |
| `std::make_unique/make_shared` | Créer l'objet géré par le pointeur intelligent. |
| `std::move(x)` | Autorise le transfert des ressources de `x` ; ne signifie pas à lui seul qu'une opération réseau a lieu. |
| `emplace(...)` | Construire directement un objet dans son emplacement, par exemple une session dans un `optional`. |
| Liste après `:` du constructeur | Initialisation des membres avant le corps. L'ordre réel suit leur déclaration dans la classe. |
| `static constexpr` | Constante associée à la classe et connue à la compilation. |
| `enum class` | Énumération typée, par exemple `MessageType` et `Receiver`. |
| `template<class Message>` | Code générique utilisé avec différentes classes de messages. |
| `virtual ... = 0` | Méthode abstraite que les implémentations doivent fournir. |
| `steady_clock` | Horloge adaptée aux durées, insensible aux corrections de l'heure civile. |
| `std::thread` | Thread d'exécution, utilisé pour Asio. |
| `std::mutex` + `lock_guard` | Exclusion mutuelle ; déverrouillage automatique à la sortie du bloc. |
| `std::atomic_size_t` | Compteur accessible depuis plusieurs threads sans simple accès concurrent non protégé. |
| `std::bind_front` | Préremplit des arguments d'une fonction : souvent `this`, puis les ressources nécessaires au callback. |
| `throw` / `catch` | Propager puis traiter une erreur. Les paquets invalides peuvent être rejetés sans arrêter le serveur. |
| `volatile sig_atomic_t` | Type adapté au petit drapeau modifié par le gestionnaire de signal. `volatile` n'est pas un remplacement général des atomiques ou mutex. |
| `\|`, `&`, `~`, `<<`, `>>` | Opérations bit à bit : réunir des flags, tester des bits, inverser un masque et décaler les bits. |

## 4. Carte des fichiers

Les fichiers `.hpp` déclarent les types et interfaces ; les `.cpp` les implémentent ; les `.tpp` contiennent les définitions de templates incluses par les en-têtes.

| Fichier / famille | Rôle |
|---|---|
| [game/server/src/main.cpp](../../game/server/src/main.cpp) | Assembler configuration, Python, fabrique et serveur, puis lancer la boucle. |
| [game/client/src/main.cpp](../../game/client/src/main.cpp) | Lire les options et construire `OnlineClient`. |
| [NetworkSettings.cpp](../../engine/network/src/NetworkSettings.cpp) | Lire et contrôler les paramètres utiles au réseau. |
| [ServerRunner.cpp](../../engine/network/src/ServerRunner.cpp) | Boucle, horloge, ticks fixes, signaux et arrêt. |
| [GameServer.cpp](../../engine/network/src/GameServer.cpp) | Sessions, routage des messages, salles et choix des snapshots. |
| `ClientSession.hpp/.cpp` | État conservé côté serveur pour un endpoint. |
| `GameRoom.hpp/.cpp` | Composition d'une salle et possession de sa simulation. |
| `IGameSession.hpp`, `ISessionFactory.hpp` | Contrats permettant au réseau d'utiliser une simulation sans connaître son contenu concret. |
| `SimulationFactory.hpp/.cpp` | Créer une simulation indépendante pour une salle. |
| [Simulation.cpp](../../engine/simulation/src/Simulation.cpp) | Monde autoritaire, inputs, événements et états à répliquer. |
| [UdpTransport.cpp](../../engine/net/src/UdpTransport.cpp) | Interface publique du transport, adresses et erreurs. |
| [UdpTransportImpl.cpp](../../engine/net/src/UdpTransportImpl.cpp) | Socket Asio, thread, réceptions, envois et files. |
| `UdpTransportImpl.hpp` dans `internal/` | Membres privés dépendants d'Asio. |
| `Endpoint.hpp/.cpp` | Adresse et port. |
| `Datagram.hpp/.cpp` | Endpoint source et octets reçus. |
| `ByteReader.hpp/.cpp`, `ByteWriter.hpp/.cpp` | Lire et écrire les primitives binaires en big-endian. |
| `NetError.hpp` | Type d'erreur réseau dérivé de la hiérarchie d'erreurs du moteur. |
| [Protocol.hpp](../../engine/network/include/Protocol.hpp), `Protocol.cpp` | Numéros des messages, constantes et politique de fiabilité. |
| `Packet.hpp/.cpp/.tpp` | Enveloppe en mémoire avec en-tête logique et `variant` de messages. |
| [Codec.cpp](../../engine/network/src/Codec.cpp), `Codec.tpp` | Encodage, décodage et validation des messages. |
| `*Message.hpp/.cpp` | Pour chaque message : champs, accesseurs, `isValid()`, `write()` et `read()`. Catalogue en section 14. |
| `TextRules.hpp/.cpp` | Longueurs et caractères ASCII imprimables. |
| `ChatMessage.hpp/.cpp` | Message fiable client → serveur : token et texte. |
| `ChatBroadcastMessage.hpp/.cpp` | Message fiable serveur → membres : slot de l'auteur et texte. |
| `RoomInfo.hpp/.cpp` | Résumé d'une salle dans une liste. |
| `EntityState.hpp/.cpp`, `PlayerStatus.hpp/.cpp` | Entité répliquée et état d'un joueur dans un snapshot. |
| `ReliableReceiver.hpp/.cpp` | Fenêtre des ACK et mémoire des identifiants fiables traités. |
| `ReliableSender.hpp/.cpp`, `PendingMessage.hpp/.cpp` | Messages en attente, RTT et retransmissions. |
| `SnapshotHistory.hpp/.cpp` | Côté serveur, correspondance paquets envoyés ↔ snapshots et baseline acquittée. |
| `SnapshotDiff.hpp/.cpp`, `EntityChange.hpp/.cpp` | Construire/appliquer les différences et encoder les changements d'entités. |
| [ClientConnection.cpp](../../engine/network/src/ClientConnection.cpp) | Session réseau du client, ACK, commandes et reconstruction des snapshots. |
| [OnlineClient.cpp](../../engine/client/src/OnlineClient.cpp) | Relier réseau, menus, touches et vue du jeu. |
| [GameView.cpp](../../engine/client/src/GameView.cpp) | Monde visuel, HUD et application des positions reçues. |
| `DebugMetrics.hpp/.cpp` | Affichage des FPS, ping, octets/s, entités et mémoire. |
| `scripts/netem_client.cpp`, `scripts/netem_test.py` | Intégration avec le vrai client C++ sous perturbations Linux. |
| `scripts/protocol_probe.py` | Client Python parlant le protocole, avec pertes simulées à la réception. |
| `scripts/load_test.py` | Générateur de plusieurs parties ; ne remplace pas un test du rendu ou de tous les mécanismes du vrai client. |

## 5. Démarrage dans main.cpp

Le point d'entrée serveur est `int main(int argc, char **argv)`. `argc` indique le nombre d'arguments ; `argv` les contient. `argv[0]` est le nom ou chemin utilisé pour lancer l'exécutable, pas une garantie de chemin absolu canonique.

```cpp
const engine::CommandLine options(argc, argv, usage, false, game);
const std::filesystem::path data = engine::AssetPath::findData(argv[0]);
const auto settings = engine::network::NetworkSettings::fromGame(
    engine::scene::DataNode::fromFile(data / (options.game() + "_game.json")));
engine::scripting::ScriptEngine scripts(std::filesystem::path(argv[0]).parent_path() / "python");
engine::simulation::SimulationFactory factory(data, options.game(), settings, scripts);
engine::network::GameServer server(options.port().value_or(settings.defaultPort()), settings, factory);
engine::network::ServerRunner::run(server, settings.tickRate());
```

Dans cet ordre :

1. `CommandLine` analyse `-p port` et `-g game`. Le serveur n'accepte pas `-h`, réservé au client. `--help` seul affiche l'aide et retourne 0.
2. `AssetPath::findData()` cherche le dossier `data` près du chemin d'exécution, sinon `game/data` depuis le dossier courant.
3. `DataNode::fromFile()` ouvre et parse le JSON via `nlohmann::json`. `data / nom` assemble un chemin.
4. `NetworkSettings::fromGame()` extrait les paramètres réseau, le tick rate et certains nombres de valeurs, puis vérifie leur cohérence de taille.
5. `ScriptEngine` démarre l'interpréteur Python intégré et expose l'API C++ aux scripts via pybind11. Le dossier `python` est le runtime, pas `game/scripts`.
6. `SimulationFactory` conserve les informations nécessaires pour créer ensuite des parties.
7. Construire `GameServer` construit aussi `m_transport(port)` : la socket commence son activité avant `ServerRunner::run()`.
8. `ServerRunner::run()` fait vivre le serveur jusqu'au signal d'arrêt.

Le port fourni en CLI remplace le port du JSON grâce à `optional::value_or()`. `PortOption` impose 1 à 65 535. Le port 0 est autorisé par l'API du transport pour obtenir un port éphémère, mais pas par la CLI.

Une exception `EngineError` ou `std::exception` atteint le `catch` du main, est affichée et entraîne un retour **84**. La destruction des objets locaux libère ensuite les ressources. L'ordre inverse de destruction garde notamment Python vivant pendant que le serveur et ses simulations sont détruits.

## 6. Configuration complète

Source : [game/data/rtype_game.json](../../game/data/rtype_game.json), et non les anciennes notes `doc_perso.md`.

| Champ | Valeur actuelle | Sens |
|---|---:|---|
| `tickRate` à la racine | 60 | Ticks de simulation cibles par seconde. |
| `network.defaultPort` | 4242 | Port UDP du serveur sans `-p`. |
| `maxPlayers` | 4 | Places par salle. |
| `maxEntities` | 130 | Limite d'entités répliquées prévue pour les snapshots. |
| `snapshotEveryTicks` | 3 | Un snapshot tous les trois ticks : cible de 20/s. |
| `inputIntervalMs` | 33 | Intervalle minimal d'émission des inputs : environ 30,3/s si la boucle suit. |
| `inputReleaseDelayMs` | 500 | Remise à zéro des commandes côté serveur après absence d'input frais. |
| `heartbeatIntervalMs` | 1000 | Période des heartbeats clients. |
| `connectRetryIntervalMs` | 1000 | Période de répétition de `CONNECT`. |
| `connectAttempts` | 5 | Maximum d'envois de `CONNECT` sans réponse acceptée. |
| `sessionTimeoutMs` | 5000 | Délai de silence avant perte de session. |
| `maxClients` | **192** | Sessions clientes sur tout le serveur, pas par salle. |
| `maxRooms` | **48** | Salles simultanément présentes. |
| `roomNameMaxLength` | 16 | Octets ASCII maximum pour un nom. |
| `roomCodeMaxLength` | 8 | Octets ASCII maximum pour le code de salle. |
| `maxJoinAttempts` | 5 | Nombre de codes incorrects avant blocage des demandes pour rejoindre pendant cette session. |
| `chatMaxLength` | 64 | Nombre maximal d'octets ASCII d'un message de chat ; un texte envoyé doit être non vide. |
| `chatIntervalMs` | 500 | Délai minimal entre deux messages de chat acceptés pour une même session. |
| `playerNameMaxLength` | 12 | Longueur maximale du pseudo ASCII ; le nom vide est autorisé. |

Les deux limites actuelles correspondent à `48 × 4 = 192` places potentielles, mais les clients au lobby et les réservations non confirmées consomment aussi `maxClients`. Une limite configurée n'est pas une preuve de performance mesurée à cette limite.

`NetworkSettings` retient aussi les nombres suivants, pas tous les libellés :

| Tableau JSON | Nombre | Éléments |
|---|---:|---|
| `worldValues` | 5 | `stage`, `phase`, `progress`, `bossHealth`, `shake`. |
| `playerValues` | 3 | `weapon`, `force`, `charge`. |
| `rules` | 2 | `difficulty` : NORMAL/HARD/EASY ; `friendlyFire` : OFF/ON. |
| `eventSounds` | 3 | `warning`, `powerup`, `bossDeath`. |

Le monde mesure 1 920 × 1 080. Le JSON contient aussi fenêtre, couches, prefabs, systèmes, ressources, scènes et textes d'interface. `NetworkSettings` n'est pas un objet contenant tout ce JSON.

**Validation :** les entiers obligatoires doivent être positifs et respecter leur borne (`MAX_PORT=65535`, plafonds de joueurs/entités/clients/salles à 255, longueurs de noms/codes à 32, `MAX_CHAT_LIMIT=128`, autres compteurs/durées à 1 000 000). Le chat est configuré à 64 malgré sa borne technique de configuration à 128. Les petits champs du protocole imposent d'autres contraintes pratiques. La taille maximale d'un snapshot complet et d'une liste de salles est vérifiée contre 1 200 octets. Les durées sont converties en `chrono::milliseconds`.

## 7. Boucle serveur et temps fixe

Source : [ServerRunner.cpp](../../engine/network/src/ServerRunner.cpp), `Clock` et `FixedTimestep`.

```cpp
while (s_running) {
    server.poll();
    timestep.addTime(clock.restart());
    while (timestep.step())
        server.tick(timestep.stepSeconds());
    std::this_thread::sleep_for(IDLE_DELAY);
}
server.shutdown();
```

`Clock::restart()` fournit le temps écoulé depuis le précédent redémarrage. `FixedTimestep` l'accumule. Chaque fois que l'accumulateur contient au moins `1/60 s`, `step()` consomme cette durée et autorise un tick.

Exemple : après environ 40 ms écoulées, on peut exécuter deux ticks de 16,67 ms et conserver le reste. On ne simule pas un tick de 40 ms. La durée ajoutée par tour est plafonnée à 0,25 s pour éviter d'accumuler sans limite un très long blocage.

**Le `while` interne peut exécuter zéro, un ou plusieurs ticks.** Le `sleep_for(1 ms)` évite une boucle occupée permanente ; il ne définit pas un tick rate de 1 000 Hz et ne constitue pas une promesse de réveil à la microseconde près.

`poll()` est donc appelé selon le rythme de la boucle principale, pas uniquement 60 fois/s. Les snapshots sont produits selon les ticks, pas à chaque `poll()`. Les callbacks Asio tournent indépendamment sur le thread réseau.

`SIGINT` correspond notamment à Ctrl+C et `SIGTERM` à une demande de terminaison. Le handler `stop()` ne fait que mettre `s_running` à 0. Le vrai arrêt est effectué ensuite dans le thread principal. Une terminaison non interceptable ou un crash ne garantit pas l'envoi de `SERVER_SHUTDOWN`.

## 8. GameServer : mémoire et fonctions

### Membres principaux

| Membre | Ce qu'il conserve |
|---|---|
| `m_settings` | Configuration copiée dans le serveur. |
| `m_codec` | Encodeur/décodeur configuré pour ce jeu. |
| `m_transport` | Transport UDP unique utilisé pour tous les clients et salles. |
| `m_factory` | Référence vers la fabrique de simulations ; doit vivre plus longtemps que le serveur. |
| `m_clients` | `vector<optional<ClientSession>>` de taille `maxClients`. Un emplacement vide peut être réutilisé. |
| `m_rooms` | `map<uint16_t, GameRoom>` : identifiant de salle vers salle. |
| `m_random` | Générateur `mt19937`, initialisé avec `random_device`, utilisé pour les tokens. |
| `m_nextRoomId` | Prochain candidat pour un ID de salle non nul et libre. |
| `m_rateWindow`, `m_connectRequests` | Début de fenêtre et compteur global de demandes de connexion. |
| `m_lastRoomUpdate` | Dernière émission périodique des compositions de salles. |

### Toutes les responsabilités de ses fonctions

| Fonction | Action |
|---|---|
| Constructeur | Initialiser les membres, ouvrir le transport, afficher le port et éventuellement l'IPv4 locale. |
| `poll()` | Expirer les sessions, récupérer/traiter les datagrammes, renvoyer les messages fiables expirés. |
| `tick(dt)` | Avancer les salles en cours, puis envoyer périodiquement les compositions de salles. |
| `shutdown()` | Envoyer une notification d'arrêt aux sessions confirmées. |
| `receive()` | Décoder, traiter `CONNECT` séparément, vérifier endpoint/token, traiter ACK et doublons, router le message. |
| `handleConnect()` | Quota, version, emplacement, réservation de session et réponse `ACCEPT`. |
| `handleSessionPacket()` | Contrôler les séquences concernées, rafraîchir `lastSeen`, confirmer et répartir les types. |
| `handleDisconnect()` | Éviter qu'un ancien `DISCONNECT` ne retire la session après des inputs/heartbeats plus récents. |
| `acceptSequence()` | Rejeter inputs et heartbeats non plus récents, avec un dernier numéro séparé par catégorie. |
| `handleInput()` | Marquer l'input frais, retrouver salle/slot et transmettre le masque à la simulation si la partie tourne. |
| `handleCreateRoom()` | Retourner la salle actuelle si déjà membre, contrôler le nombre de salles, créer la simulation et placer le propriétaire. |
| `handleJoinRoom()` | Vérifier blocage, existence, place et code, puis placer le client. |
| `handleStartGame()` | Vérifier le propriétaire, activer la salle, ajouter ses joueurs et diffuser `GAME_STARTED`. |
| `handleKick()` | Autoriser le propriétaire à exclure un autre slot ; lui envoyer `KICKED`, puis le retirer de la salle. |
| `handleDeleteRoom()` | Vérifier le propriétaire, notifier la fermeture et supprimer la salle. |
| `handleChat()` | Vérifier l'appartenance à une salle, le délai anti-spam et le slot ; diffuser le texte à tous les membres, émetteur compris. |
| `confirm()` | Marquer la session confirmée et journaliser la connexion ; ne crée pas directement un vaisseau. |
| `remove()` | Faire quitter la salle puis supprimer la session réseau. |
| `tickRoom()` | Relâcher les commandes anciennes, simuler, diffuser les événements, puis produire/envoyer les snapshots au bon tick. |
| `seatClient()` | Enregistrer client ↔ salle, réinitialiser l'historique de snapshots, ajouter le joueur si la partie tourne et notifier. |
| `leaveRoom()` | Retirer le joueur ; si propriétaire, fermer la salle ; mettre à jour les autres membres. |
| `closeRoom()` | Détacher les membres et détruire la salle/simulation. |
| `roomOf()` | Retrouver la salle d'un client, sinon `nullptr`. |
| `ownedRoomSlot()` | Retrouver le slot et vérifier son rôle de propriétaire ; refus explicite si un membre non propriétaire agit. |
| `newRoomId()` | Choisir un ID non nul et non occupé, avec bouclage du compteur 16 bits. |
| `sendRoomList()` | Construire un résumé des salles existantes pour un client demandeur. |
| `sendRoomJoined()` | Envoyer ID, nom et slot de la salle au client concerné. |
| `sendRoomUpdate()` | Construire les noms depuis les sessions, puis diffuser propriétaire, occupation et nom de chaque slot. |
| `broadcastRoom()` | Répéter `sendTo()` pour chaque membre ; ce sont des envois UDP individuels, pas un broadcast IP. |
| `sendTo()` | Affecter séquence/ACK/ID fiable pour une session, suivre les fiables, encoder et transmettre. |
| `sendSnapshot()` | Choisir complet/delta selon la baseline acquittée, enregistrer le snapshot envoyé. |
| `send()` | Envoyer directement à un endpoint, utilisé notamment pour un refus sans session établie. |
| `reject()` | Construire un `REJECT` et le transmettre à l'endpoint. |
| `findClient()` | Rechercher une session dont l'endpoint correspond à la source. |
| `findFreeClient()` | Rechercher un emplacement vide dans `m_clients`. |
| `newToken()` | Générer un token non nul distinct de ceux des sessions actuellement actives. |
| `tokenOf()` | Extraire le token selon le type de message, sinon `nullopt`. Ne compare pas lui-même le token. |

À chaque `receive()` d'une session connue, les ACK sont traités avant l'action du message. Un doublon fiable peut donc rafraîchir les ACK et `lastSeen` sans réexécuter l'action.

## 9. Connexion et contrôle d'accès

### Connexion pas à pas

```text
Client                               Serveur
CONNECT(nom), sans token ──────────→ Vérifier quota et version
                                    Réserver un emplacement
                                    Générer un token
                     ←───────────── ACCEPT(token, indice client)
INPUT / HEARTBEAT / action + token → Vérifier endpoint et token
                                    Confirmer la session
```

Le client répète `CONNECT` toutes les secondes, jusqu'à cinq envois, si aucune réponse acceptée n'arrive. Le serveur ne retransmet pas spontanément `ACCEPT` : il répond aux nouvelles demandes. Un `CONNECT` depuis le même endpoint retrouve la réservation existante et réutilise son token.

En version 9, CONNECT contient une chaîne de nom, même si elle est vide : le payload minimal est **un octet de longueur à zéro**. Le nom valide contient de 0 à 12 caractères ASCII imprimables. `ClientConnection` conserve `m_name` et l'envoie à chaque tentative. Le serveur mémorise un nom non vide dans `ClientSession::m_name`, avec `name()` et `setName()` ; une chaîne vide n'efface pas un nom déjà mémorisé sur une session connue. Le nom est ensuite propagé par ROOM_UPDATE.

Un `ACCEPT` signifie que le client a une session serveur. **Il ne signifie pas qu'il possède déjà une salle ou un vaisseau.** Son `playerId` est ici l'indice global dans `m_clients`. La place dans la salle arrive plus tard avec `ROOM_JOINED.slot`.

La confirmation est faite au premier message de session accepté par `handleSessionPacket()`, pas uniquement au premier heartbeat. Le traitement d'un `DISCONNECT` passe avant cette confirmation. Une réservation non confirmée compte tout de même dans la capacité et expire si elle n'est pas entretenue.

### Les refus et les demandes ignorées

| Situation | Réponse |
|---|---|
| Plus de 32 demandes dans la fenêtre globale d'une seconde | Ignorer sans réponse ; le quota inclut toutes les sources et les demandes qui échoueront ensuite. |
| Version différente de 9, si le CONNECT a pu être décodé | `REJECT_VERSION_MISMATCH` : raison 2. |
| Nouvel endpoint et aucun emplacement libre | `REJECT_SERVER_FULL` : raison 1. |
| Endpoint connu, dernier `ACCEPT` envoyé depuis moins de 200 ms | Ignorer cette répétition sans supprimer la session. |
| Paquet invalide ou inconnu | Généralement abandon silencieux par le décodeur. |
| Endpoint inconnu sur un message nécessitant une session | Abandon silencieux. |
| Token absent ou différent du token enregistré | Abandon silencieux. |

Le quota utilise une fenêtre remise à zéro lorsqu'une demande arrive au moins une seconde après son début. Ce n'est pas une fenêtre glissante exacte : des rafales sont possibles aux frontières.

**Compatibilité :** l'exception de version pour CONNECT ne garantit pas que son ancien payload soit lisible. Un ancien CONNECT version 8 sans aucun payload ne fournit même pas l'octet de longueur attendu en version 9 : le codec peut l'abandonner avant `handleConnect()`, donc sans réponse de refus. Il faut des exécutables compatibles avec la même version et le même format.

### Vérification endpoint + token

```cpp
const auto client = findClient(datagram.sender());
if (!client || tokenOf(*packet) != m_clients[*client]->token())
    return;
```

Même bon token depuis un autre endpoint : rejet. Même endpoint avec mauvais token : rejet. Cela réduit les injections triviales, sans fournir de chiffrement, d'identité personnelle ni d'authentification cryptographique.

`tokenOf()` reconnaît les **onze** types post-connexion client → serveur contenant un token : Input, Heartbeat, Disconnect, ListRooms, CreateRoom, JoinRoom, LeaveRoom, StartGame, KickPlayer, DeleteRoom et Chat. CONNECT contient un nom, mais toujours aucun token.

## 10. Salles et simulations indépendantes

`GameRoom` possède ID, nom, code, état `playing`, slots et un `unique_ptr<IGameSession>`. Ses slots sont des `optional<size_t>` contenant des indices de clients du serveur. Le propriétaire est fixé à **`OWNER_SLOT = 0`**.

### Les identifiants à ne pas mélanger

| Identifiant | Exemple | Portée |
|---|---|---|
| Endpoint | `192.168.1.20:53012` | Adresse réseau d'une session. |
| Indice client | 12 | Emplacement dans `m_clients` du serveur. |
| Token | `0xA5CACAA6` | Contrôle de la session cliente. |
| ID de salle | 7 | Clé dans `m_rooms`. |
| Slot | 1 | Place dans la salle 7, indépendante de l'indice client 12. |
| ID d'entité | 42 | Identité répliquée dans la simulation de cette salle. |
| Numéro de paquet | 300 | Ordre d'émission sur une relation client/serveur. |
| ID fiable | 9 | Identité d'un événement conservée lors de ses retransmissions. |
| Tick | 600 | Étape de simulation de la partie, soit environ 10 s à 60 Hz. |

### Créer, rejoindre, démarrer, quitter

`CREATE_ROOM` transporte le nom, un code optionnel et les choix de règles. Le serveur crée une simulation par `m_factory.createSession(request.rules())` et installe le créateur au slot 0. Un client ne peut appartenir qu'à une salle à la fois.

Pour rejoindre, le serveur vérifie, dans cet ordre :

1. Si le client est déjà dans une salle : il reçoit de nouveau les informations de cette salle.
2. Si son compteur de mauvais codes atteint 5 : refus `ROOM_TOO_MANY_ATTEMPTS`.
3. Si la salle existe : sinon `ROOM_NOT_FOUND`.
4. S'il reste une place : sinon `ROOM_FULL`.
5. Si le code convient : sinon `ROOM_WRONG_CODE` et incrément du compteur.

Le compteur n'augmente que sur un code incorrect. Il n'est pas remis à zéro au succès ou au changement de salle. Une nouvelle session réseau est un nouveau contexte de compteur.

`seatClient()` inscrit l'association dans les deux sens. Si la partie tourne déjà, le joueur est ajouté à la simulation et reçoit aussi `GAME_STARTED`. **Rejoindre une partie en cours est donc prévu.** Sinon, il attend le démarrage.

Seul le propriétaire démarre, exclut ou supprime. Il ne peut pas s'exclure lui-même par `KICK_PLAYER`. Si le propriétaire quitte, la salle ferme ; il n'y a pas de transfert automatique de propriété. Exclure quelqu'un le retire de la salle, pas nécessairement de sa session serveur et du lobby.

`ROOM_UPDATE` est envoyé aux membres après les changements et chaque seconde. Il transporte maintenant l'occupation **et le nom de chaque slot**, lus depuis les sessions. La liste des salles est obtenue par requête ; `OnlineClient` la redemande toutes les deux secondes au lobby. Elle n'est pas poussée spontanément toutes les secondes à tous les clients.

### Chat et noms dans une salle

```text
Champ playerName → ClientConnection::m_name → CONNECT(name)
  → ClientSession::m_name → ROOM_UPDATE(occupation, noms)
  → OnlineClient::m_names → noms des membres et des auteurs du chat

Champ chatInput → action send_chat → ClientConnection::sendChat(text)
  → CHAT(token, text) → GameServer::handleChat()
  → ChatBroadcast(slot, text) à chaque membre de la salle
  → OnlineClient::showChat() → UiStore["chat"] → ListView
```

Le serveur accepte le chat d'un membre qu'il soit propriétaire ou non. Il n'exige pas que la partie ait commencé : `handleChat()` vérifie la salle, mais pas `room.playing()`. Ce n'est ni un chat global du serveur ni un chat entre les clients du lobby sans salle.

Le texte contient entre **1 et 64 octets ASCII imprimables**, validés dans `ChatMessage::isValid()`. Le serveur ne reprend pas un slot d'auteur fourni par le client : il le retrouve dans la salle, puis construit `ChatBroadcastMessage(slot, text)`. Le sender fait partie des destinataires, ce qui permet l'affichage de sa propre ligne après retour du serveur.

`ClientSession` possède `m_lastChat`, `lastChat()` et `markChat(now)`. Si le dernier message accepté remonte à moins de **500 ms**, la nouvelle demande est ignorée. Le timestamp est mis à jour seulement après les contrôles, avant la diffusion. La limite est **par session**, pas par nom, et n'est pas remise à zéro par un simple changement de salle.

**Fiabilité et anti-spam sont deux décisions différentes.** CHAT et ChatBroadcast utilisent la fiabilité sélective. Toutefois, les ACK sont traités avant `handleChat()` : une demande reçue trop tôt peut être acquittée puis volontairement ignorée. Sa retransmission dédupliquée ne la fera pas apparaître plus tard. « Fiable » signifie ici récupération des pertes de transport dans les limites du mécanisme, pas acceptation inconditionnelle de chaque texte.

Le pseudo n'est pas répété dans chaque diffusion : elle porte seulement le slot, et le client consulte les noms reçus dans ROOM_UPDATE. Un nom absent/vide est affiché sous la forme `P1`, `P2`, etc. Deux joueurs peuvent avoir le même pseudo. Ni réservation de nom, ni authentification personnelle, ni historique persistant de chat ne sont fournis.

### Multi-instance ne signifie pas un thread par partie

Un processus serveur, une socket de transport, plusieurs salles et simulations. `tick()` parcourt les salles et exécute leurs updates **successivement dans le thread principal**. Les mondes sont distincts, mais leurs calculs ne tournent pas simultanément sur plusieurs cœurs dans cette implémentation.

Le moteur utilise des interfaces : `ISessionFactory::createSession(rules)` renvoie un `IGameSession`, dont le contrat est `addPlayer`, `removePlayer`, `setInput`, `update`, `tick`, `snapshot`, `takeEvents`. L'implémentation concrète est `Simulation`. Cela permet de réutiliser le réseau sans coder les ennemis de R-Type dans `GameServer`.

## 11. Transport UDP et Asio

### UdpTransport : ce que tu lis actuellement

`UdpTransport` ne connaît ni token, ni input, ni salle. Il connaît des endpoints et des octets. Son interface cache Asio dans une classe privée `Impl` : c'est le principe **Pimpl**, pour réduire les dépendances exposées par l'en-tête public.

| Fonction | Explication exacte |
|---|---|
| `UdpTransport(port)` | Construit `Impl(port)`. Traduit une exception système Asio de construction en `NetError` contextualisée avec le port. |
| Destructeur | Détruit `m_impl`, ce qui déclenche l'arrêt du thread et de la socket. |
| `numericEndpoint(address, port)` | Vérifie une IPv4 numérique et construit l'endpoint ; ne fait pas de DNS. |
| `resolve(host, port)` | Essaye l'IPv4 numérique, sinon utilise un resolver UDP IPv4 et prend la première adresse obtenue. Cette résolution est synchrone. |
| `primaryLocalAddress()` | Ouvre une socket temporaire, la connecte à une destination de référence, lit l'adresse locale choisie par le routage, retourne `nullopt` si impossible ou loopback. |
| `send(peer, bytes)` | Vérifie l'erreur éventuelle du worker, refuse un tampon > 1 200 octets puis délègue l'envoi. |
| `poll()` | Vérifie l'erreur du worker et récupère les datagrammes déjà mis en file, sans attendre une nouvelle arrivée. |
| `checkHealth()` | Propage une exception stockée par le thread I/O. Ne prouve pas que le serveur distant est joignable. |

`primaryLocalAddress()` utilise `8.8.8.8:53` comme référence de route. **Ce n'est pas une requête DNS applicative** : le code n'envoie pas de contenu à cette destination. `connect()` sur cette socket UDP choisit un correspondant et permet d'obtenir l'interface locale ; ce n'est pas un handshake TCP. L'adresse ainsi obtenue sert au message d'aide affiché, pas à la découverte des salles.

Le client peut utiliser un nom résolu par `resolve()`, même si une ancienne documentation indique « numeric addresses only ». La famille reste IPv4. La logique de saisie `host:port` du client n'est pas un parseur d'adresses IPv6.

### Impl : où la socket est réellement ouverte

```cpp
UdpTransport::Impl::Impl(std::uint16_t port)
    : m_socket(m_io, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)),
      m_buffer(), m_pending(0)
{
    receive();
    m_worker = std::thread(&Impl::runIo, this);
}
```

L'endpoint IPv4 local générique et le port conduisent Asio à ouvrir et lier la socket. `receive()` programme une réception. `m_worker` démarre `runIo()`, qui appelle `m_io.run()` pour exécuter les callbacks.

### Membres privés du transport

| Membre | Rôle |
|---|---|
| `m_io` | Contexte d'exécution Asio : opérations postées et callbacks I/O. |
| `m_socket` | Socket UDP réelle. |
| `m_buffer` | Tampon réutilisé pour la réception, de **1 201 octets**, soit limite + 1, pour ne pas accepter simplement un paquet dépassant de peu la limite. |
| `m_sender` | Endpoint renseigné par Asio pour le datagramme reçu. |
| `m_mutex` | Protège la file de réception et l'exception partagée. |
| `m_incoming` | Datagrams déjà reçus, en attente de collecte par le thread principal. |
| `m_failure` | Exception du worker, conservée avec `exception_ptr`. |
| `m_pending` | Compteur atomique des envois en attente. |
| `m_worker` | Thread qui exécute Asio. |

### Réception, étape par étape

1. `receive()` appelle `async_receive_from(buffer, sender, callback)` et rend la main.
2. À l'arrivée, Asio appelle `onReceive(error, size)` dans le worker.
3. Si aucune erreur et taille ≤ 1 200 : `store(size)`.
4. `store()` verrouille le mutex et copie source + octets vers un nouveau `Datagram`.
5. Si la file contient déjà 256 datagrammes, le nouveau est abandonné.
6. Sauf annulation, `onReceive()` programme la réception suivante.
7. Dans le thread principal, `takeIncoming()` échange `m_incoming` avec un vector vide sous verrou, puis retourne le lot collecté.

La copie dans `store()` est nécessaire : `m_buffer` sera réutilisé pour le prochain datagramme. Le jeu ne doit pas conserver une vue vers ce tampon mutable.

### Envoi, étape par étape

1. `Impl::send()` transforme l'endpoint du projet en endpoint Asio.
2. `m_pending.fetch_add(1)` réserve une place ; à 256 envois déjà en attente, la réservation est annulée et le paquet abandonné.
3. Un `shared_ptr<vector<uint8_t>>` possède les octets à envoyer.
4. `asio::post()` programme `startSend()` sur le contexte du worker.
5. `startSend()` appelle `async_send_to()` et conserve le tampon dans le callback.
6. `onSent()` décrémente le nombre d'envois en attente ; le tampon est libéré quand il n'est plus référencé.

**Retourner de `send()` ne veut pas dire « reçu par le client ».** Cela ne garantit même pas une livraison réseau : saturation de file, erreur système ou perte peuvent suivre. C'est la couche fiable qui détecte l'absence d'ACK pour ses messages.

## 12. Threads, mémoire et arrêt

| Thread | Ce qu'il manipule |
|---|---|
| Principal serveur | Sessions, salles, décodage, ACK, simulation ECS et Python. |
| I/O serveur | Socket, callbacks et tampons réseau. |
| Principal client | Fenêtre, événements, protocole, monde visuel et rendu. |
| I/O client | Socket et callbacks de son transport. |

Dans ce découpage, chaque transport crée un worker. Un client de test contenant deux `ClientConnection` possède donc deux transports/workers dans le même processus ; « deux threads » décrit surtout chaque programme habituel avec un seul transport.

Le mutex ne protège pas tout le jeu. Il protège les quelques données réellement échangées entre les deux contextes. Les sessions et l'ECS restent dans le thread principal. `swap()` limite la durée du verrou : le traitement des paquets se fait après récupération de la file.

`runIo()` attrape les exceptions qui s'échappent de `m_io.run()` ; `fail()` stocke un `NetError`. Le thread principal le retrouvera dans `checkHealth()`. En revanche, les codes d'erreur des callbacks ne sont pas tous propagés : `onSent()` ignore son code d'erreur, et la réception est relancée après les erreurs hors annulation. Il n'y a pas un diagnostic applicatif pour chaque datagramme perdu.

À la destruction, `Impl` attend au maximum **200 ms** que `m_pending` diminue, en vérifiant toutes les millisecondes ; puis `m_io.stop()` et `join()` arrêtent/récupèrent le worker. Cette attente concerne les opérations locales, **pas les ACK distants**, et ne garantit pas la réception du dernier message.

Le Python standard embarqué utilise un verrou global d'interpréteur dans cette configuration. Le code actuel appelle les scripts depuis le thread principal ; il ne prévoit pas un pool de workers exécutant les simulations Python en parallèle. Ajouter des threads de simulation demanderait aussi de revoir la propriété des données et les règles d'accès à Python.

## 13. Protocole, Packet et Codec

### Constantes et en-tête

| Propriété | Valeur actuelle |
|---|---|
| `Protocol::MAGIC` | `0x454E`, octets `45 4E`, lettres ASCII `EN`. |
| `Protocol::VERSION` | 9. |
| `HEADER_SIZE` | 20 octets. |
| `MAX_PACKET_BYTES` | 1 200 octets applicatifs, en-tête compris. |
| Unité | Un message par datagramme UDP. |
| Entiers multi-octets | Big-endian. |

| Offset | Taille | Champ | Sens |
|---:|---:|---|---|
| 0 | 2 | `magic` | Reconnaître le format du moteur ; pas une signature de sécurité. |
| 2 | 1 | `version` | Version de protocole. |
| 3 | 1 | `type` | Catégorie du message ; bit `0x80` pour serveur → client. |
| 4 | 4 | `sequence` | Numéro de ce paquet. |
| 8 | 4 | `ack` | Plus haute séquence reçue du correspondant selon la comparaison modulaire. |
| 12 | 4 | `ackBits` | Réception des 32 séquences précédant `ack`. |
| 16 | 2 | `reliableId` | Identité du message fiable, 0 pour non fiable. |
| 18 | 2 | `payloadSize` | Taille après l'en-tête. |

L'en-tête de 20 octets appartient au protocole du jeu. L'en-tête UDP et l'en-tête IP sont ajoutés par le système ; ils ne sont pas inclus dans `HEADER_SIZE`.

### Packet : représentation en mémoire

`Packet` contient type, version, séquence, ACK, bits d'ACK, ID fiable et `Body`, un `std::variant` de toutes les classes de messages. `holds<T>()` teste le type, `as<T>()` donne accès à sa valeur et lance une erreur si le type ne convient pas. Les constructeurs templates utilisent `Message::TYPE`.

On n'envoie jamais `sizeof(Packet)` octets de sa mémoire : cela contiendrait une représentation de `variant`, de conteneurs et potentiellement de pointeurs, dépendante du compilateur et sans sens chez le correspondant.

### Encodage

`Codec::encode()` valide le message, crée un `ByteWriter` borné à 1 200, écrit l'en-tête, réserve deux octets pour la longueur, écrit le corps selon le type, puis utilise `patchU16(18, taille)` pour corriger la longueur. `release()` rend le vector d'octets.

### Décodage et rejet

`Codec::decode(bytes, Receiver::Server/Client)` renvoie un `optional<Packet>`. Le décodeur contrôle :

1. Taille entre 20 et 1 200.
2. Magic correct.
3. Sens du type compatible avec le destinataire.
4. Taille du payload égale aux octets réellement présents.
5. Version compatible, sauf `CONNECT` et `REJECT`, qui doivent permettre un refus compréhensible.
6. Type connu et champs lisibles intégralement.
7. Aucun octet non consommé à la fin.
8. Validation sémantique par `Message::isValid(settings)`.

Un `NetError` de lecture est capturé et converti en `nullopt` : un datagramme tronqué n'est pas une raison de faire tomber le serveur. Un type invalide finit également rejeté. La validation d'un paquet ne remplace pas la vérification de session réalisée ensuite par `GameServer`.

## 14. Catalogue de tous les messages

Notation : `u8/u16/u32` = entiers non signés de 1/2/4 octets ; `str` = longueur `u8` suivie des octets ASCII, sans zéro terminal. Les tailles ci-dessous concernent le **payload**. Ajouter 20 pour obtenir la taille applicative du datagramme.

`R` signifie que `Protocol::isReliable()` sélectionne le message pour ACK/retransmission à l'émission. `P` signifie périodique ou remplacé par des données plus fraîches. `S` signifie traitement particulier, sans fiabilité générale.

### Client → serveur

| Code | Classe / message | Contenu du payload | Taille | Livraison |
|---|---|---|---:|---|
| `01` | `ConnectMessage` / CONNECT | Nom du joueur `str`, vide autorisé. | `1 + nom` | S : tentatives de connexion. |
| `02` | `InputMessage` / INPUT | `token:u32`, `buttons:u8`. | 5 | P. |
| `03` | `HeartbeatMessage` / HEARTBEAT | `token:u32`, `timestamp:u32`. | 8 | P. |
| `04` | `DisconnectMessage` / DISCONNECT | `token:u32`. | 4 | S : timeout en secours. |
| `05` | `ListRoomsMessage` / LIST_ROOMS | `token:u32`. | 4 | P : requête réitérée au lobby. |
| `06` | `CreateRoomMessage` / CREATE_ROOM | Token, nom `str`, code `str`, nombre de règles `u8`, choix `u8[]`. | `7 + nom + code + règles` | R. |
| `07` | `JoinRoomMessage` / JOIN_ROOM | Token, `roomId:u16`, code `str`. | `7 + code` | R. |
| `08` | `LeaveRoomMessage` / LEAVE_ROOM | Token. | 4 | R. |
| `09` | `StartGameMessage` / START_GAME | Token. | 4 | R. |
| `0A` | `KickPlayerMessage` / KICK_PLAYER | Token, `slot:u8`. | 5 | R. |
| `0B` | `DeleteRoomMessage` / DELETE_ROOM | Token. | 4 | R. |
| `0C` | `ChatMessage` / CHAT | `token:u32`, texte `str` non vide. | `5 + texte` | R, avec contrôle anti-spam métier. |

### Serveur → client

| Code | Classe / message | Contenu du payload | Taille | Livraison |
|---|---|---|---:|---|
| `81` | `AcceptMessage` / ACCEPT | `token:u32`, indice client `u8`. | 5 | S : réponse aux CONNECT répétés. |
| `82` | `RejectMessage` / REJECT | `reason:u8`. | 1 | S. |
| `83` | `SnapshotMessage` / SNAPSHOT | Tick, phase, valeurs monde, joueurs et entités. | Variable, section 17. | P. |
| `84` | `HeartbeatAckMessage` / HEARTBEAT_ACK | Horodatage client `u32` renvoyé. | 4 | P ; c'est une réponse de ping, pas l'ACK général des messages fiables. |
| `85` | `EntityDestroyedMessage` / ENTITY_DESTROYED | `entityId:u16`. | 2 | R. |
| `86` | `PlayerJoinedMessage` / PLAYER_JOINED | `playerId:u8`. | 1 | R, type disponible mais pas émis par le parcours actuel vérifié. |
| `87` | `PlayerKilledMessage` / PLAYER_KILLED | `playerId:u8`. | 1 | R, produit par la simulation lors d'une mort. |
| `88` | `PlayerLeftMessage` / PLAYER_LEFT | `playerId:u8`, `reason:u8`. | 2 | R, type disponible mais pas émis par le parcours actuel vérifié. |
| `89` | `ServerShutdownMessage` / SERVER_SHUTDOWN | Aucun. | 0 | S : timeout en secours. |
| `8A` | `RoomListMessage` / ROOM_LIST | Nombre `u8`, puis résumés `RoomInfo`. | `1 + somme des entrées` | P. |
| `8B` | `RoomJoinedMessage` / ROOM_JOINED | `roomId:u16`, nom `str`, `slot:u8`. | `4 + nom` | R. |
| `8C` | `RoomRejectedMessage` / ROOM_REJECTED | `reason:u8`. | 1 | R. |
| `8D` | `RoomUpdateMessage` / ROOM_UPDATE | `ownerSlot:u8`, nombre `u8`, puis pour chaque slot : occupation `u8` et nom `str`. | `2 + somme(2 + longueur du nom)` | P. |
| `8E` | `GameStartedMessage` / GAME_STARTED | Aucun. | 0 | R. |
| `8F` | `KickedMessage` / KICKED | Aucun. | 0 | R. |
| `90` | `RoomClosedMessage` / ROOM_CLOSED | Aucun. | 0 | R. |
| `91` | `PlaySoundMessage` / PLAY_SOUND | Index du son `u8`. | 1 | R. |
| `92` | `SnapshotDeltaMessage` / SNAPSHOT_DELTA | Tick, baseline et listes des changements. | Variable, section 18. | P. |
| `93` | `ChatBroadcastMessage` / ChatBroadcast | Slot de l'auteur `u8`, texte `str` non vide. | `2 + texte` | R. |

`ChatBroadcast` est le nom de l'énumération C++ pour le type `0x93`. La documentation du protocole et la sonde Python le désignent aussi par **CHAT_MESSAGE** : c'est le même type sur le réseau, pas un 32e message.

Avec les limites actuelles, CONNECT coûte de **21 à 33 octets** en-tête compris, CHAT au plus **89 octets** et ChatBroadcast au plus **86 octets**. ROOM_UPDATE pour quatre slots avec quatre noms de 12 octets coûte au maximum **78 octets** : `20 + 2 + 4 × (1 + 1 + 12)`. Les noms n'augmentent pas le snapshot de gameplay ni les entrées de ROOM_LIST.

ROOM_UPDATE valide le nombre de slots, le nombre de noms, leurs longueurs/caractères et le fait que le propriétaire soit un slot occupé. À la lecture, l'octet d'occupation est converti par `!= 0` : le code n'impose pas strictement que sa représentation brute soit uniquement 0 ou 1, même si l'encodeur produit ces deux valeurs.

**Présence dans le protocole ≠ utilisation complète dans le jeu.** Le code actuel gère surtout l'arrivée et le départ via les messages de salle et les snapshots. `OnlineClient::handleEvent()` exploite notamment sons et destructions ; il n'a pas d'action spécifique pour chaque type d'événement joueur décodé.

### Codes de refus et états

| Groupe | Valeurs |
|---|---|
| Refus serveur | 1 : plein ; 2 : version incompatible. |
| Départ joueur | 1 : départ volontaire ; 2 : timeout. Types définis, même si le message PLAYER_LEFT n'est pas émis ici. |
| Refus salle | 1 : introuvable ; 2 : pleine ; 3 : mauvais code ; 4 : limite de salles ; 5 : nom invalide ; 6 : trop de mauvais codes ; 7 : action non autorisée. |
| État joueur | 0 : slot vide ; 1 : vivant ; 2 : mort. |
| État partie | 0 : attente ; 1 : en cours ; 2 : terminée. |

Un code de refus défini n'est pas nécessairement envoyé dans tous les cas imaginables. Par exemple, un nom de salle mal formé est bloqué par le client normal et/ou rejeté par le codec ; un client qui n'appartient à aucune salle peut voir son action ignorée sans `ROOM_NOT_ALLOWED`.

### RoomInfo, entrée de liste

`id:u16`, nom `str`, nombre de joueurs `u8`, capacité `u8`, flags `u8`. Flag `01` : code requis ; flag `02` : partie en cours. Taille : **6 + longueur du nom**.

Avec des noms de 16 octets : 22 octets par salle. Liste maximale actuelle : `20 + 1 + 48×22 = 1 077 octets`. Avec cette longueur, 53 salles tiennent dans la limite (`1 187 octets`), 54 non (`1 209`). Augmenter le nombre de salles nécessiterait de raccourcir les entrées ou de paginer la liste.

## 15. Sérialisation et packing

### Big-endian

```text
Valeur 0x12345678 → octets 12 34 56 78
```

`ByteWriter::writeBigEndian()` décale la valeur et ne garde que chaque octet utile. `ByteReader::readBigEndian()` reconstruit en répétant :

```cpp
value = (value << 8) | prochainOctet;
```

`<< 8` libère huit bits à droite ; `|` y insère l'octet suivant. `readU8/U16/U32`, `writeU8/U16/U32` utilisent ce mécanisme. Les coordonnées signées utilisent `readI16/writeI16` ; la lecture emploie `bit_cast` pour conserver la représentation des bits.

`readString()` lit une longueur sur un octet puis les caractères. La limite technique de ce préfixe est 255 octets ; le jeu impose 16 pour un nom de salle, 8 pour son code, 12 pour un pseudo et 64 pour un texte de chat. `TextRules` accepte seulement les caractères ASCII de `0x20` à `0x7E` : pas un système de noms Unicode/UTF-8 générique. Les accents, emojis et sauts de ligne ne sont donc pas des textes réseau valides pour ces champs.

`remaining()` sert à vérifier les tailles avant lecture. Un dépassement produit une erreur, pas une lecture hors limites volontaire. `ByteWriter` contrôle la taille totale avant chaque écriture et utilise `patchU16()` pour la longueur finale.

### Boutons sur un octet

| Commande | Masque |
|---|---|
| Haut | `0x01` |
| Bas | `0x02` |
| Gauche | `0x04` |
| Droite | `0x08` |
| Tir | `0x10` |
| Action spéciale | `0x20` |
| Bits réservés | `0x40`, `0x80` ; masque `0xC0`. |

```cpp
buttons = BUTTON_RIGHT | BUTTON_FIRE; // 0x18, soit 00011000
if (buttons & BUTTON_FIRE) { /* le tir est maintenu */ }
```

On envoie l'**état actuel** des boutons, pas seulement des événements « pressé » et « relâché ». Si le paquet de relâchement est perdu, le prochain état peut réparer. Après 500 ms sans nouvel input frais, le serveur force tout à zéro. Une action extrêmement brève entre deux échantillonnages peut toutefois être manquée : ce mécanisme ne garantit pas chaque impulsion individuelle.

### Quantification du monde

Les positions flottantes de la simulation sont arrondies avec `lround` vers des entiers 16 bits. On économise de la place mais on perd la précision sous le pixel et on suppose des coordonnées représentables. La vie est transmise en pourcentage sur un octet. L'angle est quantifié sur 128 positions : environ **2,8125° par pas**. Le bit supérieur du même octet indique le flash d'impact.

Ce sont plusieurs optimisations différentes : sérialisation sans padding, petits types, masques de bits et quantification. Elles ne sont pas équivalentes à une compression générale zlib/LZ4, absente ici.

### Exemple construit d'un INPUT

Pour une séquence 24, ACK 22, aucun bit d'historique posé, token `0xA5CACAA6`, droite + tir :

```text
45 4E 09 02  00 00 00 18  00 00 00 16  00 00 00 00  00 00  00 05
A5 CA CA A6 18
```

20 octets d'en-tête + 4 de token + 1 de boutons = **25 octets**. C'est un exemple de construction du format, pas une nouvelle capture réseau réalisée pour cette fiche.

## 16. Séquences, ACK et retransmissions

### Trois compteurs qui répondent à trois questions

| Champ | Question |
|---|---|
| `sequence` | Quel paquet a été envoyé/reçu ? |
| `reliableId` | Est-ce le même message métier qu'une tentative précédente ? |
| `tick` | De quel instant de la simulation parle cet état ? |

Les séquences client → serveur et serveur → client sont indépendantes. Côté serveur, chaque `ClientSession` possède son propre compteur sortant : il ne faut pas imaginer une numérotation globale partagée par tous les clients.

`Codec::isNewer(candidate, previous)` calcule une différence non signée : plus récent si elle est non nulle et strictement inférieure à `2^31`. Cela gère le bouclage 32 bits tant que les comparaisons portent sur des écarts raisonnables. Un simple `candidate > previous` échouerait au bouclage.

Les compteurs de paquet commencent à 1. L'ACK 0 est utilisé pour « rien reçu ». À très long terme, le compteur 32 bits peut néanmoins repasser par 0 : il ne faut pas transformer cette convention initiale en preuve de fonctionnement illimité sans cas particuliers.

### La fenêtre d'ACK

`ReliableReceiver` retient `m_latest` et `m_history`. Pour :

```text
ack = 43
ackBits = ...00001101
```

on acquitte 43, et :

| Bit | Séquence visée | État |
|---:|---:|---|
| 0 | 42 | Reçue. |
| 1 | 41 | Non indiquée reçue. |
| 2 | 40 | Reçue. |
| 3 | 39 | Reçue. |

Le bit `i` concerne `ack - 1 - i`. Un paquet peut donc porter jusqu'à 33 confirmations. Si un paquet plus récent arrive, la fenêtre est décalée ; un ancien paquet encore dans les 32 positions remplit le trou correspondant. Le code distingue les décalages égaux ou supérieurs à 32, pour ne pas effectuer un décalage binaire invalide.

Les ACK sont **transportés dans les en-têtes des paquets déjà envoyés** : inputs, heartbeats, états, réponses. Il n'y a pas un type de paquet spécial pour chaque ACK général. `HEARTBEAT_ACK` est, lui, un message applicatif d'écho d'horodatage, qui possède aussi son propre en-tête d'ACK.

### Exemple de fiabilité

```text
Message CREATE_ROOM, reliableId=7
  tentative 1 : sequence=100 → perdue
  délai de retransmission écoulé
  tentative 2 : sequence=110, reliableId=7 → reçue et exécutée
  le serveur indique ACK=110 dans une réponse
  le client retire cette tentative des messages en attente
```

Même message, **nouvelle séquence**, **même ID fiable**. Si la tentative originale arrive aussi, l'identifiant fiable permet de ne pas recréer la salle. Le receiver enregistre néanmoins le paquet pour les ACK avant d'ignorer son action.

`ReliableReceiver` conserve les **256 derniers identifiants fiables** dans un ensemble et une deque. L'ensemble teste les doublons ; la deque permet d'oublier les plus anciens. Ce n'est pas une mémoire infinie des événements.

### ReliableSender et PendingMessage

`PendingMessage` conserve `Packet` et l'instant `sentAt`. `ReliableSender` garde une map indexée par séquence :

- `nextReliableId()` choisit un ID 16 bits non nul, avec bouclage.
- `track()` ajoute une tentative fiable à la map.
- `acknowledge()` cherche les séquences couvertes par ACK/bitfield.
- `confirm()` met à jour l'estimation du RTT et retire la tentative confirmée.
- `expired()` extrait et retire les tentatives dont le délai est dépassé ; `GameServer` ou `ClientConnection` les renvoient et les enregistrent avec de nouvelles séquences.
- `pending()` indique le nombre d'attentes.

L'estimation initiale du RTT est 100 ms. À confirmation d'une tentative suivie :

```text
échantillon = maintenant − instant d'envoi de cette tentative
RTT lissé = (7 × ancien RTT + échantillon) / 8
délai de retransmission = max(100 ms, RTT lissé × 1,5)
```

Le premier délai est donc environ 150 ms. Avec un RTT lissé de 200 ms : environ 300 ms. Le plancher de 100 ms évite de retransmettre sur un LAN avant que le correspondant n'ait eu le temps d'envoyer son prochain paquet avec ACK.

Cette estimation provient des **messages fiables suivis**, pas d'une mesure dédiée sur chaque datagramme. Si aucun nouveau fiable n'est acquitté, elle peut rester ancienne. Le délai d'attente de l'ACK chez le correspondant fait partie de l'échantillon.

### Limites de cette fiabilité

- Maximum **64 tentatives fiables en attente par correspondant** ; à saturation, `track()` supprime l'entrée au début de la map. Hors bouclage, elle correspond normalement à la plus ancienne séquence.
- Aucun ordre métier garanti entre messages fiables distincts : un message plus récent peut être exécuté avant un ancien retransmis. Il n'y a pas un tampon d'ordonnancement de flux comme TCP.
- L'historique de déduplication est borné à 256 ; les ID fiables bouclent sur 16 bits. Des retards extrêmes et la réutilisation des ID demanderaient des garanties supplémentaires.
- Un ACK dit « reçu à la couche protocole », pas « action autorisée et réussie ». Un `START_GAME` interdit peut être reçu et acquitté, puis refusé.
- Une retransmission n'assure pas qu'une liaison coupée indéfiniment finira par fonctionner. Les timeouts et les bornes de mémoire restent applicables.
- `DISCONNECT` et `SERVER_SHUTDOWN` ne sont pas suivis comme fiables ; le silence est détecté ensuite par timeout.

Ainsi, dire **« fiabilité sélective au-dessus d'UDP, avec limites bornées »** est juste. Dire « UDP devient un TCP parfaitement fiable et ordonné » serait faux.

## 17. Snapshots complets

Un `SnapshotMessage` fournit un état autonome : tick, phase, valeurs monde, tous les slots de joueurs et une liste d'entités répliquées. Il ne contient pas les textures, le code Python ou l'intégralité de l'ECS.

### Format

```text
tick:u32
gameState:u8
valueCount:u8 + values:u16[valueCount]
playerCount:u8 + PlayerStatus[playerCount]
entityCount:u8 + EntityState[entityCount]
```

Les champs fixes représentent 8 octets. Avec `W` valeurs monde, `P` joueurs, `V` valeurs par joueur et `N` entités :

```text
payload complet = 8 + 2W + P × (10 + 2V) + 8N
datagramme applicatif = 20 + payload
```

Avec W=5, P=4 et V=3 : **102 + 8N octets**, en-tête compris. Avec 130 entités : **1 142 octets**, inférieur à 1 200.

### PlayerStatus : 16 octets dans ce jeu

| Champ | Taille | Signification |
|---|---:|---|
| `state` | 1 | Vide, vivant ou mort. |
| `lives` | 1 | Vies restantes. |
| `health` | 1 | Santé en pourcentage. |
| `score` | 4 | Score non signé. |
| `avatarId` | 2 | ID réseau du vaisseau visible ; zéro si pas d'avatar vivant. |
| Nombre de valeurs | 1 | Trois ici. |
| Valeurs | 6 | Arme, force et charge, chacune sur 16 bits. |

Un joueur logique peut continuer d'exister sans avatar visible entre sa mort et sa réapparition. Le slot n'est donc pas équivalent à une entité graphique.

### EntityState : 8 octets

| Champ | Taille | Signification |
|---|---:|---|
| `id` | 2 | ID non nul, unique dans ce snapshot. |
| `kind` | 1 | Type de prefab visuel défini par le jeu. |
| `x`, `y` | 2 + 2 | Positions entières signées. |
| `visual` | 1 | Bit 7 : flash ; bits 0 à 6 : rotation. |

Les types (`kind`) sont associés aux prefabs du client via le JSON. L'identité ECS locale du serveur n'est pas envoyée comme un pointeur : le composant `Replicated` fournit un ID réseau.

### Contrôles et fraîcheur

Le snapshot complet doit avoir les nombres attendus de valeurs et de slots, des IDs d'entités uniques et non nuls et une phase connue. Un joueur vivant doit référencer un avatar présent dans la liste, avoir des vies et une santé admissible. Un slot vide doit avoir les principaux compteurs à zéro et le bon nombre de valeurs. Le validateur ne vérifie pas pour autant toutes les invariantes possibles de tous les modes de jeu.

Le client n'applique un snapshot que si son tick est plus récent que `m_lastTick`. Les états anciens ou dupliqués ne doivent pas faire reculer le monde. `takeSnapshot()` ne conserve que le dernier état accepté disponible, ce qui évite de rendre inutilement une longue file d'états déjà dépassés.

## 18. Compression delta et historiques

### Pourquoi un delta

Si beaucoup d'entités gardent le même type, la même apparence et changent peu de position, un état complet répète des données. `SnapshotDiff::build(base, current)` n'écrit que les différences.

```text
Baseline tick 30 : entité 42, kind 3, x=100, y=200, visual=0
État tick 33     : entité 42, kind 3, x=110, y=200, visual=0
Delta           : entité 42, champ dx présent, dx=+10
```

La simulation produit toujours un état complet. C'est le serveur réseau qui choisit le delta pour chaque client ; après reconstruction, la vue cliente reçoit encore un état complet.

### Contenu de SnapshotDeltaMessage

```text
tick:u32, baselineTick:u32, gameState:u8
worldCount:u8   + changements {index:u8, valeur:u16}
playerCount:u8  + changements {slot:u8, PlayerStatus complet}
entityCount:u8  + EntityChange[]
removedCount:u8 + identifiants supprimés:u16[]
```

Base fixe du payload : **13 octets**. Une valeur monde modifiée coûte 3 octets ; un joueur modifié coûte `1 + taille de PlayerStatus`, donc 17 ici ; une suppression d'entité coûte 2 octets. Une liste omise signifie « garder la baseline », pas « tout supprimer ».

### Masque EntityChange

| Bit | Champ suivant | Coût |
|---|---|---:|
| `0x01` | `dx` signé | 1 octet. |
| `0x02` | `dy` signé | 1 octet. |
| `0x04` | Position absolue `x:i16,y:i16` | 4 octets. |
| `0x08` | `kind:u8` | 1 octet. |
| `0x10` | `visual:u8` | 1 octet. |

Il faut ajouter l'ID 2 octets et le masque 1 octet. Un déplacement seulement en X dans [−128,127] coûte donc **4 octets**, contre 8 pour une entité complète. Un déplacement sur X et Y coûte 5. Si un déplacement dépasse cette plage sur un axe, le code envoie les deux coordonnées absolues. Le bit position absolue est incompatible avec les bits dx/dy.

Une création porte position, type et apparence : 9 octets avec ID/masque. Une entité inchangée ne coûte rien dans la liste des changements. Un changement sans baseline connue et sans position absolue ne permet pas de reconstruire une entité.

### Baseline spécifique à chaque client

`SnapshotHistory`, conservé dans chaque `ClientSession`, garde jusqu'à **16 snapshots envoyés en attente de reconnaissance**, indexés par séquence de paquet, plus une baseline promue et le tick du dernier complet envoyé.

1. Envoyer un état associe sa séquence au snapshot complet correspondant.
2. Les ACK de ce client permettent de retrouver quels états envoyés ont été reçus au niveau protocole.
3. `promote()` conserve le plus récent de ces états comme baseline.
4. Le prochain delta est calculé relativement à cette baseline, pas arbitrairement au dernier état envoyé.

Deux clients ayant perdu des paquets différents peuvent donc recevoir des deltas différents pour le **même tick de simulation**.

### Quand envoyer un complet

`sendSnapshot()` utilise un delta seulement si la baseline existe, est plus ancienne que le nouvel état mais de moins d'**une seconde de ticks**, n'a pas le tick zéro et si aucune keyframe n'est due. Il envoie un complet :

- au départ, sans baseline reconnue ;
- si la baseline est absente, trop ancienne ou inutilisable ;
- lorsque **5 secondes de ticks** se sont écoulées depuis le dernier complet envoyé.

Ces durées sont exprimées via `tickRate`, pas via un timer réseau indépendant. Le « dernier complet » mémorise un **envoi**, pas une livraison garantie. Une keyframe perdue ne garantit pas un rétablissement dans exactement cinq secondes.

### Reconstruction côté client

Le client garde jusqu'à **32 snapshots acceptés**, dans `m_applied`, indexés par tick. Pour un delta, il recherche exactement `baselineTick`, copie les valeurs/slots de cette baseline, applique les changements, retire les IDs supprimés et résout les entités modifiées. Le résultat passe ensuite dans le filtre des ticks récents.

Si la baseline manque, le delta est ignoré. Appliquer à « peu près le bon état » produirait de mauvaises positions ; le code ne le fait pas. Changer de salle et recevoir les événements de démarrage nettoie les historiques concernés côté client ; entrer/quitter réinitialise l'historique correspondant côté serveur.

### Les limites à savoir expliquer

Un ACK de paquet n'atteste pas explicitement que le delta a été appliqué : le client remplit ses ACK avant son traitement applicatif. Le serveur s'appuie donc sur le fonctionnement normal des historiques et des échanges ; les complets périodiques servent de rattrapage. Ne pas présenter l'ACK comme une preuve cryptographique ou comme une confirmation explicite de la baseline reconstruite.

Un delta n'est pas toujours plus petit qu'un complet. `sendSnapshot()` choisit actuellement selon la fraîcheur de baseline et les keyframes, **sans comparer les deux tailles encodées**. Avec beaucoup de créations, suppressions ou changements, un delta peut grossir. La vérification de configuration borne le snapshot complet et la liste de salles, pas tous les pires cas de delta ; dépasser 1 200 provoque une erreur d'encodage. Un repli vers un complet quand le delta est trop grand serait une amélioration pertinente.

Les historiques ordonnés par clé numérique et les ID réutilisés ne constituent pas non plus une preuve de fonctionnement sans ambiguïté pendant une durée infinie avec des retards arbitraires. Ce sont des structures bornées adaptées à la fenêtre de trafic attendue.

## 19. Simulation et scripts

### Simulation autoritaire

Chaque `Simulation` construit son registre ECS, ses prefabs, ses systèmes, ses valeurs monde, ses règles et son compteur de ticks. **ECS** signifie Entity Component System : des identifiants d'entités, des données sous forme de composants et des systèmes qui les traitent.

Les composants importants pour le réseau sont :

| Composant | Rôle |
|---|---|
| `Transform` | Position réelle dans le monde simulé. |
| `Replicated` | Type répliqué et ID réseau de l'entité. Seules les données sélectionnées sont décrites aux clients. |
| `Player` | Slot et lien vers l'avatar ; sépare joueur logique et vaisseau visible. |
| `PlayerControl` | Direction, tir et action spéciale issus des boutons. |
| `Lives`, `Score`, `Health` | Données dont est construit le `PlayerStatus`. |
| `PlayerValues` | Valeurs configurées par joueur pour le HUD. |
| `Rotation`, `HitFlash` | Sources du champ `visual`. |
| `Destroyed` | État de destruction utilisé par le cycle de vie et le nettoyage. |

`setInput()` retrouve l'avatar du slot et met à jour son contrôle. Il ne reçoit pas directement des coordonnées décidées par le client. `update(dt)` exécute les systèmes, attribue les IDs réseau, collecte les morts et sons, vide les files d'événements internes, nettoie les entités, suit la phase et incrémente le tick.

`snapshot()` construit tous les slots et les entités décrites. `collectEntities()` donne d'abord la priorité aux avatars des joueurs pour qu'ils soient représentés, puis ajoute les autres entités jusqu'à la limite. `canSpawn()` empêche normalement de dépasser le budget de réplication et ne permet le spawn de gameplay que dans la phase appropriée.

`nextNetworkId()` utilise un entier 16 bits, saute zéro et les IDs encore en usage. **Cette identité n'inclut pas de génération réseau.** La réutilisation à long terme peut être ambiguë face à des messages très anciens ; elle n'est pas équivalente à un identifiant mondial permanent.

`collectDeaths()` produit notamment `EntityDestroyedMessage` et `PlayerKilledMessage`. `sendSounds()` transforme les événements de sons en `PlaySoundMessage` via l'index déclaré dans le JSON. `takeEvents()` transfère les événements au serveur et vide leur file.

### Python

`ScriptEngine` embarque Python via pybind11, garde un cache de modules et expose un module `engine` avec les types `World` et `Entity`. `ScriptSystem` appelle les hooks des fichiers de `game/scripts`, notamment `on_spawn`, `on_update`, `on_damage`, `on_death`, `on_contact` lorsqu'ils sont présents.

Exemples : un ennemi modifie sa vitesse, tire vers un joueur, le directeur fait apparaître une vague, un boss change de phase. Les scripts changent le monde du serveur ; le résultat est répliqué aux clients. Ils ne voyagent pas avec chaque snapshot et ne sont pas exécutés sur le client pour recalculer les ennemis.

Le moteur expose des actions comme `set_velocity`, `fire_at_player`, `spawn`, `world.set`, `world.rule`, `world.play_sound`. Les valeurs monde/joueur sont indexées grâce aux tables du JSON. Les erreurs des hooks sont journalisées et le comportement de script concerné peut être désactivé pour l'entité ; une erreur d'initialisation de Python reste une erreur de démarrage.

## 20. Client réseau et affichage

### Trois responsabilités distinctes

| Classe | Responsabilité |
|---|---|
| `ClientConnection` | Protocole, transport, session, fiabilité, délais et reconstruction des snapshots. |
| `OnlineClient` | Événements de menus, choix des salles, boutons, passage d'une scène à l'autre et application du réseau à la vue. |
| `GameView` | Fenêtre, ressources, entités visuelles, HUD, sons et rendu. |

### Cycle de ClientConnection::update()

1. Récupérer les datagrammes et ignorer les sources autres que l'endpoint serveur configuré.
2. Compter les octets reçus depuis cet endpoint, même si le décodage échouera ensuite.
3. Décoder/valider les paquets.
4. Mettre à jour la fenêtre des ACK et confirmer les propres messages fiables en attente.
5. Filtrer les ID fiables déjà vus ; un doublon reconnu peut entretenir `m_lastReceive`.
6. Traiter la réponse de connexion, l'événement, le snapshot complet ou le delta.
7. Si pas encore connecté, effectuer les tentatives de `CONNECT` nécessaires.
8. Sinon, vérifier le timeout de réception.
9. Renvoyer les fiables expirés et envoyer périodiquement inputs/heartbeats.

La socket cliente est construite avec le port par défaut de `UdpTransport`, soit 0 : port local éphémère. **Le port serveur 4242 n'est pas automatiquement le port source du client.**

`connected()` teste `m_token != 0`. L'acceptation mémorise le token et la séquence serveur. Le client n'accepte un `ACCEPT` qu'après avoir tenté une connexion. Une fois connecté, les anciens `ACCEPT/REJECT` sont ignorés comme réponses de connexion.

### Fonctions à reconnaître dans ClientConnection

| Fonction | Rôle |
|---|---|
| Constructeur | Préparer paramètres, endpoint, compteur à 1 et historiques vides ; son membre transport ouvre la socket. |
| `update(buttons)` | Cycle complet décrit ci-dessus. |
| `disconnect()` | Envoyer une fois DISCONNECT si connecté et remettre le token à zéro. |
| `connected()` | Tester si un token a été accepté. |
| `handleConnectionReply()` | Interpréter ACCEPT/REJECT. |
| `retryConnect()` | Espacer les tentatives, compter jusqu'à la limite, lancer une erreur en cas d'épuisement. |
| `handlePacket()` | Orienter les états/événements, filtrer certains paquets anciens et actualiser le ping. |
| `handleSnapshot()` | Refuser les ticks non plus récents, mémoriser puis publier le dernier état. |
| `handleDelta()` | Chercher la baseline et reconstruire un complet. |
| `rememberSnapshot()` | Garder un historique borné à 32 états acceptés. |
| `takeSnapshot()` | Transférer le dernier snapshot disponible et vider cet emplacement. |
| `takeEvents()` | Transférer le lot d'événements et vider sa file. |
| `send()` | Choisir ID fiable, séquence et ACK, enregistrer l'attente, encoder et compter/transmettre les octets. |
| `sendPeriodic()` | Émettre INPUT toutes les 33 ms environ et HEARTBEAT toutes les secondes. |
| `listRooms/createRoom/joinRoom/leaveRoom` | Construire les requêtes correspondantes si connecté. |
| `startGame/kickPlayer/deleteRoom` | Construire les commandes de gestion de salle ; le serveur contrôle les permissions. |
| `sendChat(text)` | Envoyer un CHAT fiable si connecté et texte non vide ; longueur et caractères sont vérifiés par le codec, appartenance et cadence par le serveur. |
| `bytesReceived/bytesSent` | Exposer les compteurs d'octets applicatifs. |
| `pingMs()` | Dernier RTT mesuré par écho heartbeat. |
| `roundTripMs()` | RTT lissé de `ReliableSender`. Ce n'est pas exactement la même mesure que `pingMs()`. |
| `nowMs()` | Convertir l'horloge monotone en millisecondes sur 32 bits pour les horodatages. |

Les heartbeats et les inputs sont envoyés périodiquement tant que la session est connectée, y compris avec un masque de boutons nul dans les menus. Le heartbeat contient une valeur de l'horloge **du client** que le serveur renvoie inchangée : pas besoin que les horloges de deux machines soient synchronisées pour calculer ce RTT.

Pour les événements non fiables, `handlePacket()` utilise une dernière séquence serveur ; pour les snapshots, c'est le tick ; pour les événements fiables, c'est l'ID de déduplication. Il n'existe pas un filtre unique « tous les paquets anciens sont ignorés » : cela empêcherait la récupération d'un événement fiable retransmis.

### OnlineClient et les scènes

`run()` collecte les événements fenêtre, appelle `updateNetwork()`, rend la vue, publie les métriques et traite les actions d'interface. `buttons()` renvoie les commandes seulement en scène de jeu ; ailleurs, zéro.

`connect()` lit l'adresse saisie et `field("playerName")`, sépare le port éventuel, appelle `UdpTransport::resolve()` et construit `ClientConnection(endpoint, settings, name)`. Le troisième argument de nom est désormais **obligatoire** dans la signature C++, même si la chaîne peut être vide. `handleEvent()` utilise notamment :

- `ROOM_LIST` pour afficher les salles ;
- `ROOM_JOINED` pour mémoriser le slot local et ouvrir la salle ;
- `ROOM_UPDATE` pour mémoriser `m_names`, puis afficher les membres, leurs noms et les droits visibles ;
- `ChatBroadcastMessage` pour ajouter une ligne à l'historique visuel ;
- `GAME_STARTED` pour passer au jeu ;
- `KICKED/ROOM_CLOSED` pour retourner au lobby ;
- `PLAY_SOUND/ENTITY_DESTROYED` pour le son et les effets/destructions en jeu ;
- `ROOM_REJECTED` pour afficher un message compréhensible.

Les permissions visibles dans l'interface ne remplacent jamais celles du serveur. Un client modifié peut essayer d'envoyer START_GAME ; le serveur doit encore vérifier son statut.

`OnlineClient` surveille aussi un silence de salle de **4 secondes** via les messages de composition, distinct du timeout réseau de 5 secondes. En cas d'erreur réseau attrapée, il déconnecte l'objet et revient au menu avec un message. Il ne faut donc pas reprendre l'ancienne affirmation « chaque erreur réseau ferme obligatoirement toute la fenêtre ».

### Interface du chat et pseudos

Le menu comporte un `TextField` nommé `playerName`, limité à 12 caractères. Dans le JSON actuel, le panneau de saisie du chat est installé dans la scène **`room`** : il ne faut pas affirmer qu'un panneau est déjà présent dans toutes les scènes, notamment en jeu. Le protocole, lui, permet au serveur de relayer le chat tant que le client est membre de la salle, même après démarrage.

| Élément | Rôle |
|---|---|
| Champ `chatInput` | Saisie de 64 caractères maximum dans l'interface de salle. |
| `submit: "send_chat"` | Action associée à la validation du champ ; `FocusNavigationSystem` la publie quand le champ courant est validé. |
| Bouton `send_chat` | Autre moyen de déclencher la même action. |
| `OnlineClient::sendChat()` | Ignore un champ vide, appelle le réseau, puis vide le champ. Il n'attend pas une confirmation d'acceptation métier. |
| `OnlineClient::showChat()` | Retrouve l'auteur via `m_names[slot]` ou le fallback `P#`, ajoute `author/text/mine` à la deque. |
| `CHAT_HISTORY = 5` | Cinq dernières lignes conservées par la vue ; les plus anciennes sont retirées. Ce n'est pas une base de messages serveur. |
| `UiStore` et `ListView(source="chat")` | Fournissent puis affichent les lignes, avec un indicateur distinguant celles de l'utilisateur. |
| `playerName(update, slot)` | Résout le nom d'un membre à afficher, avec fallback si vide/absent. |
| `clearChat()` | Vide l'historique, la liste UI et le champ ; appelé lors d'une entrée nouvelle en salle et d'un départ volontaire. |

L'auteur et le texte apparaissent après réception de la diffusion serveur. Un envoi trop rapide peut donc faire disparaître la saisie sans produire de ligne : le serveur l'a volontairement ignoré pour anti-spam. Le client ne présente pas de statut détaillé « refusé par limite de fréquence ».

Comme ROOM_UPDATE est périodique non fiable et ChatBroadcast fiable, une ligne peut arriver avant la mise à jour de noms correspondante. Le fallback permet un affichage, mais l'auteur résolu n'est pas une identité permanente et les lignes déjà construites ne sont pas renommées rétroactivement. Les événements fiables ne sont pas garantis dans l'ordre métier ; le chat ne possède ni horodatage métier ni numéro d'ordre de conversation supplémentaire.

### Application d'un état à l'écran

`applySnapshot()` effectue une synchronisation :

1. `beginSync()` vide l'ensemble des IDs vus dans ce lot.
2. Pour chaque `EntityState`, `syncEntity()` retrouve le prefab par `kind`, crée si nécessaire l'entité visuelle et **affecte directement** la position reçue.
3. `endSync()` retire les objets réseau qui ne figurent plus dans l'état.
4. La phase, les vies, scores, santé et valeurs sont reportés dans le HUD et les joueurs visuels.

`m_networkEntities` associe les IDs réseau aux entités ECS locales de la vue. Les effets purement visuels peuvent vivre localement sans être chacun une entité du snapshot.

Il n'y a pas actuellement de buffer d'interpolation des positions ni de prédiction du mouvement du joueur. Une animation locale d'inclinaison de sprite ou une animation de particules n'est pas une prédiction de la simulation réseau.

## 21. Lag et techniques de synchronisation

### Ce que notre architecture fait déjà

Elle sépare simulation et fréquence des envois, filtre les vieux inputs/états, reconstruit les deltas et répare une partie des pertes avec de nouveaux états. Cela protège la cohérence des données acceptées, mais ne supprime pas le délai physique.

Pour voir à l'écran un mouvement validé par le serveur, il faut notamment attendre : prochain envoi d'input, trajet aller, prochain tick, prochain snapshot, trajet retour et prochaine image. Ces attentes dépendent de leur phase et du trafic. **Un ping stable ne garantit pas un contrôle instantané.**

### Techniques à connaître pour la track

| Technique | Principe | Présente ici ? |
|---|---|---|
| Interpolation des entités | Afficher avec un petit retard volontaire entre deux états connus, pour lisser les mouvements et absorber le jitter. Exemple : `p = (1−alpha)×p0 + alpha×p1`. | Non. |
| Extrapolation | Prolonger temporairement une trajectoire connue quand le futur état n'est pas encore arrivé ; peut nécessiter une correction. | Non comme mécanisme général de réplication. |
| Prédiction côté client | Appliquer immédiatement les commandes locales à une simulation locale pour répondre sans attendre le serveur. | Non. |
| Réconciliation serveur | Corriger la prédiction à partir d'un état autoritaire, puis rejouer les commandes locales non encore traitées. | Non. |
| Lag compensation serveur | Revenir à un état historique pertinent pour juger une action selon ce que le joueur voyait, avec une politique explicite. | Non. |
| Input delaying | Attendre volontairement des commandes pour synchroniser les participants ; ajoute du délai mais peut simplifier la cohérence. | Non comme stratégie dédiée. |
| Rollback netcode | Revenir à un état antérieur et resimuler lorsque des inputs retardataires arrivent. | Non. |

Pour une réconciliation robuste, il faudrait par exemple un identifiant d'input et une indication du dernier input traité dans les états autoritaires, un historique de commandes et une simulation locale compatible. **La séquence du paquet et le bitfield d'ACK ne fournissent pas à eux seuls cette sémantique.**

Un delta diminue les octets ; une interpolation lisse l'affichage ; une prédiction réduit l'attente ressentie. Ce sont trois problèmes différents.

## 22. Mesures, formules et netem

### Les unités

| Unité / calcul | Exemple |
|---|---|
| Octet, byte, B | 8 bits. |
| kbit/s | Milliers de bits par seconde, unité de capacité de lien fréquente. |
| KiB/s | 1 024 octets/s. 2 KiB/s représentent 16,384 kbit/s. |
| Débit moyen | `(compteur_fin − compteur_début) / durée`. |
| Fréquence d'états | `nombre d'états livrés / durée`. |
| Cadence de simulation observée | `(dernier tick − premier tick) / temps entre leurs réceptions`. C'est une estimation depuis le réseau, pas un chronomètre CPU du serveur. |
| Estimation sans pertes | `taille moyenne × messages/s × clients`. Il faut ajouter événements, ACK dans les headers, heartbeats et en-têtes réseau pour le trafic réel. |

Un INPUT représente 25 octets applicatifs ; à `1000/33 ≈ 30,3` envois/s : environ **758 octets/s**, plus un HEARTBEAT de 28 octets/s. En IPv4 sans options, UDP ajoute 8 octets et IP généralement 20, donc cet INPUT est déjà 53 octets au niveau IP ; Ethernet/tunnels et autres coûts viennent encore au-delà.

Un complet maximal de 1 142 octets envoyé 20 fois/s ferait **22 840 octets/s par client**, soit environ 22,3 KiB/s ou 182,7 kbit/s applicatifs. Ce calcul est un plafond illustratif de la configuration en complets, pas le débit moyen mesuré avec les deltas.

### DebugMetrics

Le client publie via `UiStore` : FPS, dernier ping, nombre d'entités, mémoire résidente et débits entrant/sortant calculés par différence des compteurs. Un historique de ping alimente l'affichage de debug.

Ces compteurs mesurent des octets applicatifs. Un envoi compté avant passage dans le transport peut ensuite être abandonné : « octets demandés à l'émission » n'est pas une capture prouvant leur arrivée sur le câble. Les paquets invalides provenant de l'endpoint attendu peuvent entrer dans le compteur reçu avant leur rejet par le codec. Des changements de session réinitialisent aussi les compteurs de connexion ; il faut en tenir compte dans une analyse de métriques.

### Mesure locale réellement effectuée sous netem

Source : [rapport du 8 novembre 2026](../measurements/netem-2026-11-08/rapport.md). **Mesures historiques du protocole version 8, avant ajout des pseudos et du chat.** Deux clients utilisant `ClientConnection` réel, une salle, un serveur neuf par scénario, **20 secondes de mesure par scénario**, après création et démarrage. Perturbations appliquées à la boucle locale d'un namespace réseau isolé. Aucun de ces sept scénarios n'a été rejoué en version 9 pour mettre à jour cette fiche.

| Condition | Paramètres netem | RTT médian A/B | Snapshots appliqués/s A/B | Réception A/B |
|---|---|---:|---:|---:|
| Référence | Aucune perturbation ajoutée | 2 / 2 ms | 20 / 20 | 2,04 / 2,04 KiB/s. |
| Latence | Délai 100 ms par trajet | 203 / 202 ms | 20 / 20 | 3,76 / 3,76 KiB/s. |
| Pertes | `loss 10%` | 2 / 2 ms | 18,15 / 18,20 | 1,84 / 1,84 KiB/s. |
| Doublons | `duplicate 10%` | 2 / 2 ms | 20 / 20 | 2,20 / 2,21 KiB/s. |
| Désordre | Délai 50 ms, `reorder 25% 50%` | 102 / 103 ms | 18,65 / 18,90 | 2,66 / 2,66 KiB/s. |
| Combiné | Délai 80 ± 20 ms, distribution normale ; pertes 5 %, doublons 2 %, reorder 25 % avec corrélation 50 % | 163 / 155 ms | 17,15 / 17,25 | 2,66 / 2,69 KiB/s. |
| Débit limité | Délai 30 ms, `rate 128kbit` | 88 / 86 ms | 20 / 20 | 2,72 / 2,74 KiB/s. |

Le paramètre de corrélation `50%` appartient à la configuration du réordonnancement. `±20 ms` décrit ici le paramètre de variation, pas une garantie de bornes absolues avec une distribution normale.

Les sept scénarios passent les critères du banc : connexion/salle/démarrage, au moins 10 snapshots livrés à chaque client, aucun tick appliqué en arrière, une seule livraison applicative de ROOM_JOINED et GAME_STARTED par client et aucun silence des snapshots de 5 secondes. Les clients observent environ 60 ticks/s dans ces essais. Le plus grand intervalle de snapshots mesuré atteint environ 0,20 s dans le scénario pertes.

**Ce que ces essais montrent :** le parcours réel fonctionne sous ces perturbations, les états acceptés restent chronologiques et les transitions testées arrivent sans doublon applicatif. Avec les pertes, on accepte moins d'états ; avec le délai, ils peuvent arriver régulièrement tout en étant anciens.

**Ce qu'ils ne prouvent pas :** le fonctionnement du nouveau chat version 9 et des pseudos, la livraison de tous les événements possibles, l'absence de défaut sous toutes les séquences de perte, la stabilité sur des heures, la capacité à 192 clients, la saturation effective d'un lien à 128 kbit/s, ni la fluidité du client graphique. Le ping est un RTT de heartbeat, pas une mesure touche → pixel. Les pourcentages configurés sont aléatoires et ne doivent pas être confondus avec un taux de pertes applicatif exactement mesuré.

Le débit supérieur sous latence peut notamment être influencé par la baseline plus ancienne, des deltas plus volumineux et les retransmissions ; ces chiffres ne permettent pas d'attribuer une proportion précise à chaque cause.

### Autres outils du dépôt

`protocol_probe.py` peut supprimer artificiellement des paquets reçus par un client Python et comparer les identifiants fiables. Sa version actuelle utilise la version 9, transmet les noms A/B dans CONNECT et ajoute six textes de chat : trois par auteur, envoyés en alternance avec 0,6 seconde d'attente entre envois. Elle attend six réceptions chez chaque client et intègre ce compte au verdict. Cela décrit le scénario du script, **pas un nouveau résultat mesuré pour cette fiche**. Le simple compte de six ne prouve pas à lui seul l'identité, l'ordre et l'unicité exacts des six textes ; les logs doivent être examinés pour une vérification plus forte. Ce n'est pas un test complet du `ClientConnection` C++ ni une simulation bidirectionnelle par le noyau.

`load_test.py` crée plusieurs parties et déduit leur cadence depuis les ticks reçus. Il annonce maintenant la version 9 et envoie `b"\x00"` dans CONNECT pour coder un nom vide. Les chiffres historiques de l'étude comparative sont des résultats rapportés dans leur contexte, **pas des essais rejoués lors de la rédaction de cette fiche**.

**Point à corriger avant de recompiler le banc netem actuel :** `scripts/netem_client.cpp` construit encore ses `ClientConnection` avec seulement endpoint et settings, alors que la signature version 9 exige aussi un `std::string name`. Il faut donc adapter ces appels (par exemple avec deux noms distincts ou des chaînes vides), recompiler serveur et sonde compatibles, puis relancer les scénarios. Un ancien exécutable restant dans `build/netem` peut encore produire un essai version 8 ; il ne faut pas le confondre avec un test du code actuel. Le banc ne comporte pas non plus de scénario d'échange de chat à ce stade.

Pour relancer netem : [instructions](../measurements/README.md). Les logs et JSON bruts ont été placés dans les règles locales de `.gitignore` ; le rapport Markdown et les scripts restent versionnables. L'absence des fichiers bruts sur un autre clone est donc possible.

## 23. Sécurité et limites réelles

### Mesures effectivement présentes

- Décodage borné, contrôle des tailles, types et directions ; pas de cast direct des octets vers une structure mémoire.
- Token non nul et unique parmi les sessions actives, contrôlé avec l'endpoint source.
- Droits de gestion vérifiés sur le serveur, pas uniquement dans l'interface.
- Serveur autoritaire : pas de coordonnées, vie ou score imposés directement par les messages d'inputs.
- Quota de CONNECT, espacement d'ACCEPT, limitation des mauvais codes de salle et délai anti-spam du chat par session.
- Mémoire bornée : 1 200 octets par datagramme, 256 réceptions en file, 256 envois en attente, 64 fiables en attente, historiques de taille fixe.
- Timeouts et remise à zéro des commandes en cas de silence.

### Ce qui n'est pas fourni

| Sujet | Limite à expliquer |
|---|---|
| Identité utilisateur | Le pseudo est un nom d'affichage libre et non unique. Un token représente une session réseau, pas un compte connecté avec mot de passe ou identité persistante. |
| Chiffrement | Aucun chiffrement du trafic applicatif. Les tokens et codes de salles circulent en clair. |
| Authentification cryptographique | Pas de signature/MAC de message ni de canal sécurisé. `mt19937` n'est pas un générateur cryptographique. |
| ACK hostiles | Les ACK/IDs ne sont pas protégés cryptographiquement ; la couche fiable suppose un correspondant qui respecte le protocole. |
| Protection contre le déni de service | Les bornes limitent certaines consommations, mais le quota de connexion global peut aussi être consommé par une source malveillante. Pas de contrôle de débit adaptatif complet par client. |
| Anti-triche complète | Autorité serveur utile, mais insuffisante pour prouver l'absence de toute triche ou exploitation de commandes. |
| Amplification | Une réponse peut être plus grande que la requête, par exemple ACCEPT face à CONNECT ou une liste face à LIST_ROOMS. Ne pas affirmer une absence totale d'amplification ; certaines réponses exigent une session. |
| Changement de réseau | L'endpoint étant lié à la session, changement d'IP/port NAT non pris en charge comme migration automatique de session. |
| Reconnexion | Retour menu et nouvelle connexion possibles ; pas de reprise transparente d'une identité et de son état de partie. |
| NAT/Internet | Pas de STUN/TURN, UPnP ou redirection automatique. L'hébergement distant peut nécessiter une redirection UDP. |
| IPv6 | Transport construit exclusivement en IPv4. |
| Découverte automatique | La liste des salles fonctionne après connexion à un serveur connu ; pas de recherche automatique de serveurs sur le LAN. |
| Chat/voix | Chat textuel par salle implémenté, limité et fiable au niveau transport. Voix, chat global, modération avancée et persistance absents. |
| Classement global | Score de partie répliqué, pas une base persistante ni un ranking global. |
| Administration | Gestion par propriétaire de salle, pas un tableau de bord global ni un système de ban serveur. |
| Fiabilité absolue | Fenêtres et files bornées ; pas de livraison garantie sous coupure illimitée ni d'ordre garanti entre toutes les actions. |
| Contrôle de congestion | Cadences fixes et files bornées ; pas un mécanisme complet adaptant le débit au lien disponible. |
| Observabilité | Ping et octets disponibles, mais pas tous les compteurs détaillés de pertes réelles, rejets, files pleines et événements manquants. |

### Écarts subtils entre intention et preuve

Le code sélectionne la fiabilité à l'émission par `Protocol::isReliable()`. Cela ne constitue pas une validation complète à la réception de tous les usages possibles de `reliableId`. Un client hostile ne doit pas être supposé se comporter comme le client fourni.

Le valideur d'un delta vérifie ses champs, mais le résultat reconstruit n'est pas passé par tous les contrôles de `SnapshotMessage::isValid()` avant publication. La robustesse face à des deltas volontairement incohérents peut donc être renforcée. La fiche décrit ce que fait le code, sans certifier une sécurité exhaustive.

Des messages fiables de différentes catégories peuvent arriver dans un ordre différent de l'ordre métier attendu. Il n'y a pas d'identifiant de génération de salle/session dans chaque état serveur permettant de résoudre tous les cas d'anciens événements retardés après un changement de contexte.

Les noms sont lus dans CONNECT avant le contrôle de token des messages de session : sur un endpoint déjà connu, un CONNECT non vide peut mettre à jour le nom mémorisé. Ce n'est pas une procédure de changement de pseudo authentifiée, et la chaîne reçue ne prouve aucune identité. Le slot d'auteur du chat est en revanche déterminé par le serveur à partir de l'appartenance à la salle, après contrôle endpoint/token sur CHAT.

## 24. Lien avec chaque thème de la track

### Multi-instance Server

| Attente / piste de la track | Situation du projet |
|---|---|
| Plusieurs parties | Oui : plusieurs `GameRoom` avec leurs propres `Simulation`. Updates successifs sur un thread, pas pool de simulation par salle. |
| Lobby / découverte des instances | Oui : LIST_ROOMS/ROOM_LIST, création, code, arrivée, départ et composition. |
| Gestion des utilisateurs | Sessions temporaires endpoint/token et pseudos d'affichage ; pas de comptes, d'unicité ni stockage d'identité persistant. |
| Communication entre utilisateurs | Chat textuel fiable par salle avec limite de 64 octets et intervalle de 500 ms. Voix et chat global absents. |
| Règles par instance | Oui : choix transportés lors de CREATE_ROOM et utilisés par la simulation/scripts. |
| Scoreboard / ranking global | Score de partie oui, classement global persistant non. |
| Administration | Actions du propriétaire : start/kick/delete ; administration globale absente. |

### Data Transmission Efficiency and Reliability

| Piste | Implémentation |
|---|---|
| Types adaptés et absence de padding | Entiers de taille fixe et sérialisation explicite. |
| Packing | Boutons sur un octet, flags, angle/flash partagés, champs de delta conditionnels. |
| Quantification | Positions en pixels entiers, angle sur 128 pas, vie sur 0–100. |
| Compression générale | Pas de RLE/Huffman/LZ4/zlib dans le trafic du jeu. |
| Delta snapshot compression | Oui, par client selon baseline acquittée, avec keyframes. |
| Pertes | Inputs périodiques, états renouvelés, messages importants retransmis. |
| Désordre | Séquences d'inputs/heartbeats, ticks de snapshots, déduplication des IDs fiables. |
| Duplication | États anciens refusés et messages fiables dédupliqués. |
| Fiabilité | ACK + bitfield + retransmission sélective au-dessus d'UDP, y compris CHAT et ChatBroadcast ; aucun TCP parallèle. |
| Congestion | Bornes de files et fréquence fixe, mais pas d'adaptation complète du débit. |

### High-level Networking Engine Architecture

| Piste | Implémentation |
|---|---|
| Source de vérité autoritaire | Oui : simulation serveur. |
| Fréquence d'états inférieure au tick rate | Oui : 20 snapshots/s pour 60 ticks/s. |
| Interpolation | Absente. |
| Prédiction et réconciliation | Absentes. |
| Lag compensation / rollback | Absents. |
| Mesures concrètes | Oui : résultats netem Linux version 8 avec ping, octets et états/s, plus outils d'étude de protocole et charge. Pas de nouveau résultat netem version 9/chat dans cette mise à jour. |
| Mesure du ressenti sous lag | RTT mesuré ; touche → affichage et évaluation visuelle restent à réaliser séparément. |

Les coches de la track sont des thèmes à examiner. Il faut défendre les choix faits et leurs preuves, sans présenter toutes les techniques citées comme déjà implémentées.

## 25. Démonstration et diagnostic

### Démarrer en local

Après compilation du code actuel :

```sh
./build/bin/r-type_server -p 4242
./build/bin/r-type_client -h 127.0.0.1 -p 4242
```

Le client initialise son adresse à partir des options/réglages et l'utilisateur lance la connexion dans l'interface. Pour une autre machine, remplacer 127.0.0.1 par l'IPv4 ou un nom résolvable du serveur. Les binaires doivent correspondre à la même version de protocole ; un ancien `build/bin` ne prouve pas que tu exécutes les dernières sources.

### Démonstration orale utile

1. Montrer le JSON : 60 Hz, snapshot tous les 3 ticks, limites de clients/salles.
2. Connecter plusieurs clients au même serveur.
3. Créer deux salles, montrer que leurs membres et mondes sont indépendants.
4. Tester un code de salle incorrect puis correct et expliquer le compteur d'échecs ; vérifier les pseudos et échanger des messages dans le panneau de salle, en espaçant les envois d'au moins 500 ms.
5. Démarrer par le propriétaire, montrer les tirs et l'état partagé.
6. Exclure un membre puis quitter comme propriétaire pour expliquer la fermeture.
7. Montrer le rapport netem et expliquer la différence entre un ping élevé et une baisse de cadence.
8. Expliquer les limites de l'essai et montrer les fichiers qui les implémentent.

Cette liste propose un scénario ; elle ne prétend pas que chaque manipulation graphique a été effectuée lors des tests sans fenêtre.

### Diagnostic

| Symptôme | Pistes |
|---|---|
| Port impossible à ouvrir | Déjà occupé, port invalide, restrictions système ; lire le `NetError`. |
| Aucune connexion | Bon serveur/binaire, host, port UDP, pare-feu, NAT, quota de CONNECT et slots disponibles. |
| Version refusée | Même version 9 et même contrat entre les exécutables. Un ancien CONNECT version 8 peut aussi être ignoré faute de payload lisible. |
| Message de chat absent | Client membre d'une salle, texte non vide/ASCII ≤64 octets, intervalle ≥500 ms ; distinguer pertes réseau et abandon anti-spam. |
| Nom affiché P1/P2 | Nom vide ou ROOM_UPDATE pas encore reçu ; fallback visuel, pas un changement d'identité. |
| Le banc netem ne compile plus | Ajouter le troisième argument de nom aux constructions de ClientConnection, puis recompiler les binaires version 9. |
| « Server full » | Capacité globale atteinte, y compris clients au lobby et réservations. |
| Salle introuvable/pleine | Vérifier ID, membres, capacité et rafraîchissement de liste. |
| Trop de tentatives | Cinq mauvais codes dans cette session ; le compteur n'est pas remis à zéro au succès. |
| Joueur immobile | Partie démarrée, focus clavier, boutons, slot, avatar vivant et circulation des inputs. |
| Mouvement saccadé | Jitter/pertes, états reçus moins souvent, application directe sans interpolation. |
| Ping élevé mais états réguliers | Délai fixe : les informations arrivent avec retard sans forcément réduire leur fréquence. |
| Retour au lobby malgré des heartbeats | Surveillance de salle distincte à 4 s ; la session réseau peut encore être vivante. |
| Retour au menu | Erreur réseau traitée par OnlineClient ; vérifier logs et timeout. |
| Paquet invalide sans log | Rejet silencieux prévu par le codec ; les rejets ne sont pas tous instrumentés. |
| Arrêt sous forte activité | Examiner files, tailles d'encodage, exceptions de simulation/scripts et volume des deltas. |
| Includes rouges dans VS Code | Configuration IntelliSense/compilation ; ce n'est pas automatiquement un défaut du protocole. Utiliser les vrais chemins/compile_commands. |

## 26. Questions de soutenance avec réponses

**1. Par quel fichier commence le réseau ?** Le programme serveur commence dans `game/server/src/main.cpp`. La construction de `GameServer` construit le transport ; la socket est réellement ouverte dans `UdpTransport::Impl`.

**2. Pourquoi UDP ?** Pour transmettre des états récents sans imposer la livraison ordonnée de tous les anciens états, en choisissant explicitement les messages à retransmettre. Cela implique d'implémenter et tester les garanties nécessaires.

**3. UDP est-il toujours plus rapide que TCP ?** Ce n'est pas une garantie générale. Ici, l'intérêt est surtout le contrôle sur la fraîcheur, les retransmissions et les catégories de données.

**4. Comment êtes-vous « connectés » avec UDP ?** Par une session applicative : CONNECT, ACCEPT, token, endpoint et timers. Pas par un handshake de connexion UDP fourni par le système.

**5. Une socket par joueur ?** Une seule socket transporte les échanges du serveur avec tous les clients. Chaque client normal possède sa socket locale.

**6. Une socket par salle ?** Non. Le serveur route les messages d'une session vers sa salle ; les salles partagent le transport.

**7. Client 12 signifie joueur 12 à l'écran ?** Non. L'indice serveur est global ; le slot dans une salle peut être 0, 1, 2 ou 3.

**8. Qu'est-ce qui autorise une action d'administration de salle ?** La vérification serveur que le slot du demandeur est celui du propriétaire, pas seulement l'existence du bouton dans l'interface.

**9. Qui contrôle les positions ?** La simulation serveur. Le client envoie un masque de commandes.

**10. Un token ou un pseudo est-il un mot de passe sécurisé ?** Non. Le token est un identifiant temporaire non nul, associé à un endpoint et transmis en clair. Le pseudo est une chaîne d'affichage libre, sans authentification ni unicité.

**11. Qu'est-ce qui se passe si CONNECT ou ACCEPT est perdu ?** Le client réémet CONNECT ; le serveur retrouve la réservation existante et peut renvoyer ACCEPT, avec limitation de fréquence.

**12. Que fait le heartbeat ?** Il entretient le trafic de présence et permet un RTT par écho de timestamp. Il ne remplace pas le système général d'ACK.

**13. Pourquoi un timeout alors que vous avez DISCONNECT ?** Un client peut crasher, perdre son réseau ou perdre le datagramme de départ. On ne peut pas dépendre d'un dernier message.

**14. Le serveur simule-t-il à chaque paquet reçu ?** Non. Les paquets mettent à jour les commandes ; les ticks fixes font avancer la simulation.

**15. Pourquoi 60 ticks et seulement 20 snapshots ?** Distinguer précision de simulation et coût réseau. Le monde avance plus souvent que ses états ne sont transmis.

**16. Est-ce que 20 snapshots/s donne 20 FPS ?** Non. La fenêtre peut afficher plus souvent ; sans interpolation, les positions autoritaires restent mises à jour selon les états acceptés.

**17. Pourquoi éviter d'envoyer les structures mémoire ?** Padding, taille des types, endianness et objets dynamiques ne sont pas un contrat binaire portable. Le codec écrit chaque champ explicitement.

**18. Pourquoi 1 200 octets ?** Budget applicatif prudent pour limiter les gros datagrammes et la fragmentation ; ce n'est pas une garantie universelle de chemin réseau ni la taille maximale théorique d'UDP.

**19. Que contient le header ?** Magic, version, type, séquence, ACK, bitfield de 32 ACK précédents, ID fiable et longueur utile ; 20 octets au total.

**20. Comment gérez-vous les paquets dans le désordre ?** Par catégorie : derniers numéros pour inputs/heartbeats, ticks pour états, ID fiable pour déduplication des messages importants.

**21. Pourquoi séparer sequence et reliableId ?** Chaque tentative doit avoir son numéro de paquet, alors que l'identité de l'action doit rester stable entre les tentatives.

**22. À quoi sert ackBits ?** Confirmer jusqu'à 32 paquets avant le plus récent, malgré trous et ordre variable, sans un paquet d'ACK par message.

**23. Si un ACK est perdu ?** La réception peut être rappelée dans les prochains en-têtes tant qu'elle reste dans la fenêtre ; sinon une retransmission permet une nouvelle tentative identifiable.

**24. ACK reçu signifie action réussie ?** Non. Une commande peut être bien reçue mais interdite. Le succès métier est indiqué par la réponse ou par l'état applicatif.

**25. Comment fixer le délai de retransmission ?** Maximum entre 100 ms et 1,5 fois le RTT lissé des fiables, avec initialisation à 100 ms de RTT.

**26. Pourquoi ne pas retransmettre les snapshots ?** Un état plus récent est généralement plus utile. Les deltas dépendent d'une baseline, mais les complets et le choix de référence permettent le rattrapage.

**27. Une baseline peut-elle être commune à tous les clients ?** Non en général : ils n'ont pas reçu/acquitté les mêmes états. L'historique est par session.

**28. Si une baseline manque ?** Le client ignore le delta, sans l'appliquer au mauvais état, puis attend un état exploitable.

**29. Un delta est-il toujours plus petit ?** Non. Les créations/suppressions et nombreux changements peuvent coûter plus cher ; l'implémentation ne compare pas encore les deux encodages pour un repli automatique.

**30. Pourquoi une keyframe ?** Fournir périodiquement un état autonome et réduire la dépendance prolongée aux historiques. Sa transmission reste non fiable.

**31. Que protège le mutex ?** Les données partagées du transport, notamment la file de réception et l'exception. Pas toute la simulation, qui reste sur un thread.

**32. Pourquoi shared_ptr pour l'envoi ?** `async_send_to` peut finir après le retour de la fonction ; les octets doivent rester vivants jusqu'au callback.

**33. Votre send() bloque jusqu'à réception ?** Non. Il programme une opération ; la réception est vérifiée séparément pour les messages fiables.

**34. Plusieurs salles signifie-t-il calcul parallèle sur plusieurs cœurs ?** Non. Plusieurs instances logiques sont simulées successivement par la même boucle ; le réseau dispose d'un worker distinct.

**35. Avez-vous de la compression ?** Oui, différentielle et spécifique au jeu ; pas de compression générique type zlib/LZ4.

**36. Avez-vous de la prédiction ?** Non. Les positions reçues sont directement appliquées. Les analyses de prédiction/interpolation ne doivent pas être présentées comme du code livré.

**37. Comment avez-vous testé les pertes réelles du système ?** Avec netem dans un namespace Linux isolé et des clients utilisant le vrai ClientConnection version 8, en plus des sondes Python historiques. Le chat ajouté en version 9 n'est pas couvert par ce rapport netem.

**38. Que montre le test à 10 % de pertes ?** Environ 18,2 états appliqués/s au lieu de 20 sur cet essai, connexions/transitions réussies, pas de recul des ticks ; pas une preuve universelle d'absence de défaut.

**39. Que montre le test à 100 ms par trajet ?** Environ 202–203 ms de RTT, tout en gardant 20 états/s. Fréquence et retard sont différents.

**40. Peut-on annoncer que le jeu est fluide à 200 ms ?** Pas avec ces seuls tests sans fenêtre. Il faut observer et mesurer le client graphique, idéalement jusqu'au délai touche → affichage.

**41. Avez-vous mesuré la capacité à 192 joueurs sous netem ?** Non. Les sept essais locaux portent sur deux clients et une salle ; les essais de charge sont une démarche distincte.

**42. Peut-on parler de matchmaking ?** On peut parler de découverte et sélection de salles. Il n'y a pas d'algorithme d'appariement automatique par niveau/région prouvé par ce code.

**43. Les scores sont-ils persistants ?** Non : état de partie, sans classement global stocké.

**44. Peut-on utiliser un nom DNS ?** `UdpTransport::resolve()` le permet vers IPv4 ; `numericEndpoint()` seul ne le permet pas. Attention aux anciennes notes.

**45. Où sont les preuves et limites ?** Dans les sources pour le comportement ; dans les rapports et scripts pour les scénarios mesurés ; dans cette fiche et l'étude comparative pour les limites à expliquer.

**46. Pourquoi une version 9 ?** CONNECT contient maintenant le pseudo, ROOM_UPDATE les noms des slots, et CHAT/ChatBroadcast complètent le catalogue. Le changement de structure rend les anciens clients incompatibles.

**47. Pourquoi ne pas remettre le pseudo dans chaque texte de chat ?** Le client le connaît par ROOM_UPDATE ; un slot sur un octet évite de répéter jusqu'à 13 octets de chaîne de nom. Cela dépend toutefois de l'arrivée des mises à jour de noms.

**48. Puis-je usurper un autre slot dans CHAT ?** Le client transmet token et texte, pas de slot d'auteur. Le serveur détermine le slot à partir de sa session et de sa salle. Un pseudo identique à celui d'un autre reste possible, puisqu'il n'est pas unique.

**49. Pourquoi un CHAT fiable peut-il ne jamais être affiché ?** La demande peut être reçue et acquittée, puis refusée silencieusement par le délai anti-spam ou l'absence de salle. Fiabilité de transport et autorisation métier sont distinctes.

**50. Combien de messages de chat sont conservés ?** La vue garde les cinq dernières lignes. Le serveur ne fournit pas d'historique persistant ni de rattrapage des anciennes conversations à l'arrivée.

**51. Le chat fonctionne-t-il en jeu ?** Le serveur l'autorise pour tout membre d'une salle, sans tester si elle est en cours. Le panneau de saisie du JSON actuel est dans la scène de salle ; ne pas confondre capacité du protocole et présence de l'interface dans la scène de jeu.

**52. Les sept PASS netem prouvent-ils le nouveau chat ?** Non : rapport version 8 antérieur au chat. La sonde Python actuelle propose un scénario de chat, mais aucune exécution nouvelle ni mesure version 9 n'est annoncée dans cette mise à jour.

## 27. Pièges et ordre de révision

### Les confusions qui coûtent des points

| Formulation à éviter | Formulation précise |
|---|---|
| « Toutes les communications UDP sont fiables. » | Certains messages sont suivis et retransmis ; les états récents sont renouvelés sans retransmission générale. |
| « Les messages fiables arrivent dans l'ordre. » | Ils sont dédupliqués et retransmis, sans service général d'ordonnancement métier. |
| « Le heartbeat ACK acquitte tous les événements. » | Le header ACK/bitfield est général ; le message HEARTBEAT_ACK sert à l'écho du timestamp. |
| « Le token chiffre la connexion. » | Il contrôle une session en clair avec l'endpoint. |
| « Une room est un thread ou un processus. » | C'est une instance logique et sa simulation, dans ce serveur partagé. |
| « 60 ticks veut dire 60 envois/s et 60 FPS. » | Trois cadences différentes : simulation, réseau, affichage. |
| « Le netem 100 ms donne 100 ms de ping. » | Ici le délai s'applique aux deux trajets : environ 200 ms de RTT. |
| « Le projet gère encore seulement 16 clients/16 salles. » | Le JSON actuel prévoit 192 clients et 48 salles. |
| « Header 10 octets, magic RT, version 1. » | Ancienne version : actuellement 20 octets, magic EN, version 9. |
| « CONNECT n'a aucun payload. » | Depuis la version 9, il contient le pseudo préfixé par sa longueur, y compris un octet pour un nom vide. |
| « Le projet n'a pas de chat. » | Chat par salle présent, voix et chat global absents. |
| « Un message fiable ne peut jamais être ignoré. » | Le serveur peut l'acquitter au niveau protocole puis le refuser pour une règle métier, notamment anti-spam. |
| « Pseudo = identité authentifiée. » | Nom libre et potentiellement partagé, lié seulement à la session temporaire. |
| « Les mesures version 8 valident le chat version 9. » | Elles précèdent son ajout ; les tests doivent être adaptés et rejoués pour une preuve actuelle. |
| « Le client ne sait pas résoudre de nom. » | Faux pour `resolve()` actuel ; IPv4 uniquement demeure vrai. |
| « PLAYER_JOINED existe donc il est forcément envoyé. » | Le catalogue du protocole et le parcours réellement utilisé sont distincts. |
| « Les ACK prouvent que le delta a été affiché. » | Ils confirment la réception de paquet à la couche concernée, pas le rendu ni une confirmation explicite d'application. |
| « Les tests prouvent la stabilité en production. » | Ils attestent de scénarios précis avec charge et durée définies. |

### Ordre pour continuer le cours

1. `UdpTransport.cpp` : interface, endpoints, résolution et erreurs — **fichier actuel**.
2. `UdpTransportImpl.hpp/.cpp` : socket, worker, callbacks, files, durée de vie.
3. `Endpoint`, `Datagram`, `ByteWriter`, `ByteReader` : unités transportées et octets.
4. `Protocol`, `Packet`, `Codec` et quelques messages simples : INPUT puis CONNECT/ACCEPT.
5. `ReliableReceiver`, `PendingMessage`, `ReliableSender` : ACK, identité fiable et retransmissions.
6. `EntityState`, `PlayerStatus`, `SnapshotMessage` : représentation du monde.
7. `EntityChange`, `SnapshotDiff`, `SnapshotDeltaMessage`, `SnapshotHistory` : deltas.
8. `ClientConnection` : refaire le parcours depuis le client.
9. `ChatMessage`, `ChatBroadcastMessage`, les noms dans `ConnectMessage`/`RoomUpdateMessage`, puis `OnlineClient`, les méthodes de synchronisation de `GameView` et `DebugMetrics`.
10. Revenir à `GameServer::receive/sendTo/sendSnapshot/tickRoom` pour assembler les couches.
11. Revoir les scripts de mesure et préparer une démonstration avec les limites annoncées.

Pour chaque fonction, savoir répondre : **qui l'appelle, à quel rythme, dans quel thread, avec quelles données, quels contrôles, quels effets et que se passe-t-il si le paquet est perdu/dupliqué/retardé ?**

### Présentation courte à mémoriser

> Notre moteur utilise un serveur autoritaire qui héberge plusieurs salles indépendantes. Les clients transmettent leurs commandes en UDP/IPv4 et le serveur simule à 60 ticks par seconde, avec une cible de 20 snapshots par seconde. Le protocole binaire version 9 comprend 31 types de messages, sérialise explicitement ses champs et limite les datagrammes applicatifs à 1 200 octets. Les messages importants utilisent ACK, historique de réception, identifiants stables et retransmissions ; les états utilisent des snapshots complets ou des deltas calculés selon une baseline propre au client. Nous avons aussi des pseudos et un chat textuel fiable par salle, avec limitation anti-spam. Un thread Asio gère les entrées-sorties et le thread principal possède les sessions et la simulation. Les essais netem version 8 avec deux clients passent les sept conditions historiques mesurées, mais ne valident pas le chat ajouté en version 9. Les limites principales restent l'absence d'interpolation/prédiction, les garanties bornées de fiabilité, l'absence de sécurité cryptographique et les essais version 9, longs et sous forte charge à poursuivre.

### Références du dépôt

- [Contrat binaire du protocole](../protocol.md) — à lire avec le code, certaines formulations sont plus générales que les garanties réellement vérifiées.
- [Organisation réseau](../architecture/networking.md) et [threads](../architecture/threading.md).
- [Étude comparative réseau](../comparative-study/networking.md) — choix et mesures historiques, à distinguer des nouveaux essais locaux.
- [Rapport netem](../measurements/netem-2026-11-08/rapport.md) et [instructions du banc](../measurements/README.md).
- [Guide de scripts](scripting.md) pour approfondir le lien Python/simulation.
