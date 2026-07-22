# M_LiquidFillBar — Build Guide

A bar-fill material that reads as a viscous liquid: wobbling surface line, slow
internal turbulence driven by noise textures, and bubbles drifting up through the body.
One master material handles both horizontal and vertical bars via a static switch.

Built for a **3D mesh** used as a worldspace UI element — not a UMG widget.

**Create at:** `Content/Materials/UI_Materials/M_LiquidFillBar`

---

## 0. Material settings

Select the output node, then in Details:

| Setting | Value |
| --- | --- |
| Material Domain | **Surface** |
| Blend Mode | **Translucent** |
| Shading Model | **Unlit** |
| Two Sided | on, if the bar is ever seen from behind |

**Unlit** is the important one. A UI element has to read the same regardless of where
it sits in the level — under Default Lit a health bar goes dim in a cave and blows out
next to a torch. Unlit means the colour you author is the colour on screen.

With Unlit + Translucent the active output pins are **Emissive Color** and **Opacity**.
Everywhere below that says "Final Color", wire it to **Emissive Color** instead.

Three notes before you start:

- **The mesh needs UV channel 0 mapped 0–1 across the bar face.** Everything here is
  driven by `TextureCoordinate`, so a plane with default UVs is ideal. If the UVs are
  tiled or offset the fill won't line up.
- **Noise texture import settings.** Select each noise texture you plan to use →
  turn **sRGB off**, set **Texture Address X/Y = Wrap** (not Clamp — the panners rely
  on tiling). On each sampler node set **Sampler Type = Linear Grayscale**.
- Everything below is one continuous graph. Section letters are just for reading; lay
  them out left-to-right in that order and the wires stay short.

### Drawing order

Worldspace UI usually needs to sit on top of the geometry behind it. If the bar gets
clipped by walls or sorts oddly against other translucent surfaces:

- **Disable Depth Test** (material → Translucency) draws it through everything. Use
  sparingly — it will also draw through the character it belongs to.
- **Translucency Sort Priority** on the mesh *component* resolves fights between
  several translucent objects. Higher draws later.

---

## A. Bar space — the direction switch

This block builds a single UV where **X always runs from the empty end to the full
end**, whichever way the bar points. Every later section works in this space, so the
switch only has to exist once.

1. `TextureCoordinate` (Coordinate Index 0) → call it **TexUV**
2. `ComponentMask` on TexUV, **R only** → `U`
3. `ComponentMask` on TexUV, **G only** → `V`
4. `OneMinus` on `V` → `InvV`
5. `AppendVector`: A = `U`, B = `V` → **HorizUV** *(fill runs left → right)*
6. `AppendVector`: A = `InvV`, B = `U` → **VertUV** *(fill runs bottom → top)*
7. `StaticSwitchParameter` named **`bVertical`**, Default Value = `false`
   - True  ← **VertUV**
   - False ← **HorizUV**
   - Output → **BarUV**
8. `ComponentMask` on BarUV, **R only** → **Along** *(0 = empty end, 1 = full end)*
9. `ComponentMask` on BarUV, **G only** → **Across** *(runs along the surface line)*

> Because it's a *static* switch, each instance compiles only the branch it uses —
> there's no runtime cost to supporting both directions.

---

## A2. Bar size — world space

This block makes the material **size-independent**. It takes the bar's real dimensions
so noise density, bubble size, wave wavelength and edge softness stay visually constant
whether the mesh is 30×4 cm or 200×25 cm, and whether it's scaled in the level or not.
Build it once here; every later section just divides by it.

Units are **centimetres** (Unreal world units) rather than pixels. The maths is
identical — it's a reference scale, not a screen measurement — so all the `Px`-suffixed
parameters below now mean *cm*. Keep the names or rename them to `Cm`, your call; just
be consistent.

### Getting the size

10. `VectorParameter` named **`BarSize`**, **Default Value `(30, 4, 0, 0)`**
    — the mesh's dimensions at scale 1, in cm. Read them off the static mesh's
    bounds, or just measure the plane you're using.
11. `ObjectScale` node → `ComponentMask` **RG** → `scaleXY`
12. `Multiply`: A = **BarSize** (masked **RG**), B = `scaleXY` → `SizeCm`
13. `ComponentMask` **R** → **WidthCm**, `ComponentMask` **G** → **HeightCm**

