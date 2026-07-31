# M_LiquidBar3D — Build Guide

A bar-fill material for a **3D mesh used as a worldspace UI element**. It reads as a
viscous liquid: a wobbling surface line, slow internal turbulence driven by noise
textures, and bubbles drifting up through the body.

One master material serves both horizontal and vertical bars via a static switch.

**Create at:** `Content/Materials/UI_Materials/M_LiquidBar3D`

Everything works in normalised UV space (0–1 across the mesh face), so there is no
size-compensation machinery — you tune it once against your bar's proportions and it
holds.

---

## 1. Before you start

### Material settings

Create the material, select the **output node**, and set these in Details:

| Setting | Value |
| --- | --- |
| Material Domain | **Surface** |
| Blend Mode | **Translucent** |
| Shading Model | **Unlit** |
| Two Sided | on, if the bar is ever seen from behind |

**Unlit** is the one that matters. A UI element has to read identically wherever it
sits — under Default Lit a health bar goes dim in a cave and blows out next to a torch.
Unlit means the colour you author is the colour on screen.

With Unlit + Translucent, the only live output pins are **Emissive Color** and
**Opacity**. Those are the two things this whole graph builds.

### The mesh

UV channel 0 must map **0–1 across the bar face**. A default plane is ideal. Tiled or
offset UVs will break the fill alignment, since every calculation here starts from
`TextureCoordinate`.

### The textures

You need two cloudy noise textures and one cellular one. For each:

- **sRGB off**
- **Texture Address X/Y = Wrap** — not Clamp. The panners rely on tiling, and Clamp
  produces visible smearing at the edges.
- On the sampler node in the graph: **Sampler Type = Linear Grayscale**

Picks from this project:

| Slot | Suggestion |
| --- | --- |
| `NoiseA` | `Content/Textures/VFXNoiseTexPack/Textures/NoiseMaskPacked/T_NoiseMask_Merged_Liq_01` |
| `NoiseB` | any soft, cloudy `T_NoiseMask_*` from `.../Textures/Noises/` |
| `BubbleNoise` | a **cellular/voronoi** texture — `Content/Materials/Waterfall/T_Voronoi01` works |

For `NoiseA`/`NoiseB`, avoid the high-frequency grainy entries — they read as static
rather than fluid. `BubbleNoise` specifically needs visible discrete cells, because
step 40 thresholds cell cores into round blobs; cloudy perlin gives you shapeless
smears instead.

### One rule that will save you an hour

**Every parameter needs an explicit Default Value.** A fresh `VectorParameter` defaults
to black and a fresh `ScalarParameter` to `0`. Leave them at defaults and the bar
renders solid black even when the rest of the graph is perfect. Set each one as you
create it.

---

## 2. Bar space — the direction switch

This builds a UV where **X always runs from the empty end to the full end**, whichever
way the bar points. Everything downstream works in that space, so the orientation
switch only has to exist once.

1. `TextureCoordinate` (Coordinate Index 0) → call it **TexUV**
2. `ComponentMask` on TexUV, **R only** → `U`
3. `ComponentMask` on TexUV, **G only** → `V`
4. `OneMinus` on `V` → `InvV`
5. `AppendVector`: A = `U`, B = `V` → **HorizUV** *(fill runs left → right)*
6. `AppendVector`: A = `InvV`, B = `U` → **VertUV** *(fill runs bottom → top)*
7. `StaticSwitchParameter` named **`bVertical`**, Default Value `false`
   - True ← **VertUV**
   - False ← **HorizUV**
   - Output → **BarUV**
8. `ComponentMask` on BarUV, **R only** → **Along** *(0 = empty end, 1 = full end)*
9. `ComponentMask` on BarUV, **G only** → **Across** *(runs along the surface line)*

> Because it's a *static* switch, each instance compiles only the branch it uses.
> Supporting both directions costs nothing at runtime.

**Keep `Along` and `Across` straight** — they're easy to confuse and mixing them up is
the single most common wiring mistake in this graph:

