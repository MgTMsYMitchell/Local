# Rune System — Windows Bootstrap
# Installs all required tools via winget.
# Run from an elevated PowerShell:
#   powershell -ExecutionPolicy Bypass -File scripts\bootstrap.ps1

#Requires -RunAsAdministrator

Write-Host "=== Rune System Windows Bootstrap ===" -ForegroundColor Cyan
Write-Host ""

$tools = @(
    @{ id = "Git.Git";                                    name = "Git"              },
    @{ id = "Kitware.CMake";                              name = "CMake"            },
    @{ id = "Ninja-build.Ninja";                          name = "Ninja"            },
    @{ id = "OpenJS.NodeJS.LTS";                          name = "Node.js LTS"      },
    @{ id = "Microsoft.VisualStudio.2022.BuildTools";     name = "MSVC BuildTools"  },
    @{ id = "Facebook.Zstd";                              name = "zstd"             }
)

foreach ($t in $tools) {
    Write-Host "Installing $($t.name)..." -ForegroundColor Yellow
    winget install --id $t.id -e --accept-source-agreements --accept-package-agreements
    if ($LASTEXITCODE -ne 0) {
        Write-Host "  NOTE: $($t.name) may already be installed or requires manual install." -ForegroundColor DarkYellow
    }
}

Write-Host ""
Write-Host "Bootstrap complete." -ForegroundColor Green
Write-Host "Restart your terminal to pick up PATH changes, then run:"
Write-Host ""
Write-Host "  powershell -ExecutionPolicy Bypass -File scripts\check-deps.ps1"
Write-Host ""
Write-Host "Build backend:"
Write-Host "  cd rune-backend"
Write-Host "  cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release"
Write-Host "  cmake --build build"
Write-Host ""
Write-Host "Start web UI (separate terminal):"
Write-Host "  cd rune-web && npm install && npm start"
Write-Host ""
Write-Host "Run backend (separate terminal):"
Write-Host "  .\rune-backend\build\rune.exe"
Write-Host ""
Write-Host "Run agents (separate terminal):"
Write-Host "  cd rune-agents"
Write-Host "  node chat-agent.js"