`ObjectScale` is what makes step 11–12 worth the two extra nodes: scale the mesh in the
level and the material compensates, so a bar stretched to twice the width keeps the same
bubble size and ripple count instead of smearing.

> If your mesh's local X/Y don't line up with its UV U/V — common on a plane that's been
> rotated in its source file — swap the mask channels in step 11 until it looks right.
> Skip `ObjectScale` entirely if you never scale the mesh non-uniformly.

### Swapping them for orientation

`Along` and `Across` swap meaning when the bar is vertical, so their lengths have to
swap with them.

14. `AppendVector`: A = **WidthCm**, B = **HeightCm** → `SizeH`
15. `AppendVector`: A = **HeightCm**, B = **WidthCm** → `SizeV`
16. `StaticSwitchParameter` named **`bVertical`** — same name as step 7
    - True ← `SizeV`, False ← `SizeH` → **BarSizeCm**
17. `ComponentMask` **R** → `alongRaw` → `Max`(A = `alongRaw`, B = `Constant` `0.01`)
    → **AlongPx**   *(bar length along the fill axis)*
18. `ComponentMask` **G** → `acrossRaw` → `Max`(A = `acrossRaw`, B = `Constant` `0.01`)
    → **AcrossPx**  *(bar width across the surface)*

> Two `StaticSwitchParameter` nodes sharing the name `bVertical` are the *same*
> parameter — set it once on the instance and both flip together. That's why this is a
> second node rather than a wire dragged from step 7.

The `Max` clamps in 17–18 are cheap insurance. Everything downstream divides by these,
so a zero — from a mesh scaled to 0 on an axis, or a parameter left unset — would
otherwise produce infinities and blank the material.

---

## B. Noise scale

Noise density is now specified as **how many pixels one tile of noise covers** — so it
holds steady at any widget size.

19. `ScalarParameter` **`NoiseScalePx`**, default `15`
20. `Multiply`: A = **BarUV**, B = **BarSizeCm** → `BarPx` *(position in cm)*
21. `Divide`: A = `BarPx`, B = **NoiseScalePx** → **NoiseUV**

`NoiseTiling` from the old version is gone — no more hand-matching the aspect ratio.
Lower `NoiseScalePx` for finer, busier turbulence; raise it for broad slow swirls.

This also fixes flow speed for free: `FlowSpeed` is in tiles/sec, and a tile is now a
fixed pixel count, so the liquid drifts at the same visual rate on every bar size.

---

## C. Flow noise — the body turbulence

Two noise samples at different scales and speeds, drifting toward the surface. Layering
two kills the "obviously one tiling texture" look.

22. `ScalarParameter` **`FlowSpeed`**, default `0.06`
23. `Constant2Vector` `(-1.0, 0.15)` → `Multiply` by **FlowSpeed** → `SpeedA`
24. `Panner`: Coordinate = **NoiseUV**, Speed = `SpeedA` → `FlowUV_A`
25. `TextureSampleParameter2D` named **`NoiseA`** → UVs = `FlowUV_A` → take **R** → `nA`
26. `Multiply`: **NoiseUV** × `Constant` `0.63` → `NoiseUV_B`
    `Constant2Vector` `(-0.6, -0.22)` → `Multiply` by **FlowSpeed** → `SpeedB`
    `Panner`: Coordinate = `NoiseUV_B`, Speed = `SpeedB` → `FlowUV_B`
27. `TextureSampleParameter2D` named **`NoiseB`** → UVs = `FlowUV_B` → take **R** → `nB`
28. `Add` `nA` + `nB` → `Multiply` by `0.5` → `Noise01`
29. `Subtract` `Noise01` − `0.5` → **NoiseC** *(centred, −0.5 … 0.5)*

**NoiseC is reused four times** — surface wobble, bubble distortion, density variation.
That shared source is what makes the layers feel like one fluid instead of three effects.

**Texture picks:** start with `T_NoiseMask_Merged_Liq_01`
(`Content/Textures/VFXNoiseTexPack/Textures/NoiseMaskPacked/`) for `NoiseA` — it's the
liquid-oriented packed set, and its other channels are worth auditioning for `NoiseB`.
Otherwise browse `.../Textures/Noises/T_NoiseMask_*` and pick two soft, cloudy ones;
avoid the high-frequency grainy entries, they read as static rather than fluid.