```
HORIZONTAL bar (bVertical = false)      VERTICAL bar (bVertical = true)

  Across                                   ┌─────────┐  ▲
    ▲  ┌──────────┬─────────┐              │░░░░░░░░░│  │ Along
    │  │▓▓▓▓▓▓▓▓▓▓│░░░░░░░░░│              ├─────────┤  │
    │  │▓▓ full ▓▓│  empty  │              │▓▓▓▓▓▓▓▓▓│  │
    │  └──────────┴─────────┘              │▓▓ full ▓│  │
    └──────────►                           └─────────┘  │
         Along                              ─────────►
                                              Across
```

`Along` is the fill direction. `Across` walks along the liquid surface — it's what
makes the wave ripple sideways instead of the whole surface bobbing as one flat block.

---

## 3. Noise UVs

10. `VectorParameter` **`NoiseTiling`**, default `(4, 1, 0, 0)` → `ComponentMask` **RG**
    → `Tiling`
11. `Multiply`: A = **BarUV**, B = `Tiling` → **NoiseUV**

Set X to roughly your bar's **aspect ratio** so the noise cells stay square instead of
smearing into streaks. A 30×4 cm bar wants about `7.5`. Because this is applied to
`BarUV` rather than the raw UVs, the same value stays correct when `bVertical` flips.

---

## 4. Flow noise — the body turbulence

Two noise samples at different scales and speeds, drifting toward the surface. Layering
two of them kills the "obviously one tiling texture" look.

12. `ScalarParameter` **`FlowSpeed`** = `0.06`
13. `Constant2Vector` `(-1.0, 0.15)` → `Multiply` by **FlowSpeed** → `SpeedA`
14. `Panner`: Coordinate = **NoiseUV**, Speed = `SpeedA` → `FlowUV_A`
15. `TextureSampleParameter2D` named **`NoiseA`** → UVs = `FlowUV_A` → **R** → `nA`
16. `Multiply`: **NoiseUV** × `Constant 0.63` → `NoiseUV_B`
    `Constant2Vector` `(-0.6, -0.22)` → `Multiply` by **FlowSpeed** → `SpeedB`
    `Panner`: Coordinate = `NoiseUV_B`, Speed = `SpeedB` → `FlowUV_B`
17. `TextureSampleParameter2D` named **`NoiseB`** → UVs = `FlowUV_B` → **R** → `nB`
18. `Add` `nA` + `nB` → `Multiply` by `0.5` → `Noise01`
19. `Subtract` `Noise01` − `0.5` → **NoiseC** *(centred, −0.5 … 0.5)*

The `Panner` adds `Speed × Time` to the UV, so a **negative** X speed makes the pattern
appear to travel toward the full end. That's why both speed vectors lead with a negative.

**NoiseC is reused three times** — surface wobble, bubble distortion, and density
variation. That shared source is what makes the layers feel like one fluid rather than
three unrelated effects stacked on each other.

---

## 5. Surface wave

The fill line isn't flat. Two out-of-phase sine waves plus a noise offset.

20. `ScalarParameter` **`WaveFrequency`** = `8`, **`WaveSpeed`** = `1.1`,
    **`WaveAmplitude`** = `0.03`
21. `Multiply` **Across** × **WaveFrequency** → **Cycles**
22. **Wave 1:** `Add` **Cycles** + (`Time` × **WaveSpeed**) → `Sine`
    → `Multiply` × **WaveAmplitude** → `Wave1`
23. **Wave 2:** `Multiply` **Cycles** × `Constant 0.37` → `Add`
    (`Time` × **WaveSpeed** × `Constant -0.7`) → `Sine`
    → `Multiply` × (**WaveAmplitude** × `Constant 0.6`) → `Wave2`
24. `ScalarParameter` **`SurfaceNoise`** = `0.5`
    `Multiply` **NoiseC** × **SurfaceNoise** × **WaveAmplitude** → `WaveNoise`
25. `Add` `Wave1` + `Wave2` → `Add` `WaveNoise` → **WaveOffset**

