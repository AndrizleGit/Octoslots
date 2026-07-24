# Toon Water Shader — Node-by-Node Tutorial (UE 5.7)

A stylized, translucent toon water material for a flat plane, with:

- Depth-tinted shallow → deep color gradient (see-through)
- Hard-edged, noise-eroded **intersection foam** around rocks, shorelines, and anything touching the water (distance-field based)
- Animated **shore waves** that roll toward the beach
- Panning **normal ripples** with sun specular
- Real **vertex-offset waves** (World Position Offset), parameterized from pond-calm to ocean-rough
- Everything exposed as parameters, grouped for Material Instances
- Clearly marked **CEL-SHADING HOOK** boxes where you can later insert banding

Node labels like `[C3]` are used so wiring instructions are unambiguous. "→" means *connect output to input*.

---

## Part 0 — Project prep (5 minutes)

1. **Enable distance fields**: Project Settings → Rendering → Software Ray Tracing → check **Generate Mesh Distance Fields**. Restart the editor when prompted (one-time shader recompile).
2. **Water plane mesh**: WPO waves need vertices to move. The default SM_Plane (2 triangles) will not wave.
   - Modeling Mode (the sculpting toolbar) → **Create → Rectangle**, set subdivisions to something like **100 × 100**, size it roughly to your water body, accept it as a new static mesh (e.g. `SM_WaterPlane_Sub100`).
   - For very large seas, place several tiles instead of one giant one (better culling).
3. **Exclude the water plane from distance fields** (critical — otherwise the water "sees itself" and the whole surface becomes foam):
   - Select the placed water plane actor → Details → Lighting → uncheck **Affect Distance Field Lighting**.
4. **Pick textures from your library** (all should be seamless/tiling):
   | Slot | What to pick | Used for |
   |---|---|---|
   | Foam Noise | Mid-frequency grayscale noise (cloudy Perlin or Voronoi) | Eroding foam edges, wobbling shore waves |
   | Wave Height Noise | *Smooth, blurry* low-frequency grayscale noise (soft Perlin). Avoid high-frequency/grainy — it makes jittery geometry | Vertex wave heightmap |
   | Ripple Normal A / B | Two different water-ripple normal maps from your normal-noise library | Surface ripples + specular glints |

---

## Part 1 — Create the material

Create `M_ToonWater` (suggest next to `M_Stylized_Water` in `Content/LevelPrototyping/Materials/`). Open it, select the main output node, and set in Details:

| Setting | Value |
|---|---|
| Material Domain | Surface |
| Blend Mode | **Translucent** |
| Shading Model | Default Lit |
| Two Sided | Off |
| Translucency → Lighting Mode | **Surface ForwardShading** ← required for real specular glints |
| Translucency → Screen Space Reflections | Optional, on if you want sky/scene reflections |

**Parameter conventions used below:**
- "Scalar param `Foam Distance` (=120)" means: right-click → *Scalar Parameter*, name it `Foam Distance`, default value 120.
- Set every parameter's **Group** in its Details panel as noted, so the Material Instance is tidy: `01 Color`, `02 Foam`, `03 Shore Waves`, `04 Ripples`, `05 Vertex Waves`.

---

## Part 2 — Reusable function: MF_WorldPanUV

Every texture in this material uses **world-aligned panning UVs** — so you can move/scale/tile the plane freely with zero stretching or seams. We build it once as a Material Function and reuse it five times.

Create a Material Function `MF_WorldPanUV` (right-click in Content Browser → Material → Material Function). Inside:

| # | Node | Settings |
|---|---|---|
| [U1] | FunctionInput | Name `TileSize`, Input Type **Scalar**, Preview Value 1000, check *Use Preview Value as Default* |
| [U2] | FunctionInput | Name `Direction`, Input Type **Vector2**, Preview (1, 0), use preview as default |
| [U3] | FunctionInput | Name `Speed`, Input Type **Scalar**, Preview 0.05, use preview as default |
| [U4] | WorldPosition | Default (Absolute World Position) |
| [U5] | ComponentMask | R and G checked. [U4] → [U5] |
| [U6] | Divide | A = [U5], B = [U1] |
| [U7] | Time | — |
| [U8] | Multiply | A = [U7], B = [U3] |
| [U9] | Multiply | A = [U8], B = [U2] |
| [U10] | Add | A = [U6], B = [U9] → **Output Result** |

