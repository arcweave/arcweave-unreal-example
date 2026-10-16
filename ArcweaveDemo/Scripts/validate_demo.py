"""Run with Unreal's PythonScript commandlet and the local JSON setting enabled.

Exercises the bundled demo export through the real Arcweave subsystem and native
interpreter. Requires PythonScriptPlugin, enabled for the validation invocation.
"""

import unreal


def require(condition, message):
    if not condition:
        raise RuntimeError(message)


subsystem = unreal.get_engine_subsystem(unreal.ArcweaveSubsystem)
settings = subsystem.load_arcweave_settings()
require(
    settings.get_editor_property("enable_receive_method_from_local_json"),
    "Enable local JSON import before running this check; API requests are not tested.",
)


def variables():
    project = subsystem.get_arcweave_project_data()
    return {value.name: value.value for value in project.current_vars.values()}


def transpile(element_id):
    element, success = subsystem.transpile_object(element_id)
    require(success, "Could not transpile demo element " + element_id)
    return element


subsystem.fetch_data("", "")
try:
    project = subsystem.get_arcweave_project_data()
    require(project.name == "TEST-Game Engine Example", "Expected the bundled demo export")
    require(len(project.boards) == 5, "Expected five demo boards")
    require(len(project.components) == 7, "Expected seven demo components")
    require(
        variables() == {"have_potion": "false", "healer_health": "50", "wanda_health": "20"},
        "Unexpected initial demo variables",
    )

    wounded = transpile("5265ceac-f13f-47fc-a02c-6eda7eea6b90")
    require("I am wounded" in wounded.content, "Wanda should initially ask for help")

    transpile("d852a577-bd1f-44cf-8187-77a86f97baef")
    require(variables()["have_potion"] == "true", "The healer should give the player a potion")

    transpile("045ab2b6-6d77-43f7-a7b4-e275f41667c3")
    require(variables()["wanda_health"] == "70", "The potion should heal Wanda by 50")
    require(variables()["have_potion"] == "false", "Healing Wanda should consume the potion")

    healed = transpile("5265ceac-f13f-47fc-a02c-6eda7eea6b90")
    require("Thank you for saving my life" in healed.content, "Wanda's dialogue should reflect healing")
    unreal.log("ARCWEAVE_DEMO_VALIDATION_PASSED: import, dialogue, potion, healing, and conditional text")
finally:
    # Restore the authored values after the check, including when an assertion fails.
    subsystem.fetch_data("", "")