**Cycles** feeds both waves, so step 23 only needs one extra multiply. UE's `Sine` node
has Period = 1 by default, meaning an input of `1.0` is one full cycle — so
`WaveFrequency = 8` gives exactly 8 ripples along the surface.

**The odd constants are deliberate.** `0.37` and `-0.7` are non-harmonic ratios against
Wave 1. Use `0.5` and `-1.0` instead and the two sines lock into a short repeating
pattern you'll notice within seconds. Wave 2 is also smaller (`0.6`) and travels the
opposite direction, so the combined surface never visibly loops.

Keep `WaveFrequency` at **6 or higher**. Below about 3, Wave 2 (at `0.37×`) never
completes a full period across the bar — it degenerates into a slight tilt and the
surface reads as a single plain sine instead of irregular liquid motion.

### Fade the wave at the extremes

Without this the bar looks broken at 0% — a sliver of liquid sloshing at the empty end —
and at 100%, where the wave cuts a notch out of the full end.

26. `ScalarParameter` **`Fill`** = `0.5`, Slider Min `0` / Max `1`
27. `Multiply` **Fill** × `Constant 10` → `Saturate` → `fadeLo`
28. `OneMinus` **Fill** → `Multiply` × `Constant 10` → `Saturate` → `fadeHi`
29. `Multiply` `fadeLo` × `fadeHi` → **WaveFade**
30. `Multiply` **WaveOffset** × **WaveFade** → **WaveOffsetFaded**

---

## 6. Fill line and liquid mask

31. `Add` **Fill** + **WaveOffsetFaded** → **FillLine**
32. `ScalarParameter` **`EdgeSoftness`** = `0.006`
33. `Subtract` **FillLine** − **EdgeSoftness** → `edgeMin`
    `Add` **FillLine** + **EdgeSoftness** → `edgeMax`
34. `SmoothStep`: Min = `edgeMin`, Max = `edgeMax`, Value = **Along**
    → `OneMinus` → `maskRaw`
35. `Multiply` **Fill** × `Constant 50` → `Saturate` → `zeroGuard`
36. `Multiply` `maskRaw` × `zeroGuard` → **LiquidMask**

Steps 35–36 guarantee a hard empty at `Fill = 0`. The smoothstep alone leaves a
hairline of liquid at the empty edge.

`EdgeSoftness` controls antialiasing on the fill line. Too small and it shimmers as the
bar moves on screen; too large and the liquid looks like it's fading out rather than
ending. On a 3D bar viewed from a distance, err slightly softer.

---

## 7. Surface band — the meniscus

The bright lip right at the liquid line. This is most of what sells "liquid" — without
it the fill reads as a flat coloured rectangle.

37. `Subtract` **FillLine** − **Along** → `Abs` → `distToSurface`
38. `ScalarParameter` **`SurfaceThickness`** = `0.05`
39. `Divide` `distToSurface` / **SurfaceThickness** → `Saturate` → `OneMinus`
    → `Power` (Exp = `2`) → `SurfaceBand`
40. `Multiply` `SurfaceBand` × **LiquidMask** → **SurfaceMask**

The multiply in step 40 clips the band to the liquid side. If you'd rather have a faint
glow spilling *above* the line, skip it and add `SurfaceBand` into Opacity instead
(see §10).

---

## 8. Bubbles

Blobs rising toward the surface, their paths wobbled by the same noise that drives the
body — so they look carried by the fluid rather than sliding across it.

41. `ScalarParameter`: **`BubbleTiling`** = `6`, **`BubbleSpeed`** = `0.25`,
    **`BubbleWobble`** = `0.03`, **`BubbleThreshold`** = `0.62`,
    **`BubbleContrast`** = `14`, **`BubbleAmount`** = `1`
42. `Multiply` **NoiseC** × **BubbleWobble** → `bubbleDistort`
43. `Multiply` **BarUV** × **BubbleTiling** → `Add` `bubbleDistort` → `BubbleUV0`
44. `Constant2Vector` `(-1, 0)` → `Multiply` by **BubbleSpeed** → `bubbleVel`
    `Panner`: Coordinate = `BubbleUV0`, Speed = `bubbleVel` → `BubbleUV`
