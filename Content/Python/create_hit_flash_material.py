"""
Builds M_HitFlash_Overlay (+ MI_HitFlash_Overlay) under
/Game/Materials/VFX_Materials/HitFlash.

This is a *mesh overlay* material, not something you wire into a character's
own material. UHitFlashComponent calls SetOverlayMaterial() on the character's
meshes for the ~0.12s the flash lasts and nulls it out again afterwards, so the
mesh renders a second time additively on top of itself and no existing material
in the project needs touching. Idle cost is zero -- there is no overlay
material assigned when nothing is being hit.

Why additive+unlit rather than replacing base colour:

  * Additive keeps the silhouette and the underlying shading readable. A flat
    colour replacement is the other standard look, but it needs the flash
    plumbed into every character master material, which this avoids entirely.
  * Unlit means the flash is constant regardless of what lights the level. A
    hit at night has to read exactly as hard as a hit at noon.
  * Emissive is pushed well past 1 (Glow defaults to 6) so bloom does the
    work. Requires bloom to be on in the post process volume.

The fresnel term makes the rim burn brighter than the interior, which is what
keeps the flash reading as the *shape of the character* instead of a coloured
smear -- important when a dozen enemies overlap.

HitFlashAmount is driven from C++ and stays at 0 in the editor. To preview the
flash, open MI_HitFlash_Overlay and drag HitFlashAmount to 1.

Run from the editor:  Tools > Execute Python Script...  and pick this file.
The script refuses to overwrite; delete both assets first to rebuild.
"""

import unreal

PACKAGE_PATH = "/Game/Materials/VFX_Materials/HitFlash"
MAT_NAME = "M_HitFlash_Overlay"
MI_NAME = "MI_HitFlash_Overlay"

G_FLASH = "01 - Hit Flash"

MEL = unreal.MaterialEditingLibrary
EAL = unreal.EditorAssetLibrary
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()


def fail(msg):
    unreal.log_error("[HitFlash] " + msg)
    raise RuntimeError(msg)


def warn(msg):
    unreal.log_warning("[HitFlash] " + msg)


def guard_existing():
    for name in (MAT_NAME, MI_NAME):
        path = "{0}/{1}".format(PACKAGE_PATH, name)
        if EAL.does_asset_exist(path):
            fail("{0} already exists. Delete or rename it first -- this script "
                 "will not overwrite an existing asset.".format(path))


# --------------------------------------------------------------------------- #
# graph
# --------------------------------------------------------------------------- #

def make_graph(mat):
    """Builds the flash. Returns (emissive_expression, amount_expression)."""

    def node(cls, x, y, **props):
        n = MEL.create_material_expression(mat, cls, x, y)
        for k, v in props.items():
            n.set_editor_property(k, v)
        return n

    def link(src, dst, dst_input, src_output=""):
        MEL.connect_material_expressions(src, src_output, dst, dst_input)

    def link_any(src, dst, candidates, label):
        """Connects to the first input name that takes -- some pin names have
        moved between engine versions."""
        for name in candidates:
            try:
                if MEL.connect_material_expressions(src, "", dst, name):
                    return True
            except Exception:
                pass
        warn("Could not connect {0} automatically -- hook that pin up by hand "
             "in the material editor.".format(label))
        return False

    def scalar(name, default, x, y, group):
        return node(unreal.MaterialExpressionScalarParameter, x, y,
                    parameter_name=name, default_value=default, group=group)

    def vector(name, colour, x, y, group):
        return node(unreal.MaterialExpressionVectorParameter, x, y,
                    parameter_name=name, default_value=colour, group=group)

    def mul(a, b, x, y):
        """a * b; b may be a node or a float."""
        n = node(unreal.MaterialExpressionMultiply, x, y)
        link(a, n, "A")
        if isinstance(b, unreal.MaterialExpression):
            link(b, n, "B")
        else:
            n.set_editor_property("const_b", float(b))
        return n

    def add(a, b, x, y):
        n = node(unreal.MaterialExpressionAdd, x, y)
        link(a, n, "A")
        if isinstance(b, unreal.MaterialExpression):
            link(b, n, "B")
        else:
            n.set_editor_property("const_b", float(b))
        return n

    def saturate(a, x, y):
        n = node(unreal.MaterialExpressionClamp, x, y)
        link(a, n, "")
        return n

    def rgb(source, x, y):
        """RGB mask -- vector parameters output float4 and the Substrate
        inputs refuse a size mismatch."""
        m = node(unreal.MaterialExpressionComponentMask, x, y,
                 r=True, g=True, b=True, a=False)
        link(source, m, "")
        return m

    # ---------------------------------------------------------------- #
    # the driver: 0 = invisible, 1 = full flash. Set from C++ every frame
    # the flash is running. Clamped because an overshoot past 1 turns the
    # character into a white blob.
    # ---------------------------------------------------------------- #
    amount_param = scalar("HitFlashAmount", 0.0, -1200, 0, G_FLASH)
    amount = saturate(amount_param, -1000, 0)

    flash_col = vector("HitFlashColor",
                       unreal.LinearColor(1.0, 0.03, 0.02, 1.0),
                       -1200, -260, G_FLASH)
    flash_rgb = rgb(flash_col, -1000, -260)

    # ---------------------------------------------------------------- #
    # rim: brighter at grazing angles so the silhouette stays legible in
    # a crowd of overlapping enemies.
    # ---------------------------------------------------------------- #
    rim_exp = scalar("HitFlashRimExponent", 4.0, -1200, 320, G_FLASH)
    fres = node(unreal.MaterialExpressionFresnel, -1000, 320,
                exponent=4.0, base_reflect_fraction=0.0)
    link_any(rim_exp, fres, ["ExponentIn", "Exponent"], "Fresnel exponent")

    rim_strength = scalar("HitFlashRim", 2.0, -1000, 500, G_FLASH)
    rim_term = mul(fres, rim_strength, -780, 380)
    # 1 + rim, so the interior still glows at full colour and the edge goes
    # hotter on top of it rather than the interior going dark.
    rim_mul = add(rim_term, 1.0, -600, 380)

    # ---------------------------------------------------------------- #
    # emissive: colour * amount * glow * rim
    # ---------------------------------------------------------------- #
    glow = scalar("HitFlashGlow", 6.0, -1000, -100, G_FLASH)

    col_amount = mul(flash_rgb, amount, -780, -180)
    col_glow = mul(col_amount, glow, -600, -180)
    emissive = mul(col_glow, rim_mul, -400, -180)

    return emissive, amount


