[CmdletBinding()]
param(
  [switch]$Once
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = Resolve-Path (Join-Path $scriptDir "..")

function Invoke-DebugBuild {
  Write-Host ""
  Write-Host ("[{0}] Building Debug preset..." -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss"))
  & cmake --build --preset build-debug
  if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
  }
}

Push-Location $repoRoot
try {
  foreach ($tool in @("cmake", "watchexec")) {
    if (-not (Get-Command $tool -ErrorAction SilentlyContinue)) {
      throw "Required tool not found on PATH: $tool"
    }
  }

  if ($Once) {
    Invoke-DebugBuild
    exit 0
  }

  Write-Host ("[{0}] Configuring Debug preset..." -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss"))
  & cmake --preset debug
  if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
  }

  Invoke-DebugBuild

  $self = (Resolve-Path $MyInvocation.MyCommand.Path).Path

  Write-Host ""
  Write-Host "Watching src/ for *.cpp and *.h changes..."
  Write-Host "Press Ctrl+C to stop."

  & watchexec `
    --watch src `
    --exts cpp,h `
    --delay-run 300ms `
    -- powershell -NoProfile -ExecutionPolicy Bypass -File $self -Once

  exit $LASTEXITCODE
}
finally {
  Pop-Location
}
