"""
Repoints the Level Instance inside L_MainMenu from Level_FC3 to Level_FC3_TestLevel.

The main menu holds the playable world as a LevelInstance actor
(PersistentLevel.LevelInstance_2), not as a streaming sublevel -- so the swap is
a single WorldAsset change on that actor. The script only touches level
instances that currently point at OLD_LEVEL, so re-running it is a no-op.

Run from the editor:  Tools > Execute Python Script...  and pick this file.
"""

import unreal

MENU_MAP = "/Game/Levels/UI/L_MainMenu"
OLD_LEVEL = "/Game/Levels/Level_FC3.Level_FC3"
NEW_LEVEL = "/Game/Levels/Level_FC3_TestLevel.Level_FC3_TestLevel"

EAL = unreal.EditorAssetLibrary


def _current_path(level_instance):
    value = level_instance.get_editor_property("world_asset")
    if value is None:
        return ""
    # TSoftObjectPtr comes back as SoftObjectPath on some engine versions and as
    # the loaded UWorld on others.
    if isinstance(value, unreal.SoftObjectPath):
        return str(value)
    return value.get_path_name()


def main():
    if not EAL.does_asset_exist(NEW_LEVEL.split(".")[0]):
        unreal.log_error("Target level not found: %s" % NEW_LEVEL)
        return

    unreal.EditorLoadingAndSavingUtils.load_map(MENU_MAP)

    actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    swapped = 0

    for actor in actor_subsystem.get_all_level_actors():
        if not isinstance(actor, unreal.LevelInstance):
            continue

        current = _current_path(actor)
        unreal.log("Level instance %s -> %s" % (actor.get_name(), current))

        if current != OLD_LEVEL:
            continue

        actor.set_editor_property("world_asset", unreal.SoftObjectPath(NEW_LEVEL))
        swapped += 1
        unreal.log("  repointed to %s" % NEW_LEVEL)

    if swapped == 0:
        unreal.log_warning("No level instance pointing at %s -- nothing changed." % OLD_LEVEL)
        return

    editor_world = unreal.get_editor_subsystem(
        unreal.UnrealEditorSubsystem
    ).get_editor_world()
    unreal.EditorLoadingAndSavingUtils.save_map(editor_world, MENU_MAP)
    unreal.log("Saved %s (%d level instance(s) repointed)" % (MENU_MAP, swapped))


main()
