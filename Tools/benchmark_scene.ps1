param(
 [ValidateSet('Legacy','KayKit')][string]$Scene='KayKit',
 [string]$UnrealEditor='C:/Program Files/Epic Games/UE_5.6/Engine/Binaries/Win64/UnrealEditor-Cmd.exe',
 [string]$LegacyProject,
 [ValidateRange(640,7680)][int]$Width=1280,
 [ValidateRange(480,4320)][int]$Height=720,
 [ValidateSet('Project','FXAA','TAA','TSR')][string]$AntiAliasing='Project'
)
$ErrorActionPreference='Stop'
$workspaceRoot=Split-Path $PSScriptRoot
$resultsRoot=Join-Path $workspaceRoot 'ArcweaveDemo/Saved/Validation/Benchmarks'
[System.IO.Directory]::CreateDirectory($resultsRoot) | Out-Null
$projectPath=Join-Path $workspaceRoot 'ArcweaveDemo/ArcweaveDemo.uproject'
if($Scene -eq 'Legacy') {
 if(!$LegacyProject -or !(Test-Path -LiteralPath $LegacyProject)) { throw 'Legacy requires -LegacyProject pointing to a built checkout of the original example.' }
 $projectPath=(Resolve-Path -LiteralPath $LegacyProject).Path
 if(!(Test-Path -LiteralPath (Join-Path (Split-Path $projectPath) 'Content/TopDown/Maps/TopDownMap_Day.umap'))) { throw 'The specified checkout does not contain the legacy village map.' }
}
$projectPath=$projectPath.Replace('\','/')
$profileRoot=Join-Path (Split-Path $projectPath) 'Saved/Profiling/CSV'
$captureName='benchmark-'+$Scene+'-'+$Width+'x'+$Height+'-'+$AntiAliasing
$logPath=Join-Path $resultsRoot ($captureName+'.log')
$logPath=$logPath.Replace('\','/')
$map=if($Scene -eq 'Legacy'){'/Game/TopDown/Maps/TopDownMap_Day'}else{'/Game/KayKit/Maps/ArcweaveDemo'}
$benchmarkArguments=@('"'+$projectPath+'"',$map,'-game','-RenderOffscreen','-windowed',('-ResX='+$Width),('-ResY='+$Height),'-ForceRes','-NoVSync','-NoSound','-benchmark','-fps=60','-seconds=25','-csvCompression=0','-unattended','-NoSplash','-ExecCmds="csvprofile frames=1000,r.GPUCsvStatsEnabled 1,r.VSync 0,t.MaxFPS 0,r.ScreenPercentage 100,sg.ViewDistanceQuality 3,sg.ShadowQuality 3,sg.TextureQuality 3"','-csvExecCmds="200:Shot"','-abslog="'+$logPath+'"')
if($Scene -eq 'Legacy') {
 $benchmarkArguments+=@('-ini:Engine:[/Script/EngineSettings.GameMapsSettings]:GlobalDefaultGameMode=/Script/ArcweaveDemo.ArcweaveDemoGameMode','-ini:Engine:[/Script/EngineSettings.GameMapsSettings]:GameInstanceClass=/Game/ArcweaveDemo/Blueprints/BP_Arcweave_GameInstance.BP_Arcweave_GameInstance_C','-ini:Engine:[/Script/Engine.RendererSettings]:r.DynamicGlobalIlluminationMethod=1','-ini:Engine:[/Script/Engine.RendererSettings]:r.ReflectionMethod=1','-ini:Engine:[/Script/Engine.RendererSettings]:r.Shadow.Virtual.Enable=1','-ini:Engine:[/Script/Engine.RendererSettings]:r.GenerateMeshDistanceFields=True','-ini:Engine:[/Script/Engine.RendererSettings]:r.AntiAliasingMethod=4','-ini:Engine:[/Script/Engine.RendererSettings]:r.DefaultFeature.AutoExposure=True','-ini:Engine:[/Script/Engine.RendererSettings]:r.DefaultFeature.MotionBlur=True')
}
if($AntiAliasing -ne 'Project') {
 $method=@{FXAA=1;TAA=2;TSR=4}[$AntiAliasing]
 $benchmarkArguments+=('-ini:Engine:[/Script/Engine.RendererSettings]:r.AntiAliasingMethod='+$method)
}
$runStart=Get-Date
[System.IO.File]::WriteAllText((Join-Path $resultsRoot 'benchmark-arguments.txt'),($benchmarkArguments -join ' '))
$process=Start-Process -FilePath $UnrealEditor -ArgumentList ($benchmarkArguments -join ' ') -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $resultsRoot 'benchmark_stdout.txt') -RedirectStandardError (Join-Path $resultsRoot 'benchmark_stderr.txt')
$processHandle=$process.Handle
$process.WaitForExit()
Write-Output ('Unreal exit code: '+$process.ExitCode)
if($process.ExitCode -ne 0) { throw "Unreal benchmark failed. Inspect $logPath" }
$capture=Get-ChildItem -LiteralPath $profileRoot -Filter '*.csv' | Where-Object LastWriteTime -ge $runStart | Sort-Object LastWriteTime -Descending | Select-Object -First 1
if(!$capture){throw "No CSV capture produced. Inspect $logPath"}
$destination=Join-Path $resultsRoot ($captureName+'.csv')
Copy-Item -LiteralPath $capture.FullName -Destination $destination
Write-Output "$Scene finished: exit $($process.ExitCode), CSV $destination"
