# =============================================================================
# roc-ai-vision · one-click launcher
#
# Right-click → "Run with PowerShell" (or invoke from a terminal).
# Starts the C++ orchestrator + SvelteKit dashboard, opens the browser.
# Ctrl-C in this window cleanly stops both child processes.
# =============================================================================

$ErrorActionPreference = 'Stop'

$Root        = Split-Path -Parent $MyInvocation.MyCommand.Path
$Orch        = Join-Path $Root 'core\build\roc_vision.exe'
$Dashboard   = Join-Path $Root 'dashboard'
$Model       = Join-Path $Root 'models\yolo26n-coco.onnx'
$DashUrl     = 'http://127.0.0.1:5173'
$WsPort      = 8765
$MjpegPort   = 8766

Write-Host ''
Write-Host '  roc-ai-vision launcher' -ForegroundColor Cyan
Write-Host '  ----------------------'  -ForegroundColor DarkGray

if (-not (Test-Path $Orch))      { throw "Orchestrator missing: $Orch (build first: cmake --build core/build)" }
if (-not (Test-Path $Model))     { throw "Model missing: $Model" }
if (-not (Test-Path $Dashboard)) { throw "Dashboard dir missing: $Dashboard" }

# Probe camera 0 via ffmpeg dshow if available — purely informational, the
# orchestrator will print the actual resolved capture size on startup.
$ffmpeg = (Get-Command ffmpeg.exe -ErrorAction SilentlyContinue)?.Source
if ($ffmpeg) {
    try {
        $devList = & $ffmpeg -hide_banner -f dshow -list_devices true -i dummy 2>&1
        $videoDevs = $devList | Select-String -Pattern '"([^"]+)" \(video\)' | ForEach-Object { $_.Matches[0].Groups[1].Value }
        if ($videoDevs.Count -gt 0) {
            Write-Host "  Video devices: " -NoNewline -ForegroundColor DarkGray
            Write-Host ($videoDevs -join ', ') -ForegroundColor Gray
        }
    } catch { }
}

$procs = @()

function Stop-All {
    foreach ($p in $procs) {
        if ($p -and -not $p.HasExited) {
            try { $p.CloseMainWindow() | Out-Null; Start-Sleep -Milliseconds 200 } catch {}
            try { if (-not $p.HasExited) { $p.Kill($true) } } catch {}
        }
    }
}

# Ensure children die when this script exits (Ctrl-C or normal exit).
$null = Register-EngineEvent PowerShell.Exiting -Action { Stop-All } -SupportEvent

try {
    Write-Host ''
    Write-Host "  [1/3] Starting orchestrator..." -ForegroundColor Cyan
    Write-Host "        $Orch --model models/yolo26n-coco.onnx --camera 0" -ForegroundColor DarkGray

    $orchPsi = New-Object System.Diagnostics.ProcessStartInfo
    $orchPsi.FileName  = $Orch
    $orchPsi.Arguments = "--model `"$Model`" --camera 0 --port $WsPort --mjpeg-port $MjpegPort"
    $orchPsi.WorkingDirectory     = $Root
    $orchPsi.UseShellExecute      = $false
    $orchPsi.RedirectStandardOutput = $false
    $orchPsi.RedirectStandardError  = $false
    $orchPsi.CreateNoWindow       = $false
    $orchProc = [System.Diagnostics.Process]::Start($orchPsi)
    $procs += $orchProc

    Write-Host ''
    Write-Host "  [2/3] Starting dashboard dev server..." -ForegroundColor Cyan
    Write-Host "        npm run dev (in $Dashboard)"      -ForegroundColor DarkGray

    $dashPsi = New-Object System.Diagnostics.ProcessStartInfo
    $dashPsi.FileName  = 'cmd.exe'
    $dashPsi.Arguments = "/c npm run dev"
    $dashPsi.WorkingDirectory     = $Dashboard
    $dashPsi.UseShellExecute      = $false
    $dashPsi.CreateNoWindow       = $false
    $dashProc = [System.Diagnostics.Process]::Start($dashPsi)
    $procs += $dashProc

    Write-Host ''
    Write-Host "  [3/3] Waiting for dashboard to come up at $DashUrl ..." -ForegroundColor Cyan

    $deadline = (Get-Date).AddSeconds(45)
    $up = $false
    while ((Get-Date) -lt $deadline) {
        try {
            $r = Invoke-WebRequest -Uri $DashUrl -UseBasicParsing -TimeoutSec 1 -ErrorAction Stop
            if ($r.StatusCode -ge 200 -and $r.StatusCode -lt 500) { $up = $true; break }
        } catch { Start-Sleep -Milliseconds 500 }
    }

    Write-Host ''
    if ($up) {
        Write-Host "  Dashboard:    " -NoNewline -ForegroundColor Green
        Write-Host $DashUrl -ForegroundColor White
        Write-Host "  WebSocket:    " -NoNewline -ForegroundColor Green
        Write-Host "ws://127.0.0.1:$WsPort" -ForegroundColor White
        Write-Host "  MJPEG video:  " -NoNewline -ForegroundColor Green
        Write-Host "http://127.0.0.1:$MjpegPort/mjpeg/0" -ForegroundColor White
        Start-Process $DashUrl
    } else {
        Write-Host "  Dashboard didn't respond within 45s. It may still be compiling — check the dashboard terminal window." -ForegroundColor Yellow
        Write-Host "  Try opening $DashUrl manually." -ForegroundColor Yellow
    }

    Write-Host ''
    Write-Host '  Ctrl-C here to stop both processes cleanly.' -ForegroundColor DarkGray
    Write-Host ''

    # Wait for either child to exit, then tear the other down.
    while (-not $orchProc.HasExited -and -not $dashProc.HasExited) {
        Start-Sleep -Milliseconds 500
    }
    if ($orchProc.HasExited) {
        Write-Host "  Orchestrator exited (code $($orchProc.ExitCode)) — stopping dashboard." -ForegroundColor Yellow
    } else {
        Write-Host "  Dashboard exited (code $($dashProc.ExitCode)) — stopping orchestrator." -ForegroundColor Yellow
    }
}
finally {
    Stop-All
}
