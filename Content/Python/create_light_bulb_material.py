"""
Builds M_LightBulb (+ MI_LightBulb) under /Game/Materials/LightFlicker.

Replacement for M_BlinkingLight, which drove Emissive from a Floor/Fmod square
wave: the bulb snapped between two flat colours, so it read as a coloured dot
switching on and off rather than as a lamp. This one keeps the bulb *shaped*
and gives the blink a filament curve.

What makes it read as a bulb:

  * Fresnel rim. The glass edge stays bright even when the bulb is dim, so an
    off bulb still reads as a bulb instead of a black dot.
  * Inverse fresnel core. The centre of the sphere -- the bit facing camera --
    is the hottest, which is what sells a small round mesh as something lit
    from the inside rather than a flat emissive blob.
  * Hot-core desaturation. The brighter the core gets, the further it lerps
    towards HotColor (near white). Real emitters blow out to white in the
    middle and keep their saturation at the edges; holding one flat colour at
    every intensity is most of why the old one looked cheap.

What makes the blink read as a filament:

  * Asymmetric pulse. Fast rise (RiseTime), slow decay (FallTime) -- a filament
    heats quickly and cools slowly. Instant on/off is the giveaway of a
    material-editor blink.
  * A shallow flicker riding on top (two detuned sines, no texture) so a fully
    lit bulb never sits perfectly still.
  * Chase support without a material per bulb: ChaseFrequency offsets the phase
    by local position along PhaseAxis, so every bulb on the same mesh lights in
    sequence around the machine. Leave it at 0 and they all blink together.

Everything is unlit -- bulbs light themselves, and Brightness is meant to be
pushed past 1 so bloom does the glow. No light components required.

Run from the editor:  Tools > Execute Python Script...  and pick this file.
The script refuses to overwrite; delete both assets first to rebuild.
"""

import unreal

PACKAGE_PATH = "/Game/Materials/LightFlicker"
MAT_NAME = "M_LightBulb"
MI_NAME = "MI_LightBulb"

G_BLINK = "01 - Blink"
G_FLICKER = "02 - Flicker"
G_SHAPE = "03 - Bulb Shape"
G_COLOR = "04 - Colour"

MEL = unreal.MaterialEditingLibrary
EAL = unreal.EditorAssetLibrary
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()


def fail(msg):
    unreal.log_error("[LightBulb] " + msg)
    raise RuntimeError(msg)


