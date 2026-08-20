"""
Replaces the two progress-circle plane meshes on BP_Treasure with a ground decal.

BP_Treasure (the digging "treasure monkey") marked its dig spot with two flat
static-mesh planes -- ProgressCircle (M_RoundProgressBar_Inst, the dig-progress
sweep) and ProgressCircleOuter (M_RoundZone, the zone ring that fades with
player distance). Planes sit at a fixed height and clip through uneven ground;
a deferred decal projects onto whatever it lands on instead.

This script adds a DecalComponent using MI_DiggingMonkeyDecal -- the decal
material that was authored for this actor and never wired up -- sized to match
the DiggingSphere so the ring lands exactly on the dig radius, then removes the
two plane components.

MANUAL STEP AFTERWARDS: the BP graph still calls into the deleted components.
Open BP_Treasure, compile, and delete the nodes the compiler flags -- see the
list this script prints when it finishes.

Run from the editor:  Tools > Execute Python Script...  and pick this file.
"""

import unreal

BP_PATH = "/Game/Blueprints/Pickups/Treasure/BP_Treasure"
DECAL_MI = "/Game/Materials/MI_DiggingMonkeyDecal"

DECAL_NAME = "DiggingDecal"

# The progress circles carry the dig-progress readout (M_RoundProgressBar's
# "Percentage" parameter) and the distance fade (M_RoundZone). The decal has no
# equivalent, so removing them costs that feedback -- leave this False to run
# the decal alongside them, which is the setup that keeps both.
REMOVE_PROGRESS_CIRCLES = False
DOOMED = ("ProgressCircle", "ProgressCircleOuter")

# X is the projection half-depth, not a radius: the decal box reaches this far
# above and below the component. Anything outside that box -- a rock the
# treasure spawned next to, a raised bit of terrain -- gets no decal at all,
# because a deferred decal projects through a box, it does not drape over
# geometry. Keep this comfortably above the tallest prop the treasure can
# spawn beside; the cost of overshooting is that the ring can also catch a
# surface directly overhead.
PROJECTION_DEPTH = 1000.0
# Used only if DiggingSphere cannot be read for its radius.
FALLBACK_RADIUS = 300.0

EAL = unreal.EditorAssetLibrary
SUBOBJ = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
SDBFL = unreal.SubobjectDataBlueprintFunctionLibrary


def fail(msg):
    unreal.log_error("[TreasureDecal] " + msg)
    raise RuntimeError(msg)


def warn(msg):
    unreal.log_warning("[TreasureDecal] " + msg)


def info(msg):
    unreal.log("[TreasureDecal] " + msg)


# --------------------------------------------------------------------------- #
# subobject helpers
# --------------------------------------------------------------------------- #

def handles(blueprint):
    return SUBOBJ.k2_gather_subobject_data_for_blueprint(blueprint)


def template_of(handle, blueprint):
    """The archetype object a handle stands for, or None."""
    data = SUBOBJ.k2_find_subobject_data_from_handle(handle)
    if data is None:
        return None
    if blueprint is not None:
        fn = getattr(SDBFL, "get_object_for_blueprint", None)
        if fn is not None:
            try:
                obj = fn(data, blueprint)
                if obj is not None:
                    return obj
            except Exception:
                pass
    try:
        return SDBFL.get_object(data)
    except Exception:
        return None


def name_of(handle, blueprint=None):
    """Display name of a subobject, however this engine version spells it."""
    data = SUBOBJ.k2_find_subobject_data_from_handle(handle)
    if data is None:
        return ""
    for getter in ("get_variable_name", "get_display_name"):
        fn = getattr(SDBFL, getter, None)
        if fn is None:
            continue
        try:
            value = str(fn(data))
            if value:
                return value
        except Exception:
            pass
    obj = template_of(handle, blueprint)
    return obj.get_name() if obj else ""


def find_handle(blueprint, wanted):
    for handle in handles(blueprint):
        # Components are named "ProgressCircle" but their templates carry a
        # "_GEN_VARIABLE" suffix, so match on the stem either way.
        current = name_of(handle, blueprint)
        if current == wanted or current == wanted + "_GEN_VARIABLE":
            return handle
    return None


def find_root_handle(blueprint):
    """The scene root, skipping the actor-context entry at index 0."""
    for handle in handles(blueprint):
        obj = template_of(handle, blueprint)
        if isinstance(obj, unreal.SceneComponent):
            return handle
    fail("BP_Treasure has no scene component to parent the decal to.")


