# Validate the Arcweave integration

The example includes a local export in `ArcweaveDemo/Content/ArcweaveExport`.
Validation does not require an Arcweave API token.

After changing branches or pulling an update, initialize the pinned plugin:

```bash
git submodule update --init --recursive
```

Build the `ArcweaveDemoEditor` target in Development configuration. The commands
below use PowerShell; set the engine and project paths for your machine.

```powershell
$EngineDir = 'C:\Program Files\Epic Games\UE_5.6'
$ProjectFile = 'C:\Projects\arcweave-unreal-example\ArcweaveDemo\ArcweaveDemo.uproject'
$EditorCmd = Join-Path $EngineDir 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$BuildCmd = Join-Path $EngineDir 'Engine\Build\BatchFiles\Build.bat'

& $BuildCmd ArcweaveDemoEditor Win64 Development "-Project=$ProjectFile" -WaitMutex
```

Compile the Blueprints and run the plugin's scoped-variable automation test:

```powershell
& $EditorCmd $ProjectFile -run=CompileAllBlueprints -unattended -NullRHI -nosound

& $EditorCmd $ProjectFile -unattended -NullRHI -nosound `
    '-ExecCmds=Automation RunTests Arcweave.Project.ComponentAndBoardVariables' `
    '-TestExit=Automation Test Queue Empty'
```

Run the bundled story check with local JSON import enabled for this invocation:

```powershell
$ValidationScript = Join-Path (Split-Path $ProjectFile) 'Scripts\validate_demo.py'
& $EditorCmd $ProjectFile -run=pythonscript "-script=$ValidationScript" `
    -EnablePlugins=PythonScriptPlugin -unattended -NullRHI -nosound `
    '-ini:Game:[/Script/arcweave.ArcweaveSettings]:EnableReceiveMethodFromLocalJSON=True'
```

The script checks import of the demo boards, components and initial variables,
then runs the actual dialogue elements to obtain a potion and heal Wanda. It
verifies both variable changes and the resulting conditional dialogue. A passing
run logs `ARCWEAVE_DEMO_VALIDATION_PASSED`. The script restores the authored
variables and visits after the check.

These checks cover Blueprint compilation and the native data/interpreter path.
Also launch the default day map in Play mode and in a packaged build to check
movement, NPC interaction, dialogue choices and HUD rendering.

## Engine versions

The plugin update was built and tested on Windows with Unreal **5.6.1** and
MSVC **14.38**. The project still declares Unreal **5.5**; validation on that exact
engine version remains outstanding.
