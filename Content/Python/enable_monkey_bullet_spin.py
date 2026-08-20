"""
Turns on the mesh spin for the ranged monkey's banana.

ABaseProjectile (C++) can spin its visual mesh while the projectile flies. The
mesh spins in its own local space, so the movement component still points the
actor down the travel direction and the collision sphere is untouched.

This flips the flag on BP_MonkeyBullet, the projectile BP_RangedMonkey fires,
and gives it a tumble rate. Everything here is the same edit as ticking the box
in the Details panel -- tune SPIN_RATE in the Blueprint afterwards if you want.

Run from the editor:  Tools > Execute Python Script...  and pick this file.
"""

import unreal

BULLET_BP_PATH = "/Game/Blueprints/Projectiles/BP_MonkeyBullet"

# degrees per second, in the mesh's local space.
# roll = barrel roll along the travel direction, pitch = end over end tumble.
SPIN_RATE = unreal.Rotator(0.0, 720.0, 0.0)  # roll, pitch, yaw

EAL = unreal.EditorAssetLibrary


def warn(msg):
    unreal.log_warning("[MonkeyBulletSpin] " + msg)


def info(msg):
    unreal.log("[MonkeyBulletSpin] " + msg)


def run():
    blueprint = EAL.load_asset(BULLET_BP_PATH)
    if blueprint is None:
        warn(BULLET_BP_PATH + " not found -- nothing changed.")
        return

    cdo = unreal.get_default_object(blueprint.generated_class())
    if cdo is None:
        warn("Could not reach the class defaults of BP_MonkeyBullet.")
        return

    try:
        cdo.set_editor_property("b_spin_mesh_while_traveling", True)
        cdo.set_editor_property("spin_rate", SPIN_RATE)
    except Exception as exc:
        warn("Could not set the spin properties (" + str(exc)
             + ") -- rebuild the C++ module, then set them in the Details panel.")
        return

    try:
        EAL.save_asset(BULLET_BP_PATH, only_if_is_dirty=False)
    except Exception as exc:
        warn("Could not save BP_MonkeyBullet: " + str(exc))
        return

    info("BP_MonkeyBullet now spins its mesh at " + str(SPIN_RATE) + " deg/s.")


run()
