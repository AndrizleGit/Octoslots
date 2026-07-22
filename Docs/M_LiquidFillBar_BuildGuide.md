# M_LiquidFillBar — Build Guide

A UMG bar-fill material that reads as a viscous liquid: wobbling surface line, slow
internal turbulence driven by noise textures, and bubbles drifting up through the body.
One master material handles both horizontal and vertical bars via a static switch.

**Create at:** `Content/Materials/UI_Materials/M_LiquidFillBar`

---

## 0. Material settings

Select the output node, then in Details:

| Setting | Value |
| --- | --- |
| Material Domain | **User Interface** |
| Blend Mode | **Translucent** |
| Used with UI | (auto) |

Only **Final Color** and **Opacity** are active on the output node in this domain.

Two notes before you start:

- **Noise texture import settings.** Select each noise texture you plan to use →
  turn **sRGB off**, set **Texture Address X/Y = Wrap** (not Clamp — the panners rely
  on tiling). On each sampler node set **Sampler Type = Linear Grayscale**.
- Everything below is one continuous graph. Section letters are just for reading; lay
  them out left-to-right in that order and the wires stay short.

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

## A2. Widget size — pixel space

This block makes the material **resolution- and size-independent**. It reads the
widget's actual pixel dimensions, so noise density, bubble size, wave wavelength and
edge softness stay visually constant whether the bar is 120×20 or 900×80. Build it
once here and every later section just divides by it.

### Getting the size

The widget's pixel size comes in as a parameter, pushed from the widget Blueprint.
The default value keeps the Material Editor preview looking correct while you build.

10. `VectorParameter` named **`WidgetSize`**, **Default Value `(300, 40, 0, 0)`**
11. `ComponentMask` **R** → **WidthPx**
12. `ComponentMask` **G** → **HeightPx**

Then in the widget Blueprint (`Tick`, or wherever you already update `Fill`):

```
Get Tick Space Geometry → Get Local Size → (break to X, Y)
    → Make LinearColor (R = X, G = Y)
    → Set Vector Parameter Value  (Parameter Name = "WidgetSize")
```

> **Don't use `DDX`/`DDY` for this.** Screen-space derivatives look like an elegant
> way to measure the quad automatically, but they read the *actual rasterised
> geometry* — in the Material Editor that's the preview sphere or plane, not a
> widget. The result is a bogus `AlongPx` that makes every pixel→normalised divide
> below explode, and the classic symptom is the whole bar rendering as a flat
> `SurfaceColor`. They also break under widget rotation.

### Swapping them for orientation

`Along` and `Across` swap meaning when the bar is vertical, so their pixel lengths
have to swap with them.

14. `AppendVector`: A = **WidthPx**, B = **HeightPx** → `SizeH`
15. `AppendVector`: A = **HeightPx**, B = **WidthPx** → `SizeV`
16. `StaticSwitchParameter` named **`bVertical`** — same name as step 7
    - True ← `SizeV`, False ← `SizeH` → **BarSizePx**
17. `ComponentMask` **R** → `alongRaw` → `Max`(A = `alongRaw`, B = `Constant` `1.0`)
    → **AlongPx**   *(bar length along the fill axis, in px)*
18. `ComponentMask` **G** → `acrossRaw` → `Max`(A = `acrossRaw`, B = `Constant` `1.0`)
    → **AcrossPx**  *(bar width across the surface, in px)*

> Two `StaticSwitchParameter` nodes sharing the name `bVertical` are the *same*
> parameter — set it once on the instance and both flip together. That's why this is a
> second node rather than a wire dragged from step 7.

The `Max` clamps in 17–18 are cheap insurance. Everything downstream divides by these,
so a zero — from a Blueprint that hasn't ticked yet, or a widget with no layout size on
its first frame — would otherwise produce infinities and blank the material. Flooring at
1 px means a bad frame renders slightly wrong instead of catastrophically.

---

## B. Noise scale

Noise density is now specified as **how many pixels one tile of noise covers** — so it
holds steady at any widget size.

19. `ScalarParameter` **`NoiseScalePx`**, default `150`
20. `Multiply`: A = **BarUV**, B = **BarSizePx** → `BarPx` *(position in pixels)*
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

30. `ScalarParameter` **`WaveLengthPx`** = `7`, **`WaveSpeed`** = `1.1`,
    **`WaveAmplitudePx`** = `3.5`
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

