# Fiche de révision — questions sur le réseau actuel

Questions possibles de soutenance, réponses adaptées à **notre implémentation**, fichiers à montrer et limites à annoncer. Cette fiche est centrée sur le réseau : les anciens critères généraux de build, de prototype et d'accessibilité n'en constituent plus le plan.

**Sources revérifiées le 10 novembre 2026, commit `25bc836`.** Il s'agit d'une lecture du code, pas d'une nouvelle campagne de tests. Les résultats netem cités sont ceux du 8 novembre, antérieurs aux évolutions actuelles.

**Pour approfondir les trois sujets prioritaires :** [Asio dans notre projet](#17-cours-détaillé-asio-dans-notre-projet), [netem et notre banc de mesure](#18-cours-détaillé-netem-et-notre-banc-de-mesure), [tailles de paquets et calculs](#19-cours-détaillé-tailles-de-paquets-et-calculs). Ces parties détaillent le code, les exemples, les limites et les questions pièges.

**Point de vigilance :** le code déclare toujours la version de protocole **9**, mais comporte désormais **33 types de messages** et une numérotation différente de l'ancien état également appelé version 9. Même numéro de version ne signifie donc pas nécessairement même format dans l'historique de ce projet. Pour jouer ensemble, utiliser des builds compatibles issus du même état des sources et des données.

## Sommaire

1. [Architecture et séparation du réseau](#1-architecture-et-séparation-du-réseau)
2. [UDP, sockets et threads](#2-udp-sockets-et-threads)
3. [Protocole binaire actuel](#3-protocole-binaire-actuel)
4. [Connexion, sessions et tokens](#4-connexion-sessions-et-tokens)
5. [Salles, règles et multi-instance](#5-salles-règles-et-multi-instance)
6. [Serveur autoritaire et boucle de jeu](#6-serveur-autoritaire-et-boucle-de-jeu)
7. [Pertes, séquences, ACK et fiabilité](#7-pertes-séquences-ack-et-fiabilité)
8. [Packing, tailles et bande passante](#8-packing-tailles-et-bande-passante)
9. [Snapshots complets et deltas](#9-snapshots-complets-et-deltas)
10. [Pseudos et chat](#10-pseudos-et-chat)
11. [Console et commandes distantes](#11-console-et-commandes-distantes)
12. [Synchronisation et lag](#12-synchronisation-et-lag)
13. [Mesures et tests](#13-mesures-et-tests)
14. [Connexion entre deux PC](#14-connexion-entre-deux-pc)
15. [Robustesse et sécurité](#15-robustesse-et-sécurité)
16. [Chiffres et fichiers à mémoriser](#16-chiffres-et-fichiers-à-mémoriser)
17. [Cours détaillé Asio dans notre projet](#17-cours-détaillé-asio-dans-notre-projet)
18. [Cours détaillé netem et notre banc de mesure](#18-cours-détaillé-netem-et-notre-banc-de-mesure)
19. [Cours détaillé tailles de paquets et calculs](#19-cours-détaillé-tailles-de-paquets-et-calculs)

## 1. Architecture et séparation du réseau

**« Quelle est votre architecture réseau ? »**

Client-serveur avec serveur autoritaire. Les clients transmettent leurs commandes au serveur et reçoivent les états de sa simulation. Ils ne s'échangent pas directement leurs positions entre eux. Un serveur peut héberger plusieurs salles indépendantes.

**« Par quel fichier commence le serveur actuellement ? »**

Par [engine/apps/src/ServerMain.cpp](../../engine/apps/src/ServerMain.cpp). Le code a été déplacé depuis l'ancien `game/server/src/main.cpp`. Le main charge les paramètres, initialise Python, crée SimulationFactory et GameServer, puis appelle ServerRunner. Construire GameServer construit déjà son transport UDP.

**« Comment avez-vous séparé transport et protocole ? »**

| Couche | Responsabilité |
|---|---|
| `engine/net` | Socket, endpoints, datagrammes, lecture/écriture d'octets. |
| `engine/network/protocol` | Packet, Codec, règles de format, fiabilité et historiques. |
| `engine/network/messages` | Champs et validation des messages, regroupés par connexion, lobby, jeu et console. |
| `engine/network/server` | Sessions, salles, routage vers les simulations. |
| `engine/network/client` | Session cliente, commandes, ACK et reconstruction des états. |
| `engine/simulation` | Monde autoritaire et production de ses états/événements. |
| `engine/client` | Interface, commandes utilisateur et présentation des données reçues. |

**« Votre transport connaît-il les joueurs ou les salles ? »**

Non. UdpTransport connaît des endpoints et des tampons d'octets. GameServer connaît les sessions et les salles ; Simulation connaît le monde. C'est cette répartition qu'il faut montrer pour défendre le découplage.

**« Comment le réseau utilise-t-il une simulation sans connaître ses détails ? »**

Grâce à IGameSession et ISessionFactory. La première expose notamment addPlayer, removePlayer, setInput, command, update, tick, snapshot, takeEvents et finished. La fabrique crée une nouvelle simulation pour une salle. Le serveur réseau n'a pas à connaître le script de chaque boss.

**« À quoi servent les visiteurs ajoutés au protocole ? »**

Packet possède un variant de messages et expose `visit()`. MessageValidator appelle `isValid(settings)` sur le type contenu ; MessageWriter appelle `write(writer)` ; TokenReader extrait `token()` si cette méthode existe, sinon retourne nullopt. Cela factorise certaines opérations au lieu de recopier une branche par message. Le décodage garde une sélection du type reçu.

**À montrer :** [GameServer.cpp](../../engine/network/server/src/GameServer.cpp), [PacketVisitors.tpp](../../engine/network/protocol/include/PacketVisitors.tpp), [IGameSession.hpp](../../engine/network/server/include/IGameSession.hpp).

## 2. UDP, sockets et threads

**« Pourquoi UDP plutôt que TCP ? »**

Pour choisir les garanties par catégorie de données. Un ancien état manquant ne doit pas imposer l'attente de tous les suivants. Les messages importants sont suivis et retransmis par notre couche applicative ; les positions et commandes sont fréquemment renouvelées. UDP ne garantit cependant ni livraison, ni ordre, ni absence de doublon. Ce choix ne signifie pas qu'UDP est toujours plus rapide dans toutes les situations.

**« Utilisez-vous quand même TCP pour le lobby ou le chat ? »**

Non. Le trafic applicatif du jeu, y compris lobby, chat et console, passe par notre protocole binaire UDP/IPv4. Il n'y a pas de canal TCP complémentaire pour ces messages.

**« Qu'est-ce qu'un endpoint et une socket ? »**

Un endpoint est un couple IP + port, par exemple `192.168.1.20:53012`. Une socket est la ressource système qui envoie et reçoit. Le serveur a une socket partagée entre ses clients/salles ; chaque client normal possède sa socket locale.

**« Le client utilise-t-il aussi le port local 4242 ? »**

Pas automatiquement. Le transport client est construit avec le port local 0 : le système choisit un port éphémère. 4242 est le port de destination du serveur par défaut. Les deux extrémités n'ont pas besoin d'avoir le même numéro de port.

**« Où la socket est-elle ouverte ? »**

Dans le constructeur de UdpTransport::Impl, lorsque la socket Asio est créée avec un endpoint local IPv4 et le port demandé. `receive()` programme une réception asynchrone, puis un thread démarre `m_io.run()` pour exécuter les callbacks.

**« Le serveur attend-il une réponse réseau avant de simuler ? »**

Non. Le worker I/O reçoit les datagrammes et les place dans une file. Le thread principal récupère ceux déjà disponibles avec poll(). Il n'attend pas que tous les clients répondent pour avancer le prochain tick.

**« Que protège le mutex ? »**

La file de réception et l'exception partagée du transport. Le jeu et les sessions restent dans le thread principal. `takeIncoming()` échange la file avec un vector vide sous verrou, puis le traitement se poursuit sans garder ce verrou.

**« Pourquoi garder les octets d'envoi dans un shared_ptr ? »**

L'envoi asynchrone peut terminer après le retour de send(). Le tampon doit rester vivant jusqu'au callback ; le shared_ptr capturé avec l'opération en assure la durée de vie. Le tampon de réception, lui, est copié avant d'être réutilisé.

**« Quelles sont les bornes et comment se passe l'arrêt ? »**

256 datagrammes en réception et 256 envois en attente au maximum ; la saturation abandonne les nouveaux éléments concernés. À la destruction, le transport attend au plus 200 ms les opérations d'envoi locales, puis arrête Asio et rejoint le worker. Cela ne garantit pas la réception distante des derniers messages.

**À montrer :** [UdpTransport.cpp](../../engine/net/src/UdpTransport.cpp), [UdpTransportImpl.cpp](../../engine/net/src/UdpTransportImpl.cpp), [implémentation privée](../../engine/net/internal/UdpTransportImpl.hpp).

## 3. Protocole binaire actuel

**« Quel est le format de votre en-tête ? »**

| Offset | Octets | Champ |
|---:|---:|---|
| 0 | 2 | magic = `0x454E`, lettres EN. |
| 2 | 1 | version = 9. |
| 3 | 1 | type ; bit `0x80` pour serveur → client. |
| 4 | 4 | sequence : numéro de cette tentative de paquet. |
| 8 | 4 | ack : plus récente séquence reçue du correspondant. |
| 12 | 4 | ackBits : statut des 32 séquences précédentes. |
| 16 | 2 | reliableId : identité stable du message fiable, ou 0. |
| 18 | 2 | payloadSize : nombre d'octets après l'en-tête. |

Total : **20 octets applicatifs**, avant le payload. UDP et IP ajoutent leurs propres en-têtes, qui ne sont pas inclus ici.

**« Pourquoi ne pas envoyer une struct ou Packet tel quel ? »**

Padding, ordre des octets, tailles dépendantes de la plateforme et objets dynamiques ne constituent pas un format portable. Packet contient notamment un variant de messages ; le codec écrit explicitement les champs utiles avec ByteWriter, puis ByteReader les reconstruit.

**« Que signifie big-endian ? »**

Octet le plus significatif d'abord : `0x12345678` devient `12 34 56 78`. À la lecture, on décale la valeur accumulée de huit bits et on y ajoute l'octet suivant. Les tailles sont explicites : u8 = 1 octet, u16 = 2, u32 = 4.

**« Comment sait-on où finit un message ? »**

UDP conserve les limites des datagrammes. Un datagramme transporte un message du protocole ; payloadSize doit correspondre exactement à sa taille restante. Le décodeur exige aussi que tous les champs soient entièrement lus et qu'il ne reste pas d'octets inutilisés.

**« Quels messages existent aujourd'hui ? »**

Catalogue tiré de Protocol.hpp ; **R** = sélectionné par `Protocol::isReliable()` à l'émission, **—** = pas de suivi fiable général. Les types disponibles ne sont pas tous nécessairement émis dans chaque scénario du jeu.

| Code | Classe du message | Sens | Suivi fiable |
|---|---|---|---|
| `01` | ConnectMessage | Client → serveur | —, retry de connexion. |
| `02` | InputMessage | Client → serveur | —. |
| `03` | HeartbeatMessage | Client → serveur | —. |
| `04` | DisconnectMessage | Client → serveur | —. |
| `05` | ListRoomsMessage | Client → serveur | —. |
| `06` | CreateRoomMessage | Client → serveur | R. |
| `07` | JoinRoomMessage | Client → serveur | R. |
| `08` | LeaveRoomMessage | Client → serveur | R. |
| `09` | StartGameMessage | Client → serveur | R. |
| `0A` | KickPlayerMessage | Client → serveur | R. |
| `0B` | DeleteRoomMessage | Client → serveur | R. |
| `0C` | ConsoleCommandMessage | Client → serveur | —. |
| `0D` | ChatMessage | Client → serveur | R. |
| `81` | AcceptMessage | Serveur → client | —. |
| `82` | RejectMessage | Serveur → client | —. |
| `83` | SnapshotMessage | Serveur → client | —. |
| `84` | HeartbeatAckMessage | Serveur → client | —. |
| `85` | EntityDestroyedMessage | Serveur → client | R. |
| `86` | PlayerJoinedMessage | Serveur → client | R. |
| `87` | PlayerKilledMessage | Serveur → client | R. |
| `88` | PlayerLeftMessage | Serveur → client | R. |
| `89` | ServerShutdownMessage | Serveur → client | —. |
| `8A` | RoomListMessage | Serveur → client | —. |
| `8B` | RoomJoinedMessage | Serveur → client | R. |
| `8C` | RoomRejectedMessage | Serveur → client | R. |
| `8D` | RoomUpdateMessage | Serveur → client | —. |
| `8E` | GameStartedMessage | Serveur → client | R. |
| `8F` | KickedMessage | Serveur → client | R. |
| `90` | RoomClosedMessage | Serveur → client | R. |
| `91` | PlaySoundMessage | Serveur → client | R. |
| `92` | ConsoleTextMessage | Serveur → client | —. |
| `93` | SnapshotDeltaMessage | Serveur → client | —. |
| `94` | ChatBroadcastMessage | Serveur → client | R. |

**« Comment empêchez-vous un client d'envoyer un état serveur ? »**

Le codec vérifie le sens via le type et le Receiver attendu. Un paquet de sens incompatible est abandonné. Ce contrôle de format ne remplace pas l'authentification cryptographique, absente du protocole.

**« Le numéro de version suffit-il à garantir la compatibilité ? »**

Il devrait identifier un contrat stable, mais l'historique actuel contient des formats différents sous le numéro 9. Anciennement Chat était `0x0C`, SnapshotDelta `0x92`, ChatBroadcast `0x93` ; ces codes ont changé. Il faut compiler les deux côtés depuis un même état compatible et faire évoluer la version lors de changements incompatibles. Deux bins affichant 9 ne suffisent pas à prouver leur compatibilité.

**À montrer :** [Protocol.hpp](../../engine/network/protocol/include/Protocol.hpp), [Codec.cpp](../../engine/network/protocol/src/Codec.cpp), [Packet.hpp](../../engine/network/protocol/include/Packet.hpp), [spécification](../reference/protocol.md).

## 4. Connexion, sessions et tokens

**« Comment êtes-vous connectés alors qu'UDP n'établit pas de connexion applicative ? »**

Par un protocole de session : CONNECT avec pseudo, ACCEPT avec token/indice de client, puis messages portant le token. Le serveur conserve endpoint, timers, historiques et salle éventuelle. Un ACCEPT ne crée pas automatiquement un vaisseau ou une salle.

**« Que se passe-t-il si CONNECT ou ACCEPT est perdu ? »**

Le client répète CONNECT toutes les secondes, au plus cinq envois sans réponse acceptée. Pour un endpoint déjà connu, le serveur réutilise la réservation et peut renvoyer ACCEPT avec le même token, au plus une réponse toutes les 200 ms.

**« Dans quels cas refuse-t-on ou ignore-t-on la connexion ? »**

Plus de 32 CONNECT dans la fenêtre globale d'une seconde : abandon silencieux. Version incompatible si le paquet a pu être décodé : REJECT raison 2. Plus de session libre : REJECT raison 1. Répétition trop rapide d'un endpoint connu : ignorée sans supprimer sa session. Paquet mal formé : rejet du codec avant ce traitement.

**« Un ancien CONNECT est-il toujours refusé avec une réponse lisible ? »**

Non. L'exception de contrôle de version autorise un refus, mais il faut d'abord pouvoir lire le corps. Un ancien CONNECT sans l'octet de longueur du pseudo peut être rejeté comme tronqué avant d'atteindre le contrôle de version de GameServer.

**« Que protège le token ? »**

Le serveur retrouve le client par IP + port source, puis compare son token. Un bon token provenant d'un autre endpoint ne passe pas ce contrôle. Le token est aléatoire non nul et unique parmi les sessions actives ; il est transmis en clair et ne représente ni compte ni identité authentifiée.

**« Où tokenOf trouve-t-il le token ? »**

La version actuelle utilise `packet.visit(TokenReader())`. Le visiteur appelle `token()` sur les messages qui possèdent cette méthode, dont la console et le chat ; pour les autres il retourne nullopt. Ce n'est plus la longue suite de if de l'ancienne version.

**« Quand la session devient-elle confirmée ? »**

Lors du premier message de session accepté par le parcours de handleSessionPacket, sauf le traitement particulier de DISCONNECT qui intervient avant. Ce n'est pas uniquement un heartbeat. Une réservation non confirmée consomme déjà une place jusqu'à expiration.

**« Comment détectez-vous un crash ? »**

Timeout de cinq secondes sans message qui entretient la réception attendue. Heartbeat périodique et trafic de jeu entretiennent normalement la session. DISCONNECT n'est pas garanti : il ne suffit pas pour détecter toutes les disparitions.

**À montrer :** [GameServer.cpp](../../engine/network/server/src/GameServer.cpp), [ClientSession.hpp](../../engine/network/server/include/ClientSession.hpp), [ClientConnection.cpp](../../engine/network/client/src/ClientConnection.cpp).

## 5. Salles, règles et multi-instance

**« Une salle correspond-elle à un thread ou un processus ? »**

Non. Chaque GameRoom possède une simulation, des membres et ses règles. Les salles partagent le processus/transport ; leurs updates sont exécutés successivement sur le thread principal. Multi-instance logique ne signifie pas calcul parallèle sur un cœur par salle.

**« Combien de joueurs peut accueillir une salle ? »**

Le maximum configuré est quatre, mais `roomCapacity(rules)` peut le réduire selon les règles. Actuellement le mode COOP prévoit quatre places, VERSUS deux. ROOM_UPDATE accepte un nombre de slots non nul inférieur ou égal au plafond, et pas seulement exactement quatre.

**« Quelle différence entre client, slot et entité ? »**

Client = indice global de session, par exemple 12. Slot = place locale dans une salle, par exemple 1. Entité = objet répliqué dans sa simulation. Token, ID de salle, ID d'entité, séquence et tick ont encore d'autres rôles : on ne les mélange pas.

**« Qui peut créer, démarrer et administrer ? »**

Un client connecté peut créer une salle si la limite n'est pas atteinte. Le créateur est propriétaire au slot 0. Le serveur contrôle les droits de démarrage, exclusion et suppression ; cacher un bouton dans l'interface ne suffit pas.

**« Pourquoi peut-on être refusé en rejoignant ? »**

Compteur de mauvais codes atteint, salle inexistante, salle pleine ou code incorrect. Seules les erreurs de code incrémentent ce compteur ; le seuil actuel est cinq et le compteur est attaché à la session. Un client déjà dans une salle reçoit de nouveau les informations de cette salle au lieu d'en rejoindre une deuxième.

**« Peut-on rejoindre en cours de partie ? »**

Oui si les contrôles passent et qu'une place est disponible. seatClient associe client et salle, réinitialise son historique de snapshots, ajoute le joueur à la simulation en cours et lui envoie les messages nécessaires.

**« Que se passe-t-il au départ d'un membre ou du propriétaire ? »**

Un membre est retiré ; en partie, le serveur diffuse actuellement PlayerLeftMessage avec la raison, puis ROOM_UPDATE. Le propriétaire qui quitte ferme la salle : pas de transfert automatique. KICKED retire de la salle, pas nécessairement de toute session serveur.

**« Que se passe-t-il quand la simulation est terminée ? »**

Si `IGameSession::finished()` le signale, GameServer::returnToRoom remet la salle en attente, recrée une simulation avec les mêmes règles, oublie les historiques de snapshots et envoie ROOM_JOINED puis ROOM_UPDATE. Ce n'est pas une fermeture obligatoire de la salle ni une nouvelle connexion UDP.

**« Comment découvrez-vous les salles ? »**

LIST_ROOMS/ROOM_LIST sur un serveur déjà connu. Le client du lobby redemande la liste toutes les deux secondes ; les compositions de salle sont mises à jour environ toutes les secondes. Il n'y a pas de recherche automatique de serveurs sur le LAN ni d'algorithme de matchmaking par niveau/région.

**À montrer :** [GameServerRooms.cpp](../../engine/network/server/src/GameServerRooms.cpp), [GameRoom.hpp](../../engine/network/server/include/GameRoom.hpp), [NetworkSettings.cpp](../../engine/network/protocol/src/NetworkSettings.cpp).

## 6. Serveur autoritaire et boucle de jeu

**« Qui décide de la position et des dégâts ? »**

Le serveur. Les clients transmettent des intentions sous forme de boutons. Simulation et systèmes/scripts calculent déplacements, tirs, collisions, vies et scores. Un INPUT n'est pas une demande « accepte ma position » ou « j'ai tué cet ennemi ».

**« Quel parcours fait une commande ? »**

```text
Touches → ClientConnection → INPUT → octets UDP
  → transport serveur → Codec → endpoint/token/contrôles de réception
  → handleInput → salle et slot → Simulation::setInput
  → update du tick → snapshot complet ou delta
  → ClientConnection → SnapshotPresenter → GameView
```

**« Le serveur simule-t-il à chaque paquet ? »**

Non. Les messages mettent les commandes à jour ; ServerRunner exécute les ticks à pas fixe. Il peut exécuter zéro, un ou plusieurs ticks dans un tour de boucle selon le temps accumulé.

**« Quelle différence entre tick rate, snapshots et FPS ? »**

60 ticks/s de calcul cible, un snapshot tous les trois ticks donc 20/s, et une fréquence d'affichage propre au client. `sleep_for(1 ms)` limite l'occupation de la boucle mais ne définit pas le tick rate.

**« Que se passe-t-il si les inputs s'arrêtent sans déconnexion complète ? »**

Après 500 ms sans nouvel input frais, le serveur force les boutons à zéro pour éviter un mouvement/tir bloqué. Le masque est l'état des touches, régulièrement renouvelé, pas seulement une série d'événements pressé/relâché.

**À montrer :** [ServerRunner.cpp](../../engine/network/server/src/ServerRunner.cpp), [Simulation.cpp](../../engine/simulation/src/Simulation.cpp), [SnapshotPresenter.cpp](../../engine/client/src/SnapshotPresenter.cpp).

## 7. Pertes, séquences, ACK et fiabilité

**« Que faites-vous contre les pertes UDP ? »**

Trois réponses complémentaires : nouvelles commandes périodiques, nouveaux états du monde, retransmission sélective des messages importants. Rien de tout cela ne supprime le délai physique ou ne rend une liaison coupée utilisable.

**« À quoi servent sequence, reliableId et tick ? »**

sequence identifie une tentative de paquet ; reliableId identifie un message malgré ses retransmissions ; tick identifie un instant de simulation. Chaque côté a son compteur sortant, et le serveur possède un compteur par session cliente.

**« Comment comparez-vous les séquences quand elles bouclent ? »**

`Codec::isNewer()` utilise la différence non signée : non nulle et strictement inférieure à `2^31`. Cela évite un simple test numérique qui échouerait au retour du compteur à zéro. Les fenêtres doivent rester compatibles avec cette comparaison modulaire.

**« Comment fonctionne ackBits ? »**

Le bit i concerne `ack − 1 − i`. Avec ack=43 et les bits bas 1101, les séquences 42, 40 et 39 sont indiquées reçues, mais pas 41. L'ACK principal confirme 43 : jusqu'à 33 paquets peuvent donc être reconnus dans un en-tête.

**« Faut-il un paquet d'ACK pour chaque message ? »**

Non. ACK et bitfield voyagent dans les en-têtes des paquets déjà envoyés. HEARTBEAT_ACK est un message distinct d'écho de timestamp pour mesurer le ping ; il ne constitue pas à lui seul tout le mécanisme de fiabilité.

**« Comment une retransmission évite-t-elle une double action ? »**

Le message garde son reliableId mais reçoit une nouvelle sequence. Le receiver mémorise les IDs fiables traités. Même si deux tentatives arrivent, l'action ne doit pas être répétée dans la fenêtre de déduplication ; les informations de réception sont néanmoins actualisées.

**« Quel délai choisissez-vous avant de renvoyer ? »**

`max(100 ms, RTT lissé × 1,5)`. Le RTT initial des fiables est 100 ms. Un nouvel échantillon est incorporé avec un poids d'un huitième. Le plancher évite de retransmettre trop vite sur LAN avant que le correspondant ait pu émettre ses ACK.

**« Quelles sont les limites de cette couche ? »**

64 attentes fiables par correspondant et mémoire des 256 derniers IDs fiables. Saturation et retards extrêmes limitent les garanties. Les messages fiables distincts ne bénéficient pas d'un service général d'ordre métier comparable à un flux TCP.

**« Un ACK signifie-t-il que la commande a réussi ? »**

Non. Il indique une réception au niveau concerné, pas l'autorisation ni l'application complète de l'action. Une demande interdite ou trop rapide peut être reçue et acquittée puis ignorée/refusée par la logique métier.

**« Tous les événements sont-ils fiables ? »**

Non : le catalogue précédent donne la politique exacte. Les actions principales de salle, le chat et certains événements le sont. Les commandes/retours de console, DISCONNECT et SERVER_SHUTDOWN ne sont pas sélectionnés actuellement. Ne pas généraliser à partir de leur importance apparente.

**À montrer :** [ReliableReceiver.cpp](../../engine/network/protocol/src/ReliableReceiver.cpp), [ReliableSender.cpp](../../engine/network/protocol/src/ReliableSender.cpp), [Protocol.cpp](../../engine/network/protocol/src/Protocol.cpp).

## 8. Packing, tailles et bande passante

**« Quelles optimisations de représentation avez-vous ? »**

Petits types explicites, absence de padding transmis, masques de boutons, quantification des coordonnées et de l'angle, flags visuels et champs conditionnels des deltas. Il n'y a pas de compression générale zlib/LZ4 dans le trafic gameplay.

**« Combien de commandes tiennent dans l'octet de boutons ? »**

Actuellement sept bits utilisés : haut 01, bas 02, gauche 04, droite 08, tir 10, action 20, bombe 40, en hexadécimal. Seul le bit 80 est réservé. L'ancienne fiche indiquait encore six commandes et deux bits réservés.

**« Que contient EntityState ? »**

ID u16, kind u8, x et y i16, visual u8 : **huit octets**. Les positions flottantes sont arrondies en pixels entiers ; l'angle tient sur 128 pas d'environ 2,8125°, avec le flash dans le bit supérieur. On économise en perdant une partie de la précision.

**« Quelle est la taille d'un snapshot avec les données actuelles ? »**

La formule, en-tête compris, est `20 + 8 + 2W + P × (10 + 2V) + 8N`. Aujourd'hui W=16 valeurs monde, P=4 slots décrits et V=4 valeurs par joueur, donc **132 + 8N octets**. Avec N=130 : **1 172 octets**, pas les 1 142 de l'ancienne configuration. Le nombre de slots décrit par la simulation reste le plafond configuré, même si la salle VERSUS n'offre que deux places.

**« Quelles valeurs répliquez-vous et comment le réseau connaît-il leur sens ? »**

Les tables viennent du JSON : progression, phase, valeurs coop/versus, bombe, musique, etc., plus arme/force/drone/charge par joueur. Le protocole transporte indices et nombres ; le client utilise les mêmes tables pour les labels. Des fichiers de données incompatibles peuvent donc casser l'interprétation malgré un code de transport compatible.

**« Débit faible et ping faible sont-ils équivalents ? »**

Non. Le débit est une quantité d'octets par seconde ; le ping est un aller-retour. Les compteurs du client mesurent des octets applicatifs, sans en-têtes UDP/IP/Ethernet. Un envoi compté peut ensuite être abandonné : ce n'est pas une capture du trafic effectivement reçu.

**À montrer :** [Protocol.hpp](../../engine/network/protocol/include/Protocol.hpp), [SimulationSnapshot.cpp](../../engine/simulation/src/SimulationSnapshot.cpp), [rtype_game.json](../../game/data/rtype_game.json).

## 9. Snapshots complets et deltas

**« Pourquoi ne pas renvoyer tout le monde à chaque fois ? »**

Les types, apparences et nombreuses valeurs changent peu. SnapshotDiff ne transmet que les valeurs modifiées, entités nouvelles/modifiées et IDs supprimés. La simulation produit toujours un complet ; la couche réseau choisit sa représentation par client.

**« Qu'est-ce qu'une baseline ? »**

Un état précisément identifié servant de référence. SnapshotHistory retient les snapshots envoyés et utilise les ACK du client pour promouvoir une baseline. Elle est propre à chaque session, car les clients ne perdent pas les mêmes paquets.

**« Quel gain donne un changement d'entité ? »**

Un changement seulement en X dans [−128,127] peut coûter quatre octets : ID deux, masque un, dx un. Une entité complète en coûte huit. Si le déplacement ne tient pas dans un int8, on envoie les coordonnées absolues. Un delta n'est pas une simple soustraction envoyée sans identité ni masque.

**« Comment le client applique-t-il un delta ? »**

Il retrouve exactement baselineTick parmi ses états conservés, copie cette référence puis applique remplacements, valeurs et suppressions. Sans baseline, il ignore le delta plutôt que l'appliquer au mauvais monde. Le résultat passe ensuite par le contrôle du tick plus récent.

**« Quels historiques conservez-vous ? »**

Côté serveur : jusqu'à 16 snapshots envoyés dans l'historique en attente, plus la baseline et le dernier complet. Côté client : jusqu'à 32 états acceptés pour reconstruire les deltas. Ces nombres ne sont pas la fenêtre des ACK ni l'historique des IDs fiables.

**« Quand repassez-vous en snapshot complet ? »**

Sans baseline exploitable, si sa fraîcheur n'est plus inférieure à une seconde de ticks, ou si cinq secondes de ticks se sont écoulées depuis le dernier complet envoyé. Ce dernier timestamp constate un envoi, pas une réception garantie à intervalle fixe.

**« Un delta est-il forcément plus petit et sûr à encoder ? »**

Non. Beaucoup de créations/suppressions peuvent dépasser la taille du complet. Le choix actuel ne compare pas systématiquement les deux tailles et ne replie pas automatiquement sur un complet trop grand. La configuration vérifie notamment le complet et la liste de salles ; elle ne garantit pas tous les pires cas de delta. ByteWriter reste borné à 1 200 et signale un dépassement.

**« Acquitter le paquet prouve-t-il que le delta a été affiché ? »**

Non. Le client remplit ses ACK avant le traitement applicatif. Ils ne sont pas une confirmation explicite de reconstruction ni de rendu. Les historiques et complets périodiques participent au rattrapage, avec des garanties qui restent bornées.

**À montrer :** [SnapshotDiff.cpp](../../engine/network/protocol/src/SnapshotDiff.cpp), [EntityChange.cpp](../../engine/network/messages/game/src/EntityChange.cpp), [SnapshotHistory.cpp](../../engine/network/protocol/src/SnapshotHistory.cpp).

## 10. Pseudos et chat

**« Comment apprenez-vous le nom des autres joueurs ? »**

CONNECT transporte le pseudo ; ClientSession le conserve ; ROOM_UPDATE transporte le nom de chaque slot avec son occupation. ChatBroadcast n'a donc pas à répéter la chaîne de nom à chaque ligne. Un nom vide est autorisé et peut être remplacé visuellement par P1/P2, etc.

**« Quelles sont les bornes du chat ? »**

Pseudo : 0 à 12 octets. Texte : 1 à 64 octets ASCII imprimables. Un message accepté par session au plus toutes les 500 ms. Le serveur vérifie salle et slot, diffuse aux membres y compris l'auteur. Ce n'est pas un chat global du lobby ou une voix sur IP.

**« Peut-on prétendre parler au nom d'un autre slot ? »**

Le client transmet token et texte, pas un slot d'auteur à choisir. Le serveur détermine le slot réel. En revanche, les pseudos ne sont ni uniques ni authentifiés ; deux personnes peuvent choisir la même chaîne.

**« Pourquoi un message de chat fiable peut-il disparaître ? »**

Le serveur peut l'acquitter au niveau protocole puis l'ignorer pour anti-spam ou absence de salle. La retransmission du même ID dédupliqué ne contourne pas ce refus. Il faut distinguer perte de transport et règle métier.

**« Conservez-vous la conversation ? »**

La vue conserve cinq lignes dans son historique d'affichage ; le serveur n'offre pas d'archive persistante ni de récupération complète à l'arrivée. L'ordre métier du chat n'a pas de séquence supplémentaire de conversation. L'auteur peut être affiché par fallback si la mise à jour des noms est en retard.

**À montrer :** [ChatMessage.cpp](../../engine/network/messages/lobby/src/ChatMessage.cpp), [ChatBroadcastMessage.cpp](../../engine/network/messages/lobby/src/ChatBroadcastMessage.cpp), [OnlineClientLobby.cpp](../../engine/client/src/OnlineClientLobby.cpp).

## 11. Console et commandes distantes

**« Qu'ajoutent les messages de console ? »**

ConsoleCommandMessage transporte token et texte vers le serveur ; ConsoleTextMessage rapporte du texte vers le client. Les textes sont validés en ASCII imprimable, limités à 200 octets ; la commande doit être non vide, le retour peut être vide. Ils sont distincts des messages de discussion entre joueurs.

**« Tout joueur connecté peut-il exécuter des commandes serveur ? »**

Non. handleConsole exige un slot appartenant à une salle et correspondant au propriétaire. Les autres reçoivent un texte de refus. Selon la règle console configurée, les commandes peuvent aussi être bloquées lorsque cheats est OFF, sauf commandes explicitement libres.

**« D'où viennent ces permissions et exceptions ? »**

Le JSON actuel indique `consoleRule: cheats` et `freeCommands: [help, next, restart]`. NetworkSettings trouve l'index de règle et compare le premier mot de commande à cette liste. Une commande libre ne supprime pas la restriction de propriétaire appliquée auparavant.

**« Que se passe-t-il avant le démarrage ? »**

Les commandes autorisées sont mises en attente dans la salle ; le serveur retourne un texte indiquant qu'elles seront exécutées au lancement. handleStartGame transfère ensuite ces commandes à IGameSession::command. En partie, la commande est directement transmise à la simulation.

**« Les commandes de console ont-elles la même fiabilité que le chat ? »**

Non. Protocol::isReliable ne sélectionne actuellement ni ConsoleCommand ni ConsoleText. Une commande ou son retour peut donc être perdu sans retransmission générale. Ce n'est pas une console d'administration globale sécurisée avec comptes et rôles persistants.

**À montrer :** [GameServerRooms.cpp](../../engine/network/server/src/GameServerRooms.cpp), [ConsoleCommandMessage.cpp](../../engine/network/messages/console/src/ConsoleCommandMessage.cpp), [ConsoleTextMessage.cpp](../../engine/network/messages/console/src/ConsoleTextMessage.cpp).

## 12. Synchronisation et lag

**« Le jeu est-il fluide dès qu'il reçoit 20 états/s ? »**

Pas forcément : ils peuvent être retardés ou arriver irrégulièrement. La latence décrit le délai, le jitter sa variation. Le mouvement affiché dépend aussi de la présentation et du rythme des images.

**« Avez-vous interpolation, prédiction et réconciliation ? »**

Pas de mécanisme réseau général de ces types dans les fichiers vérifiés. SnapshotPresenter synchronise les objets puis GameView affecte les positions reçues. Une animation locale de sprite n'est pas une prédiction de la simulation.

**« À quoi serviraient ces mécanismes ? »**

L'interpolation afficherait entre deux états connus avec un retard volontaire pour lisser. La prédiction appliquerait immédiatement les inputs locaux. La réconciliation corrigerait depuis le serveur et rejouerait les inputs non encore traités. Cela demanderait une sémantique de dernier input traité et des historiques adaptés, pas seulement un ACK de paquet.

**« Pourquoi ne pas confondre compression et lissage ? »**

Un delta réduit le trafic. Une interpolation rend des transitions plus régulières. Une prédiction réduit l'attente locale ressentie. Optimiser l'un ne fournit pas automatiquement les deux autres.

**« Avez-vous rollback ou lag compensation serveur ? »**

Non comme mécanismes implémentés de ce projet. Ils supposeraient notamment des états historiques et une politique de retour/rejeu ou de jugement d'action. Il faut les présenter comme techniques connues, pas comme fonctionnalités livrées.

**À montrer :** [SnapshotPresenter.cpp](../../engine/client/src/SnapshotPresenter.cpp), [GameView.cpp](../../engine/client/src/GameView.cpp), [OnlineClientGame.cpp](../../engine/client/src/OnlineClientGame.cpp).

## 13. Mesures et tests

**« Qu'avez-vous mesuré avec netem ? »**

Sept scénarios historiques : référence, délai, pertes, duplication, réordonnancement, combinaison, débit limité. Deux clients utilisant alors le vrai ClientConnection, une salle, serveur neuf et 20 secondes de mesure par scénario dans un namespace réseau isolé. Les sept satisfont les critères de ce banc.

**« Quelles valeurs sont parlantes ? »**

| Condition historique | Ping médian | États appliqués/s |
|---|---|---|
| Référence | Environ 2 ms. | 20 par client. |
| Délai 100 ms par trajet | Environ 202–203 ms. | 20 par client. |
| Pertes 10 % configurées | Environ 2 ms. | Environ 18,2 par client. |
| Perturbations combinées | Environ 155–163 ms. | Environ 17,2 par client. |

Pas de recul de tick ni de doublon des confirmations de salle/démarrage dans ces essais. Ces critères ne vérifient pas la livraison parfaite de tous les événements imaginables.

**« Cela prouve-t-il le fonctionnement du chat et de la console actuels ? »**

Non : rapport du 8 novembre, protocole version 8 avant ces évolutions. Ni chat actuel, ni console, ni charge à 192 clients, ni stabilité sur des heures ne sont prouvés par ces seules mesures. Les pourcentages netem sont des paramètres aléatoires, pas un taux applicatif exactement observé.

**« Le banc netem est-il toujours cassé par le constructeur avec pseudo ? »**

Cette incompatibilité de source a été corrigée : netem_client.cpp transmet désormais des noms probe1/probe2 au constructeur. Il faut néanmoins recompiler les sources actuelles et relancer pour obtenir des résultats à jour. La correction d'un appel ne prouve pas l'exécution de nouveaux essais ni une couverture du chat/console.

**« Comment mesurez-vous ping et débit ? »**

Le heartbeat renvoie le timestamp client et fournit un RTT, sans synchroniser les horloges des deux machines. ReliableSender a aussi sa propre estimation lissée issue des fiables acquittés. Le débit est une différence de compteurs d'octets divisée par la durée ; ces compteurs sont applicatifs et non une capture Ethernet.

**« Pourquoi les scripts Python ne remplacent-ils pas tous les tests ? »**

Ils parlent le format binaire et sont utiles pour isoler des comportements, mais ne sont pas l'implémentation cliente C++. Leurs constantes doivent suivre les nouveaux codes de message. Une sonde qui ignore volontairement des réceptions n'est pas une perturbation bidirectionnelle du noyau ni un test du rendu.

**À montrer :** [rapport netem](../measurements/netem-2026-11-08/report.md), [netem_client.cpp](../../scripts/netem_client.cpp), [netem_test.py](../../scripts/netem_test.py), [instructions](../measurements/index.md).

## 14. Connexion entre deux PC

**« Pourquoi 127.0.0.1 ne rejoint-il pas mon ami ? »**

Parce qu'il désigne ta propre machine. Sur le même LAN, le client doit viser l'IPv4 du PC qui exécute le serveur et son port UDP, par exemple `192.168.1.67:4242`. Le serveur affiche normalement une adresse locale suggérée.

**« Faut-il démarrer un serveur sur chaque PC ? »**

Non. Un serveur héberge la partie ; les deux clients s'y connectent. Si chacun utilise son propre serveur local, ils peuvent chacun voir un lobby fonctionnel sans partager les mêmes salles.

**« Quelles vérifications faire si CONNECTING finit en erreur ? »**

Serveur réellement lancé, bonne IP de destination, bon port, même état compatible des exécutables et fichiers data, trafic UDP autorisé sur le PC serveur, absence d'isolation entre clients Wi-Fi. Une réussite en loopback ne prouve pas que le serveur est accessible depuis un autre hôte.

**« Pourquoi WSL ou une machine virtuelle compliquent-ils l'accès ? »**

Une interface virtuelle ou un NAT peut rendre l'adresse interne inaccessible directement depuis l'autre PC. Il faut vérifier la topologie et les règles de pare-feu concernées ; la documentation du dépôt propose notamment des cas WSL. Ne pas désactiver aveuglément toutes les protections pour tester.

**« Et si nous ne sommes pas sur le même réseau ? »**

Une IPv4 privée du LAN n'est normalement pas directement joignable depuis un autre réseau Internet. Il faut un chemin adapté : serveur accessible avec redirection UDP correctement configurée, ou réseau privé virtuel approprié. Le jeu ne réalise pas automatiquement la traversée du NAT et n'a pas de service de relais intégré.

**« Qu'est-ce qu'une liste de salles vide peut indiquer ? »**

Une session réseau peut être établie sur un serveur différent de celui de l'ami, ou la salle ne pas exister/avoir fermé. Distinguer échec de connexion au serveur et refus d'entrée dans une salle. Salle pleine, mauvais code et blocage des tentatives sont des problèmes applicatifs distincts d'un pare-feu.

**À montrer :** [network-play.md](../getting-started/network-play.md), [OnlineClient.cpp](../../engine/client/src/OnlineClient.cpp), [UdpTransport.cpp](../../engine/net/src/UdpTransport.cpp).

## 15. Robustesse et sécurité

**« Que contrôlez-vous avant de lire et agir ? »**

Taille 20–1 200, magic, direction, type connu, version selon les exceptions prévues, longueur exacte, champs lisibles et validation du message. Ensuite endpoint/token, déduplication/séquences pertinentes et permissions métier. Aucun de ces contrôles n'autorise à ignorer les autres.

**« Quels abus sont limités ? »**

CONNECT est limité globalement, ACCEPT espacé, les codes incorrects de salle comptés, le chat régulé, les tailles et files bornées. Un quota global peut aussi être consommé par une seule source : ce n'est pas une protection complète contre un déni de service.

**« Le token chiffre ou signe-t-il les messages ? »**

Non. Il est généré par mt19937, non cryptographique, puis transmis en clair. Le trafic, les ACK et les codes de salle ne sont pas cryptographiquement authentifiés. Une identité utilisateur persistante n'est pas fournie.

**« Peut-on annoncer un serveur sans triche ni faille ? »**

Non. L'autorité serveur bloque certaines falsifications directes du monde, pas tous les abus de commandes ou du protocole. La lecture des sources et les tests bornés ne constituent pas une preuve de sécurité exhaustive.

**« Que reste-t-il à améliorer concrètement ? »**

Télémétrie des rejets et files pleines, diagnostic des erreurs I/O, congestion/débit adaptatif, repli si delta trop grand, gestion des contextes d'événements retardés, validation des états reconstruits et compatibilité de protocole. Selon l'objectif d'exposition : identité/authentification et protection du trafic.

**« Avez-vous une administration globale et un classement persistant ? »**

Le propriétaire gère sa salle et dispose de commandes de console restreintes. Ce n'est pas un service global d'administration sécurisé avec bans et comptes. Les scores répliqués restent des états de partie, pas un classement global stocké en base.

## 16. Chiffres et fichiers à mémoriser

| Élément | État actuel |
|---|---|
| Version déclarée / nombre de types | **9 / 33**, avec attention aux évolutions incompatibles sous le même numéro. |
| Transport / port par défaut | UDP/IPv4 / 4242. |
| En-tête / limite applicative | 20 / 1 200 octets. |
| Tick rate / cadence d'états | 60 Hz / tous les trois ticks, cible de 20 Hz. |
| Inputs / heartbeat | 33 ms / 1 000 ms. |
| Relâchement forcé / timeout | 500 ms / 5 000 ms. |
| CONNECT / ACCEPT | 32 demandes par fenêtre globale de 1 s / répétition d'ACCEPT ≥200 ms. |
| Sessions / salles configurées | 192 / 48. |
| Places | Jusqu'à 4 ; règle VERSUS actuelle : 2. |
| Pseudo / chat / délai de chat | 12 / 64 octets ASCII / 500 ms. |
| Console | Textes ≤200 octets, commandes propriétaire, pas de suivi fiable général actuel. |
| Fiables en attente / IDs fiables vus | 64 / 256 par correspondant. |
| Historique snapshots serveur / client | 16 en attente / 32 états acceptés. |
| Baseline fraîche / keyframe | Moins de 1 s de ticks / après 5 s de ticks depuis le dernier complet envoyé. |
| Valeurs monde / joueur / règles / sons | 16 / 4 / 7 / 6 dans le JSON R-Type actuel. |
| Snapshot complet actuel | `132 + 8N` octets applicatifs ; 1 172 avec 130 entités. |

**Les sources à ouvrir selon la question :**

| Sujet | Fichier |
|---|---|
| Démarrage et boucle | [ServerMain.cpp](../../engine/apps/src/ServerMain.cpp), [ServerRunner.cpp](../../engine/network/server/src/ServerRunner.cpp). |
| Réception, fiabilité et états serveur | [GameServer.cpp](../../engine/network/server/src/GameServer.cpp). |
| Salles, console et chat | [GameServerRooms.cpp](../../engine/network/server/src/GameServerRooms.cpp). |
| Octets et socket | [UdpTransport.cpp](../../engine/net/src/UdpTransport.cpp), [UdpTransportImpl.cpp](../../engine/net/src/UdpTransportImpl.cpp). |
| Contrat binaire | [Protocol.hpp](../../engine/network/protocol/include/Protocol.hpp), [Codec.cpp](../../engine/network/protocol/src/Codec.cpp). |
| ACK et retransmissions | [ReliableReceiver.cpp](../../engine/network/protocol/src/ReliableReceiver.cpp), [ReliableSender.cpp](../../engine/network/protocol/src/ReliableSender.cpp). |
| Deltas | [SnapshotDiff.cpp](../../engine/network/protocol/src/SnapshotDiff.cpp), [SnapshotHistory.cpp](../../engine/network/protocol/src/SnapshotHistory.cpp). |
| Côté client | [ClientConnection.cpp](../../engine/network/client/src/ClientConnection.cpp), [SnapshotPresenter.cpp](../../engine/client/src/SnapshotPresenter.cpp). |
| Configuration actuelle | [rtype_game.json](../../game/data/rtype_game.json). |
| Preuve historique des mesures | [rapport netem](../measurements/netem-2026-11-08/report.md). |

**Méthode pour répondre à une question :** expliquer le problème, donner notre mécanisme, ouvrir la preuve et annoncer sa limite. Ne pas remplacer un résultat mesuré par une fonctionnalité simplement présente dans le code.

**Attention à l'ancienne fiche longue :** [reference-technique-reseau.md](reference-technique-reseau.md) conserve le détail historique antérieur à cette restructuration. Plusieurs chemins, valeurs et types y sont anciens ; pour l'état actuel, privilégier cette fiche, les sources liées et les références actuelles du dépôt.

## 17. Cours détaillé Asio dans notre projet

### 17.1. Ce qu'Asio fait et ce que nous ajoutons

**Asio est la bibliothèque d'I/O réseau, pas notre protocole de jeu.** Elle fournit les sockets, adresses, résolveurs, opérations asynchrones et leur exécution. Notre code ajoute le format binaire, le contrôle de session, les salles, la fiabilité sélective et la réplication du monde.

```text
Notre jeu : touches, salles, monde, chat
  → notre protocole : Packet, Codec, ACK et messages
  → notre transport : UdpTransport
  → Asio : socket et opérations UDP
  → système d'exploitation : pile IP, routage, files réseau
  → interface/réseau
```

Le projet utilise **Standalone Asio** : `#include <asio.hpp>`, espace de noms `asio`, cible CMake `asio::asio` et définition `ASIO_STANDALONE`. Boost.Asio emploierait les en-têtes/espace de noms Boost correspondants. Nous ne dépendons pas de Boost.Asio dans ce module.

**À ouvrir :** [engine/CMakeLists.txt](../../engine/CMakeLists.txt), [UdpTransportImpl.hpp](../../engine/net/internal/UdpTransportImpl.hpp), [UdpTransport.cpp](../../engine/net/src/UdpTransport.cpp).

### 17.2. Pourquoi un wrapper et une implémentation privée ?

L'en-tête public UdpTransport expose nos propres types Endpoint et Datagram. Son membre est `std::unique_ptr<Impl>`, avec la classe Impl seulement déclarée. Les détails et types Asio sont dans l'en-tête privé : c'est **Pimpl**, pour cacher l'implémentation.

Le code de jeu ne doit donc pas manipuler directement `asio::ip::udp::socket`. Il demande « envoie ces octets à cet endpoint » ou « donne les datagrammes déjà reçus ». Asio est une dépendance privée de la cible de transport.

Le transport ne peut pas être copié : copier arbitrairement l'objet qui possède socket et thread rendrait la propriété des ressources ambiguë. Le unique_ptr et les destructeurs participent à la gestion **RAII** des ressources.

Les callbacks préparés par `std::bind_front(&Impl::onReceive, this)` conservent l'adresse de l'Impl, pas une copie de l'objet ni un shared_ptr vers lui. Son existence doit donc couvrir l'exécution des callbacks. L'arrêt puis le join avant la destruction des membres font partie de cette discipline de durée de vie.

### 17.3. Inventaire des éléments Asio utilisés

| Élément exact | Définition | Emploi chez nous |
|---|---|---|
| `asio::io_context` | Contexte d'exécution des opérations/handlers. | Contexte durable du transport et contextes temporaires pour résolution/adresse locale. |
| `asio::ip::udp` | Types et protocole UDP. | Construction des sockets/endpoints et choix IPv4. |
| `asio::ip::udp::v4()` | Désigne UDP sur IPv4. | Socket locale, résolution et sonde d'adresse. |
| `asio::ip::udp::socket` | Ressource socket avec interface C++. | Une socket durable par transport. |
| `asio::ip::udp::endpoint` | Adresse IP et port typés. | Destination, liaison locale et expéditeur reçu. |
| `asio::ip::make_address_v4()` | Parse une IPv4 numérique. | Conversion d'un texte en adresse. |
| `asio::ip::udp::resolver` | Résout un host/service en endpoints. | Résolution synchrone d'un nom vers IPv4. |
| `resolver.resolve()` | Effectue cette résolution. | Première IPv4 obtenue utilisée par notre resolve(). |
| `socket.open()` | Ouvre explicitement une socket. | Socket temporaire de primaryLocalAddress(). |
| `socket.connect()` | Définit un correspondant UDP. | Sonde de route, pas connexion applicative des joueurs. |
| `socket.local_endpoint()` | Lit l'adresse/port locaux de la socket. | Affichage d'une adresse locale suggérée au lancement. |
| `asio::buffer()` | Décrit une zone mémoire, sans en devenir propriétaire. | Tampon de réception ou octets d'envoi. |
| `async_receive_from()` | Programme une réception avec source. | Enchaînement continu des réceptions du worker. |
| `async_send_to()` | Programme un envoi vers une destination. | Envoi des vecteurs d'octets. |
| `asio::post()` | Soumet une fonction à exécuter par le contexte. | Passage d'une demande d'envoi au worker. |
| `io_context::run()` | Exécute les handlers et attend le travail restant. | Appelé dans notre thread réseau. |
| `io_context::stop()` | Demande l'arrêt des appels run. | Arrêt du transport. |
| `asio::error_code` | Résultat d'erreur sans exception de l'opération concernée. | Résolution, sonde d'adresse et callbacks. |
| `asio::system_error` | Exception système Asio. | Traduction d'une erreur de construction en NetError. |
| `asio::error::operation_aborted` | Annulation d'une opération. | Ne pas réarmer la réception après cette annulation. |

`std::bind_front`, thread, mutex et shared_ptr appartiennent à C++, pas à Asio. Leurs rôles sont néanmoins indispensables à notre utilisation de la bibliothèque.

### 17.4. Membres du transport et propriété des données

| Membre de Impl | Propriété / rôle |
|---|---|
| `m_io` | Contexte Asio du transport ; déclaré avant la socket qu'il sert. |
| `m_socket` | Socket UDP durable. |
| `m_buffer` | Array de réception réutilisé, de 1 201 octets. |
| `m_sender` | Endpoint qu'Asio remplit à la réception. |
| `m_mutex` | Verrou des données partagées protégées. |
| `m_incoming` | Vector de Datagram en attente du thread principal. |
| `m_failure` | Exception du worker éventuellement conservée. |
| `m_pending` | Compteur atomique des envois locaux en attente. |
| `m_worker` | Thread exécutant m_io.run(). |

Un transport est partagé entre le thread principal et son worker. Les sessions, salles, la simulation et les objets graphiques restent dans le thread principal. Ce n'est pas un partage de tout l'ECS entre les deux threads.

### 17.5. Construction : ouverture, bind et démarrage du worker

Dans [UdpTransportImpl.cpp](../../engine/net/src/UdpTransportImpl.cpp) :

```cpp
UdpTransport::Impl::Impl(std::uint16_t port)
    : m_socket(m_io, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)),
      m_buffer(), m_pending(0)
{
    receive();
    m_worker = std::thread(&Impl::runIo, this);
}
```

1. Construire la socket avec cet endpoint local l'ouvre et la lie à l'adresse IPv4 locale générique et au port.
2. Côté serveur, le port vient de la CLI ou du JSON. L'adresse générique permet l'écoute sur les interfaces IPv4 locales.
3. Côté client, le port local 0 laisse le système choisir un port éphémère.
4. `receive()` programme une première réception avant le lancement du worker.
5. Le worker appelle runIo(), puis m_io.run().

On n'appelle pas `listen()`/`accept()` : ces opérations de connexion TCP ne correspondent pas à notre modèle UDP. ACCEPT est chez nous un message métier.

### 17.6. io_context n'est pas un thread

`m_io` contient les mécanismes d'exécution, mais ne lance pas magiquement un thread de notre application. C'est `std::thread(&Impl::runIo, this)` qui exécute run(). Dans notre transport, un seul thread appelle run() ; les handlers de ce contexte ne sont donc pas exécutés simultanément par un pool de workers.

run() peut attendre les événements : **ce blocage du worker n'est pas un blocage de la boucle de jeu**. La réception asynchrone maintient du travail en cours et est réarmée après les terminaisons ordinaires. Nous n'utilisons pas d'executor_work_guard pour maintenir artificiellement un contexte sans travail, ni de strand ou de coroutine co_await.

Ne pas appeler run() sans comprendre sa durée de vie : il peut retourner quand le travail est terminé ou après stop(), et une exception de handler peut en sortir. Notre runIo() intercepte les exceptions et les mémorise ; il ne relance pas automatiquement run() après un échec fatal du worker.

### 17.7. Réception complète, avec les arguments et le callback

```cpp
m_socket.async_receive_from(
    asio::buffer(m_buffer),
    m_sender,
    std::bind_front(&Impl::onReceive, this));
```

- `asio::buffer(m_buffer)` désigne les octets disponibles ; il ne les copie pas et ne les possède pas.
- `m_sender` recevra l'endpoint source.
- `bind_front` fixe l'objet sur lequel appeler onReceive ; Asio fournira error et size à la terminaison.
- Le résultat peut être connu bien après le retour de receive(). Les objets nécessaires doivent rester vivants.

```cpp
void UdpTransport::Impl::onReceive(const asio::error_code &error, std::size_t size)
{
    if (!error && size <= MAX_DATAGRAM_SIZE)
        store(size);
    if (error != asio::error::operation_aborted)
        receive();
}
```

Une réception sans erreur et dans la limite est stockée. Les autres ne produisent pas de Datagram applicatif dans ce callback. Sauf annulation, une nouvelle réception est programmée. Il n'y a qu'une réception de cet enchaînement en attente à la fois, ce qui permet de réutiliser m_buffer et m_sender.

Les callbacks ne décodent pas le jeu : ils ne vérifient ni token, ni ACK, ni règle de salle. Ces contrôles auront lieu dans le thread principal.

### 17.8. Pourquoi copier à la réception ? Pourquoi swap ensuite ?

store() construit un Datagram contenant notre Endpoint et un nouveau vector d'octets copiés depuis m_buffer. Cette copie sépare le paquet conservé du tampon que la réception suivante va écraser.

```cpp
std::lock_guard lock(m_mutex);
std::vector<Datagram> incoming;
incoming.swap(m_incoming);
return incoming;
```

Avant swap, m_incoming possède les paquets et incoming est vide ; après, c'est l'inverse. Les vectors échangent leurs ressources sans recopier individuellement tous les paquets. Le verrou reste acquis jusqu'à la sortie de la fonction ; le décodage du lot retourné se déroule ensuite hors verrou.

**Ce n'est pas du zero-copy de bout en bout** : store() a bien effectué une copie des octets. L'avantage de swap est de ne pas recopier encore tout le lot lors de sa collecte.

### 17.9. Envoi complet et passage au worker

```cpp
if (m_pending.fetch_add(1) >= MAX_QUEUE) {
    m_pending.fetch_sub(1);
    return;
}
asio::post(m_io,
    std::bind_front(&Impl::startSend, this, destination,
        std::make_shared<std::vector<std::uint8_t>>(std::move(bytes))));
```

1. Le thread appelant construit la destination Asio depuis notre Endpoint.
2. Il réserve atomiquement une place d'envoi ; au-delà de la limite, il annule la réservation et abandonne la demande.
3. Il transfère les octets dans un vector possédé par shared_ptr.
4. post() soumet startSend au contexte. La tâche n'est pas exécutée inline dans l'appelant par post() ; le worker peut l'exécuter concurremment quand son contexte la prend en charge.
5. startSend appelle async_send_to(). Son callback retient aussi le shared_ptr.
6. onSent décrémente le nombre d'envois en attente.

`std::move()` autorise le transfert des ressources du vector ; `shared_ptr` maintient ensuite le tampon jusqu'à la fin de l'opération. `asio::buffer(*data)` seul ne suffirait pas à garantir cette durée de vie.

**Trois événements différents :** le retour de send(), la fin de l'envoi local et la réception distante. Seul notre échange d'ACK peut reconnaître la réception des paquets concernés ; Asio n'ajoute pas cette fiabilité applicative à UDP.

### 17.10. Compteur atomique, mutex et saturation

`fetch_add(1)` incrémente m_pending et retourne **son ancienne valeur**. À 255, il devient 256 et l'envoi est autorisé. À 256, il devient temporairement 257 puis fetch_sub annule la réservation ; aucun nouvel envoi n'est programmé par cette branche.

Le callback diminue le compteur même si l'opération rapporte une erreur : il compte la fin des opérations, pas les livraisons réussies. Le destructeur lit ce compteur avec load(). Une variable atomique n'autorise pas des accès concurrents non protégés au vector m_incoming : ce sont deux protections distinctes.

À la réception, store() abandonne le nouveau datagramme lorsque m_incoming contient déjà 256 entrées. Les files sont donc bornées par notre politique, pas par une capacité immuable intrinsèque de std::vector. Ces plafonds ne remplacent ni les buffers du noyau ni un contrôle adaptatif de congestion.

### 17.11. Opérations synchrones : resolve et adresse locale

resolve() essaie d'abord une IPv4 numérique avec make_address_v4. Sinon, un io_context temporaire et un resolver effectuent une résolution **synchrone** vers IPv4. Le code prend la première adresse renvoyée. Il ne faut pas dire « tout est asynchrone dans le réseau » ; aucune invocation séparée de run() n'est nécessaire pour cette opération synchrone.

primaryLocalAddress() crée une autre socket temporaire, l'ouvre et la connecte à `8.8.8.8:53`, puis demande local_endpoint(). Ce code n'envoie pas de requête DNS ni de paquet de jeu à Google : il exploite le choix de route/adresse source du système.

Un connect UDP n'est pas un handshake TCP. L'adresse obtenue n'est ni la preuve que le PC de l'ami est joignable, ni la découverte de l'adresse publique après NAT. VPN, plusieurs cartes ou VM peuvent rendre la suggestion peu adaptée au LAN concerné. Cette fonction sert au log de démarrage ; le transport serveur n'est pas limité à cette adresse suggérée.

### 17.12. Gestion des erreurs et limites précises

| Situation | Comportement actuel |
|---|---|
| Exception système à la construction | UdpTransport traduit en NetError avec le port. |
| Échec de résolution | NetError avec le host. |
| Échec de la sonde d'adresse locale | nullopt, pas forcément arrêt du serveur. |
| Callback de réception avec erreur | Ne stocke pas ce résultat ; réarme hors operation_aborted. |
| Callback d'envoi avec erreur | Décrémente m_pending ; le code d'erreur est actuellement ignoré. |
| Exception sortant de m_io.run() | runIo l'intercepte, fail() mémorise une exception sous mutex. |
| Prochain send()/poll()/checkHealth() | Peut repropager cette exception au thread principal. |
| File pleine | Abandon sans erreur détaillée par paquet. |

`checkHealth()` ne teste pas si l'autre PC répond. Un envoi UDP peut localement terminer sans qu'un serveur distant soit disponible. Les timeouts applicatifs détectent le silence.

Il reste des possibilités d'amélioration : compteurs de saturation/erreurs, traitement détaillé des codes d'erreur et stratégie de reprise du worker. Par exemple, la réservation m_pending n'est pas entourée d'une annulation générale pour toute exception possible survenant pendant la préparation/soumission d'un envoi ; il ne faut pas prétendre que tous les chemins d'échec ont un diagnostic et une récupération parfaits.

### 17.13. Arrêt et durée de vie

Le destructeur attend au plus 200 ms les opérations locales en attente, avec des pauses de 1 ms. Il appelle ensuite m_io.stop() puis join() si le worker est joignable. Les membres sont finalement détruits, dont la socket et le contexte.

stop() ne signifie pas « vide tous les messages et reçois leurs ACK ». Il peut empêcher l'exécution de handlers encore en attente. La destruction de la socket libère la ressource système ; notre code ne fait pas un close POSIX explicite dans les fichiers de jeu.

Le préavis SERVER_SHUTDOWN et DISCONNECT reste un meilleur effort, avec timeout en secours. Le délai de vidange local de 200 ms n'est pas le timeout de session de cinq secondes et ne constitue pas une garantie réseau.

### 17.14. Les noms proches à ne pas confondre

| Terme rencontré | Ce qu'il signifie ici |
|---|---|
| UdpTransport::poll() | Collecter nos Datagram déjà reçus. |
| GameServer::poll() | Expirer les sessions, traiter les paquets et les retransmissions. |
| io_context::run() | Faire exécuter les handlers Asio par un thread. |
| Appel système poll()/epoll | Mécanismes bas niveau non programmés directement dans notre transport. |
| Message ACCEPT | Réponse applicative à CONNECT. |
| Appel TCP accept() | Pas utilisé directement pour les joueurs UDP. |
| Socket UDP connect() de sonde | Choix de correspondant/route pour la socket temporaire. |
| CONNECT du protocole | Début de notre session applicative avec pseudo. |

Nous n'implémentons pas de reactor/proactor maison, de thread par client, de strand, de pool de workers I/O ou de coroutines réseau. Le backend système exact d'Asio dépend de sa construction et de la plateforme : sans vérification de celle-ci, ne pas annoncer qu'on a écrit/utilisé explicitement epoll ou IOCP dans notre code.

**Réponse orale à savoir faire :** « Notre wrapper Asio possède une socket IPv4 et un io_context exécuté par un worker. Les callbacks reçoivent et mettent les octets en file ; le thread principal récupère le lot sous mutex et traite le protocole. Les envois sont postés au worker et leurs tampons restent vivants jusqu'à la fin de l'opération. La fiabilité des messages de jeu est ajoutée par nos classes, pas par UDP ou Asio. »

## 18. Cours détaillé netem et notre banc de mesure

### 18.1. Définition et place dans le parcours

**netem est un émulateur de conditions réseau du noyau Linux.** Il ne modifie pas le code du jeu et ne remplace pas le serveur. L'outil `tc` configure une qdisc, une discipline de file d'attente attachée à une interface, qui peut retarder, perdre, dupliquer ou réordonner les paquets et limiter leur débit.

```text
ClientConnection réel
  → notre Codec et UdpTransport
  → socket et pile réseau Linux
  → qdisc netem : traitement des paquets sortants de l'interface
  → réception par l'autre socket
  → notre transport et notre protocole
```

Contrairement à la sonde Python qui peut ignorer des réceptions dans son propre programme, les perturbations netem interviennent avant que notre application reçoive les datagrammes. Dans notre banc, les clients C++ ne suppriment pas volontairement les paquets reçus.

**Sources :** [netem_test.py](../../scripts/netem_test.py), [netem_client.cpp](../../scripts/netem_client.cpp), [instructions de mesure](../measurements/index.md). Les paramètres netem ont aussi été vérifiés dans le manuel système `tc-netem(8)` disponible sur la machine.

### 18.2. Pourquoi un namespace réseau isolé ?

Les essais utilisent un namespace réseau jetable : sa boucle locale `lo`, ses interfaces et sa configuration sont distinctes du réseau hôte. Le serveur et les deux clients de test sont lancés à l'intérieur. Ils continuent à utiliser 127.0.0.1, mais il s'agit du loopback de cet environnement isolé.

```sh
NETEM_HOST_NS=$(readlink /proc/self/ns/net)
unshare --user --map-root-user --net \
  python3 scripts/netem_test.py --host-netns "$NETEM_HOST_NS" \
  --server build/bin/r-type_server \
  --client build/bin/netem_client \
  --seconds 20 --output docs/measurements/my-run
```

- `readlink /proc/self/ns/net` retient l'identité du namespace avant isolation.
- `--user --map-root-user` crée un contexte utilisateur où le processus est root dans l'espace créé ; ce n'est pas une connexion root générale au PC hôte.
- `--net` crée le namespace réseau.
- Le script compare son namespace courant avec celui fourni, puis active son propre lo avec `ip link set lo up`.
- Les permissions du système doivent autoriser cette isolation et l'administration réseau dans ce contexte. `tc`/`ip` viennent d'iproute2 ; le noyau doit fournir netem.

La vérification repose sur l'identifiant fourni par l'appelant ; ce n'est pas une preuve indépendante contre un utilisateur qui transmettrait volontairement une mauvaise valeur. Pour le banc, on passe réellement l'identifiant obtenu avant unshare.

La destruction du namespace supprime sa configuration. La portée de nos commandes est donc limitée à l'environnement de test lorsque cette procédure est respectée.

### 18.3. Lire exactement notre commande tc

Le script construit :

```sh
tc qdisc add dev lo root netem limit 1000 <options du scénario>
```

| Mot | Signification |
|---|---|
| `tc` | Outil Linux de traffic control. |
| `qdisc` | Discipline de traitement de la file de paquets. |
| `add` | Ajouter la discipline. Le script tente de supprimer la précédente avant. |
| `dev lo` | Interface visée : boucle locale du namespace. |
| `root` | Position racine de la qdisc dans la hiérarchie, pas un mot qui rend le processus administrateur. |
| `netem` | Type de discipline appliqué. |
| `limit 1000` | Au plus 1 000 paquets dans cette qdisc ; ce n'est pas 1 000 octets ni 1 000 paquets/s. |

**Ne pas confondre trois niveaux de limites :**

- netem `limit 1000` : paquets en attente dans l'émulateur du noyau ;
- transport `MAX_QUEUE=256` : datagrammes reçus ou envois locaux suivis dans notre application ;
- protocole `MAX_PACKET_BYTES=1200` : octets applicatifs d'un seul datagramme.

Dans notre configuration, netem traite tout le trafic pertinent passant par lo dans ce namespace, pas seulement les snapshots : CONNECT, ACK portés par les paquets, inputs, événements, etc. Aucun filtre ne cible uniquement un type de message.

### 18.4. Les sept scénarios, sans inventer d'autres options

Chaque ligne est ajoutée après `netem limit 1000` :

| Nom dans CASES | Options exactes | Ce qu'on met à l'épreuve |
|---|---|---|
| `baseline` | Aucune option supplémentaire. | Référence sans perturbation volontaire ajoutée ; la qdisc possède quand même sa limite. |
| `delay_100ms` | `delay 100ms` | Délai constant ajouté à chaque trajet. |
| `loss_10pct` | `loss 10%` | Perte aléatoire de paquets. |
| `duplicate_10pct` | `duplicate 10%` | Arrivées répétées et déduplication. |
| `reorder_25pct` | `delay 50ms reorder 25% 50%` | Paquets qui se dépassent, avec corrélation. |
| `combined` | `delay 80ms 20ms distribution normal loss 5% duplicate 2% reorder 25% 50%` | Délai variable, pertes, répétitions et désordre ensemble. |
| `rate_128kbit` | `delay 30ms rate 128kbit` | Délai et sérialisation/attente sous débit limité. |

Le banc ne configure pas de corruption de bits, de modèle de pertes en rafales Gilbert-Elliott, de coupure à 100 %, de slots temporels, ni de graine aléatoire fixe. Ce sont des expériences possibles, pas les sept essais enregistrés.

### 18.5. Pourquoi 100 ms donnent environ 200 ms de ping

netem agit sur la sortie de l'interface choisie. Ici les deux sens passent par la même interface lo, puisque serveur et clients sont dans ce namespace : chaque trajet traverse la qdisc.

```text
Client → serveur : environ +100 ms
Serveur → client : environ +100 ms
RTT              : environ +200 ms, plus l'attente et le traitement
```

Ce n'est pas une règle affirmant que `delay 100ms` ajoute toujours 200 ms quel que soit le montage. Si seule une interface d'un seul hôte était perturbée, le raisonnement serait différent. Notre topologie explique la mesure du rapport.

### 18.6. Pertes : paramètre configuré et perte observée

`loss 10%` désigne une probabilité de perte configurée, pas exactement un paquet sur dix dans chaque échantillon de vingt secondes. Le taux réellement observé fluctue, et pertes des inputs, ACK et états n'ont pas les mêmes conséquences.

Un raisonnement illustratif pour une requête et sa réponse avec deux pertes indépendantes de 10 % serait `0,9 × 0,9 = 0,81` de probabilité que les deux paquets passent sans nouvelle tentative. Ce n'est pas notre mesure d'un taux de succès métier : répétitions, ACK et snapshots suivants changent le résultat applicatif.

Une fréquence d'environ 18 états/s au lieu de 20 ne suffit pas à déduire tous les paquets perdus, car certains sont filtrés, des deltas peuvent manquer de baseline et les compteurs regroupent plusieurs types de trafic.

### 18.7. Réordonnancement et jitter : lecture précise

`delay 50ms reorder 25% 50%` laisse sortir certains paquets plus tôt que les paquets retardés, ce qui permet de dépasser un paquet précédent. `25%` est le paramètre de cette décision de réordonnancement ; le second `50%` est sa **corrélation**, pas une seconde perte ni un délai de 50 ms.

Le délai est nécessaire au mécanisme de réordonnancement configuré. La proportion de messages de jeu effectivement vus en désordre dépend aussi de leurs instants d'émission : ne pas transformer le paramètre 25 % en un résultat applicatif mesuré de 25 %.

Dans le scénario combiné, `delay 80ms 20ms distribution normal` ajoute une variation de délai selon la distribution choisie. La notation commode « 80 ± 20 ms » ne signifie pas que tous les délais restent strictement entre 60 et 100 ms avec une distribution normale. Les 50 % de corrélation placés après reorder concernent ce paramètre, pas automatiquement toutes les autres perturbations.

### 18.8. Que signifie rate 128kbit ?

128 kbit/s correspond nominalement à 128 000 bits/s, soit 16 000 octets/s. netem ajoute une attente dépendant de la taille des paquets et du débit configuré. Le timing réel dépend aussi des horloges et de l'ordonnancement du noyau.

Chez nous, ce débit est **partagé sur la qdisc de lo** par le trafic des deux clients et du serveur. Ce n'est pas une allocation de 128 kbit/s par joueur ou par sens. Les octets comptabilisés à ce niveau ne sont pas identiques au seul payload métier ; nous n'ajoutons pas de paramètre explicite d'overhead/cellule dans la commande rate.

Un PASS sous 128 kbit/s ne démontre pas que le lien a été saturé. Un trafic inférieur au plafond peut passer sans congestion importante. Pour étudier la saturation, il faudrait mesurer la charge, l'attente, les pertes et varier le plafond ou la charge.

### 18.9. Déroulement exact de netem_test.py

1. Lire les chemins serveur/client, la durée, la sortie et éventuellement les scénarios choisis avec `--case`.
2. Vérifier le namespace et enregistrer métadonnées : date UTC, noyau, commit, git status, hashes des exécutables/configuration et version de tc.
3. Activer lo.
4. Pour chaque scénario, supprimer l'ancienne qdisc si présente puis ajouter les paramètres voulus.
5. Lancer un **nouveau serveur**, avec le port `44440 + indice du scénario dans cette exécution`.
6. Attendre une seconde et vérifier qu'il n'a pas déjà quitté.
7. Lancer la sonde C++ avec chemin du JSON, port et durée de mesure.
8. Lire son enregistrement `NETEM_RESULT`, vérifier ses codes de sortie et celui du serveur.
9. Terminer le serveur ; attendre jusqu'à cinq secondes, puis le tuer si nécessaire.
10. Enregistrer `tc -s qdisc show dev lo`, puis results.json et report.md.
11. Retirer la qdisc dans le finally, même si le parcours sort avec une erreur prise en charge.

La limite du processus sonde est `durée + 25 s` ; elle est distincte des vingt secondes de mesure. Les temps de démarrage et de préparation de salle s'ajoutent. Le port ne représente pas un état persistant entre scénarios et les sessions ne sont pas réutilisées d'un serveur à l'autre.

### 18.10. Ce que fait réellement le client C++ de test

La sonde contient deux Player, chacun avec son vrai ClientConnection, donc son transport et son worker. Elle fournit maintenant les noms probe1/probe2 au constructeur. Le programme de sonde a ainsi un thread principal et deux workers réseau ; le serveur est un autre processus avec son propre transport.

Préparation : connecter les deux clients, créer une salle depuis A, faire rejoindre B, demander le démarrage et attendre les deux événements GAME_STARTED. Le contrôle de préparation comporte un délai de 18 secondes, mais une exception du ClientConnection peut l'interrompre avant.

Les commandes créer/rejoindre/démarrer ne sont émises qu'une fois par la logique de sonde ; leurs éventuelles retransmissions doivent venir de la couche fiable C++. Les règles de création sont un vector de la longueur configurée rempli de zéros, donc les premiers choix de chaque règle du JSON.

Après cette préparation, la mesure commence : update() est appelée en boucle avec BUTTON_FIRE et une pause de 2 ms environ. Cette cadence d'appel n'est pas la cadence d'envoi des inputs : ClientConnection conserve ses propres périodes. L'ancien snapshot éventuellement pris pendant la préparation ne fait pas partie du comptage de mesure.

La sonde recueille takeSnapshot(), les événements ROOM_JOINED/GAME_STARTED et les compteurs réseau ; elle échantillonne le dernier ping environ une fois par seconde. Elle ne rend pas d'images, n'envoie pas de conversation de chat, ne valide pas la console et ne simule pas le comportement complet d'un utilisateur.

### 18.11. Définition exacte de PASS

Pour chaque client, le code exige :

```cpp
p.snapshots >= 10 && p.backwards == 0
    && p.joinedEvents == 1 && p.startedEvents == 1
    && p.maxGap < 5
```

Les deux doivent satisfaire le critère. Une exception, un serveur arrêté pendant l'essai ou un échec de processus peut aussi faire échouer le scénario dans le runner.

**Cela ne teste pas une promesse de performance maximale.** Le seuil de dix snapshots sur vingt secondes est un critère minimal de fonctionnement, pas « au moins 19 états/s ». Aucun plafond de RTT ni seuil explicite de cadence de simulation n'entre dans cette expression de PASS.

Les ticks viennent des états acceptés par le vrai ClientConnection : le test vérifie le parcours appliqué, pas le nombre brut de datagrammes que le noyau aurait présentés avant filtrage. Le décompte exact d'événements suppose un scénario avec une seule entrée/démarrage ; une évolution qui produit légitimement un nouveau ROOM_JOINED en fin de partie doit être prise en compte avant d'accuser la déduplication.

### 18.12. Calcul des métriques et unités

| Champ de résultat | Calcul / interprétation |
|---|---|
| `setup_seconds` | Temps jusqu'à la préparation complète des deux clients. |
| `measurement_seconds` | Durée réellement écoulée dans la boucle de mesure. |
| `snapshots` | Nombre de snapshots fournis par takeSnapshot pendant la mesure. |
| `snapshots_per_second` | Ce nombre divisé par la durée de mesure. |
| `simulation_ticks_per_second` | Écart de tick entre premier et dernier état divisé par le temps entre leurs réceptions. Estimation réseau, pas durée CPU d'un tick. |
| `max_snapshot_gap_seconds` | Plus grand intervalle, y compris début→premier et dernier→fin. |
| `backwards_snapshots` | Décompte d'états livrés dont le tick ne progresse pas. |
| `room_joined_events`, `game_started_events` | Événements de ces types livrés à l'application. |
| `rx_bytes_per_second`, `tx_bytes_per_second` | Différences de compteurs / durée. Incluent les en-têtes de notre protocole, pas UDP/IP/Ethernet. |
| `ping_samples_ms` | Dernière valeur heartbeat observée à chaque échantillonnage. |
| `ping_median_ms`, `ping_max_ms` | Statistiques de ces échantillons, pas de chaque paquet. |

Un ping peut être réutilisé si la prochaine réponse heartbeat se perd ; les échantillons ne sont pas forcément des nouvelles mesures indépendantes. Les compteurs entrants incluent des octets depuis la source attendue avant certaines validations ; les sortants sont comptés avant une éventuelle perte/abandon plus bas. On ne peut pas assimiler ces valeurs à une capture exacte des octets sur un câble.

### 18.13. Lire les fichiers produits

| Fichier | Utilité |
|---|---|
| `<scénario>.server.log` | Démarrage, connexions, salle, simulation et erreurs visibles du serveur. |
| `<scénario>.client.log` | Messages de la sonde, NETEM_READY et NETEM_RESULT sur son parcours normal. |
| `<scénario>.qdisc.txt` | Paramètres actifs et statistiques noyau de la qdisc après l'essai. |
| `results.json` | Métadonnées, commandes, résultats numériques et statistiques. |
| `report.md` | Tableau de lecture et critères/limites. |

Dans `tc -s`, Sent, dropped et backlog concernent la qdisc, tous les flux/types concernés. Des doublons et des pertes font aussi varier ses comptes. `backlog 10p` signifie des paquets encore en attente, pas dix messages fiables manquants. Les timestamps, le protocole et les critères doivent être rapprochés pour expliquer une panne.

Les métadonnées et hashes aident à identifier le binaire essayé. Sans graine imposée, avec un noyau non temps réel et des timings variables, deux exécutions ne produisent pas nécessairement les mêmes chiffres. Les fichiers bruts sont actuellement ignorés par les règles Git ; seul le rapport historique est conservé dans le dépôt.

### 18.14. Résultats historiques complets

Rapport : [8 novembre 2026](../measurements/netem-2026-11-08/report.md), **protocole version 8 de cette époque**.

| Scénario | Verdict historique | RTT médian A/B (ms) | États/s A/B | Réception A/B (KiB/s) | Intervalle max A/B (s) |
|---|---|---|---|---|---|
| baseline | PASS | 2 / 2 | 20 / 20 | 2,04 / 2,04 | 0,05 / 0,05 |
| delay_100ms | PASS | 203 / 202 | 20 / 20 | 3,76 / 3,76 | 0,05 / 0,05 |
| loss_10pct | PASS | 2 / 2 | 18,15 / 18,20 | 1,84 / 1,84 | 0,15 / 0,20 |
| duplicate_10pct | PASS | 2 / 2 | 20 / 20 | 2,20 / 2,21 | 0,05 / 0,05 |
| reorder_25pct | PASS | 102 / 103 | 18,65 / 18,90 | 2,66 / 2,66 | 0,10 / 0,10 |
| combined | PASS | 163 / 155 | 17,15 / 17,25 | 2,66 / 2,69 | 0,19 / 0,15 |
| rate_128kbit | PASS | 88 / 86 | 20 / 20 | 2,72 / 2,74 | 0,06 / 0,06 |

**Lecture correcte :** dans ces essais courts, les clients restent dans le parcours de partie et reçoivent des états exploitables malgré les perturbations. Les pertes réduisent les états appliqués ; le délai peut conserver leur régularité en retardant tout le flux. Un trafic plus élevé sous latence peut avoir plusieurs causes, dont les baselines plus anciennes et les retransmissions : le tableau ne les quantifie pas séparément.

**Ce que cela ne valide pas :** chat/console actuels, modifications du gameplay et des valeurs répliquées, charge à 192 clients, longue durée, sécurité, taux exact de perte applicative et ressenti visuel. Le banc est sans fenêtre ; il ne mesure pas touche→pixel. Les commandes du script actuel peuvent être rejouées après compilation, mais aucun nouveau PASS n'a été produit pour rédiger cette section.

### 18.15. Questions pièges à savoir traiter

- **« netem remplace Asio ? »** Non : le noyau perturbe les paquets du code qui continue à utiliser Asio.
- **« 10 % de pertes = 10 % des actions perdues ? »** Non : actions fiables et états périodiques réagissent différemment ; le paramètre est aléatoire.
- **« Reorder 25% 50% signifie 75 % de désordre ? »** Non : probabilité et corrélation sont deux paramètres différents.
- **« Les 1 000 paquets de netem sont les 256 de notre transport ? »** Non : noyau et application sont deux files distinctes.
- **« PASS garantit 60 ticks/s ou une bonne jouabilité ? »** Non : lire les critères exacts et les métriques séparément.
- **« 128 kbit/s est le débit reçu par chacun ? »** Non : plafond partagé de la qdisc, pas mesure applicative par joueur.
- **« Pourquoi ne pas annoncer nos sept PASS comme tests du chat actuel ? »** Ils précèdent son ajout et ne contiennent pas ce scénario.

**Réponse orale à savoir faire :** « Nous utilisons tc netem sur la boucle locale d'un namespace isolé. Le serveur et deux clients C++ réels subissent des délais, pertes, doublons, désordre ou débit limité, dans les deux sens. Le banc mesure états, RTT et octets et vérifie des critères de fonctionnement précis. Les sept PASS conservés concernent la version historique testée ; il faut recompiler et rejouer pour les évolutions actuelles. »

## 19. Cours détaillé tailles de paquets et calculs

### 19.1. Toujours préciser de quelle taille on parle

```text
Message métier en mémoire C++
  → sérialisation : en-tête de notre protocole + payload
  → ce bloc devient le payload d'un datagramme UDP
  → UDP ajoute son en-tête, puis IP le sien
  → le réseau local/tunnel peut ajouter d'autres en-têtes
```

| Niveau | Exemple INPUT | Exemple de complet à 130 entités |
|---|---:|---:|
| Payload de notre message | 5 octets. | 1 152 octets. |
| Paquet applicatif, header du jeu inclus | **25 octets**. | **1 172 octets**. |
| Datagramme UDP, en-tête UDP de 8 inclus | 33 octets. | 1 180 octets. |
| Paquet IPv4 sans options, en-tête IP de 20 inclus | **53 octets**. | **1 200 octets**. |
| Trame Ethernet II sans VLAN, header 14 et FCS 4 inclus, à titre illustratif | 71 octets. | 1 218 octets. |

La ligne Ethernet est une illustration de réseau Ethernet, pas une description d'une trame réellement capturée sur notre loopback de test. Wi-Fi, tunnels, VLAN, préambule et temps intertrame changent encore le coût. Les compteurs ClientConnection et le rapport netem n'incluent pas ces en-têtes réseau.

Le maximum autorisé du **paquet applicatif** est 1 200, donc un tel paquet maximal donnerait **1 228 octets IPv4 sans options**. Ne pas confondre cela avec le complet actuel de 1 172 qui, lui, donne 1 200 au niveau IP : cette égalité numérique ne définit pas notre limite.

### 19.2. Pourquoi 1 200, MTU, fragmentation et marge

La limite est déclarée à la fois dans Protocol::MAX_PACKET_BYTES et UdpTransport::MAX_DATAGRAM_SIZE. Elle inclut notre header de vingt octets ; le payload métier dispose donc d'au plus **1 180 octets**.

Sur un lien à MTU IP de 1 500, IPv4 sans options et UDP laissent `1500 − 20 − 8 = 1472` octets de payload UDP. Un budget applicatif de 1 200 conserve une marge dans ce cas. Cette marge ne prouve pas que tous les chemins/tunnels Internet ont une MTU suffisante.

Fragmentation IP signifie que le paquet doit être découpé plus bas, avec un coût et un risque supplémentaires si un fragment manque. Notre protocole n'a pas de messages de fragmentation/réassemblage applicatifs. Il n'envoie pas un gros snapshot en plusieurs morceaux automatiquement.

Ni la largeur u16 de payloadSize ni la limite théorique d'UDP ne nous autorisent à ignorer 1 200 : c'est une règle plus stricte du moteur. La MTU, le maximum d'un datagramme et la taille d'une structure C++ sont trois notions différentes. MSS concerne les segments TCP, pas une limite que nous avons configurée pour nos messages UDP.

### 19.3. Où les limites sont-elles contrôlées ?

| Emplacement | Contrôle |
|---|---|
| `NetworkSettings::checkPacketFits()` | Taille maximale calculée d'un complet et d'une liste de salles au chargement du jeu. |
| `Codec::encode()` | Validité du message, puis ByteWriter construit avec budget 1 200. |
| `ByteWriter::writeBigEndian()` | Refuse toute écriture qui dépasserait le budget. |
| `UdpTransport::send()` | Refuse un vector de plus de 1 200 octets. |
| `m_buffer` de réception | 1 201 octets, pour distinguer notamment un dépassement juste au-delà de 1 200. |
| `onReceive()` | Stocke seulement sans erreur et avec size ≤1 200. |
| `Codec::decode()` | Refuse moins de 20 ou plus de 1 200, puis vérifie payloadSize et lecture exacte. |

Un datagramme plus gros que le tampon peut aussi être tronqué/signaler une erreur selon le système ; le byte supplémentaire et le contrôle d'erreur empêchent de traiter simplement 1 201 octets comme un paquet valide de 1 200. La validation exacte du protocole reste nécessaire après réception.

Les durées et limites de file sont contrôlées ailleurs. On peut avoir un paquet valide de 25 octets abandonné parce que la file est pleine, ou un paquet trop grand refusé alors que la file est vide.

### 19.4. Les vingt octets du header, addition et offsets

```text
magic:u16       2
version:u8      1
type:u8         1
sequence:u32    4
ack:u32         4
ackBits:u32     4
reliableId:u16  2
payloadSize:u16 2
----------------
Total          20 octets
```

payloadSize commence à l'offset **18**, pas 20. L'offset est une position dans le tableau ; 20 est l'offset de début du payload.

Codec écrit d'abord une valeur zéro de taille dans l'en-tête, sérialise le corps, puis fait :

```cpp
writer.patchU16(PAYLOAD_SIZE_OFFSET,
    static_cast<std::uint16_t>(writer.size() - Protocol::HEADER_SIZE));
```

Cette longueur ne compte pas les vingt octets de header. Le receveur exige qu'elle corresponde exactement aux octets disponibles, sans faire confiance à une longueur déclarée arbitraire.

Les champs ACK occupent huit octets (`ack` 4 + `ackBits` 4), la séquence quatre et l'ID fiable deux autres. Les confirmations ne nécessitent pas un datagramme dédié à chaque fois. Tous les types portent le même header, même si leur corps est vide ou qu'ils ne sont pas suivis comme fiables.

### 19.5. Primitives et chaînes : calculer au lieu d'utiliser sizeof

| Élément sérialisé | Coût |
|---|---:|
| u8 ou déplacement i8 | 1 octet. |
| u16 ou i16 | 2 octets. |
| u32 | 4 octets. |
| Occupation de slot | 1 octet écrit explicitement par writeU8. |
| Chaîne de L octets | **1 + L**, longueur sur u8 puis octets, sans zéro terminal. |
| Entité complète | 8 octets. |
| Slot joueur | **10 + 2V**, V étant son nombre de valeurs. |

Une std::string en mémoire contient de la gestion de stockage ; son sizeof ne mesure pas le texte sérialisé. Le vector d'entités en mémoire n'est pas non plus envoyé avec ses pointeurs/capacité. Le bool de l'occupation est encodé explicitement, pas par un memcpy de son conteneur.

writeString() a une limite technique de 255 octets de texte avec ce préfixe ; les validateurs imposent des limites plus petites. Une chaîne vide coûte encore **un octet**. Les textes sont en ASCII imprimable dans notre protocole actuel : les limites s'expriment en octets, pas en nombre arbitraire de caractères Unicode.

### 19.6. Tailles de tous les messages actuels

Notation : n = longueur du nom concerné, c = longueur du code de salle, r = nombre de règles, t = longueur d'un texte, S = nombre de slots décrits. Colonne « total » = header applicatif inclus, **pas UDP/IP**. Les maxima viennent du JSON courant et des classes de console, pas d'une limite universelle de chaque version future.

#### Client → serveur

| Message | Payload, calcul | Total applicatif | Maximum actuel / valeur fixe |
|---|---|---|---:|
| CONNECT | `1 + n` | `21 + n` | 33 avec pseudo de 12 ; 21 si vide. |
| INPUT | `4 + 1 = 5` | 25 | 25. |
| HEARTBEAT | `4 + 4 = 8` | 28 | 28. |
| DISCONNECT | Token 4 | 24 | 24. |
| LIST_ROOMS | Token 4 | 24 | 24. |
| CREATE_ROOM | `4 + (1+n) + (1+c) + 1 + r` | `27 + n + c + r` | **58** : n=16, c=8, r=7. |
| JOIN_ROOM | `4 + 2 + (1+c)` | `27 + c` | 35 avec code de 8. |
| LEAVE_ROOM | Token 4 | 24 | 24. |
| START_GAME | Token 4 | 24 | 24. |
| KICK_PLAYER | `4 + 1 = 5` | 25 | 25. |
| DELETE_ROOM | Token 4 | 24 | 24. |
| CONSOLE_COMMAND | `4 + (1+t)` | `25 + t` | **225** avec texte de 200. |
| CHAT | `4 + (1+t)` | `25 + t` | **89** avec texte de 64. |

#### Serveur → client

| Message | Payload, calcul | Total applicatif | Maximum actuel / valeur fixe |
|---|---|---|---:|
| ACCEPT | Token 4 + indice 1 | 25 | 25. |
| REJECT | Raison 1 | 21 | 21. |
| SNAPSHOT | Voir formule détaillée ci-dessous. | `132 + 8N` actuellement | **1 172** à N=130. |
| HEARTBEAT_ACK | Timestamp 4 | 24 | 24. |
| ENTITY_DESTROYED | ID 2 | 22 | 22. |
| PLAYER_JOINED | Slot 1 | 21 | 21. |
| PLAYER_KILLED | Slot 1 | 21 | 21. |
| PLAYER_LEFT | Slot 1 + raison 1 | 22 | 22. |
| SERVER_SHUTDOWN | Vide 0 | 20 | 20. |
| ROOM_LIST | `1 + somme(6+n de chaque salle)` | `21 + somme(6+n)` | **1 077** : 48 noms de 16. |
| ROOM_JOINED | ID 2 + chaîne `1+n` + slot 1 | `24 + n` | 40 avec nom de 16. |
| ROOM_REJECTED | Raison 1 | 21 | 21. |
| ROOM_UPDATE | `2 + somme(2+n de chaque slot)` | `22 + somme(2+n)` | **78** : 4 noms de 12. |
| GAME_STARTED | Vide 0 | 20 | 20. |
| KICKED | Vide 0 | 20 | 20. |
| ROOM_CLOSED | Vide 0 | 20 | 20. |
| PLAY_SOUND | Index 1 | 21 | 21. |
| CONSOLE_TEXT | `1 + t` | `21 + t` | **221** avec texte de 200. |
| SNAPSHOT_DELTA | Voir formule détaillée ci-dessous. | Variable ; base 33 | Pas de maximum garanti par la seule borne du complet. |
| ChatBroadcast / CHAT_MESSAGE | Slot 1 + chaîne `1+t` | `22 + t` | **86** avec texte de 64. |

La retransmission d'un même message conserve normalement la même longueur des champs, même si sequence et ACK changent. Elle augmente le **volume cumulé** et le nombre de paquets, pas la largeur du header d'une tentative.

### 19.7. Exemple octet par octet : INPUT

Exemple construit avec version 9, type INPUT=02, sequence=1, ack/ackBits/reliableId à zéro, token `0x01020304`, droite+tir (`0x18`) :

```text
45 4E          magic
09             version
02             type INPUT
00 00 00 01    sequence
00 00 00 00    ack
00 00 00 00    ackBits
00 00          reliableId = 0
00 05          payloadSize = 5
01 02 03 04    token
18             boutons = droite | tir
```

Les vingt premiers octets ne contiennent pas encore les commandes du joueur : ils décrivent le paquet et sa réception. Le token et les boutons constituent le payload. Cet exemple explique l'encodage ; ce n'est pas une nouvelle capture réseau.

Le header représente `20/25 = 80 %` de cet INPUT applicatif, ce qui montre pourquoi les petits messages ont un overhead proportionnel important. Cela ne signifie pas que supprimer les contrôles ou les ACK serait une optimisation correcte ; on doit regarder les garanties et le coût total du système.

### 19.8. Le snapshot complet, sans sauter les compteurs

| Champs fixes hors listes | Octets |
|---|---:|
| tick | 4. |
| gameState | 1. |
| world valueCount | 1. |
| playerCount | 1. |
| entityCount | 1. |
| Total fixe du corps | **8**. |

Un PlayerStatus contient state 1 + lives 1 + health 1 + score 4 + avatarId 2 + valueCount 1 = **10**, puis `2V` octets de valeurs. Avec V=4 : **18 octets par slot**.

Un EntityState contient id 2 + kind 1 + x 2 + y 2 + visual 1 = **8 octets**. Les champs visuels et positions ne sont pas des floats transmis tels quels.

```text
H = 20, W = valeurs monde, P = slots décrits, V = valeurs par slot, N = entités

Total = H + 8 + 2W + P × (10 + 2V) + 8N
      = 20 + 8 + 2×16 + 4×(10 + 2×4) + 8N
      = 20 + 8 + 32 + 72 + 8N
      = 132 + 8N
```

| N | Total applicatif calculé |
|---:|---:|
| 0 | 132. |
| 10 | 212. |
| 50 | 532. |
| 100 | 932. |
| 130, limite configurée | **1 172**. |
| 133, calcul théorique avec les mêmes métadonnées | 1 196, tient dans 1 200. |
| 134, même hypothèse | 1 204, dépasse. |

Les dernières lignes sont des calculs de budget, pas une nouvelle configuration ni la preuve qu'une partie sans entité et des slots vivants serait sémantiquement valide. Le plafond actuel reste 130.

La simulation décrit toujours `settings.maxPlayers()` slots, y compris les vides. Une salle VERSUS limitée à deux membres ne divise donc pas automatiquement par deux les métadonnées joueurs du snapshot. ROOM_UPDATE, lui, reflète la capacité de la salle et peut avoir deux slots.

Les anciennes formules `102 + 8N` et 1 142 à 130 entités correspondaient à W=5 et V=3. Elles ne sont plus valables pour notre JSON actuel à W=16 et V=4. Les noms, codes et lignes de chat ne sont pas ajoutés dans chaque snapshot.

### 19.9. Deltas : formule et champ par champ

Le payload fixe vaut 13 : tick 4 + baselineTick 4 + gameState 1 + quatre compteurs de listes 4. Le header applicatif apporte encore 20.

```text
Total delta = 33
            + 3 × nombre de valeurs monde changées
            + somme(1 + taille du PlayerStatus changé)
            + somme(taille des EntityChange)
            + 2 × nombre d'entités supprimées
```

Avec quatre valeurs par joueur, chaque entrée joueur modifiée coûte `slot 1 + status 18 = 19`. Une valeur monde modifiée coûte `index 1 + valeur 2 = 3`. Une suppression coûte ID 2.

| EntityChange | Calcul | Coût |
|---|---|---:|
| Entité inchangée | Absente de la liste. | 0. |
| dx seulement | ID 2 + masque 1 + dx 1. | 4. |
| dy seulement | Même principe. | 4. |
| dx et dy | ID 2 + masque 1 + deux déplacements 1. | 5. |
| kind seulement ou visual seulement | ID 2 + masque 1 + champ 1. | 4. |
| Position absolue seule | ID 2 + masque 1 + x/y 4. | 7. |
| Création complète | ID 2 + masque 1 + x/y 4 + kind 1 + visual 1. | 9. |

Si un déplacement sur un axe dépasse [−128,127], le code transmet les deux coordonnées absolues, pas un dx saturé. Le bit position absolue est incompatible avec les bits dx/dy.

Exemple construit : dix entités modifiées uniquement en X, un joueur modifié, deux valeurs monde et trois suppressions : `33 + 10×4 + 19 + 2×3 + 3×2 = 104 octets`. Un delta sans entrée modifiée coûte encore **33 octets** : tick, baseline et compteurs ne disparaissent pas.

### 19.10. Pourquoi un delta peut dépasser 1 200

Une création complète coûte neuf octets dans un delta, contre huit dans le complet, à cause du masque. Les listes ajoutent aussi leurs IDs/indices, et les suppressions n'existent pas dans le complet qui décrit simplement le monde actuel.

Exemple de budget avec ancien et nouveau mondes de 130 entités aux IDs disjoints, seize valeurs monde et quatre joueurs tous modifiés :

```text
Base delta                33
16 valeurs × 3            48
4 joueurs × 19            76
130 créations × 9      1 170
130 suppressions × 2      260
---------------------------
Total                  1 587 octets
```

Le complet du nouveau monde reste à 1 172. Le code actuel ne compare pas les tailles avant le choix différentiel ; la validation des nombres ne garantit pas ici le respect de 1 200. ByteWriter lève une erreur lors du dépassement. Dans le parcours serveur, sans traitement local de cet encodage, l'exception peut remonter jusqu'au main et arrêter le programme avec l'erreur.

Ce calcul ne prétend pas que cette situation a été observée dans les essais. Il montre une limite réelle de budget : une amélioration serait d'estimer/encoder le delta et de revenir à un complet lorsqu'il est plus gros ou ne tient pas. Il ne faut pas résoudre cela en affirmant simplement « UDP découpera automatiquement notre protocole en plusieurs messages ».

### 19.11. Les listes de salles et compositions

Une entrée RoomInfo coûte ID 2 + préfixe du nom 1 + nom n + joueurs 1 + capacité 1 + flags 1 = **6+n**. Il faut encore le compteur de liste 1 et le header 20.

```text
48 salles, noms de 16 : 20 + 1 + 48×22 = 1 077
53 salles, mêmes noms : 20 + 1 + 53×22 = 1 187
54 salles, mêmes noms : 20 + 1 + 54×22 = 1 209 → dépasse
```

Augmenter maxRooms n'est donc pas une simple promesse de capacité : il faut aussi garantir le format de découverte, raccourcir les entrées ou paginer si nécessaire.

ROOM_UPDATE est différent : ownerSlot 1 + slotCount 1, puis pour chaque slot occupation 1 + préfixe de nom 1 + nom n. Son total vaut `22 + 2S + somme des noms` :

- quatre noms de douze octets : `22 + 8 + 48 = 78` ;
- deux noms de douze : `22 + 4 + 24 = 50` ;
- quatre noms vides : `22 + 8 = 30`, même si certains slots sont inoccupés.

Le code envoie un préfixe de chaîne pour chaque slot, y compris vide. Le résumé ROOM_LIST ne contient pas tous ces pseudos.

### 19.12. Calculer un débit sans le confondre avec une mesure

Un octet vaut huit bits. kbit/s utilise ici 1 000 bits/s et KiB/s 1 024 octets/s.

```text
Inputs théoriques par seconde ≈ 1000 / 33 = 30,303
INPUT applicatif             = 25 octets
HEARTBEAT par seconde        = 28 octets
Débit client périodique       ≈ 25×30,303 + 28 = 785,6 octets/s
```

C'est hors actions, chat, console et retransmissions, et suppose une boucle assez rapide. Ajouter UDP/IP sans options apporte encore 28 octets par datagramme : `785,6 + 28×(30,303+1) ≈ 1 662,1 octets/s`. Ne pas ajouter seulement 28 octets **par seconde**, car l'overhead est payé pour chaque datagramme.

Avec un complet maximal à 1 172 et 20 envois/s :

```text
Par client, applicatif : 1172×20 = 23 440 octets/s
                      ≈ 22,89 KiB/s ≈ 187,52 kbit/s
Avec UDP/IPv4         : (1172+28)×20 = 24 000 octets/s
```

Avec 192 destinataires recevant tous un tel volume, l'ordre de grandeur applicatif serait `23 440×192 = 4 500 480 octets/s`, soit environ 36 Mbit/s, avant les autres messages et en-têtes. C'est une projection hypothétique en complets maximaux, **pas un résultat de capacité ni notre débit normal avec deltas**. La charge CPU des salles est aussi une autre contrainte.

Le débit historique autour de 2 KiB/s par client sous netem décrivait une scène et une version plus légères, avec deltas. Il n'est pas contradictoire avec un calcul de complets maximaux du JSON actuel ; on ne compare pas le même trafic.

### 19.13. Taille en mémoire, réserve et taille envoyée

ByteWriter réserve de la capacité avec `reserve(maxSize)`, mais la taille du vector augmente seulement quand des octets sont écrits. Un paquet INPUT de size 25 n'est donc pas complété automatiquement par 1 175 zéros jusqu'à 1 200. `asio::buffer(*data)` utilise la taille des données du vector, pas sa capacité réservée.

La taille mémoire d'un Packet, des strings, historiques et buffers est différente du nombre d'octets réseau. Un budget brut de `256×1200 = 307 200 octets`, soit 300 KiB de contenu pour une file remplie au maximum, n'est pas son coût mémoire total : objets, allocations, capacités et autres structures s'ajoutent.

Asio conserve une vue buffer pour l'opération, le shared_ptr possède nos octets et le noyau possède ses propres mécanismes de transport. Dire « on a un buffer de 1 201 » ne décrit pas tous les buffers de la pile réseau ni une absence de copies système.

### 19.14. Quantification concrète : ce qu'on économise et ce qu'on perd

La simulation garde des coordonnées flottantes ; describe() arrondit avec lround puis convertit vers int16 pour le réseau. Par exemple 123,72 devient 124. On perd la précision sous le pixel et la valeur doit rester représentable dans le format signé. La santé est transformée en pourcentage entier sur un octet.

L'angle utilise sept bits, 128 pas sur 360°, donc `360/128 = 2,8125°` par pas. Le serveur calcule `round(angle/360×128)` et garde les sept bits ; le client reconstruit `pas×360/128`. À 10°, on obtient le pas 4 puis 11,25°. L'écart maximal circulaire d'arrondi est d'environ 1,40625°.

L'octet visual combine l'angle avec le flash : bit 7 pour le flash, bits 0–6 pour l'angle. Quantification = réduction des niveaux de précision ; packing = partage de l'octet ; delta = omission de ce qui n'a pas changé. Ce sont trois optimisations distinctes.

**À ouvrir :** [SimulationSnapshot.cpp](../../engine/simulation/src/SimulationSnapshot.cpp), [GameViewSync.cpp](../../engine/client/src/GameViewSync.cpp), [EntityChange.cpp](../../engine/network/messages/game/src/EntityChange.cpp), [ByteWriter.cpp](../../engine/net/src/ByteWriter.cpp).

### 19.15. Questions de contrôle et réponses attendues

- **« Les 1 200 incluent UDP ? »** Non : ils sont le payload UDP complet, header de jeu inclus. Ajouter huit octets UDP et généralement vingt IPv4 sans options.
- **« Un paquet du jeu mesure toujours 1 200 ? »** Non : c'est un plafond. INPUT vaut 25, GAME_STARTED 20 et un snapshot dépend de son contenu.
- **« sizeof(Packet) donne le coût réseau ? »** Non : seules les écritures du codec définissent le format.
- **« Une chaîne vide vaut zéro octet ? »** Non : il reste son préfixe u8 de longueur.
- **« Pourquoi pas payloadSize en u8 ? »** Le corps peut dépasser 255 octets ; u16 permet de représenter le budget actuel.
- **« VERSUS divise tous les paquets par deux ? »** Non : capacité de salle, nombre de slots du snapshot et taille des événements sont des choses distinctes.
- **« Le compteur d'entités en u8 autorise 255 entités actuelles dans un complet ? »** Le type peut les compter, mais le budget du paquet et maxEntities restreignent davantage.
- **« Les ACK coûtent-ils un nouveau paquet par input ? »** Non : champs de header portés par le trafic existant, sans rendre leur coût en octets nul.
- **« Delta signifie toujours gain ? »** Non : calculer champs, indices, masques et suppressions ; le repli sur un complet est à améliorer.
- **« Le test netem mesure la taille d'une trame Ethernet ? »** Non : ses compteurs applicatifs ne sont pas une capture de la couche liaison.

**Réponse orale à savoir faire :** « Notre budget de 1 200 concerne l'en-tête de jeu et son corps. Les champs sont explicitement sérialisés : un INPUT coûte 25 octets ; le complet actuel vaut 132 plus huit par entité, soit 1 172 à la limite configurée de 130. Les deltas réduisent souvent le volume mais peuvent grossir. Il faut ajouter UDP/IP pour parler du trafic réseau et distinguer ces calculs des mesures de la version historique testée. »

**Vérification des tableaux du 10 novembre :** un utilitaire C++ temporaire a appelé les méthodes write() réelles de toutes les classes de messages avec les longueurs/nombres maximaux courants. Il a notamment confirmé 58 pour CREATE_ROOM, 1 077 pour ROOM_LIST, 78 pour ROOM_UPDATE et 1 172 pour le complet de 130 entités, header de 20 ajouté. Un writer temporairement autorisé à compter au-delà du budget a confirmé 1 587 pour l'exemple de remplacement en delta. Cette dernière vérification compte les octets ; elle ne prétend pas que le codec/transport normal autorise ce paquet ni qu'un tel scénario réseau a été exécuté. Aucune nouvelle mesure netem n'a été réalisée pour cette mise à jour.
