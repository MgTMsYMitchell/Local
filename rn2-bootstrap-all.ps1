#Requires -Version 7.0
<#
.SYNOPSIS
    RN2 / RQ2 one-off bootstrap for Windows 11 + PowerShell 7.

.DESCRIPTION
    Sets up the full RN2 local-first AI ecosystem:
      - Checks/installs Python 3.11+, Node.js 20+
      - Creates the Python venv and installs PyTorch + dependencies
      - Installs Node dependencies for the runtime (node/) and UI (ui/)
      - Initialises the SQLite database from schema.sql

.NOTES
    Run from the repository root:
      pwsh -ExecutionPolicy Bypass -File .\rn2-bootstrap-all.ps1
#>

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$Root = $PSScriptRoot

# ---------------------------------------------------------------------------
# Helper
# ---------------------------------------------------------------------------
function Test-Command($cmd) {
    return $null -ne (Get-Command $cmd -ErrorAction SilentlyContinue)
}

function Assert-Command($cmd, $hint) {
    if (-not (Test-Command $cmd)) {
        Write-Error "Required command '$cmd' not found. $hint"
        exit 1
    }
}

# ---------------------------------------------------------------------------
# 1. Check prerequisites
# ---------------------------------------------------------------------------
Write-Host "`n==> Checking prerequisites..." -ForegroundColor Cyan

Assert-Command "python" "Install Python 3.11+ from https://www.python.org/downloads/"
Assert-Command "node"   "Install Node.js 20+ from https://nodejs.org/"
Assert-Command "npm"    "npm should ship with Node.js."

$pyVersion = python --version 2>&1
$nodeVersion = node --version 2>&1
Write-Host "  Python : $pyVersion"
Write-Host "  Node   : $nodeVersion"

# ---------------------------------------------------------------------------
# 2. Python venv + PyTorch
# ---------------------------------------------------------------------------
Write-Host "`n==> Setting up Python virtual environment..." -ForegroundColor Cyan

$venvPath = Join-Path $Root "backend" ".venv"
if (-not (Test-Path $venvPath)) {
    python -m venv $venvPath
    Write-Host "  Created venv at $venvPath"
} else {
    Write-Host "  venv already exists at $venvPath"
}

$pip = Join-Path $venvPath "Scripts" "pip.exe"

Write-Host "`n==> Installing Python dependencies..." -ForegroundColor Cyan
& $pip install --upgrade pip --quiet
& $pip install torch==2.2.2 --index-url https://download.pytorch.org/whl/cpu --quiet
& $pip install pyyaml --quiet
Write-Host "  PyTorch (CPU) + pyyaml installed."

# ---------------------------------------------------------------------------
# 3. Node runtime dependencies
# ---------------------------------------------------------------------------
Write-Host "`n==> Installing node/ dependencies..." -ForegroundColor Cyan
Push-Location (Join-Path $Root "node")
npm install --silent
Pop-Location

# ---------------------------------------------------------------------------
# 4. UI dependencies
# ---------------------------------------------------------------------------
Write-Host "`n==> Installing ui/ dependencies..." -ForegroundColor Cyan
Push-Location (Join-Path $Root "ui")
npm install --silent
Pop-Location

# ---------------------------------------------------------------------------
# 5. Initialise SQLite database
# ---------------------------------------------------------------------------
Write-Host "`n==> Initialising SQLite database..." -ForegroundColor Cyan

$dataDir = Join-Path $Root "backend" "data"
New-Item -ItemType Directory -Force -Path $dataDir | Out-Null

$dbPath   = Join-Path $dataDir "rn2.db"
$schemaPath = Join-Path $Root "backend" "schema.sql"

if (Test-Command "sqlite3") {
    Get-Content $schemaPath | sqlite3 $dbPath
    Write-Host "  Database initialised at $dbPath"
} else {
    Write-Warning "sqlite3 CLI not found — skipping DB init. Install from https://sqlite.org/download.html or initialise manually."
}

# ---------------------------------------------------------------------------
# 6. Create models/ directory
# ---------------------------------------------------------------------------
$modelsDir = Join-Path $Root "backend" "models"
New-Item -ItemType Directory -Force -Path $modelsDir | Out-Null

# ---------------------------------------------------------------------------
# Done
# ---------------------------------------------------------------------------
Write-Host "`n==> Bootstrap complete!" -ForegroundColor Green
Write-Host @"

Next steps:
  cd backend
  .\.venv\Scripts\Activate.ps1
  python train_rn2.py

  cd node
  npx ts-node src/index.ts

  cd ui
  npx vite
"@