**Aim for `WaveLengthPx ≈ AcrossPx / 6`.** A 40 px tall bar wants about `7`.

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
43. `ScalarParameter` **`EdgeSoftnessPx`** = `1.5`
    → `Divide` **EdgeSoftnessPx** / **AlongPx** → **EdgeSoftness**
44. `Subtract` **FillLine** − **EdgeSoftness** → `edgeMin`
    `Add` **FillLine** + **EdgeSoftness** → `edgeMax`
45. `SmoothStep`: Min = `edgeMin`, Max = `edgeMax`, Value = **Along** → `OneMinus` → `maskRaw`
46. `Multiply` **Fill** × `50` → `Saturate` → `zeroGuard`
47. `Multiply` `maskRaw` × `zeroGuard` → **LiquidMask**

Step 46–47 guarantees a hard empty at `Fill = 0`; the smoothstep alone leaves a
hairline at the empty edge.

Expressing softness in pixels is what gives you consistent antialiasing — `1.5` px
stays a clean soft edge at every bar size, where a fixed `0.004` fraction was razor-sharp
(aliased) on a long bar and visibly blurry on a short one.

---

## F. Surface band (the meniscus)

The bright lip right at the liquid line. This is most of what sells "liquid" — without
it the fill reads as a flat coloured rectangle.

48. `Subtract` **FillLine** − **Along** → `Abs` → `distToSurface`
49. `ScalarParameter` **`SurfaceThicknessPx`** = `9`
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

52. `ScalarParameter`: **`BubbleScalePx`** = `60`, **`BubbleSpeed`** = `0.25`,
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

`BubbleScalePx` is the width in pixels of one bubble-noise tile, so bubbles keep a
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
    *not* `0`) → **Final Color**

Step 61 is the viscosity cue: the depth gradient gets pushed around by the flow noise,
so the interior churns slowly instead of sitting as a clean gradient.

---

## I. Opacity

66. `TextureSampleParameter2D` named **`ShapeMask`**, default a white texture,
    UVs = **TexUV** (the *raw* coordinate from step 1, not BarUV) → **R** → `Shape`
67. `ScalarParameter` **`Opacity`** = `1`
68. `Multiply` **LiquidMask** × `Shape` × **Opacity** → **Opacity** output

`ShapeMask` is how you get rounded caps or a vial silhouette — leave it white for a
plain rectangle. It samples raw UVs on purpose so the shape stretches with the widget
and stays put when `bVertical` flips.

> If you skipped the multiply in step 51 for an over-the-line glow, `Add` `SurfaceBand` ×
> `0.4` into the opacity chain here.

---

## Instance setup

Create `MI_LiquidFillBar` from the master. `bVertical` is the only parameter you
normally *have* to set per instance — everything else now adapts on its own.

Remember to tick each parameter's **override checkbox**; an unticked value falls
through to the master default and your edit does nothing.

**Tuning order** — set these one at a time, top down; each depends on the one above
reading correctly:

1. `Fill` to `0.5`, confirm the line sits mid-bar in both directions.
2. `WaveLengthPx` then `WaveAmplitudePx` — surface motion first. Wavelength is
   measured against the bar's *short* axis; start at `AcrossPx / 6` and keep
   amplitude under half the wavelength. See "Sizing the wavelength" in §D.
3. `NoiseScalePx` / `FlowSpeed` — body churn. Slow is better; `FlowSpeed` past `0.15`
   reads as wind, not viscosity.
4. `BubbleThreshold` / `BubbleScalePx` — threshold controls *how many* bubbles,
   scale controls *how big*. Raise threshold for fewer.
5. `SurfaceThicknessPx` and `Brightness` last.

Everything with a `Px` suffix is in screen pixels and holds its look at any widget
size, so these values transfer between a chunky HUD bar and a small tooltip meter
without retuning.

## Driving it from a widget

In the widget Blueprint, on the Image's brush:

```
Get Dynamic Material  →  Set Scalar Parameter Value (Parameter Name = "Fill")
```

Interpolate the value rather than snapping it — `FInterpTo` toward the target over
~0.3 s. The wave keeps moving on its own, so an animated fill change reads as the
liquid actually sloshing to its new level.

---

## Cost

Three texture samples and ~100 instructions — cheap enough for a HUD element that's
always on screen. If it ever needs trimming, `NoiseB` (steps 16–17) is the first thing
to cut: wire `nA` straight into step 19 and raise `NoiseTiling` slightly to compensate.