def warn(msg):
    unreal.log_warning("[LightBulb] " + msg)


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
    """Builds the bulb. Returns the emissive expression."""

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

    def one_minus(a, x, y):
        n = node(unreal.MaterialExpressionOneMinus, x, y)
        link(a, n, "")
        return n

    def saturate(a, x, y):
        n = node(unreal.MaterialExpressionClamp, x, y)
        link(a, n, "")
        return n

    def power(base, exponent, x, y):
        n = node(unreal.MaterialExpressionPower, x, y)
        link(base, n, "Base")
        if isinstance(exponent, unreal.MaterialExpression):
            link(exponent, n, "Exponent")
        else:
            n.set_editor_property("const_exponent", float(exponent))
        return n

    def lerp(a, b, alpha, x, y):
        n = node(unreal.MaterialExpressionLinearInterpolate, x, y)
        if isinstance(a, unreal.MaterialExpression):
            link(a, n, "A")
        else:
            n.set_editor_property("const_a", float(a))
        if isinstance(b, unreal.MaterialExpression):
            link(b, n, "B")
        else:
            n.set_editor_property("const_b", float(b))
        link(alpha, n, "Alpha")
        return n

    def rgb(source, x, y):
        """RGB mask -- vector parameters output float4 and some nodes (Dot,
        the Substrate inputs) refuse a size mismatch."""
        m = node(unreal.MaterialExpressionComponentMask, x, y,
                 r=True, g=True, b=True, a=False)
        link(source, m, "")
        return m

    def at_least(a, floor_value, x, y):
        n = node(unreal.MaterialExpressionMax, x, y,
                 const_b=float(floor_value))
        link(a, n, "A")
        return n

    # ---------------------------------------------------------------- #
    # phase: time + per-bulb offset
    # ---------------------------------------------------------------- #
    time = node(unreal.MaterialExpressionTime, -2600, -60)

    blink_speed = scalar("BlinkSpeed", 1.5, -2600, 100, G_BLINK)
    t_scaled = mul(time, blink_speed, -2400, 0)

    phase_offset = scalar("PhaseOffset", 0.0, -2600, 260, G_BLINK)

    # Chase: local position along PhaseAxis shifts the phase, so one material
    # on one mesh lights its bulbs in sequence. ChaseFrequency is cycles per
    # local unit -- 0.01 is one full cycle per 100 units along the axis.
    pos_cls = getattr(unreal, "MaterialExpressionLocalPosition", None)
    if pos_cls is None:
        pos_cls = unreal.MaterialExpressionWorldPosition
        warn("Local Position node unavailable -- falling back to world "
             "position, so ChaseFrequency will shift with the actor.")
    local_pos = node(pos_cls, -2600, 420)

    phase_axis = vector("PhaseAxis", unreal.LinearColor(1.0, 0.0, 0.0, 1.0),
                        -2600, 580, G_BLINK)
    axis_rgb = rgb(phase_axis, -2380, 580)
    axis_dot = node(unreal.MaterialExpressionDotProduct, -2200, 460)
    link(local_pos, axis_dot, "A")
    link(axis_rgb, axis_dot, "B")

    chase_freq = scalar("ChaseFrequency", 0.0, -2380, 640, G_BLINK)
    chase_phase = mul(axis_dot, chase_freq, -2180, 500)

    phase_sum = add(t_scaled, phase_offset, -2180, 120)
    phase_total = add(phase_sum, chase_phase, -2000, 220)

    # 0..1 inside one blink cycle.
    p = node(unreal.MaterialExpressionFrac, -1840, 220)
    link(phase_total, p, "")

    # ---------------------------------------------------------------- #
    # the pulse: fast rise, slow decay
    # ---------------------------------------------------------------- #
    rise_time = scalar("RiseTime", 0.08, -1840, 400, G_BLINK)
    safe_rise = at_least(rise_time, 0.001, -1660, 400)
    rise = smoothstep_node(node, link, p, 0.0, safe_rise, -1480, 220)

    duty = scalar("DutyCycle", 0.45, -1840, 580, G_BLINK)
    fall_time = scalar("FallTime", 0.30, -1840, 740, G_BLINK)
    safe_fall = at_least(fall_time, 0.001, -1660, 740)
    fall_end = add(duty, safe_fall, -1480, 660)

    fall_ramp = smoothstep_node(node, link, p, duty, fall_end, -1300, 560)
    fall = one_minus(fall_ramp, -1080, 560)

    pulse = mul(rise, fall, -900, 320)

    # BlinkAmount 0 holds the bulb steady on, 1 is the full pulse -- handy for
    # a "win" state where the machine goes solid instead of chasing.
    blink_amount = scalar("BlinkAmount", 1.0, -900, 500, G_BLINK)
    blink = lerp(1.0, pulse, blink_amount, -720, 320)

    # ---------------------------------------------------------------- #
    # flicker: two detuned sines, no texture
    # ---------------------------------------------------------------- #
    flicker_speed = scalar("FlickerSpeed", 9.0, -1840, 900, G_FLICKER)
    f_t = mul(time, flicker_speed, -1660, 900)
    sine_a = node(unreal.MaterialExpressionSine, -1480, 860)
    link(f_t, sine_a, "")
    f_t2 = mul(f_t, 1.7, -1480, 1020)
    sine_b = node(unreal.MaterialExpressionSine, -1300, 1020)
    link(f_t2, sine_b, "")

    noise = mul(sine_a, sine_b, -1120, 920)          # -1 .. 1
    noise_01 = mul(noise, -0.5, -940, 920)
    dip_base = add(noise_01, 0.5, -780, 920)         # 0 .. 1

    flicker_amount = scalar("FlickerAmount", 0.06, -940, 1080, G_FLICKER)
    dip = mul(dip_base, flicker_amount, -620, 960)
    flicker_mul = one_minus(dip, -460, 960)

    # ---------------------------------------------------------------- #
    # heat: 0 = cold glass, 1 = fully lit
    # ---------------------------------------------------------------- #
    is_on = scalar("IsOn", 1.0, -720, 500, G_BLINK)
    gated = mul(blink, is_on, -540, 380)
    heat = mul(gated, flicker_mul, -360, 420)

    # ---------------------------------------------------------------- #
    # bulb shape: fresnel rim + inverse fresnel core
    # ---------------------------------------------------------------- #
    rim_exp = scalar("RimExponent", 3.5, -2600, 900, G_SHAPE)
    fres = node(unreal.MaterialExpressionFresnel, -2380, 860,
                exponent=3.5, base_reflect_fraction=0.0)
    link_any(rim_exp, fres, ["ExponentIn", "Exponent"], "Fresnel exponent")

    facing = one_minus(fres, -2180, 860)             # 1 at centre, 0 at edge

    core_tight = scalar("CoreTightness", 2.5, -2380, 1040, G_SHAPE)
    core = power(facing, core_tight, -2000, 900)

    core_strength = scalar("CoreStrength", 1.4, -2180, 1060, G_SHAPE)
    core_term = mul(core, core_strength, -1820, 900)

    body_fill = scalar("BodyFill", 0.35, -2000, 1080, G_SHAPE)
    body_term = add(core_term, body_fill, -1640, 940)

    rim_strength = scalar("RimStrength", 0.9, -2180, 1220, G_SHAPE)
    rim_term = mul(fres, rim_strength, -1820, 1140)

    # ---------------------------------------------------------------- #
    # colour: the hot core blows out towards white
    # ---------------------------------------------------------------- #
    hot_raw = mul(core_term, heat, -180, -700)
    hot_amt = saturate(hot_raw, -20, -700)
    white_hot = scalar("WhiteHot", 1.5, -180, -560, G_COLOR)
    white_mix = power(hot_amt, white_hot, 140, -700)

    bulb_col = vector("BulbColor", unreal.LinearColor(1.0, 0.45, 0.12, 1.0),
                      -180, -1080, G_COLOR)
    hot_col = vector("HotColor", unreal.LinearColor(1.0, 0.92, 0.75, 1.0),
                     -180, -920, G_COLOR)
    tint = lerp(bulb_col, hot_col, white_mix, 340, -900)

    body_col = mul(tint, body_term, 540, -900)

    rim_col = vector("RimColor", unreal.LinearColor(1.0, 0.72, 0.35, 1.0),
                     340, -520, G_COLOR)
    rim_col_term = mul(rim_col, rim_term, 540, -520)

    on_col = add(body_col, rim_col_term, 740, -760)
    on_heated = mul(on_col, heat, 920, -760)

    brightness = scalar("Brightness", 6.0, 740, -420, G_COLOR)
    on_emissive = mul(on_heated, brightness, 1100, -700)

    # ---------------------------------------------------------------- #
    # off state: dark glass that keeps its rim, faded out as the bulb lights
    # ---------------------------------------------------------------- #
    off_col = vector("OffColor", unreal.LinearColor(0.03, 0.02, 0.015, 1.0),
                     340, -200, G_COLOR)
    glass_rim = scalar("GlassRim", 0.25, 340, -40, G_SHAPE)
    glass_rim_term = mul(fres, glass_rim, 540, -60)
    glass_col = mul(rim_col, glass_rim_term, 740, -140)
    off_full = add(off_col, glass_col, 920, -220)

    cold = one_minus(heat, 740, 120)
    off_emissive = mul(off_full, cold, 1100, -180)

    total = add(on_emissive, off_emissive, 1320, -460)
    return rgb(total, 1480, -460)


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