45. `TextureSampleParameter2D` named **`BubbleNoise`** → UVs = `BubbleUV` → **R** → `bRaw`
46. `Subtract` `bRaw` − **BubbleThreshold** → `Multiply` × **BubbleContrast**
    → `Saturate` → `Bubbles`
47. `Multiply` `Bubbles` × **BubbleAmount** × **LiquidMask** → **BubbleMask**

`BubbleThreshold` controls **how many** bubbles (raise it for fewer), `BubbleTiling`
controls **how big** (raise it for smaller). `BubbleContrast` is the edge hardness of
each blob — drop it toward `6` for soft foam, push it past `20` for crisp spheres.

**Optional — pop at the surface:** `Subtract` **FillLine** − **Along** →
`Multiply` × `Constant 6` → `Saturate`, and multiply that into **BubbleMask**. Bubbles
then thin out as they near the line instead of clipping through it.

---

## 9. Colour

**Set a Default Value on all four of these**, or the bar renders black.

48. `VectorParameter`:
    - **`LiquidColor`** = `(1.0, 0.72, 0.15)` — bright, near the surface
    - **`LiquidColorDeep`** = `(0.45, 0.16, 0.02)` — darker, at the bottom
    - **`SurfaceColor`** = `(1.0, 0.95, 0.6)` — brightest, the meniscus
    - **`BubbleColor`** = `(1.0, 0.88, 0.45)`
49. `Subtract` **FillLine** − **Along** → `Saturate` → `Multiply` ×
    `ScalarParameter` **`DepthFalloff`** (= `2.5`) → `Saturate` → `Depth`
50. `Multiply` **NoiseC** × `ScalarParameter` **`DensityVariation`** (= `0.35`)
    → `Add` to `Depth` → `Saturate` → `DepthN`
51. `Lerp`: A = **LiquidColor**, B = **LiquidColorDeep**, Alpha = `DepthN` → `BodyColor`
52. `Lerp`: A = `BodyColor`, B = **BubbleColor**, Alpha = **BubbleMask** → `WithBubbles`
53. `Lerp`: A = `WithBubbles`, B = **SurfaceColor**, Alpha = **SurfaceMask** → `FinalRGB`
54. `Multiply` `FinalRGB` × `ScalarParameter` **`Brightness`** (= `1.0`)
    → **Emissive Color**

Step 50 is the viscosity cue: the depth gradient gets shoved around by the flow noise,
so the interior churns slowly instead of sitting as a clean static gradient.

**On `Brightness` in a 3D scene:** anything above `1.0` on Emissive feeds bloom. That
can look great on a glowing potion bar or smear the element into a haze at close range.
Start at `1.0` and judge it in the level with post-processing on — the Material Editor
preview won't show you the bloom.

---

## 10. Opacity

55. `TextureSampleParameter2D` named **`ShapeMask`**, default a **white** texture,
    UVs = **TexUV** (the raw coordinate from step 1, *not* BarUV) → **R** → `Shape`
56. `ScalarParameter` **`Opacity`** = `1`
57. `Multiply` **LiquidMask** × `Shape` × **Opacity** → **Opacity** output

`ShapeMask` gives you rounded caps or a vial silhouette without modelling them — leave
it white for a plain rectangle. It samples raw UVs on purpose, so the shape follows the
mesh and stays put when `bVertical` flips.

> If you skipped the multiply in step 40 for an over-the-line glow, `Add`
> `SurfaceBand` × `0.4` into this chain.

### Drawing order

Worldspace UI usually needs to sit on top of the geometry behind it:

- **Disable Depth Test** (material → Translucency) draws it through everything. Use
  sparingly — it will also draw through the character it belongs to.
- **Translucency Sort Priority** on the mesh *component* resolves fights between
  several translucent objects. Higher draws later.

---

## 11. Instance setup

Create `MI_LiquidBar3D` from the master. Tick each parameter's **override checkbox** —
an unticked value falls through to the master default and your edit does nothing.