Meaning of the inputs: `TileSize` = size of one texture tile in **centimeters**; `Speed` = tiles per second the texture pans; `Direction` = pan direction in the XY plane.

Save. Back in `M_ToonWater`, you place it by dragging the function in from the Content Browser (or right-click → search `MF_WorldPanUV`).

---

## Part 3 — Depth color & opacity (shallow → deep)

This is the see-through gradient: shallow water shows the ground and uses the light color; deep water goes opaque and dark.

| # | Node | Settings / wiring |
|---|---|---|
| [C1] | DepthFade | Scalar param `Depth Fade Distance` (=300, group `01 Color`) → its **Fade Distance** input. Leave **Opacity** input unplugged. Output = 0 at the shore/objects, 1 in deep water. |
| [C2] | Power | Base = [C1], Exp = scalar param `Depth Contrast` (=1.0, group `01 Color`). >1 pushes the shallow zone wider, <1 narrows it. |
| [C3] | Lerp | A = Vector param `Shallow Color` (a light cyan, e.g. 0.1, 0.65, 0.75), B = Vector param `Deep Color` (e.g. 0.02, 0.16, 0.35), Alpha = [C2]. Both group `01 Color`. |
| [C4] | Lerp | A = scalar param `Shallow Opacity` (=0.45), B = scalar param `Deep Opacity` (=0.9), Alpha = [C2]. Group `01 Color`. |

Don't plug [C3]/[C4] into the outputs yet — foam gets blended in first (Part 7).

> **CEL-SHADING HOOK #1 — banded depth colors**
> To turn the smooth gradient into hard toon bands later, insert a posterize between [C2] and the two Lerps:
> `[C2] → Multiply (B = scalar param "Depth Bands", e.g. 3) → Floor → Divide (B = "Depth Bands") → into [C3].Alpha and [C4].Alpha`.
> Set `Depth Bands` to 2–4. Delete/bypass those three nodes (or set a huge band count) to go back to smooth.

---

## Part 4 — Ripple normals & specular

Two normal-noise textures at different scales/directions, blended, then flattened by a strength parameter.

| # | Node | Settings / wiring |
|---|---|---|
| [N1] | MF_WorldPanUV | TileSize = scalar param `Ripple Tile` (=700, group `04 Ripples`); Direction = Constant2Vector (1, 0.2); Speed = scalar param `Ripple Speed` (=0.03) |
| [N2] | Texture Sample → right-click → Convert to Parameter, name `Ripple Normal A` | Assign a normal noise texture (Sampler Type auto-sets to Normal). UVs = [N1] |
| [N3] | MF_WorldPanUV | TileSize = `Ripple Tile` × 1.37 (Multiply node with Constant 1.37); Direction = Constant2Vector (-0.6, 0.8); Speed = `Ripple Speed` × 0.8 (Multiply node with Constant 0.8) |
| [N4] | Texture Sample Parameter `Ripple Normal B` | A *different* normal noise. UVs = [N3] |
| [N5] | BlendAngleCorrectedNormals (engine material function — just search the name) | BaseNormal = [N2].RGB, AdditionalNormal = [N4].RGB |
| [N6] | OneMinus | Input = scalar param `Ripple Strength` (=0.6, group `04 Ripples`) |
| [N7] | FlattenNormal (engine function) | Normal = [N5], Flatness = [N6] → **Normal** output pin |

Then set the constant surface inputs on the main node:
- Constant **0** → Metallic
- Scalar param `Roughness` (=0.08, group `04 Ripples`) → Roughness (low roughness = tight sun glint; raise toward 0.3 for a duller surface)
- Constant **0.5** → Specular

Using two different tiles/directions/speeds is what prevents the "obviously tiling, obviously panning" look.