---

## D. Surface wave

The fill line isn't flat — it's two out-of-phase sine waves plus a noise offset. Both
the wavelength and the height are specified **in pixels**, so a wide bar gets more
ripples rather than the same few stretched wider.

30. `ScalarParameter` **`WaveLengthPx`** = `0.7`, **`WaveSpeed`** = `1.1`,
    **`WaveAmplitudePx`** = `0.35`
31. Amplitude in Along-space: `Divide` **WaveAmplitudePx** / **AlongPx**
    → **WaveAmplitude**
32. Position along the surface, in cycles: `Multiply` **Across** × **AcrossPx**
    → `Divide` by **WaveLengthPx** → **Cycles**
33. Wave 1: `Add` **Cycles** + (`Time` × **WaveSpeed**) → `Sine`
    → `Multiply` × **WaveAmplitude** → `Wave1`
34. Wave 2: `Multiply` **Cycles** × `0.37` → `Add` (`Time` × **WaveSpeed** × `-0.7`)
    → `Sine` → `Multiply` × (**WaveAmplitude** × `0.6`) → `Wave2`
35. `ScalarParameter` **`SurfaceNoise`** = `0.5`
    `Multiply` **NoiseC** × **SurfaceNoise** × **WaveAmplitude** → `WaveNoise`
36. `Add` `Wave1` + `Wave2` → `Add` `WaveNoise` → **WaveOffset**

> **Cycles** is shared by both waves, so step 34 only needs one extra multiply. The
> `Sine` node treats input in whole cycles (Period = 1), which is why dividing pixel
> position by pixel wavelength lands directly on the right units.

### Sizing the wavelength

`WaveLengthPx` is measured against **AcrossPx**, and on a horizontal bar that's the
bar's *height* — the short axis, because the surface is the vertical fill edge. It's
easy to set this far too long without noticing.

**Aim for `WaveLengthPx ≈ AcrossPx / 6`.** A 4 cm tall bar wants about `0.7`.

Set it equal to `AcrossPx` and Wave 1 completes exactly one cycle while Wave 2, at
`0.37×` the frequency, never finishes a period at all — it degenerates into a slight
tilt and the surface reads as a single plain sine. You need Wave 1 at roughly 5–6
cycles before the interference between the two becomes legible as irregular motion.

Keep `WaveAmplitudePx` under about half of `WaveLengthPx`. Past that the surface gets
steeper than the edge softness can resolve and the ripples alias into jagged steps.

### Fade the wave at the extremes

Without this the bar looks broken at 0% (a sliver of liquid sloshes at the empty end)
and at 100% (the wave cuts a notch out of the top).

37. `ScalarParameter` **`Fill`**, default `0.5`, Slider Min `0` / Max `1`
38. `Multiply` **Fill** × `10` → `Saturate` → `fadeLo`
39. `OneMinus` **Fill** → `Multiply` × `10` → `Saturate` → `fadeHi`
40. `Multiply` `fadeLo` × `fadeHi` → **WaveFade**
41. `Multiply` **WaveOffset** × **WaveFade** → **WaveOffsetFaded**

---

## E. Fill line and liquid mask

42. `Add` **Fill** + **WaveOffsetFaded** → **FillLine**
43. `ScalarParameter` **`EdgeSoftnessPx`** = `0.15`
    → `Divide` **EdgeSoftnessPx** / **AlongPx** → **EdgeSoftness**
44. `Subtract` **FillLine** − **EdgeSoftness** → `edgeMin`
    `Add` **FillLine** + **EdgeSoftness** → `edgeMax`
45. `SmoothStep`: Min = `edgeMin`, Max = `edgeMax`, Value = **Along** → `OneMinus` → `maskRaw`
46. `Multiply` **Fill** × `50` → `Saturate` → `zeroGuard`
47. `Multiply` `maskRaw` × `zeroGuard` → **LiquidMask**

Step 46–47 guarantees a hard empty at `Fill = 0`; the smoothstep alone leaves a
hairline at the empty edge.

Expressing softness in world units is what gives you a consistent edge — `0.15` cm
stays a clean soft transition at every bar size, where a fixed `0.004` fraction was
razor-sharp on a long bar and visibly blurry on a short one.