**Tuning order.** Set these one at a time, top down; each depends on the one above
reading correctly.

1. **`bVertical`** and **`NoiseTiling`** X — orientation and aspect. Everything below
   is judged against these, so getting them wrong means retuning twice.
2. **`Fill`** to `0.5`. Confirm the line sits mid-bar and moves the right way.
3. **`WaveFrequency`** then **`WaveAmplitude`** — surface motion. Frequency is ripples
   along the surface; keep it 6+. Amplitude is a fraction of the bar's length, so `0.03`
   is 3%.
4. **`NoiseTiling`** / **`FlowSpeed`** — body churn. Slow is better. Past `0.15` it
   reads as wind rather than viscosity.
5. **`BubbleThreshold`** / **`BubbleTiling`** — count, then size.
6. **`SurfaceThickness`** and **`Brightness`** last.

For a different liquid, the four colour params do almost all the work. A green acid
bar, a blue mana bar and a red health bar are the same material with different vectors —
keep `SurfaceColor` as a lighter, desaturated version of `LiquidColor` and it holds up.

---

## 12. Driving it from Blueprint

On the actor holding the mesh:

```
BeginPlay
  → Create Dynamic Material Instance (target = Static Mesh Component, Element Index 0)
  → promote result to a variable  (e.g. BarMID)

whenever health changes
  → BarMID → Set Scalar Parameter Value  (Parameter Name = "Fill")
```

**Create the dynamic instance once and cache it.** Calling `Create Dynamic Material
Instance` every frame allocates a brand new material each time and will tank
performance — it's the standard trap here.

Interpolate rather than snapping: `FInterpTo` toward the target over ~0.3 s. The wave
keeps moving on its own, so an animated fill change reads as the liquid actually
sloshing to its new level.

If the bar should face the camera, handle that on the actor or component. The material
has no opinion about orientation, but `bVertical` assumes the mesh isn't rolled
relative to the viewer.

---

## 13. Troubleshooting

| Symptom | Cause |
| --- | --- |
| **Entire bar is solid black** | `VectorParameter` colours left at their black default, or `Brightness` left at `0`. Both default to zero on creation. |
| **Entire bar is one flat colour matching `SurfaceColor`** | `SurfaceThickness` is far too large, so `SurfaceBand` saturates to 1 everywhere and step 53 lerps the whole bar to it. |
| **Only one wave visible, not two** | `WaveFrequency` too low — below ~3, Wave 2 at `0.37×` can't complete a period across the bar. |
| **Surface is dead flat** | Step 21's `Multiply` missing, or `WaveAmplitude` at `0`. |
| **Noise looks like horizontal streaks** | `NoiseTiling` X doesn't match the bar's aspect ratio. |
| **Visible seams / smearing at the bar edges** | Noise textures set to Clamp instead of **Wrap**. |
| **Bubbles are shapeless smears** | `BubbleNoise` is a cloudy texture; it needs a cellular/voronoi one. |
| **Bar renders but is invisible in the level** | Blend Mode not Translucent, or `Opacity` param at `0`. |
| **Bar clipped by walls or sorting oddly** | See "Drawing order" in §10. |

**Isolation technique.** When something looks wrong and you can't tell which stage
broke it, wire the suspect value straight into **Emissive Color** and Apply. Masks
(`LiquidMask`, `SurfaceMask`, `BubbleMask`) should show as clean black-and-white
shapes; `NoiseC` should be mid-grey churn. Whichever one doesn't look right is where
the problem is, and you've narrowed it to a handful of nodes instead of the whole graph.

---

## 14. Cost

Three texture samples and roughly 90 instructions, unlit. Cheap per-bar, but unlike a
UMG widget this costs **translucent overdraw** in the scene. A handful of bars is
nothing; one over every enemy in a crowd is worth checking with `r.ShaderComplexity`.

If it needs trimming, `NoiseB` (steps 16–17) is the first thing to cut: wire `nA`
straight into step 19 and raise `NoiseTiling` slightly to compensate. You lose some of
the non-repeating quality of the churn and save a texture sample.