---

## Part 5 — Intersection foam (hard toon edges)

Distance-field foam: every pixel of the water asks "how far is the nearest mesh?" — rocks, piers, the beach, your octopus — and foams when close. Works regardless of whether the object pokes through the surface.

| # | Node | Settings / wiring |
|---|---|---|
| [F1] | DistanceToNearestSurface | (no inputs) |
| [F2] | Divide | A = [F1], B = scalar param `Foam Distance` (=120, group `02 Foam`) — foam reach in cm |
| [F3] | Saturate | Input = [F2] |
| [F4] | OneMinus | Input = [F3]. Result: 1 at contact → 0 at `Foam Distance` |
| [F5] | MF_WorldPanUV | TileSize = scalar param `Foam Noise Tile` (=500); Direction = Constant2Vector (0.7, 0.7); Speed = scalar param `Foam Noise Speed` (=0.04). Group `02 Foam` |
| [F6] | Texture Sample Parameter `Foam Noise` | Your grayscale noise. Sampler Type: Linear Color (or Masks). UVs = [F5]. Use the **R** output |
| [F7] | Multiply | A = [F6].R, B = scalar param `Foam Noise Strength` (=0.35, group `02 Foam`) |
| [F8] | Subtract | A = [F4], B = [F7] — the noise "eats" into the foam edge |
| [F9] | Subtract | A = scalar param `Foam Cutoff` (=0.4, group `02 Foam`), B = scalar param `Foam Softness` (=0.05, group `02 Foam`) |
| [F10] | Add | A = `Foam Cutoff`, B = `Foam Softness` |
| [F11] | SmoothStep | Value = [F8], Min = [F9], Max = [F10] → this is **foamMask** |

**This is your soft↔hard foam blend**: `Foam Softness` **0.01 = razor toon edge**, 0.05 = the reference-image look, 0.3 = soft painterly gradient. `Foam Cutoff` moves the foam line in/out within the gradient.

> **Alternative branch — no distance fields (fallback)**
> If you ever need this material without distance fields: replace [F1]–[F4] with a single **DepthFade** (Fade Distance = `Foam Distance`) → **OneMinus**. Everything downstream is identical. Downsides: only detects objects *behind* the water pixel, so foam can shift slightly with camera angle.

> **Optional flourish — second inner foam line**
> Classic toon water has a thin detached second line. Duplicate [F9]–[F11], on the copy set the cutoff to `Foam Cutoff × 0.45` and softness tiny (0.02), then `Max` it with a band-pass: Subtract a *second* SmoothStep at `Foam Cutoff × 0.35` to hollow it into a line. Add later if you want it — everything just `Max`es into the foam mask.

---

## Part 6 — Shore waves rolling onto the beach

Repeating bands that travel *toward* the shore, wobbled by noise, fading out with distance. Built from the same distance field.

| # | Node | Settings / wiring |
|---|---|---|
| [S1] | Divide | A = [F1] (reuse the same DistanceToNearestSurface), B = scalar param `Shore Wave Range` (=800, group `03 Shore Waves`) — how far offshore waves appear |
| [S2] | Saturate | Input = [S1]. Result: 0 at shore → 1 offshore |
| [S3] | Multiply | A = [F6].R (reuse foam noise sample), B = scalar param `Shore Wave Distortion` (=0.08, group `03 Shore Waves`) |
| [S4] | Add | A = [S2], B = [S3] — wobbles the bands so they're not perfect rings |
| [S5] | Multiply | A = [S4], B = scalar param `Shore Wave Count` (=3, group `03 Shore Waves`) |
| [S6] | Time | — |
| [S7] | Multiply | A = [S6], B = scalar param `Shore Wave Speed` (=0.35, group `03 Shore Waves`) |
| [S8] | Add | A = [S5], B = [S7] ← **Add**, not subtract: adding time makes bands travel toward the shore |
| [S9] | Frac | Input = [S8]. Repeating 0→1 sawtooth |
| [S10] | OneMinus | Input = scalar param `Shore Wave Width` (=0.3, group `03 Shore Waves`) |
| [S11] | Subtract | A = [S10], B = scalar param `Shore Wave Softness` (=0.05, group `03 Shore Waves`) |
| [S12] | Add | A = [S10], B = `Shore Wave Softness` |
| [S13] | SmoothStep | Value = [S9], Min = [S11], Max = [S12] — carves a crest band out of the sawtooth (hard leading edge from the frac reset, controllable trailing edge) |
| [S14] | OneMinus | Input = [S2] |
| [S15] | Multiply | A = [S13], B = [S14] — waves fade out as they get further from shore |
| [S16] | Max | A = [F11] foamMask, B = [S15] → **totalFoam** |