# --------------------------------------------------------------------------- #
# steps
# --------------------------------------------------------------------------- #

def dig_radius(blueprint):
    handle = find_handle(blueprint, "DiggingSphere")
    if handle is None:
        warn("DiggingSphere not found -- sizing the decal to %.0f instead."
             % FALLBACK_RADIUS)
        return FALLBACK_RADIUS

    sphere = template_of(handle, blueprint)
    if not isinstance(sphere, unreal.SphereComponent):
        warn("DiggingSphere is not a SphereComponent -- sizing the decal to "
             "%.0f instead." % FALLBACK_RADIUS)
        return FALLBACK_RADIUS

    radius = float(sphere.get_editor_property("sphere_radius"))
    scale = sphere.get_editor_property("relative_scale3d")
    radius *= max(float(scale.x), float(scale.y))
    info("DiggingSphere radius is %.0f -- decal sized to match." % radius)
    return radius


def add_decal(blueprint, radius):
    material = EAL.load_asset(DECAL_MI)
    if material is None:
        fail("Could not load " + DECAL_MI)

    # Re-running is how the projection depth gets retuned, so an existing
    # decal is updated in place rather than skipped.
    handle = find_handle(blueprint, DECAL_NAME)
    if handle is not None:
        info("%s already exists -- updating its settings." % DECAL_NAME)
    else:
        params = unreal.AddNewSubobjectParams(
            parent_handle=find_root_handle(blueprint),
            new_class=unreal.DecalComponent,
            blueprint_context=blueprint)

        handle, reason = SUBOBJ.add_new_subobject(params)
        if str(reason) not in ("", "None"):
            fail("Could not add the decal component: %s" % reason)

        SUBOBJ.rename_subobject(handle=handle, new_name=DECAL_NAME)

    decal = template_of(handle, blueprint)
    if decal is None:
        fail("Decal component was added but its template could not be read.")

    decal.set_editor_property("decal_material", material)
    decal.set_editor_property(
        "decal_size", unreal.Vector(PROJECTION_DEPTH, radius, radius))
    # A decal projects down its +X axis, so it has to be pitched to face the
    # floor -- an unrotated decal projects sideways and never lands.
    decal.set_editor_property(
        "relative_rotation", unreal.Rotator(0.0, -90.0, 0.0))
    # Top-down camera sits far enough back that the default screen-size fade
    # pops the decal out at range.
    decal.set_editor_property("fade_screen_size", 0.0)

    info("Added %s using MI_DiggingMonkeyDecal (size %.0f x %.0f x %.0f)."
         % (DECAL_NAME, PROJECTION_DEPTH, radius, radius))
    return True


def remove_circles(blueprint):
    if not REMOVE_PROGRESS_CIRCLES:
        info("Leaving the progress circles in place "
             "(REMOVE_PROGRESS_CIRCLES is False).")
        return []

    removed = []
    for wanted in DOOMED:
        handle = find_handle(blueprint, wanted)
        if handle is None:
            info("%s not found -- already gone." % wanted)
            continue
        if not SUBOBJ.delete_subobject(
                context_handle=find_root_handle(blueprint),
                subobject_to_delete=handle,
                bp_context=blueprint):
            warn("Could not delete %s -- remove it by hand." % wanted)
            continue
        removed.append(wanted)
        info("Removed " + wanted)
    return removed


# --------------------------------------------------------------------------- #
# main
# --------------------------------------------------------------------------- #

def run():
    blueprint = EAL.load_asset(BP_PATH)
    if blueprint is None:
        fail("Could not load " + BP_PATH)

    radius = dig_radius(blueprint)
    added = add_decal(blueprint, radius)
    removed = remove_circles(blueprint)

    if not added and not removed:
        info("Nothing to do -- BP_Treasure is already on the decal.")
        return

    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    EAL.save_loaded_asset(blueprint)

    if removed:
        warn("BP_Treasure will not compile clean until the graph stops "
             "calling into the deleted components. Open it and remove:")
        warn("  - function InitDynamicMaterial (both Create Dynamic Material "
             "Instance nodes) and the variables ProgressCircleInstRef / "
             "ProgressOuterCircleInstRef")
        warn("  - function SetProgressCirclePercentage, and the Timeline "
             "update pin that calls it")
        warn("  - function SetZoneOpacity, and its call in the distance-check "
             "timer")
        warn("Then compile and save again.")

    info("Done.")


run()
