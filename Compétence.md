# Compétences développées - R-Type et moteur de jeu

Bilan établi le 11 novembre 2026 à partir des sources, configurations, tests et documents présents dans ce dépôt.

## Présentation

Moteur de jeu C++20 modulaire, jeu de tir multijoueur R-Type et jeu solo Skyhop. Le README cite Mohamed Erraji, Milo Bousquet, Nathan Muzay et Erwan Ababsa.

## Durée du projet

- Début du calendrier pédagogique : **28/09/2026**.
- Échéance indiquée : **08/11/2026**.
- Fenêtre prévue, dates incluses : **42 jours calendaires, soit 6 semaines** (41 jours d’écart entre les deux dates).
- Source : capture du tableau « Projets terminés » fournie par Nathan ; lecture de la date au pictogramme vert comme début et du drapeau rouge comme échéance. La date orange n’est pas utilisée comme début de développement.
- Historique du dépôt source local consulté : premier commit listé le **2026-09-28**, dernier commit listé le **2026-11-08**. Ces dates peuvent inclure des travaux après l’échéance et ne mesurent pas le temps de travail effectif.
- Heures investies, répartition individuelle et date exacte de fin de travail : **non renseignées**. Le commit d’import du portfolio ne représente pas la durée du projet.

## Lecture des niveaux de maîtrise

Ce bilan inventorie les compétences mobilisées et étayées par les livrables. Une analyse de dépôt ne permet pas de certifier l’acquisition individuelle, l’autonomie ni l’auteur de chaque réalisation. Les niveaux ci-dessous qualifient la profondeur d’utilisation visible **dans ce projet**, pas une certification ou un pourcentage de maîtrise personnelle de Nathan. Leur attribution individuelle reste à confirmer par ses contributions et une démonstration orale/pratique.

| Niveau | Signification dans ce document |
| --- | --- |
| Ciblé | Utilisation circonscrite à une fonction ou un outil ; expérience générale non établie. |
| Opérationnel | Utilisation concrète dans plusieurs tâches ou dans une fonctionnalité intégrée. |
| Approfondi | Plusieurs mécanismes internes sont implémentés ou articulés, avec des éléments de test ou d’analyse ; cela ne signifie pas expert du domaine. |

## Technologies et profondeur d’utilisation

| Technologie ou domaine | Niveau observé | Utilisation démontrée | Preuve dans le dépôt |
| --- | --- | --- | --- |
| C++20, STL, templates, RAII | Approfondi | Moteur modulaire, registre ECS, interfaces et gestion des ressources | [engine](engine) |
| Architecture ECS | Approfondi | Entités, composants, tableaux creux, vues et systèmes | [engine/ecs](engine/ecs) |
| Asio, UDP, protocole binaire | Approfondi | Communication client/serveur, validation, ACK et retransmission | [engine/network](engine/network) |
| SFML 3 | Opérationnel | Fenêtre, rendu 2D, audio et entrées | [engine](engine) |
| Python 3, pybind11 | Opérationnel | Comportements de jeu et liaison avec le moteur C++ | [game/scripts](game/scripts) |
| JSON, nlohmann-json | Opérationnel | Scènes, entités, prefabs, paramètres et composition de jeux | [engine/scene](engine/scene) |
| CMake, vcpkg | Opérationnel | Cibles, dépendances, compilation et jeux/plugins | [CMakeLists.txt](CMakeLists.txt) |
| Bibliothèques partagées .so/.dll | Opérationnel | Chargement dynamique de nouveaux systèmes | [plugins/wobble](plugins/wobble) |
| Linux, iproute2, tc netem | Opérationnel | Mesure réseau sous délai, pertes, duplication et désordre | [scripts/netem_test.py](scripts/netem_test.py) |
| GitHub Actions, clang-format | Opérationnel | Pipeline de formatage et compilation Linux/Windows défini | [.github/workflows/ci.yml](.github/workflows/ci.yml) |
| MkDocs Material, Markdown | Opérationnel | Documentation d’architecture, guides et protocole | [mkdocs.yml](mkdocs.yml) |
| GCC, Clang, MSVC, Windows/Linux | Ciblé | Prise en compte de toolchains et plateformes dans le build | [docs/getting-started/building.md](docs/getting-started/building.md) |

## Inventaire détaillé des compétences

### Architecture et C++

- Séparer noyau, ECS, réseau, physique, rendu, audio, entrées et scripting.
- Employer templates, conteneurs STL, pointeurs intelligents et interfaces.
- Définir des entités, leur cycle de vie et des registres de composants.
- Stocker des composants dans des structures creuses et construire des vues filtrées.
- Découpler données, systèmes et orchestration de la boucle de jeu.
- Gérer les ressources et bibliothèques dynamiques avec des objets dédiés.
- Construire factories, registres de composants/systèmes et prefabs.
- Définir un moteur réutilisable par deux jeux aux modes d’exécution distincts.