`Shore Wave Softness` is the cel-shading dial here too — drop it to 0.01 for hard toon wave lines.

---

## Part 7 — Combine into the material outputs

| # | Node | Settings / wiring |
|---|---|---|
| [K1] | Lerp | A = [C3] (depth color), B = Vector param `Foam Color` (=0.92, 0.97, 1.0, group `02 Foam`), Alpha = [S16] → **Base Color** output |
| [K2] | Lerp | A = [C4] (depth opacity), B = scalar param `Foam Opacity` (=0.95, group `02 Foam`), Alpha = [S16] → **Opacity** output |
| [K3] (optional) | Multiply → Multiply | `Foam Color` × [S16] × scalar param `Foam Glow` (=0, group `02 Foam`) → **Emissive Color**. Leave 0 normally; nudge to 0.1–0.3 if foam gets lost in shadowed areas |

At this point the material is complete except for waves — apply it to your plane and you should already see the reference-image look: depth gradient, crisp foam around everything, wave bands rolling at the beach.

---

## Part 8 — Vertex waves (World Position Offset)

Two panning samples of a smooth noise, summed, centered, scaled. One material serves both ponds and ocean via `Wave Height` / `Wave Tile`.

| # | Node | Settings / wiring |
|---|---|---|
| [W1] | MF_WorldPanUV | TileSize = scalar param `Wave Tile` (=3000, group `05 Vertex Waves`); Direction = Constant2Vector (1, 0); Speed = scalar param `Wave Speed` (=0.03, group `05 Vertex Waves`) |
| [W2] | Texture Sample Parameter `Wave Height Noise` | The *smooth* grayscale noise. UVs = [W1]. Use **R** |
| [W3] | MF_WorldPanUV | TileSize = `Wave Tile` × 0.55 (Multiply + Constant); Direction = Constant2Vector (-0.5, 0.8); Speed = `Wave Speed` × 1.3 (Multiply + Constant) |
| [W4] | Texture Sample Parameter — reuse `Wave Height Noise` (same param name = same texture) | UVs = [W3]. Use **R** |
| [W5] | Add | A = [W2].R, B = [W4].R (range 0–2) |
| [W6] | Subtract | A = [W5], B = Constant 1 (now −1…+1, so waves go both up *and* down around the plane's height) |
| [W7] | Multiply | A = [W6], B = scalar param `Wave Height` (=12, group `05 Vertex Waves`) — in cm |
| [W8] | AppendVector | A = Constant2Vector (0, 0), B = [W7] → **World Position Offset** output |

**Two instances, one material**: pond instance ≈ `Wave Height` 8–15, `Wave Tile` 2000–3000. Ocean instance ≈ `Wave Height` 60–150, `Wave Tile` 6000–12000, `Wave Speed` ×2.

> **Optional — flatten waves at the beach** (stops big waves clipping through the sand)
> Between [W7] and [W8]: `[F1] DistanceToNearestSurface → Divide (B = scalar param "Wave Shore Damping" = 600) → Saturate → Multiply with [W7]`. Distance fields are readable in the vertex shader, so this works in WPO.

> **Gameplay note**: WPO is *visual only* — collision and floating actors don't know about it. Keep gameplay-relevant water gentle, or mirror the same formula (two panning noises summed) in C++/BP if something must bob accurately on the surface.

---

## Part 9 — Material Instance & defaults cheat sheet

Right-click `M_ToonWater` → Create Material Instance → `MI_ToonWater_Pond`. Apply to the subdivided plane. Full dial reference:

| Group | Parameter | Default | What it does |
|---|---|---|---|
| 01 Color | Shallow Color | light cyan | Water color at shore/over objects |
| 01 Color | Deep Color | dark blue | Color at full depth |
| 01 Color | Depth Fade Distance | 300 | View-depth (cm) over which shallow→deep transitions |
| 01 Color | Depth Contrast | 1.0 | Gradient curve; >1 widens shallow zone |
| 01 Color | Shallow / Deep Opacity | 0.45 / 0.9 | See-through amount at each end |
| 02 Foam | Foam Distance | 120 | Foam reach from objects (cm) |
| 02 Foam | Foam Cutoff | 0.4 | Where in the gradient the foam line sits |
| 02 Foam | **Foam Softness** | 0.05 | **0.01 = hard toon, 0.3 = soft** |
| 02 Foam | Foam Noise Tile / Speed / Strength | 500 / 0.04 / 0.35 | Edge erosion pattern |
| 02 Foam | Foam Color / Opacity / Glow | near-white / 0.95 / 0 | Foam appearance |
| 03 Shore Waves | Shore Wave Range | 800 | How far offshore waves appear (cm) |
| 03 Shore Waves | Shore Wave Count / Speed | 3 / 0.35 | Bands in range / travel speed |
| 03 Shore Waves | Shore Wave Width / Softness | 0.3 / 0.05 | Band thickness / edge hardness |
| 03 Shore Waves | Shore Wave Distortion | 0.08 | Wobble of the bands |
| 04 Ripples | Ripple Tile / Speed / Strength | 700 / 0.03 / 0.6 | Normal detail scale, motion, intensity |
| 04 Ripples | Roughness | 0.08 | Sun-glint tightness |
| 05 Vertex Waves | Wave Tile / Speed / Height | 3000 / 0.03 / 12 | Wavelength / motion / amplitude (cm) |

---

## Cel-shading upgrade map (for later)

All the places to harden the look, in order of impact:

1. **Foam Softness → 0.01** and **Shore Wave Softness → 0.01** — instant hard toon lines, zero graph changes.
2. **Hook #1 (Part 3)**: posterize the depth gradient into `Depth Bands` (Multiply → Floor → Divide). This gives the stepped color bands of full cel water.
3. **Ripple Strength down (0.2–0.35)** + **Roughness down (0.05)** — flatter surface with sharper, more graphic glints.
4. **Stepped lighting/specular** (true cel highlight shapes) needs a toon shading model or a post-process cel shader — separate project-wide topic, not a material-level tweak. The material above is fully compatible with adding one later.

---

## Troubleshooting

| Symptom | Fix |
|---|---|
| Entire surface is foam | Water plane is in the distance field — uncheck **Affect Distance Field Lighting** on the plane actor (Part 0.3) |
| No foam at all | Distance fields not enabled (Part 0.1), or editor not restarted after enabling |
| No foam around a specific mesh | That asset has distance fields disabled, or it's translucent/skeletal (skeletal meshes have no DFs — expected; the octopus won't foam via DF. If you need character foam, that's a decal/particle job) |
| Foam ring is offset from the object | `Foam Distance` too large, or the mesh's DF resolution is low — bump *Distance Field Resolution Scale* in that Static Mesh's Build Settings |
| Plane doesn't wave | Not enough vertices — use the subdivided plane from Part 0.2 |
| Water invisible from below | Expected (Two Sided off). Turn Two Sided on if the camera can go underwater |
| No sun glints | Translucency Lighting Mode must be **Surface ForwardShading** (Part 1) |
| Normals look like colored blotches | Ripple textures not imported as normal maps — set Compression Settings to Normalmap |
| Texture seams when moving the plane | You skipped MF_WorldPanUV somewhere and used mesh UVs |
| Waves jitter/spiky | Wave Height Noise is too high-frequency — pick a smoother/blurrier noise |
