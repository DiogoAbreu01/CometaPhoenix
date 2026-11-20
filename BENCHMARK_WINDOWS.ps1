# Executar depois de compilar/instalar o jogo. Nao instala nem modifica o sistema.
param([string]$Exe = (Join-Path $PSScriptRoot 'windows-portable\cometa_game.exe'), [int]$Frames = 600)
$ErrorActionPreference = 'Stop'
if (!(Test-Path -LiteralPath $Exe)) { throw "Executavel ausente: $Exe. Compile e instale primeiro." }
$Exe = (Resolve-Path -LiteralPath $Exe).Path
$log = Join-Path $PSScriptRoot 'BENCHMARK_GPU_WINDOWS.txt'
"PHOENIX $(Get-Date -Format o)" | Set-Content $log
Get-CimInstance Win32_VideoController | Select-Object Name,DriverVersion | Out-String | Add-Content $log
Get-CimInstance Win32_Processor | Select-Object Name,NumberOfCores | Out-String | Add-Content $log
foreach($q in 0..3) {
  foreach($scene in 'race','garage') {
    "$scene Q$q" | Tee-Object -FilePath $log -Append
    $opts = @('--benchmark','--frames', "$Frames",'--quality', "$q")
    if($scene -eq 'garage'){ $opts += '--garage' } else {$opts += @('--autopilot','--weather','rain')}
    & $Exe @opts 2>&1 | Tee-Object -FilePath $log -Append
    if($LASTEXITCODE -ne 0){ throw "Benchmark falhou ($LASTEXITCODE)" }
  }
}