### Simulation, gameplay et présentation

- Gérer le temps et une simulation à pas fixe ; le README décrit un serveur à 60 ticks/s.
- Implémenter déplacement, gravité, limites, rebonds, attachements et collisions.
- Gérer santé, dégâts, invulnérabilité, vies, respawn, scores et armes.
- Orchestrer vagues, boss, projectiles, bonus, scènes et progression.
- Construire une campagne coopérative et un mode versus.
- Séparer simulation serveur, présentation client et exécution locale sans réseau.
- Animer sprites, parallaxe, particules, flashs et effets de caméra.
- Gérer musique, sons et ressources graphiques.
- Construire HUD, menus, champs de texte, listes, boutons et navigation de focus.
- Proposer commandes reconfigurables, manette, taille du texte et volumes.
- Ajouter console développeur et affichage de débogage.

### Réseau et systèmes distribués

- Définir un serveur autoritaire et une session par client.
- Encoder et décoder des messages binaires avec contrôle de taille et de version.
- Valider types de messages, direction client/serveur et contenu.
- Gérer numéros de séquence, fenêtre d’acquittements et débordements de compteurs.
- Estimer le temps aller-retour et adapter la retransmission de messages fiables.
- Suivre connexions, heartbeats, déconnexions et arrêt serveur.
- Synchroniser l’état des entités avec snapshots et deltas.
- Conserver un historique pour reconstruire les changements d’état.
- Gérer salons publics ou protégés par code, noms, règles, chat et exclusions.
- Séparer protocole, transport, serveur, client et messages métier.
- Documenter le format réseau pour permettre une implémentation cliente indépendante.

### Scripting, données et extensibilité

- Décrire un jeu, ses scènes, composants et systèmes en JSON.
- Lire et valider les données puis instancier les objets via des factories.
- Exposer des fonctionnalités du moteur aux scripts Python avec pybind11.
- Écrire des comportements de boss, ennemis, projectiles et progression en Python.
- Créer un second jeu en réutilisant le moteur.
- Charger un plugin de système depuis une bibliothèque partagée.
- Séparer ressources tierces et leurs licences des sources du moteur.

### Mesure, outillage et livraison

- Construire un client de test sans interface graphique utilisant le vrai code réseau.
- Simuler délai, perte, duplication et réordonnancement avec netem.
- Isoler les perturbations dans un espace réseau Linux dédié.
- Automatiser lancement du serveur, scénarios, collecte et rapports.
- Développer une sonde du protocole et un test de charge multi-salons.
- Interpréter les résultats selon version du protocole, machine et environnement.
- Définir une compilation multiplateforme et une baseline de dépendances vcpkg.
- Configurer les contrôles de format et jobs de build GitHub Actions.
- Documenter installation, réseau local, pare-feu, architecture et contribution.
- Rédiger une étude comparative, des guides de création de contenu et des supports de présentation.

## Limites et compétences non attestées

- Les fonctionnalités sont collectives : leur présence ne signifie pas que Nathan a développé seul chaque module.
- La CI est définie dans le dépôt ; ce document ne certifie pas la réussite du dernier pipeline.
- Le rapport netem du 8 novembre décrit une version antérieure aux ajouts de chat et de noms ; ses résultats ne valident pas automatiquement le protocole actuel.
- Une simulation autoritaire n’équivaut pas à un audit de sécurité réseau ou à une protection anti-triche complète.
- Le projet ne démontre pas à lui seul une expertise graphique 3D, moteur physique industriel ou exploitation de serveurs à grande échelle.

## Compétences transversales

- Traduire un sujet en fonctionnalités et organiser un livrable exécutable.
- Décomposer un problème, isoler les responsabilités et rechercher les causes d’erreurs.
- Lire et produire une documentation technique, des exemples et des commandes reproductibles.
- Utiliser Git pour versionner et livrer le travail.
- Relier un choix technique à ses contraintes et expliquer ses limites.

La gestion personnelle du temps, la répartition des tâches, le leadership et l’autonomie ne peuvent pas être déduits des seuls fichiers ; ils demandent un retour d’expérience individuel.

## Vérification et preuves

Les niveaux reposent sur une inspection des sources et sur les tests/rapports déjà présents. Aucun résultat d’exécution nouveau, taux de couverture ou succès de CI n’est revendiqué dans ce document. Les liens ci-dessus permettent de retrouver les implémentations. Les rapports historiques doivent être lus avec leur date, leurs conditions et leurs limites.

## Points à préciser pour une présentation individuelle

- Modules conçus, codés, testés ou documentés personnellement par Nathan.
- Contributions réalisées en autonomie, en binôme ou avec accompagnement.
- Difficulté rencontrée, diagnostic et solution qu’il peut expliquer sans support.
- Capacité actuelle à reproduire ou adapter les réalisations dans un autre projet.
- Temps réellement consacré et résultats officiellement évalués, si disponibles.
