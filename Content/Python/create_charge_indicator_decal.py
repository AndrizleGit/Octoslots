"""
Builds M_ChargeIndicatorDecal (+ M_ChargeIndicatorDecal_Inst) under /Game/Materials.

The material is a deferred decal that draws the lane of a charge attack: two
bright rails along the edges of the lane -- so the width reads at a glance --
with a gradient falling inwards from each rail, and a fade that takes the whole
thing out towards the far end of the charge.

Everything works in normalised decal UV space, so the world size comes from the
DecalComponent's DecalSize and one material serves every charge length. The
LengthScale parameter trims the lane from the far end without touching the
component, which is what you drive at runtime if the indicator should grow or
shrink while the enemy winds up.

This project runs Substrate (r.Substrate=True), so the old Decal Blend Mode /
Shading Model dropdowns are gone -- the output goes Slab BSDF -> Convert To
Decal -> Front Material instead, same as M_AttackRangeDecal. Coverage is the
decal's blend mask, and a black Diffuse Albedo keeps the slab emissive-only,
which is what makes it read on landscape materials too.

Run from the editor:  Tools > Execute Python Script...  and pick this file.
"""

import unreal

PACKAGE_PATH = "/Game/Materials"
MAT_NAME = "M_ChargeIndicatorDecal"
MI_NAME = "M_ChargeIndicatorDecal_Inst"
GROUP = "Charge Indicator"

MEL = unreal.MaterialEditingLibrary
EAL = unreal.EditorAssetLibrary
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()


def fail(msg):
    unreal.log_error("[ChargeIndicatorDecal] " + msg)
    raise RuntimeError(msg)


