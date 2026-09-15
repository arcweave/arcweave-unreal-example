# Rebuilding the shared example scene

The checked-in `SourceAssets/KayKit/scene.json` records the Unity example at commit
`ff909af`. Its models, texture atlases, portraits, audio and CC0 notices are included,
so rebuilding the Unreal map does not require Unity.

Build the `ArcweaveDemoEditor` target with Unreal 5.6, then run:

```powershell
& "$Unreal/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" `
  "$Repo/ArcweaveDemo/ArcweaveDemo.uproject" `
  -EnablePlugins=PythonScriptPlugin -run=pythonscript `
  "-script=$Repo/Tools/build_kaykit_scene.py" -unattended
```

Use forward slashes in the script path. The builder replaces
`/Game/KayKit/Maps/ArcweaveDemo` and reuses the imported FBX assets. It explicitly
converts OBJ coordinates from metres to Unreal centimetres, retains Unity's torch
node transform, and removes coincident duplicate floor tiles. Hand edits to the
generated map will be overwritten when it is rebuilt.

To refresh from Unity, put `ExportUnrealReference.cs` and `ExportTorch.cs` in an
`Assets/Editor` directory in a temporary checkout of the Unity example. Execute
`ExportUnrealReference.Run` with Unity's `-executeMethod` option. The scripts write
`scene.json`, `torch_lit.obj` and a camera reference image to `../unity-export`.
`prepare_unity_reference.py` accepts `--unity-project <directory>` and
`--export <scene.json>`. Defaults are the sibling `arcweave-unity-example` and
`unity-export/scene.json` directories. Run it before rebuilding the Unreal scene.

The original village is available at commit
`c70fb5fc8ff61e76ca7213e6246a286f54ac8c6c`. To repeat a Legacy benchmark,
create a separate checkout of that commit, initialize its plugin submodule and
build it with the same Unreal version. Pass its `.uproject` to
`benchmark_scene.ps1 -Scene Legacy -LegacyProject <path>`. The current checkout
contains only the KayKit scene.

## Verification

Run `Arcweave.KayKit.Controls` in Unreal's Automation window, or execute
`Automation RunTests Arcweave.KayKit.Controls` in the editor console. The test
opens the generated map, starts Play, checks W movement and wall collision, uses E
to talk, then clicks through the Healer dialogue and verifies the potion and final
line. It resets story variables afterwards.

`Arcweave.KayKit.ProjectVariables` checks that the HUD follows the imported
variable list, distinguishes global and scoped names, receives value changes,
and removes stale rows when replacing or clearing a project. It also checks live
weather/time/color changes and recovery after an import failure. The plugin's
`Arcweave.Project.ComponentAndBoardVariables` test exercises parsing, script
assignments, defaults and resets for scoped attribute variables. Run
`Automation RunTests Arcweave.` to include all five automation tests.
`Arcweave.KayKit.VariablePanel` checks bounded long values, pagination and Page Down
input. `Arcweave.Project.ImportFailures` checks invalid exports, transport/HTTP
failures, retries and rejection of late cancelled responses.

`benchmark_scene.ps1 -Scene Legacy -LegacyProject <path>` and
`benchmark_scene.ps1 -Scene KayKit` capture
1,000 standalone frames at 1280×720, 100% resolution, with VSync and audio disabled.
Close other engine sessions first. The Legacy run restores the original Lumen,
virtual shadow map and TSR settings through command-line overrides; KayKit uses
the new project defaults. These are offscreen measurements of the complete scene
and rendering changes, not editor viewport FPS. Discard the first 500 frames for
warm-up. Logs and CSV files are written under `Saved/Validation/Benchmarks`.
Use `-Width 960 -Height 540` to check a smaller window, or pass another resolution.
`-AntiAliasing FXAA`, `TAA` or `TSR` overrides only anti-aliasing for comparisons;
the default `Project` uses the scene's settings. Output names include the scene,
resolution and anti-aliasing selection. Run twice and compare the final 500 frames
of the second run after shader and asset caches have warmed.

`verify_kaykit_story.py` runs inside an active Play session and checks the Healer,
Wanda and Fighter branches, inventory, health, combat unlock, menu and animations.
It writes a JSON report under `Saved/Validation` and resets the story afterwards.

For a live API check in a Shipping build, enter credentials in the Esc menu,
close and reopen it before importing, and confirm both fields remain populated.
Import a project with a different name or board count from the bundled example,
check the on-screen load confirmation and a dialogue branch, load the bundled
JSON, and import again without re-entering credentials. `LoadStory` calls the
plugin's explicit API or JSON method because `FetchData` reloads saved settings
and can override the source chosen in the runtime menu. The controller subscribes
to `OnArcweaveFetchCompleted` for failures; `CancelFetch` prevents older requests
from replacing a newer import. Check an invalid token too: the error should appear
in the menu and the previously loaded story should remain playable.

The example's source story says “40 Gold” in the Fighter introduction but charges
50 in its choice and script. This is retained from Unity so both examples use the
same story data.
