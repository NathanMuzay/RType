cmake_minimum_required(VERSION 3.21)

# Standalone engine: builds the engine_* libraries, the generic launchers and engine_add_game().
# It knows nothing about any game; a game is a folder of JSON data, Python scripts and assets.
if(NOT PROJECT_NAME)
    project(Engine VERSION 0.1.0 LANGUAGES CXX)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()
if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
endif()
if(CMAKE_CONFIGURATION_TYPES)
    set(ENGINE_OUTPUT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>" CACHE INTERNAL "")
else()
    set(ENGINE_OUTPUT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" CACHE INTERNAL "")
endif()
set(ENGINE_APPS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/apps/src" CACHE INTERNAL "")
# Run-time modularity: the engine libraries are also linked into plugins (shared libraries).
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ---------------------------------------------------------------------------
# Dependencies (vcpkg.json).
# ---------------------------------------------------------------------------

find_package(Threads REQUIRED)
find_package(asio CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
# Compile-time modularity: each switch removes a group of engine modules and its dependencies.
option(ENGINE_BUILD_CLIENT "Build the client modules (render, audio, input, client) and r-type_client" ON)
option(ENGINE_BUILD_SERVER "Build the server modules (scripting, simulation) and r-type_server" ON)
if(ENGINE_BUILD_CLIENT)
    find_package(SFML 3 COMPONENTS Graphics Window System Audio CONFIG REQUIRED)
endif()
if(ENGINE_BUILD_SERVER)
    find_package(Python3 COMPONENTS Interpreter Development.Embed REQUIRED)
    set(PYBIND11_FINDPYTHON ON)
    find_package(pybind11 CONFIG REQUIRED)
endif()

# ---------------------------------------------------------------------------
# Engine: knows nothing about R-Type. One module = one library [p.16].
# ---------------------------------------------------------------------------

# --- engine_core: errors, clock, timestep, colors, geometry, command line, asset paths, UI input, process memory
add_library(engine_core
    core/src/EngineError.cpp
    core/src/Clock.cpp
    core/src/FixedTimestep.cpp
    core/src/Color.cpp
    core/src/Vector2.cpp
    core/src/IntRect.cpp
    core/src/TextFormat.cpp
    core/src/CommandLine.cpp
    core/src/AssetPath.cpp
    core/src/PortOption.cpp
    core/src/UiInput.cpp
    core/src/ProcessMemory.cpp
    core/src/DynamicLibrary.cpp
)
target_include_directories(engine_core PUBLIC core/include)
if(WIN32)
    target_link_libraries(engine_core PRIVATE psapi)
else()
    target_link_libraries(engine_core PRIVATE ${CMAKE_DL_LIBS})
endif()

# --- engine_physics: bounding boxes and collision detection (pure geometry)
add_library(engine_physics
    physics/src/Aabb.cpp
    physics/src/Collision.cpp
    physics/src/Body.cpp
    physics/src/CollisionPair.cpp
)
target_include_directories(engine_physics PUBLIC physics/include)

# --- engine_ecs: entities, registry, components and systems
# The render and audio interfaces used here (IRenderer, IAudio) contain no SFML: engine_ecs never links SFML.
add_library(engine_ecs
    ecs/src/Entity.cpp
    ecs/src/EntityManager.cpp
    ecs/src/Registry.cpp
    ecs/src/SystemManager.cpp
    ecs/src/UiStore.cpp
    ecs/src/WorldValues.cpp
    ecs/components/gameplay/src/Script.cpp
    ecs/components/combat/src/Destroyed.cpp
    ecs/components/physics/src/Attachment.cpp
    ecs/components/combat/src/HitFlash.cpp
    ecs/components/physics/src/Rotation.cpp
    ecs/components/gameplay/src/PlayerValues.cpp
    ecs/components/gameplay/src/Tilemap.cpp
    ecs/components/presentation/src/ParticleEmitter.cpp
    ecs/components/gameplay/src/Lifetime.cpp
    ecs/components/presentation/src/MusicWhen.cpp
    ecs/components/interface/src/Choice.cpp
    ecs/components/interface/src/KeyBind.cpp
    ecs/components/interface/src/SettingSlider.cpp
    ecs/components/physics/src/Gravity.cpp
    ecs/components/physics/src/Bounce.cpp
    ecs/components/physics/src/Transform.cpp
    ecs/components/physics/src/Velocity.cpp
    ecs/components/physics/src/Collider.cpp
    ecs/components/physics/src/Bounded.cpp
    ecs/components/combat/src/Health.cpp
    ecs/components/combat/src/ContactDamage.cpp
    ecs/components/combat/src/Invulnerable.cpp
    ecs/components/physics/src/DespawnOffscreen.cpp
    ecs/components/gameplay/src/PlayerControl.cpp
    ecs/components/combat/src/Owner.cpp
    ecs/components/combat/src/ScoreValue.cpp
    ecs/components/presentation/src/Sprite.cpp
    ecs/components/presentation/src/RectShape.cpp
    ecs/components/presentation/src/Parallax.cpp
    ecs/components/presentation/src/TiltAnimation.cpp
    ecs/components/combat/src/Weapon.cpp
    ecs/components/gameplay/src/Replicated.cpp
    ecs/components/presentation/src/SpriteAnimation.cpp
    ecs/components/presentation/src/SoundCue.cpp
    ecs/components/presentation/src/Text.cpp
    ecs/components/presentation/src/Anchor.cpp
    ecs/components/gameplay/src/Player.cpp
    ecs/components/gameplay/src/Lives.cpp
    ecs/components/gameplay/src/Score.cpp
    ecs/components/gameplay/src/Respawn.cpp
    ecs/components/gameplay/src/Match.cpp
    ecs/components/presentation/src/DeathEffect.cpp
    ecs/components/gameplay/src/PlayerIndex.cpp
    ecs/components/presentation/src/PlayerLabel.cpp
    ecs/components/presentation/src/StatusText.cpp
    ecs/components/interface/src/Button.cpp
    ecs/components/interface/src/Focusable.cpp
    ecs/components/interface/src/TextField.cpp
    ecs/components/interface/src/VisibleWhen.cpp
    ecs/components/interface/src/BoundText.cpp
    ecs/components/interface/src/RowData.cpp
    ecs/components/interface/src/ListRowPart.cpp
    ecs/components/interface/src/ListView.cpp
    ecs/components/interface/src/ProgressBar.cpp
    ecs/components/interface/src/FadeWhen.cpp
    ecs/components/src/ComponentRegistration.cpp
    ecs/systems/events/src/CollisionEvent.cpp
    ecs/systems/events/src/DeathEvent.cpp
    ecs/systems/physics/src/MovementSystem.cpp
    ecs/systems/physics/src/BoundsSystem.cpp
    ecs/systems/physics/src/CollisionSystem.cpp
    ecs/systems/gameplay/src/PlayerControlSystem.cpp
    ecs/systems/gameplay/src/InvulnerabilitySystem.cpp
    ecs/systems/gameplay/src/DamageSystem.cpp
    ecs/systems/physics/src/OffscreenSystem.cpp
    ecs/systems/physics/src/CleanupSystem.cpp
    ecs/systems/presentation/src/ParallaxSystem.cpp
    ecs/systems/presentation/src/TiltAnimationSystem.cpp
    ecs/systems/presentation/src/RenderSystem.cpp
    ecs/systems/gameplay/src/WeaponSystem.cpp
    ecs/systems/presentation/src/SpriteAnimationSystem.cpp
    ecs/systems/presentation/src/AnchorSystem.cpp
    ecs/systems/presentation/src/AudioSystem.cpp
    ecs/systems/gameplay/src/MatchSystem.cpp
    ecs/systems/gameplay/src/RespawnSystem.cpp
    ecs/systems/gameplay/src/LivesSystem.cpp
    ecs/systems/gameplay/src/ScoreSystem.cpp
    ecs/systems/presentation/src/DeathEffectSystem.cpp
    ecs/systems/presentation/src/PlayerLabelSystem.cpp
    ecs/systems/presentation/src/StatusTextSystem.cpp
    ecs/systems/events/src/UiActionEvent.cpp
    ecs/systems/interface/src/UiFocus.cpp
    ecs/systems/interface/src/PointerSystem.cpp
    ecs/systems/interface/src/FocusNavigationSystem.cpp
    ecs/systems/interface/src/ButtonStyleSystem.cpp
    ecs/systems/interface/src/TextFieldSystem.cpp
    ecs/systems/interface/src/BoundTextSystem.cpp
    ecs/systems/interface/src/VisibilitySystem.cpp
    ecs/systems/interface/src/ListViewSystem.cpp
    ecs/systems/physics/src/AttachmentSystem.cpp
    ecs/systems/gameplay/src/HitFlashSystem.cpp
    ecs/systems/physics/src/SolidSystem.cpp
    ecs/systems/gameplay/src/TilemapSystem.cpp
    ecs/systems/presentation/src/ParticleSystem.cpp
    ecs/systems/presentation/src/LifetimeSystem.cpp
    ecs/systems/presentation/src/MusicSystem.cpp
    ecs/systems/presentation/src/ShakeSystem.cpp
    ecs/systems/physics/src/GravitySystem.cpp
    ecs/systems/physics/src/BounceSystem.cpp
    ecs/systems/interface/src/ChoiceSystem.cpp
    ecs/systems/interface/src/KeyBindSystem.cpp
    ecs/systems/interface/src/SettingSliderSystem.cpp
    ecs/systems/events/src/SoundEvent.cpp
    ecs/systems/events/src/ConsoleEvent.cpp
    ecs/systems/interface/src/ProgressBarSystem.cpp
    ecs/systems/interface/src/FadeSystem.cpp
)
target_include_directories(engine_ecs PUBLIC
    ecs/include
    ecs/components/include
    ecs/components/physics/include
    ecs/components/combat/include
    ecs/components/gameplay/include
    ecs/components/presentation/include
    ecs/components/interface/include
    ecs/systems/events/include
    ecs/systems/physics/include
    ecs/systems/gameplay/include
    ecs/systems/presentation/include
    ecs/systems/interface/include
    render/include
    audio/include)
target_link_libraries(engine_ecs PUBLIC engine_core engine_physics)

# --- engine_scene: JSON data, component and system factories, prefabs and scenes (nlohmann-json hidden inside)
add_library(engine_scene
    scene/src/DataNode.cpp
    scene/src/DataWriter.cpp
    scene/src/PluginRegistrar.cpp
    scene/src/LoadedPlugin.cpp
    scene/src/PluginManager.cpp
    scene/src/DataNodeImpl.cpp
    scene/src/DataReaders.cpp
    scene/src/LayerTable.cpp
    scene/src/ComponentFactory.cpp
    scene/src/EngineComponentLoaders.cpp
    scene/src/EngineComponentLoadersGameplay.cpp
    scene/src/EngineComponentLoadersPresentation.cpp
    scene/src/EngineComponentLoadersInterface.cpp
    scene/src/PrefabLibrary.cpp
    scene/src/SystemContext.cpp
    scene/src/SystemFactory.cpp
    scene/src/EngineSystemBuilders.cpp
    scene/src/SceneBuilder.cpp
    scene/src/WorldValueTable.cpp
)
target_include_directories(engine_scene PUBLIC scene/include PRIVATE scene/internal)
target_link_libraries(engine_scene PUBLIC engine_core engine_ecs PRIVATE nlohmann_json::nlohmann_json)

# --- engine_net: serialization, UDP client/server (Asio hidden inside) [p.9][p.11]
add_library(engine_net
    net/src/ByteReader.cpp
    net/src/ByteWriter.cpp
    net/src/Endpoint.cpp
    net/src/Datagram.cpp
    net/src/UdpTransport.cpp
    net/src/UdpTransportImpl.cpp
)
target_include_directories(engine_net PUBLIC net/include PRIVATE net/internal)
target_link_libraries(engine_net PUBLIC engine_core PRIVATE asio::asio Threads::Threads)
target_compile_definitions(engine_net PRIVATE ASIO_STANDALONE)
if(WIN32)
    target_compile_definitions(engine_net PRIVATE _WIN32_WINNT=0x0A00)
endif()

# --- engine_network: generic game protocol, game server and client connection on top of engine_net
add_library(engine_network
    network/protocol/src/NetworkSettings.cpp
    network/protocol/src/Codec.cpp
    network/protocol/src/PacketVisitors.cpp
    network/protocol/src/Packet.cpp
    network/protocol/src/Protocol.cpp
    network/protocol/src/PendingMessage.cpp
    network/protocol/src/ReliableReceiver.cpp
    network/protocol/src/ReliableSender.cpp
    network/messages/game/src/PlayerStatus.cpp
    network/messages/game/src/EntityState.cpp
    network/messages/connection/src/ConnectMessage.cpp
    network/messages/game/src/InputMessage.cpp
    network/messages/connection/src/HeartbeatMessage.cpp
    network/messages/connection/src/DisconnectMessage.cpp
    network/messages/connection/src/AcceptMessage.cpp
    network/messages/connection/src/RejectMessage.cpp
    network/messages/game/src/SnapshotMessage.cpp
    network/messages/connection/src/HeartbeatAckMessage.cpp
    network/messages/game/src/EntityDestroyedMessage.cpp
    network/messages/game/src/PlayerJoinedMessage.cpp
    network/messages/game/src/PlayerKilledMessage.cpp
    network/messages/game/src/PlayerLeftMessage.cpp
    network/messages/connection/src/ServerShutdownMessage.cpp
    network/messages/lobby/src/ListRoomsMessage.cpp
    network/messages/lobby/src/CreateRoomMessage.cpp
    network/messages/lobby/src/JoinRoomMessage.cpp
    network/messages/lobby/src/LeaveRoomMessage.cpp
    network/messages/lobby/src/StartGameMessage.cpp
    network/messages/lobby/src/KickPlayerMessage.cpp
    network/messages/lobby/src/DeleteRoomMessage.cpp
    network/messages/lobby/src/RoomListMessage.cpp
    network/messages/lobby/src/RoomJoinedMessage.cpp
    network/messages/lobby/src/RoomRejectedMessage.cpp
    network/messages/lobby/src/RoomUpdateMessage.cpp
    network/messages/lobby/src/GameStartedMessage.cpp
    network/messages/lobby/src/KickedMessage.cpp
    network/messages/lobby/src/RoomClosedMessage.cpp
    network/messages/game/src/PlaySoundMessage.cpp
    network/messages/console/src/ConsoleCommandMessage.cpp
    network/messages/console/src/ConsoleTextMessage.cpp
    network/messages/lobby/src/ChatMessage.cpp
    network/messages/lobby/src/ChatBroadcastMessage.cpp
    network/messages/game/src/EntityChange.cpp
    network/messages/game/src/SnapshotDeltaMessage.cpp
    network/protocol/src/SnapshotDiff.cpp
    network/protocol/src/SnapshotHistory.cpp
    network/protocol/src/TextRules.cpp
    network/messages/lobby/src/RoomInfo.cpp
    network/server/src/GameRoom.cpp
    network/server/src/ClientSession.cpp
    network/server/src/GameServer.cpp
    network/server/src/GameServerRooms.cpp
    network/server/src/ServerRunner.cpp
    network/client/src/ClientConnection.cpp
)
target_include_directories(engine_network PUBLIC
    network/protocol/include
    network/messages/connection/include
    network/messages/lobby/include
    network/messages/game/include
    network/messages/console/include
    network/server/include
    network/client/include)
target_link_libraries(engine_network PUBLIC engine_core engine_net engine_scene)

# --- engine_render: window, SFML renderer, textures, fonts (client only)
if(ENGINE_BUILD_CLIENT)
add_library(engine_render
    render/src/SfmlRenderer.cpp
    render/src/Window.cpp
    render/src/TextureManager.cpp
    render/src/FontManager.cpp
)
target_include_directories(engine_render PUBLIC render/include)
target_link_libraries(engine_render PUBLIC engine_core SFML::Graphics)

# --- engine_audio: sounds and music (client only)
add_library(engine_audio
    audio/src/SfmlAudio.cpp
)
target_include_directories(engine_audio PUBLIC audio/include)
target_link_libraries(engine_audio PUBLIC engine_core SFML::Audio)

# --- engine_input: actions, remappable key bindings (client only)
add_library(engine_input
    input/src/KeyBindings.cpp
    input/src/InputManager.cpp
)
target_include_directories(engine_input PUBLIC input/include)
target_link_libraries(engine_input PUBLIC engine_core SFML::Window)

# --- engine_client: window, resources, systems and scene of a game JSON file, fed by any transport (client only)
add_library(engine_client
    client/src/GameView.cpp
    client/src/GameViewSync.cpp
    client/src/PlayerSettings.cpp
    client/src/SceneManager.cpp
    client/src/DebugMetrics.cpp
    client/src/DevConsole.cpp
    client/src/OnlineClient.cpp
    client/src/SecretCode.cpp
    client/src/OnlineClientLobby.cpp
    client/src/OnlineClientGame.cpp
    client/src/SnapshotPresenter.cpp
    client/src/SettingsStore.cpp
)
target_include_directories(engine_client PUBLIC client/include)
target_link_libraries(engine_client PUBLIC engine_scene engine_network engine_render engine_audio engine_input)

endif()

if(ENGINE_BUILD_SERVER)
# --- engine_scripting: runs the Python behaviour scripts named by Script components (server only)
add_library(engine_scripting
    scripting/src/ScriptEngine.cpp
    scripting/src/ScriptEngineImpl.cpp
    scripting/src/ScriptApiBindings.cpp
    scripting/src/ScriptWorld.cpp
    scripting/src/ScriptEntity.cpp
    scripting/src/ScriptRecord.cpp
    scripting/src/ScriptSystem.cpp
    scripting/src/ScriptSystemImpl.cpp
    scripting/src/ScriptBuilder.cpp
)
target_include_directories(engine_scripting PUBLIC scripting/include PRIVATE scripting/internal)
target_link_libraries(engine_scripting PUBLIC engine_scene PRIVATE pybind11::embed)
set_target_properties(engine_scripting PROPERTIES CXX_VISIBILITY_PRESET hidden)

# --- engine_simulation: authoritative game world built from a game JSON file (no rendering)
add_library(engine_simulation
    simulation/src/Simulation.cpp
    simulation/src/SimulationSnapshot.cpp
    simulation/src/SimulationFactory.cpp
)
target_include_directories(engine_simulation PUBLIC simulation/include)
target_link_libraries(engine_simulation PUBLIC engine_scene engine_network engine_scripting)

if(ENGINE_BUILD_CLIENT)
# --- engine_local: solo mode, the client runs the simulation in memory without any server
add_library(engine_local
    local/src/LocalClient.cpp
)
target_include_directories(engine_local PUBLIC local/include)
target_link_libraries(engine_local PUBLIC engine_client engine_simulation)
endif()

endif()


if(ENGINE_BUILD_SERVER)
# --- engine_python_runtime: copies the Python standard library next to the server so it runs without a system Python
set(ENGINE_PYTHON_PREFIX "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}")
if(WIN32)
    set(ENGINE_PYTHON_STDLIB "${ENGINE_PYTHON_PREFIX}/tools/python3/Lib")
    set(ENGINE_PYTHON_TARGET "python/Lib")
else()
    set(ENGINE_PYTHON_STDLIB "${ENGINE_PYTHON_PREFIX}/lib/python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}")
    set(ENGINE_PYTHON_TARGET "python/lib/python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}")
endif()
add_custom_target(engine_python_runtime ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${ENGINE_PYTHON_STDLIB} ${ENGINE_OUTPUT_DIR}/${ENGINE_PYTHON_TARGET})
if(WIN32)
    add_custom_command(TARGET engine_python_runtime POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${ENGINE_PYTHON_PREFIX}/tools/python3/DLLs ${ENGINE_OUTPUT_DIR}/python/DLLs)
endif()

endif()

# engine_add_game(<name> CLIENT <exe> SERVER <exe> DATA <dir> SCRIPTS <dir> ASSETS <dir>)
# Builds the generic launchers under the game's executable names and copies its content next to them.
function(engine_add_game name)
    cmake_parse_arguments(GAME "" "CLIENT;SERVER;LOCAL;DATA;SCRIPTS;ASSETS" "PLUGINS" ${ARGN})
    add_custom_target(${name}_content ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/${GAME_ASSETS} ${ENGINE_OUTPUT_DIR}/assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/${GAME_DATA} ${ENGINE_OUTPUT_DIR}/data
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/${GAME_SCRIPTS} ${ENGINE_OUTPUT_DIR}/scripts)
    if(ENGINE_BUILD_SERVER AND GAME_SERVER)
        add_executable(${GAME_SERVER} ${ENGINE_APPS_DIR}/ServerMain.cpp)
        target_compile_definitions(${GAME_SERVER} PRIVATE ENGINE_PROGRAM_NAME="${GAME_SERVER}" ENGINE_DEFAULT_GAME="${name}")
        target_link_libraries(${GAME_SERVER} PRIVATE engine_simulation)
        add_dependencies(${GAME_SERVER} ${name}_content engine_python_runtime)
        set_target_properties(${GAME_SERVER} PROPERTIES ENABLE_EXPORTS ON)
    endif()
    if(ENGINE_BUILD_CLIENT AND GAME_CLIENT)
        add_executable(${GAME_CLIENT} ${ENGINE_APPS_DIR}/ClientMain.cpp)
        target_compile_definitions(${GAME_CLIENT} PRIVATE ENGINE_PROGRAM_NAME="${GAME_CLIENT}" ENGINE_DEFAULT_GAME="${name}")
        target_link_libraries(${GAME_CLIENT} PRIVATE engine_client)
        add_dependencies(${GAME_CLIENT} ${name}_content)
    endif()
    if(ENGINE_BUILD_CLIENT AND ENGINE_BUILD_SERVER AND GAME_LOCAL)
        add_executable(${GAME_LOCAL} ${ENGINE_APPS_DIR}/LocalMain.cpp)
        target_compile_definitions(${GAME_LOCAL} PRIVATE ENGINE_PROGRAM_NAME="${GAME_LOCAL}" ENGINE_DEFAULT_GAME="${name}")
        target_link_libraries(${GAME_LOCAL} PRIVATE engine_local)
        add_dependencies(${GAME_LOCAL} ${name}_content engine_python_runtime)
        set_target_properties(${GAME_LOCAL} PROPERTIES ENABLE_EXPORTS ON)
    endif()
    foreach(plugin IN LISTS GAME_PLUGINS)
        add_dependencies(${name}_content ${plugin})
    endforeach()
endfunction()

# engine_add_plugin(name SOURCES ... [INCLUDES ...]): builds a run-time plugin (shared library) into bin/plugins.
function(engine_add_plugin name)
    cmake_parse_arguments(PLUGIN "" "" "SOURCES;INCLUDES" ${ARGN})
    add_library(${name} SHARED ${PLUGIN_SOURCES})
    target_include_directories(${name} PRIVATE ${PLUGIN_INCLUDES})
    target_link_libraries(${name} PRIVATE engine_scene engine_ecs)
    set_target_properties(${name} PROPERTIES
        PREFIX ""
        CXX_VISIBILITY_PRESET hidden
        LIBRARY_OUTPUT_DIRECTORY ${ENGINE_OUTPUT_DIR}/plugins
        RUNTIME_OUTPUT_DIRECTORY ${ENGINE_OUTPUT_DIR}/plugins)
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_link_options(${name} PRIVATE -Wl,--exclude-libs,ALL)
    endif()
endfunction()
