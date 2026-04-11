# Rune System — Windows Dependency Check
# Run: powershell -ExecutionPolicy Bypass -File scripts\check-deps.ps1

Write-Host "=== Rune System Dependency Check ===" -ForegroundColor Cyan
Write-Host ""

$allOk = $true

function Check-Command {
    param([string]$Name, [string]$Cmd)
    try {
        $out = Invoke-Expression "$Cmd 2>&1" | Select-Object -First 1
        Write-Host ("[OK]      {0,-14} {1}" -f $Name, $out) -ForegroundColor Green
    } catch {
        Write-Host ("[MISSING] {0}" -f $Name) -ForegroundColor Red
        $script:allOk = $false
    }
}

Check-Command "git"    "git --version"
Check-Command "cmake"  "cmake --version"
Check-Command "ninja"  "ninja --version"
Check-Command "node"   "node --version"
Check-Command "npm"    "npm --version"
Check-Command "curl"   "curl --version"

# Check for zstd (used for model downloads with LM Studio)
Check-Command "zstd"   "zstd --version"

# Check C++ compiler (MSVC cl.exe or clang)
$cl    = Get-Command "cl.exe" -ErrorAction SilentlyContinue
$clang = Get-Command "clang"  -ErrorAction SilentlyContinue

if ($cl) {
    $v = & cl.exe 2>&1 | Select-Object -First 1
    Write-Host ("[OK]      {0,-14} {1}" -f "cl.exe (MSVC)", $v) -ForegroundColor Green
} elseif ($clang) {
    $v = & clang --version 2>&1 | Select-Object -First 1
    Write-Host ("[OK]      {0,-14} {1}" -f "clang", $v) -ForegroundColor Green
} else {
    Write-Host "[MISSING] C++ compiler (cl.exe or clang)" -ForegroundColor Red
    $allOk = $false
}

Write-Host ""
if ($allOk) {
    Write-Host "All dependencies satisfied. Ready to build." -ForegroundColor Cyan
} else {
    Write-Host "Some dependencies are missing." -ForegroundColor Yellow
    Write-Host "Run:  powershell -ExecutionPolicy Bypass -File scripts\bootstrap.ps1" -ForegroundColor Yellow
    exit 1
}
