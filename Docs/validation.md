# KayKit example validation

Runtime and packaging validated with Unreal Engine 5.6.1 on Windows,
14 September 2026. The original scene comparison was measured on 12 September;
the image-quality comparison was measured on 14 September.

- Editor and Windows Shipping targets built successfully with Arcweave v2.0.0.
- `Arcweave.KayKit.Controls` passed: W movement, wall collision, E interaction,
  mouse choices, potion reward and closing the final dialogue line.
- All 27 checks in `Tools/verify_kaykit_story.py` passed, including conditional
  choices, healing, animation changes, revisits, training, insufficient funds,
  attack animation, import menu and resetting the bundled story.
- Windows cooking and packaging succeeded. The only project content directory in
  the cook is `KayKit`, approximately 8 MB before packaging. Engine content,
  shaders, binaries and debug symbols are additional.
- The packaged Windows executable loaded the new map and all five bundled story
  boards without API credentials, rendered 480 frames and exited successfully.
- A subsequent Windows Shipping build passed live API import through its menu:
  a six-board project loaded successfully, its Healer dialogue ran and granted
  the potion, and switching to bundled JSON and importing the live project again
  succeeded. Token and project fields survived menu closing/reopening, including
  edits made before the first import. Credentials are retained only in memory.
- `Arcweave.Project.ComponentAndBoardVariables` passed: eligible attributes,
  global/board/component scope separation, script assignments, defaults and resets.
- `Arcweave.KayKit.ProjectVariables` passed: dynamic HUD names and values,
  empty-string attributes, variable-change events and removal of stale rows.
- The v2.0.0 Shipping build repeated the live API import, bundled JSON reload,
  credential retention and Healer potion checks. The live HUD showed
  `faction_relation` and scoped component attributes, and correctly omitted
  the bundled project's `gold` and `sword_locked` variables.
- The plugin is pinned to [v2.1.0](https://github.com/arcweave/arcweave-unreal-plugin/releases/tag/v2.1.0),
  commit `07145997c93a09560ddfcbf1045f0e3c8fc0e1f6`. It includes attribute variables
  and the API completion, cancellation, timeout and invalid-export fixes from
  plugin PR #36, including its UE 5.5-compatible test mock. The release changes
  only the descriptor's version fields from the tested merge `9335a9f`; all source
  and runtime libraries are identical. A full UE 5.5 build remains unverified.
  The Unity repository remains unchanged.
- All five automation tests passed after the cleanup. Additional checks cover
  live weather/time/color changes, failed-import recovery, long variable values,
  pagination, malformed JSON and late cancelled responses.
- The cleaned Shipping build also passed a rejected-token test and a successful
  retry through the menu. Failure preserved the bundled state, the retry loaded
  the six-board live project, and credentials remained populated on reopening.
- Unreal's asset registry reported no dependencies from the 348 KayKit assets
  to legacy project content. Removed 1,409 old asset, template and package files
  (approximately 1.22 GiB on disk). Existing Git history is unchanged.

## Performance

The complete scene and rendering changes were compared on an RTX 5070 Ti and
Ryzen 7 7800X3D. Each scene rendered offscreen in standalone mode at 1280×720 and
100% resolution, with VSync and audio disabled. The original scene used its Lumen,
virtual shadow map and TSR settings; KayKit used the initial FXAA settings.

Each capture contains 1,000 frames. Results below are from the final 500 frames
of the second capture, after a first run warmed the caches.

| Mean time | Original village | KayKit dungeon |
|---|---:|---:|
| Frame | 5.90 ms | 2.00 ms |
| GPU | 5.16 ms | 1.24 ms |
| Game thread | 2.34 ms | 1.39 ms |

Mean frame time decreased by approximately 66%. These measurements include both
the asset replacement and rendering settings changes. They do not measure editor
responsiveness or guarantee the same FPS on another machine.

Run `Tools/benchmark_scene.ps1` to repeat the comparison. Unreal 5.6's startup
`-csvCaptureFrames` option failed before launch on this installation; the script
starts capture through `CsvProfile` after engine initialization instead.

### Image quality update

TAA replaces FXAA, with rendering at 100% of the current viewport and Windows
high-DPI support enabled. The default fullscreen size is detected from the display;
no fixed monitor resolution is required. Saved window settings and command-line
window sizes remain supported. HUD glyphs use the final font size and pixel-aligned
positions, with the same font metrics for wrapping and ellipsis.

Visual checks at 960x540, 1280x720 and 2560x1080 confirmed crisp HUD text and smoother
model silhouettes. These checks used each viewport's own resolution, without
supersampling. The 2560x1080 measurement below used the same hardware and capture
method described above, comparing the pre-update build with the final update.
All five automation tests passed after the font changes. The rebuilt Shipping
executable also rendered correctly in a 1280x720 window, including its import menu.
Live API import loaded the six-board test project successfully in that build.

| Mean time at 2560x1080 | Previous FXAA and scaled fonts | TAA and final-size fonts |
|---|---:|---:|
| Frame | 2.15 ms | 2.09 ms |
| GPU | 1.62 ms | 1.78 ms |
| Game thread | 0.91 ms | 0.79 ms |

GPU time increased by about 0.17 ms; overall frame time was similar in this capture.
Small frame/thread differences should be treated as run-to-run variation.

## Scope

The default Unreal scene uses the Unity example's dungeon layout, KayKit models,
texture atlases, portraits, audio and story. Engine lighting and animation
rendering still produce visual differences. The original village assets and template code were removed after the dependency
audit. The benchmark tool accepts a separate checkout for historical comparisons.

The bundled story retains Unity's inconsistent Fighter price text: its introduction
says 40 gold, while the choice and script charge 50.