def connect_output(mat, emissive, amount):
    """Wires the flash into the material output.

    This project runs Substrate (r.Substrate=True), so the output goes through
    an Unlit BSDF -> Front Material where those nodes exist; the legacy
    Emissive + MSM_Unlit path is kept as a fallback.

    Opacity is best-effort. Emissive is already scaled by HitFlashAmount, so an
    unconnected opacity pin costs nothing visually -- at Amount 0 the material
    adds black. The connection just saves the blend some work.
    """
    try:
        MEL.connect_material_property(
            amount, "", unreal.MaterialProperty.MP_OPACITY)
    except Exception:
        warn("Could not connect Opacity -- harmless, the emissive is already "
             "scaled by HitFlashAmount.")

    unlit_cls = getattr(unreal, "MaterialExpressionSubstrateUnlitBSDF", None)
    front = getattr(unreal.MaterialProperty, "MP_FRONT_MATERIAL", None)

    if unlit_cls is None or front is None:
        MEL.connect_material_property(
            emissive, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
        return

    unlit = MEL.create_material_expression(mat, unlit_cls, -180, -180)
    connected = False
    for name in ("Emissive Color", "EmissiveColor", "Emissive"):
        try:
            if MEL.connect_material_expressions(emissive, "", unlit, name):
                connected = True
                break
        except Exception:
            pass
    if not connected:
        warn("Could not connect Emissive Color on the Unlit BSDF -- falling "
             "back to the legacy emissive output.")
        MEL.connect_material_property(
            emissive, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
        return

    MEL.connect_material_property(unlit, "", front)


# --------------------------------------------------------------------------- #
# main
# --------------------------------------------------------------------------- #

def run():
    guard_existing()

    mat = TOOLS.create_asset(MAT_NAME, PACKAGE_PATH, unreal.Material,
                             unreal.MaterialFactoryNew())
    if mat is None:
        fail("Could not create " + MAT_NAME)

    mat.set_editor_property("material_domain",
                            unreal.MaterialDomain.MD_SURFACE)
    mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_ADDITIVE)
    mat.set_editor_property("shading_model",
                            unreal.MaterialShadingModel.MSM_UNLIT)

    # The overlay pass re-renders the same skinned mesh, so the skeletal usage
    # flag has to be on or the material silently fails to bind on characters.
    for prop, value in (("used_with_skeletal_mesh", True),
                        ("two_sided", False)):
        try:
            mat.set_editor_property(prop, value)
        except Exception:
            warn("Could not set {0} -- check it by hand in the material "
                 "details panel.".format(prop))

    emissive, amount = make_graph(mat)
    connect_output(mat, emissive, amount)
    MEL.recompile_material(mat)

    mi = TOOLS.create_asset(MI_NAME, PACKAGE_PATH,
                            unreal.MaterialInstanceConstant,
                            unreal.MaterialInstanceConstantFactoryNew())
    if mi is None:
        fail("Could not create " + MI_NAME)
    MEL.set_material_instance_parent(mi, mat)

    EAL.save_asset("{0}/{1}".format(PACKAGE_PATH, MAT_NAME))
    EAL.save_asset("{0}/{1}".format(PACKAGE_PATH, MI_NAME))

    unreal.log("[HitFlash] Created {0}/{1} and {0}/{2}".format(
        PACKAGE_PATH, MAT_NAME, MI_NAME))


run()