On a 3D bar the camera distance also matters: a bar viewed from far away wants a
slightly wider softness to avoid shimmering as it shrinks on screen. If that bothers
you, raise this a little rather than reaching for a screen-space fix.

---

## F. Surface band (the meniscus)

The bright lip right at the liquid line. This is most of what sells "liquid" — without
it the fill reads as a flat coloured rectangle.

48. `Subtract` **FillLine** − **Along** → `Abs` → `distToSurface`
49. `ScalarParameter` **`SurfaceThicknessPx`** = `0.9`
    → `Divide` **SurfaceThicknessPx** / **AlongPx** → **SurfaceThickness**
50. `Divide` `distToSurface` / **SurfaceThickness** → `Saturate` → `OneMinus`
    → `Power` (Exp = `2`) → `SurfaceBand`
51. `Multiply` `SurfaceBand` × **LiquidMask** → **SurfaceMask**

The multiply in 51 clips the band to the liquid side. If you want a faint glow spilling
*above* the line, skip it and add `SurfaceBand` into Opacity instead (§I note).

---

## G. Bubbles

Blobs rising toward the surface, their paths wobbled by the same noise that drives the
body — so they look carried by the fluid rather than sliding over it.

52. `ScalarParameter`: **`BubbleScalePx`** = `6`, **`BubbleSpeed`** = `0.25`,
    **`BubbleWobble`** = `0.03`, **`BubbleThreshold`** = `0.62`,
    **`BubbleContrast`** = `14`, **`BubbleAmount`** = `1`
53. `Multiply` **NoiseC** × **BubbleWobble** → `bubbleDistort`
54. `Divide` `BarPx` (step 20) / **BubbleScalePx** → `Add` `bubbleDistort` → `BubbleUV0`
55. `Constant2Vector` `(-1, 0)` → `Multiply` by **BubbleSpeed** → `bubbleVel`
    `Panner`: Coordinate = `BubbleUV0`, Speed = `bubbleVel` → `BubbleUV`
56. `TextureSampleParameter2D` named **`BubbleNoise`** → UVs = `BubbleUV` → **R** → `bRaw`
57. `Subtract` `bRaw` − **BubbleThreshold** → `Multiply` × **BubbleContrast**
    → `Saturate` → `Bubbles`
58. `Multiply` `Bubbles` × **BubbleAmount** × **LiquidMask** → **BubbleMask**

`BubbleScalePx` is the width in cm of one bubble-noise tile, so bubbles keep a
constant physical size — a tall bar gets *more* bubbles, not bigger ones. Rise speed
comes out size-invariant for the same reason as the flow noise.

**Texture pick:** this one wants a *cellular / voronoi* texture, not cloudy perlin —
the threshold step in 57 turns cell cores into round blobs. Preview a few
`T_NoiseMask_*` entries and grab one with visible discrete cells.
`Content/Materials/Waterfall/T_Voronoi01` is already in the project and is a solid
fallback.

**Optional — pop at the surface:** `Subtract` **FillLine** − **Along** → `Multiply` × `6`
→ `Saturate`, and multiply that into **BubbleMask**. Bubbles then thin out as they
approach the line instead of clipping through it.

---

## H. Colour

**Set a Default Value on every one of these.** A fresh `VectorParameter` defaults to
black and a fresh `ScalarParameter` to `0` — leave them and the whole bar renders black
even though the mask and waves are working.

59. `VectorParameter`: **`LiquidColor`** `(1.0, 0.72, 0.15)`,
    **`LiquidColorDeep`** `(0.45, 0.16, 0.02)`, **`SurfaceColor`** `(1.0, 0.95, 0.6)`,
    **`BubbleColor`** `(1.0, 0.88, 0.45)`
60. `Subtract` **FillLine** − **Along** → `Saturate` → `Multiply` ×
    `ScalarParameter` **`DepthFalloff`** (= `2.5`) → `Saturate` → `Depth`
61. `Multiply` **NoiseC** × `ScalarParameter` **`DensityVariation`** (= `0.35`)
    → `Add` to `Depth` → `Saturate` → `DepthN`
62. `Lerp`: A = **LiquidColor**, B = **LiquidColorDeep**, Alpha = `DepthN` → `BodyColor`
63. `Lerp`: A = `BodyColor`, B = **BubbleColor**, Alpha = **BubbleMask** → `WithBubbles`
64. `Lerp`: A = `WithBubbles`, B = **SurfaceColor**, Alpha = **SurfaceMask** → `FinalRGB`
65. `Multiply` `FinalRGB` × `ScalarParameter` **`Brightness`** (default **`1.4`**,
    *not* `0`) → **Emissive Color**

