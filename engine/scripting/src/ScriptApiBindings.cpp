#include "ScriptEngineImpl.hpp"

#include <iostream>

#include <pybind11/operators.h>
#include <pybind11/stl.h>

#include "ScriptEntity.hpp"
#include "ScriptError.hpp"
#include "ScriptWorld.hpp"

namespace py = pybind11;

namespace engine::scripting {

void ScriptEngineImpl::bindApi()
{
    py::module_ api = py::module_::import("types").attr("ModuleType")(MODULE_NAME);

    py::class_<ScriptWorld>(api, "World")
        .def("time", &ScriptWorld::time)
        .def("every", &ScriptWorld::every, py::arg("name"), py::arg("seconds"))
        .def("reset_timers", &ScriptWorld::resetTimers)
        .def("random", &ScriptWorld::random, py::arg("minimum"), py::arg("maximum"))
        .def("set", &ScriptWorld::set, py::arg("name"), py::arg("value"))
        .def("set_label", &ScriptWorld::setLabel, py::arg("name"), py::arg("label"))
        .def("get", &ScriptWorld::get, py::arg("name"))
        .def("label", &ScriptWorld::label, py::arg("name"))
        .def("spawn", &ScriptWorld::spawn, py::arg("prefab"), py::arg("x"), py::arg("y"))
        .def("can_spawn", &ScriptWorld::canSpawn)
        .def("count_scripted", &ScriptWorld::countScripted, py::arg("file"))
        .def("players_alive", &ScriptWorld::playersAlive)
        .def("player_count", &ScriptWorld::playerCount)
        .def("finish_game", &ScriptWorld::finishGame)
        .def("match_phase", &ScriptWorld::matchPhase)
        .def("end_match", &ScriptWorld::endMatch, py::arg("delay") = 0.f)
        .def("width", &ScriptWorld::width)
        .def("height", &ScriptWorld::height)
        .def("log", &ScriptWorld::log, py::arg("text"))
        .def("players", &ScriptWorld::players)
        .def("in_layer", &ScriptWorld::inLayer, py::arg("layer"))
        .def("rule", &ScriptWorld::rule, py::arg("name"))
        .def("play_sound", &ScriptWorld::playSound, py::arg("name"));

    py::class_<ScriptEntity>(api, "Entity", py::dynamic_attr())
        .def("alive", &ScriptEntity::alive)
        .def("exists", &ScriptEntity::exists)
        .def("age", &ScriptEntity::age)
        .def("every", &ScriptEntity::every, py::arg("name"), py::arg("seconds"))
        .def("x", &ScriptEntity::x)
        .def("y", &ScriptEntity::y)
        .def("width", &ScriptEntity::width)
        .def("height", &ScriptEntity::height)
        .def("on_ground", &ScriptEntity::onGround)
        .def("center_x", &ScriptEntity::centerX)
        .def("center_y", &ScriptEntity::centerY)
        .def("vx", &ScriptEntity::vx)
        .def("vy", &ScriptEntity::vy)
        .def("set_position", &ScriptEntity::setPosition, py::arg("x"), py::arg("y"))
        .def("set_velocity", &ScriptEntity::setVelocity, py::arg("vx"), py::arg("vy"))
        .def("health", &ScriptEntity::health)
        .def("max_health", &ScriptEntity::maxHealth)
        .def("number", &ScriptEntity::number, py::arg("name"), py::arg("fallback") = 0.f)
        .def("text", &ScriptEntity::text, py::arg("name"), py::arg("fallback") = std::string())
        .def("fire", &ScriptEntity::fire, py::arg("prefab"), py::arg("angle"), py::arg("speed"), py::arg("dx") = 0.f,
            py::arg("dy") = 0.f)
        .def("fire_spread", &ScriptEntity::fireSpread, py::arg("prefab"), py::arg("count"), py::arg("spread"),
            py::arg("angle"), py::arg("speed"))
        .def("fire_at_player", &ScriptEntity::fireAtPlayer, py::arg("prefab"), py::arg("speed"))
        .def("nearest_player", &ScriptEntity::nearestPlayer)
        .def("angle_to_player", &ScriptEntity::angleToPlayer)
        .def("spawn", &ScriptEntity::spawn, py::arg("prefab"), py::arg("dx") = 0.f, py::arg("dy") = 0.f)
        .def("set_invulnerable", &ScriptEntity::setInvulnerable, py::arg("seconds"))
        .def("invulnerable_time", &ScriptEntity::invulnerableTime)
        .def("destroy", &ScriptEntity::destroy)
        .def("kill", &ScriptEntity::kill)
        .def("is_player", &ScriptEntity::isPlayer)
        .def("player_slot", &ScriptEntity::playerSlot)
        .def("heal", &ScriptEntity::heal, py::arg("amount"))
        .def("set_health", &ScriptEntity::setHealth, py::arg("points"), py::arg("maximum"))
        .def("set_weapon_offset", &ScriptEntity::setWeaponOffset, py::arg("dx"), py::arg("dy"))
        .def("set_weapon", &ScriptEntity::setWeapon, py::arg("prefab"), py::arg("cooldown"), py::arg("count") = 1,
            py::arg("spread") = 0.f)
        .def("input_held", &ScriptEntity::inputHeld, py::arg("name"))
        .def("input_pressed", &ScriptEntity::inputPressed, py::arg("name"))
        .def("input_released", &ScriptEntity::inputReleased, py::arg("name"))
        .def("attach_to", &ScriptEntity::attachTo, py::arg("parent"), py::arg("dx") = 0.f, py::arg("dy") = 0.f)
        .def("detach", &ScriptEntity::detach)
        .def("attached", &ScriptEntity::attached)
        .def("parent", &ScriptEntity::parent)
        .def("set_angle", &ScriptEntity::setAngle, py::arg("degrees"))
        .def("set_player_value", &ScriptEntity::setPlayerValue, py::arg("name"), py::arg("value"))
        .def("player_value", &ScriptEntity::playerValue, py::arg("name"))
        .def(py::self == py::self)
        .def_property_readonly("world", &ScriptEntity::world, py::return_value_policy::reference);

    py::module_::import("sys").attr("modules")[MODULE_NAME] = api;
}

} // namespace engine::scripting
