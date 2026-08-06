"""
Builds M_AttackRangeDecal (+ MI_AttackRangeDecal) under /Game/Materials/Decals.

The material is a deferred decal that draws a soft-edged ring with an optional
faint fill, sized entirely by the DecalComponent's DecalSize -- the graph itself
works in normalised decal UV space, so one material serves any range.

Decal Blend Mode is Emissive, which writes straight to the scene colour after
lighting. That is what makes it read the same on landscape, on the prototype
BSP-ish meshes, and in shadow -- it does not care what the receiving material's
base colour or lighting is.

Run from the editor:  Tools > Execute Python Script...  and pick this file.
"""

import unreal

PACKAGE_PATH = "/Game/Materials/Decals"
MAT_NAME = "M_AttackRangeDecal"
MI_NAME = "MI_AttackRangeDecal"

MEL = unreal.MaterialEditingLibrary
EAL = unreal.EditorAssetLibrary
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()


def fail(msg):
    unreal.log_error("[AttackRangeDecal] " + msg)
    raise RuntimeError(msg)


def guard_existing():
    for name in (MAT_NAME, MI_NAME):
        path = "{0}/{1}".format(PACKAGE_PATH, name)
        if EAL.does_asset_exist(path):
            fail("{0} already exists. Delete or rename it first -- this script "
                 "will not overwrite an existing asset.".format(path))


# --------------------------------------------------------------------------- #
# graph helpers
# --------------------------------------------------------------------------- #

def make_graph(mat):
    def node(cls, x, y, **props):
        n = MEL.create_material_expression(mat, cls, x, y)
        for k, v in props.items():
            n.set_editor_property(k, v)
        return n

    def link(src, dst, dst_input, src_output=""):
        MEL.connect_material_expressions(src, src_output, dst, dst_input)

    def scalar(name, default, x, y):
        return node(unreal.MaterialExpressionScalarParameter, x, y,
                    parameter_name=name, default_value=default,
                    group="Range Decal")

    # -- radial distance from decal centre -------------------------------- #
    # Decal UVs run 0..1 across the projection box, so centre is 0.5.
    # Remapping to -1..1 makes Length() return 1.0 exactly at the box edge,
    # which is the character's attack range.
    uv = node(unreal.MaterialExpressionTextureCoordinate, -1750, 0)
    centre = node(unreal.MaterialExpressionConstant2Vector, -1750, 160,
                  r=0.5, g=0.5)

    centred = node(unreal.MaterialExpressionSubtract, -1550, 0)
    link(uv, centred, "A")
    link(centre, centred, "B")

    scaled = node(unreal.MaterialExpressionMultiply, -1400, 0, const_b=2.0)
    link(centred, scaled, "A")

    radius = length_node(node, link, scaled, -1250, 0)

    # -- ring mask --------------------------------------------------------- #
    ring_radius = scalar("RingRadius", 0.9, -1250, 220)
    thickness = scalar("RingThickness", 0.05, -1050, 300)

    offset = node(unreal.MaterialExpressionSubtract, -1050, 0)
    link(radius, offset, "A")
    link(ring_radius, offset, "B")

    dist = node(unreal.MaterialExpressionAbs, -900, 0)
    link(offset, dist, "")

    falloff = smoothstep_node(node, link, dist, thickness, -720, 0)
    ring = node(unreal.MaterialExpressionOneMinus, -540, 0)
    link(falloff, ring, "")

    # -- faint interior fill ----------------------------------------------- #
    # saturate((RingRadius - r) * 40) gives a hard-ish disc that stops just
    # inside the ring; the *40 is only there to keep the edge from aliasing.
    inside = node(unreal.MaterialExpressionSubtract, -900, 460)
    link(ring_radius, inside, "A")
    link(radius, inside, "B")

    inside_sharp = node(unreal.MaterialExpressionMultiply, -740, 460,
                        const_b=40.0)
    link(inside, inside_sharp, "A")

    fill_mask = node(unreal.MaterialExpressionClamp, -580, 460)
    link(inside_sharp, fill_mask, "")

    fill_opacity = scalar("FillOpacity", 0.10, -740, 620)
    fill = node(unreal.MaterialExpressionMultiply, -400, 460)
    link(fill_mask, fill, "A")
    link(fill_opacity, fill, "B")

    # -- combine ----------------------------------------------------------- #
    combined = node(unreal.MaterialExpressionAdd, -240, 200)
    link(ring, combined, "A")
    link(fill, combined, "B")

    opacity = node(unreal.MaterialExpressionClamp, -80, 200)
    link(combined, opacity, "")

    # -- colour ------------------------------------------------------------ #
    colour = node(unreal.MaterialExpressionVectorParameter, -400, -260,
                  parameter_name="RingColor", group="Range Decal",
                  default_value=unreal.LinearColor(0.15, 0.85, 1.0, 1.0))
    brightness = scalar("Brightness", 2.0, -400, -110)

    emissive = node(unreal.MaterialExpressionMultiply, -180, -260)
    link(colour, emissive, "A")
    link(brightness, emissive, "B")

    MEL.connect_material_property(emissive, "",
                                  unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    MEL.connect_material_property(opacity, "",
                                  unreal.MaterialProperty.MP_OPACITY)


def length_node(node, link, source, x, y):
    """Length() with a dot/sqrt fallback in case the node class is missing."""
    cls = getattr(unreal, "MaterialExpressionLength", None)
    if cls is not None:
        n = node(cls, x, y)
        link(source, n, "")
        return n

    dot = node(unreal.MaterialExpressionDotProduct, x, y)
    link(source, dot, "A")
    link(source, dot, "B")
    root = node(unreal.MaterialExpressionSquareRoot, x + 150, y)
    link(dot, root, "")
    return root


def smoothstep_node(node, link, value, max_input, x, y):
    """SmoothStep(0, max, value) with a linear divide/clamp fallback."""
    cls = getattr(unreal, "MaterialExpressionSmoothStep", None)
    if cls is not None:
        n = node(cls, x, y, const_min=0.0)
        link(max_input, n, "Max")
        link(value, n, "Value")
        return n

    div = node(unreal.MaterialExpressionDivide, x, y)
    link(value, div, "A")
    link(max_input, div, "B")
    clamped = node(unreal.MaterialExpressionClamp, x + 150, y)
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
    mat.set_editor_property("decal_blend_mode",
                            unreal.DecalBlendMode.DBM_EMISSIVE)
    mat.set_editor_property("shading_model",
                            unreal.MaterialShadingModel.MSM_UNLIT)

    make_graph(mat)
    MEL.recompile_material(mat)

    mi = TOOLS.create_asset(MI_NAME, PACKAGE_PATH,
                            unreal.MaterialInstanceConstant,
                            unreal.MaterialInstanceConstantFactoryNew())
    if mi is None:
        fail("Could not create " + MI_NAME)
    MEL.set_material_instance_parent(mi, mat)

    EAL.save_asset("{0}/{1}".format(PACKAGE_PATH, MAT_NAME))
    EAL.save_asset("{0}/{1}".format(PACKAGE_PATH, MI_NAME))

    unreal.log("[AttackRangeDecal] Created {0}/{1} and {0}/{2}".format(
        PACKAGE_PATH, MAT_NAME, MI_NAME))


run()