def warn(msg):
    unreal.log_warning("[ChargeIndicatorDecal] " + msg)


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
    """Builds the lane shape. Returns (emissive_colour, opacity) expressions."""

    def node(cls, x, y, **props):
        n = MEL.create_material_expression(mat, cls, x, y)
        for k, v in props.items():
            n.set_editor_property(k, v)
        return n

    def link(src, dst, dst_input, src_output=""):
        MEL.connect_material_expressions(src, src_output, dst, dst_input)

    def scalar(name, default, x, y):
        return node(unreal.MaterialExpressionScalarParameter, x, y,
                    parameter_name=name, default_value=default, group=GROUP)

    def mask(source, x, y, r=False, g=False):
        m = node(unreal.MaterialExpressionComponentMask, x, y, r=r, g=g,
                 b=False, a=False)
        link(source, m, "")
        return m

    def lerp(a, b, alpha, x, y):
        n = node(unreal.MaterialExpressionLinearInterpolate, x, y)
        link(a, n, "A")
        link(b, n, "B")
        link(alpha, n, "Alpha")
        return n

    # -- decal UV -> lane space -------------------------------------------- #
    # Decal UVs run 0..1 across the projection box. Which of the two axes runs
    # along the lane depends on how the DecalComponent is oriented, and which
    # end of that axis the enemy stands on depends on which way it faces --
    # rather than bake in a guess, both are switches on the instance: AxisSwap
    # picks the lane axis, FlipDirection picks the end the charge starts from.
    uv = node(unreal.MaterialExpressionTextureCoordinate, -2350, 0)
    u = mask(uv, -2150, -120, r=True)
    v = mask(uv, -2150, 40, g=True)

    axis_swap = scalar("AxisSwap", 0.0, -2150, 220)
    flip = scalar("FlipDirection", 1.0, -2150, 380)

    across_raw = lerp(u, v, axis_swap, -1950, -160)
    along_raw = lerp(v, u, axis_swap, -1950, 120)

    along_flipped = node(unreal.MaterialExpressionOneMinus, -1800, 260)
    link(along_raw, along_flipped, "")
    # 0 at the enemy, 1 at the far end of the projection box.
    along = lerp(along_raw, along_flipped, flip, -1650, 140)

    # -- lateral distance: 0 down the centre line, 1 at either rail --------- #
    centred = node(unreal.MaterialExpressionSubtract, -1800, -160,
                   const_b=0.5)
    link(across_raw, centred, "A")

    doubled = node(unreal.MaterialExpressionMultiply, -1650, -160,
                   const_b=2.0)
    link(centred, doubled, "A")

    lateral = node(unreal.MaterialExpressionAbs, -1500, -160)
    link(doubled, lateral, "")

    # -- the two rails ------------------------------------------------------ #
    # LineWidth is a fraction of the half-lane, so 0.12 is a rail covering the
    # outer 12% of each side. The rail is solid from LineSoftness of its own
    # width inwards, so it still reads as a line and not as another gradient.
    line_width = scalar("LineWidth", 0.12, -1500, 20)
    line_soft = scalar("LineSoftness", 0.5, -1500, 180)

    inner_edge = node(unreal.MaterialExpressionSubtract, -1330, 20,
                      const_a=1.0)
    link(line_width, inner_edge, "B")

    soft_span = node(unreal.MaterialExpressionMultiply, -1330, 180)
    link(line_width, soft_span, "A")
    link(line_soft, soft_span, "B")

    soft_edge = node(unreal.MaterialExpressionAdd, -1180, 100)
    link(inner_edge, soft_edge, "A")
    link(soft_span, soft_edge, "B")

    rails = smoothstep_node(node, link, lateral, inner_edge, soft_edge,
                            -1000, -160)

    # -- gradient falling inwards from both rails --------------------------- #
    grad_power = scalar("GradientPower", 3.0, -1330, 380)
    grad = node(unreal.MaterialExpressionPower, -1180, 320)
    link(lateral, grad, "Base")
    link(grad_power, grad, "Exponent")

    inner_glow = scalar("InnerGlow", 0.35, -1180, 480)
    grad_opacity = node(unreal.MaterialExpressionMultiply, -1000, 340)
    link(grad, grad_opacity, "A")
    link(inner_glow, grad_opacity, "B")

    shape = node(unreal.MaterialExpressionAdd, -820, 60)
    link(rails, shape, "A")
    link(grad_opacity, shape, "B")

    shape_clamped = node(unreal.MaterialExpressionClamp, -680, 60)
    link(shape, shape_clamped, "")

    # -- length: trim + fade towards the far end ---------------------------- #
    # t is 0..1 over the *used* part of the lane, so the fades keep their
    # proportions when LengthScale animates. Past t = 1 the smoothstep
    # saturates and the mask hits zero, which is what cuts the lane short.
    length_scale = scalar("LengthScale", 1.0, -1650, 620)
    safe_length = node(unreal.MaterialExpressionMax, -1500, 620,
                       const_b=0.001)
    link(length_scale, safe_length, "A")

    t = node(unreal.MaterialExpressionDivide, -1330, 600)
    link(along, t, "A")
    link(safe_length, t, "B")

    tip_fade = scalar("TipFade", 0.35, -1500, 800)
    safe_tip = node(unreal.MaterialExpressionMax, -1330, 800, const_b=0.001)
    link(tip_fade, safe_tip, "A")

    tip_start = node(unreal.MaterialExpressionSubtract, -1180, 800,
                     const_a=1.0)
    link(safe_tip, tip_start, "B")

    tip = smoothstep_node(node, link, t, tip_start, 1.0, -1000, 620)
    end_mask = node(unreal.MaterialExpressionOneMinus, -840, 620)
    link(tip, end_mask, "")

    start_fade = scalar("StartFade", 0.05, -1330, 980)
    safe_start = node(unreal.MaterialExpressionMax, -1180, 980, const_b=0.001)
    link(start_fade, safe_start, "A")

    start_mask = smoothstep_node(node, link, t, 0.0, safe_start, -1000, 900)

    length_mask = node(unreal.MaterialExpressionMultiply, -700, 740)
    link(end_mask, length_mask, "A")
    link(start_mask, length_mask, "B")

    # -- combine ------------------------------------------------------------ #
    masked = node(unreal.MaterialExpressionMultiply, -500, 300)
    link(shape_clamped, masked, "A")
    link(length_mask, masked, "B")

    master_opacity = scalar("Opacity", 1.0, -500, 500)
    scaled = node(unreal.MaterialExpressionMultiply, -340, 300)
    link(masked, scaled, "A")
    link(master_opacity, scaled, "B")

    opacity = node(unreal.MaterialExpressionClamp, -180, 300)
    link(scaled, opacity, "")

    # -- colour ------------------------------------------------------------- #
    colour = node(unreal.MaterialExpressionVectorParameter, -500, -320,
                  parameter_name="LineColor", group=GROUP,
                  default_value=unreal.LinearColor(1.0, 0.18, 0.06, 1.0))
    brightness = scalar("Brightness", 3.0, -500, -170)

    emissive = node(unreal.MaterialExpressionMultiply, -280, -320)
    link(colour, emissive, "A")
    link(brightness, emissive, "B")

    return emissive, opacity


