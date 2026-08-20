"""
Creates the two poison-ball Blueprints and wires them into BP_OctopusCharacter.

APoisonProjectile (C++) is the ball the player fires forward on an attack swing
while a poison buff is active. Two Blueprints drive it:

  BP_PoisonProjectile            single ball, normal Buffs.PoisonWeapon poison,
                                 NS_PoisonBall
  BP_PoisonProjectile_ThreeKind  the three-of-a-kind fan -- stronger poison,
                                 travels further, NS_PoisonBall_PURPLE so the
                                 upgraded shot reads differently on screen

Both are created parented to APoisonProjectile, given their Niagara system, and
assigned to PoisonProjectileClass / PoisonVolleyProjectileClass on the player.
Re-running never overwrites an existing Blueprint's tuning.

Run from the editor:  Tools > Execute Python Script...  and pick this file.
"""

import unreal


def effect_mode(value_name):
    """EPoisonProjectileEffect member, or None if the enum is not exposed."""
    enum = getattr(unreal, "PoisonProjectileEffect", None)
    if enum is None:
        return None
    return getattr(enum, value_name, None)


BP_FOLDER = "/Game/Blueprints/Projectiles"
PLAYER_BP_PATH = "/Game/Blueprints/BP_OctopusCharacter"

# name, niagara system, class-default overrides, property on BP_OctopusCharacter
BLUEPRINTS = [
    {
        "name": "BP_PoisonProjectile",
        "niagara": "/Game/Niagara/PoisonBall/NS_PoisonBall",
        "player_property": "poison_projectile_class",
        "defaults": {
            "travel_distance": 800.0,
            "launch_speed": 1200.0,
            "end_speed_fraction": 0.15,
            "speed_falloff_exponent": 2.0,
            "poison_effect_mode": effect_mode("STANDARD"),
        },
    },
    {
        "name": "BP_PoisonProjectile_ThreeKind",
        "niagara": "/Game/Niagara/PoisonBall/NS_PoisonBall_PURPLE",
        "player_property": "poison_volley_projectile_class",
        # Travel distance is overridden per-shot by PoisonVolleyTravelDistance on the
        # player, so the value here only applies if that is set to 0.
        "defaults": {
            "travel_distance": 1600.0,
            "launch_speed": 1300.0,
            "end_speed_fraction": 0.2,
            "speed_falloff_exponent": 2.0,
            "poison_effect_mode": effect_mode("THREE_OF_A_KIND"),
        },
    },
]

EAL = unreal.EditorAssetLibrary


def fail(msg):
    unreal.log_error("[PoisonProjectile] " + msg)
    raise RuntimeError(msg)


def warn(msg):
    unreal.log_warning("[PoisonProjectile] " + msg)


def info(msg):
    unreal.log("[PoisonProjectile] " + msg)


def set_property(obj, names, value):
    """set_editor_property under whichever spelling this engine version uses."""
    for name in names:
        try:
            obj.set_editor_property(name, value)
            return True
        except Exception:
            continue
    return False


def get_property(obj, names):
    for name in names:
        try:
            return obj.get_editor_property(name)
        except Exception:
            continue
    return None


def create_blueprint(name):
    path = BP_FOLDER + "/" + name
    if EAL.does_asset_exist(path):
        info(path + " already exists -- reusing it, defaults left untouched.")
        return EAL.load_asset(path), False

    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", unreal.PoisonProjectile)

    blueprint = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        name, BP_FOLDER, unreal.Blueprint, factory)
    if blueprint is None:
        fail("Could not create " + path + ".")

    info("Created " + path + " (parent: APoisonProjectile).")
    return blueprint, True


def apply_defaults(blueprint, spec, is_new):
    cdo = unreal.get_default_object(blueprint.generated_class())
    if cdo is None:
        fail("Could not reach the class defaults of " + spec["name"] + ".")

    if is_new:
        for prop, value in spec["defaults"].items():
            if value is None:
                warn("Could not resolve a value for " + prop + " on " + spec["name"]
                     + " -- set it by hand in the Blueprint.")
                continue
            if not set_property(cdo, [prop], value):
                warn("Could not set " + prop + " on " + spec["name"]
                     + " -- set it by hand in the Blueprint.")

    # The Niagara system lives on the inherited PoisonVFX component. Assigning it
    # here is the same edit as picking the system in the Details panel.
    system = EAL.load_asset(spec["niagara"])
    if system is None:
        warn(spec["niagara"] + " not found -- assign the Niagara system by hand.")
        return

    vfx = get_property(cdo, ["poison_vfx", "PoisonVFX"])
    if vfx is None:
        warn("PoisonVFX component not found on " + spec["name"]
             + " -- assign the Niagara system by hand.")
        return

    if not set_property(vfx, ["asset", "Asset"], system):
        try:
            vfx.set_asset(system)
        except Exception:
            warn("Could not assign the Niagara system to " + spec["name"]
                 + " -- do it in the Details panel.")
            return

    info(spec["name"] + " -> PoisonVFX = " + system.get_name() + ".")


def wire_into_player(created):
    player_bp = EAL.load_asset(PLAYER_BP_PATH)
    if player_bp is None:
        warn(PLAYER_BP_PATH + " not found -- set the projectile classes by hand.")
        return False

    player_cdo = unreal.get_default_object(player_bp.generated_class())
    if player_cdo is None:
        warn("Could not reach BP_OctopusCharacter defaults -- set the classes by hand.")
        return False

    wired = False
    for spec, blueprint in created:
        prop = spec["player_property"]
        if set_property(player_cdo, [prop], blueprint.generated_class()):
            info("BP_OctopusCharacter -> " + prop + " = " + spec["name"] + ".")
            wired = True
        else:
            warn("Could not set " + prop + " -- set it by hand on BP_OctopusCharacter.")

    return wired


def save(path):
    try:
        EAL.save_asset(path, only_if_is_dirty=False)
    except Exception as exc:
        warn("Could not save " + path + ": " + str(exc))


def run():
    created = []
    for spec in BLUEPRINTS:
        blueprint, is_new = create_blueprint(spec["name"])
        apply_defaults(blueprint, spec, is_new)
        try:
            unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
        except Exception:
            pass
        created.append((spec, blueprint))

    wired = wire_into_player(created)

    for spec, _ in created:
        save(BP_FOLDER + "/" + spec["name"])
    if wired:
        save(PLAYER_BP_PATH)

    info("Done. Tune TravelDistance / LaunchSpeed / EndSpeedFraction / "
         "SpeedFalloffExponent and the CollisionSphere radius on each Blueprint; "
         "the fan's shape (PoisonVolleyCount, PoisonVolleySpreadDegrees, "
         "PoisonVolleyTravelDistance) lives on BP_OctopusCharacter.")


run()