def connect_output(mat, emissive):
    """Wires the bulb into the material output.

    This project runs Substrate (r.Substrate=True), so the output goes through
    an Unlit BSDF -> Front Material where those nodes exist; the legacy
    Emissive + MSM_Unlit path is kept as a fallback.
    """
    unlit_cls = getattr(unreal, "MaterialExpressionSubstrateUnlitBSDF", None)
    front = getattr(unreal.MaterialProperty, "MP_FRONT_MATERIAL", None)

    if unlit_cls is None or front is None:
        MEL.connect_material_property(
            emissive, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
        return

    unlit = MEL.create_material_expression(mat, unlit_cls, 1560, -460)
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
    mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_OPAQUE)
    mat.set_editor_property("shading_model",
                            unreal.MaterialShadingModel.MSM_UNLIT)

    emissive = make_graph(mat)
    connect_output(mat, emissive)
    MEL.recompile_material(mat)

    mi = TOOLS.create_asset(MI_NAME, PACKAGE_PATH,
                            unreal.MaterialInstanceConstant,
                            unreal.MaterialInstanceConstantFactoryNew())
    if mi is None:
        fail("Could not create " + MI_NAME)
    MEL.set_material_instance_parent(mi, mat)

    EAL.save_asset("{0}/{1}".format(PACKAGE_PATH, MAT_NAME))
    EAL.save_asset("{0}/{1}".format(PACKAGE_PATH, MI_NAME))

    unreal.log("[LightBulb] Created {0}/{1} and {0}/{2}".format(
        PACKAGE_PATH, MAT_NAME, MI_NAME))


run()