def connect_output(mat, emissive, opacity):
    """Wires the shape into the material output.

    Under Substrate the decal goes Slab BSDF -> Convert To Decal -> Front
    Material; the pre-Substrate path (Emissive + Opacity, with Decal Blend Mode
    set to Emissive) is kept as a fallback so the script still works on a
    project with r.Substrate off.
    """
    def node(cls, x, y, **props):
        n = MEL.create_material_expression(mat, cls, x, y)
        for k, v in props.items():
            n.set_editor_property(k, v)
        return n

    def link_any(src, dst, candidates, label):
        """Connects to the first input name that takes -- pin names on the
        Substrate nodes have moved around between engine versions."""
        for name in candidates:
            try:
                if MEL.connect_material_expressions(src, "", dst, name):
                    return True
            except Exception:
                pass
        warn("Could not connect {0} automatically -- hook that one pin up by "
             "hand in the material editor.".format(label))
        return False

    slab_cls = getattr(unreal, "MaterialExpressionSubstrateSlabBSDF", None)
    decal_cls = getattr(unreal, "MaterialExpressionSubstrateConvertToDecal",
                        None)
    front = getattr(unreal.MaterialProperty, "MP_FRONT_MATERIAL", None)

    if slab_cls is None or decal_cls is None or front is None:
        mat.set_editor_property("decal_blend_mode",
                                unreal.DecalBlendMode.DBM_EMISSIVE)
        mat.set_editor_property("shading_model",
                                unreal.MaterialShadingModel.MSM_UNLIT)
        MEL.connect_material_property(emissive, "",
                                      unreal.MaterialProperty.MP_EMISSIVE_COLOR)
        MEL.connect_material_property(opacity, "",
                                      unreal.MaterialProperty.MP_OPACITY)
        return

    # Black albedo keeps the slab emissive-only. Leave it at the slab default
    # (0.18 grey) and Coverage would paint a grey lane over the ground.
    black = node(unreal.MaterialExpressionConstant3Vector, -280, -140,
                 constant=unreal.LinearColor(0.0, 0.0, 0.0, 1.0))

    slab = node(slab_cls, -60, -320)
    link_any(black, slab, ["Diffuse Albedo", "DiffuseAlbedo", "Albedo"],
             "Diffuse Albedo")
    link_any(emissive, slab, ["Emissive Color", "EmissiveColor", "Emissive"],
             "Emissive Color")

    to_decal = node(decal_cls, 180, -60)
    link_any(slab, to_decal, ["", "DecalMaterial", "Decal Material",
                              "Material"], "Decal Material")
    link_any(opacity, to_decal, ["Coverage"], "Coverage")

    MEL.connect_material_property(to_decal, "", front)


def smoothstep_node(node, link, value, minimum, maximum, x, y):
    """SmoothStep(min, max, value); min and max may each be a node or a float.

    Falls back to a linear remap if the SmoothStep node class is unavailable.
    """
    def is_expr(v):
        return isinstance(v, unreal.MaterialExpression)

    cls = getattr(unreal, "MaterialExpressionSmoothStep", None)
    if cls is not None:
        props = {}
        if not is_expr(minimum):
            props["const_min"] = float(minimum)
        if not is_expr(maximum):
            props["const_max"] = float(maximum)
        n = node(cls, x, y, **props)
        if is_expr(minimum):
            link(minimum, n, "Min")
        if is_expr(maximum):
            link(maximum, n, "Max")
        link(value, n, "Value")
        return n

    span = node(unreal.MaterialExpressionSubtract, x, y + 160)
    if is_expr(maximum):
        link(maximum, span, "A")
    else:
        span.set_editor_property("const_a", float(maximum))
    if is_expr(minimum):
        link(minimum, span, "B")
    else:
        span.set_editor_property("const_b", float(minimum))

    offset = node(unreal.MaterialExpressionSubtract, x, y)
    link(value, offset, "A")
    if is_expr(minimum):
        link(minimum, offset, "B")
    else:
        offset.set_editor_property("const_b", float(minimum))

    div = node(unreal.MaterialExpressionDivide, x + 150, y)
    link(offset, div, "A")
    link(span, div, "B")

    clamped = node(unreal.MaterialExpressionClamp, x + 300, y)
    link(div, clamped, "")
    return clamped


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
                            unreal.MaterialDomain.MD_DEFERRED_DECAL)
    mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)

    emissive, opacity = make_graph(mat)
    connect_output(mat, emissive, opacity)
    MEL.recompile_material(mat)

    mi = TOOLS.create_asset(MI_NAME, PACKAGE_PATH,
                            unreal.MaterialInstanceConstant,
                            unreal.MaterialInstanceConstantFactoryNew())
    if mi is None:
        fail("Could not create " + MI_NAME)
    MEL.set_material_instance_parent(mi, mat)

    EAL.save_asset("{0}/{1}".format(PACKAGE_PATH, MAT_NAME))
    EAL.save_asset("{0}/{1}".format(PACKAGE_PATH, MI_NAME))

    unreal.log("[ChargeIndicatorDecal] Created {0}/{1} and {0}/{2}".format(
        PACKAGE_PATH, MAT_NAME, MI_NAME))


run()