Step 61 is the viscosity cue: the depth gradient gets pushed around by the flow noise,
so the interior churns slowly instead of sitting as a clean gradient.

**On `Brightness` in a 3D scene:** values above `1.0` on Emissive feed bloom. That can
look great on a glowing potion bar, or it can smear the whole element into a haze at
close range. Start at `1.0` and only push higher once you've seen it in the level with
post-processing on — the Material Editor preview won't show you the bloom.

---

## I. Opacity

66. `TextureSampleParameter2D` named **`ShapeMask`**, default a white texture,
    UVs = **TexUV** (the *raw* coordinate from step 1, not BarUV) → **R** → `Shape`
67. `ScalarParameter` **`Opacity`** = `1`
68. `Multiply` **LiquidMask** × `Shape` × **Opacity** → **Opacity** output

`ShapeMask` is how you get rounded caps or a vial silhouette — leave it white for a
plain rectangle. It samples raw UVs on purpose so the shape follows the mesh and stays
put when `bVertical` flips.

> If you skipped the multiply in step 51 for an over-the-line glow, `Add` `SurfaceBand` ×
> `0.4` into the opacity chain here.

On a 3D bar, `ShapeMask` is doing more work than it did in UMG — it's the only thing
giving the element a silhouette other than the mesh's own outline. If you want rounded
ends without modelling them, a simple capsule-shaped mask texture here is cheaper than
extra geometry.

---

## Instance setup

Create `MI_LiquidFillBar` from the master. Per instance you normally only set
**`bVertical`** and **`BarSize`** — everything else adapts on its own.

Remember to tick each parameter's **override checkbox**; an unticked value falls
through to the master default and your edit does nothing.

**Tuning order** — set these one at a time, top down; each depends on the one above
reading correctly:

1. `BarSize` to the mesh's real cm dimensions. Do this **first** — every value below
   is measured against it, and tuning on a wrong `BarSize` means retuning everything.
2. `Fill` to `0.5`, confirm the line sits mid-bar in both directions.
3. `WaveLengthPx` then `WaveAmplitudePx` — surface motion first. Wavelength is
   measured against the bar's *short* axis; start at `AcrossPx / 6` and keep
   amplitude under half the wavelength. See "Sizing the wavelength" in §D.
4. `NoiseScalePx` / `FlowSpeed` — body churn. Slow is better; `FlowSpeed` past `0.15`
   reads as wind, not viscosity.
5. `BubbleThreshold` / `BubbleScalePx` — threshold controls *how many* bubbles,
   scale controls *how big*. Raise threshold for fewer.
6. `SurfaceThicknessPx` and `Brightness` last.

Everything with a `Px` suffix is now in **centimetres** and holds its look at any bar
size, so these values transfer between a big worldspace boss bar and a small floating
meter above an enemy without retuning.

## Driving it from Blueprint

On the actor holding the mesh:

```
Create Dynamic Material Instance  (target = the Static Mesh Component, Element 0)
    → store it on a variable
    → Set Scalar Parameter Value  (Parameter Name = "Fill")
```

Create the dynamic instance once in `BeginPlay` and keep the reference — calling
`Create Dynamic Material Instance` every frame makes a new material each time and will
tank performance.

Interpolate the value rather than snapping it — `FInterpTo` toward the target over
~0.3 s. The wave keeps moving on its own, so an animated fill change reads as the
liquid actually sloshing to its new level.

If the bar should face the camera, put a **Rotating**/lookat in the actor or use a
`BillboardComponent`-style setup — the material has no opinion about orientation, but
`bVertical` assumes the mesh isn't rolled relative to the viewer.

---

## Cost

Three texture samples and ~100 instructions, unlit — cheap, but unlike a UMG widget
this now costs *translucent overdraw* in the scene. A handful of bars is nothing; a
bar over every enemy in a crowd is worth checking with `r.ShaderComplexity`.

If it needs trimming, `NoiseB` (steps 26–27) is the first thing to cut: wire `nA`
straight into step 28 and lower `NoiseScalePx` slightly to compensate.
