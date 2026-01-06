<#
.SYNOPSIS
    P_MEIS Automated Test Runner.

.DESCRIPTION
    Builds and (optionally) runs UE automation tests for the P_MEIS plugin.

    Windows-first PowerShell runner intended for CI/headless usage.

.PARAMETER ProjectPath
    Optional project root path containing a .uproject. If omitted, auto-detects by walking up from this script.

.PARAMETER UEPath
    Optional UE engine root. If omitted, uses UE_ENGINE_ROOT env var or defaults to D:\UE\UE_S.

.PARAMETER SkipBuild
    Skip the build step.

.PARAMETER RunTests
    Run UE automation tests (Automation RunTests MEIS).

.PARAMETER Timeout
    Timeout in seconds for UnrealEditor-Cmd processes.

.EXAMPLE
    .\RunMEISTests.ps1

.EXAMPLE
    .\RunMEISTests.ps1 -SkipBuild -RunTests -Timeout 600
#>

param(
    [string]$ProjectPath = $null,
    [string]$UEPath = $null,
    [switch]$SkipBuild,
    [switch]$RunTests,
    [int]$Timeout = 300,
    [switch]$VerboseOutput
)

$ErrorActionPreference = 'Stop'

Write-Host "" 
Write-Host "================================================================" -ForegroundColor Cyan
Write-Host "     P_MEIS Automated Testing Framework                          " -ForegroundColor Cyan
Write-Host "================================================================" -ForegroundColor Cyan
Write-Host ""

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$PluginRoot = (Resolve-Path (Join-Path $ScriptDir "..\..") ).Path
$DevToolsRoot = (Resolve-Path (Join-Path $ScriptDir "..") ).Path

function Write-Log {
    param([string]$Message, [string]$Level = "Info")

    $timestamp = Get-Date -Format "HH:mm:ss"
    switch ($Level) {
        "Success" { Write-Host "[$timestamp] [OK] $Message" -ForegroundColor Green }
        "Error" { Write-Host "[$timestamp] [ERROR] $Message" -ForegroundColor Red }
        "Warning" { Write-Host "[$timestamp] [WARN] $Message" -ForegroundColor Yellow }
        default { Write-Host "[$timestamp] $Message" -ForegroundColor Gray }
    }
}

function Find-ProjectFile {
    param([string]$StartPath)

    $searchPath = $StartPath
    for ($i = 0; $i -lt 6; $i++) {
        $uprojects = Get-ChildItem -Path $searchPath -Filter "*.uproject" -File -ErrorAction SilentlyContinue
        if ($uprojects) {
            return $uprojects[0]
        }
        $searchPath = Split-Path -Parent $searchPath
    }
    return $null
}

Write-Log "Detecting environment..."

# Detect project
if (-not $ProjectPath) {
    $ProjectFile = Find-ProjectFile -StartPath $PluginRoot
    if (-not $ProjectFile) {
        Write-Log "Could not detect project. Specify -ProjectPath" -Level Error
        exit 1
    }
    $ProjectPath = Split-Path -Parent $ProjectFile.FullName
}
else {
    $uprojects = Get-ChildItem -Path $ProjectPath -Filter "*.uproject" -File -ErrorAction SilentlyContinue
    if (-not $uprojects) {
        Write-Log "No .uproject found in $ProjectPath" -Level Error
        exit 1
    }
    $ProjectFile = $uprojects[0]
}

$ProjectName = [System.IO.Path]::GetFileNameWithoutExtension($ProjectFile.Name)

# Detect UE
if (-not $UEPath) {
    $UEPath = if ($env:UE_ENGINE_ROOT) { $env:UE_ENGINE_ROOT } else { "D:\UE\UE_S" }
}

$UEEditorCmd = Join-Path $UEPath "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
$UEBuildBat = Join-Path $UEPath "Engine\Build\BatchFiles\Build.bat"

Write-Log "Project: $($ProjectFile.FullName)" -Level Success
Write-Log "UE Path: $UEPath" -Level Success
Write-Log "Plugin:  $PluginRoot" -Level Success
Write-Host ""

$allPassed = $true
$results = @()

# Step 1: Build
if (-not $SkipBuild) {
    Write-Log "════════════════════════════════════════════════════════════════"
    Write-Log "Step 1: Building Project"
    Write-Log "════════════════════════════════════════════════════════════════"

    if (-not (Test-Path $UEBuildBat)) {
        Write-Log "Build.bat not found at $UEBuildBat" -Level Error
        $results += "Build: FAIL"
        $allPassed = $false
    }
    else {
        $buildArgs = @(
            "${ProjectName}Editor", "Win64", "Development",
            "-Project=`"$($ProjectFile.FullName)`"", "-WaitMutex", "-FromMsBuild"
        )

        Write-Log "Building ${ProjectName}Editor..."
        $buildProcess = Start-Process -FilePath $UEBuildBat -ArgumentList $buildArgs -NoNewWindow -Wait -PassThru

        if ($buildProcess.ExitCode -eq 0) {
            Write-Log "Build PASSED" -Level Success
            $results += "Build: PASS"
        }
        else {
            Write-Log "Build FAILED (exit code: $($buildProcess.ExitCode))" -Level Error
            $results += "Build: FAIL"
            $allPassed = $false
        }
    }
    Write-Host ""
}
else {
    Write-Log "Skipping build step (-SkipBuild)" -Level Warning
    $results += "Build: SKIP"
    Write-Host ""
}

# Step 2: Automation tests (optional)
if ($RunTests) {
    Write-Log "════════════════════════════════════════════════════════════════"
    Write-Log "Step 2: Running Automation Tests"
    Write-Log "════════════════════════════════════════════════════════════════"

    if (-not (Test-Path $UEEditorCmd)) {
        Write-Log "UnrealEditor-Cmd.exe not found at $UEEditorCmd" -Level Error
        $results += "AutomationTests: FAIL"
        $allPassed = $false
    }
    else {
        $outputDir = Join-Path $DevToolsRoot "output"
        if (-not (Test-Path $outputDir)) {
            New-Item -ItemType Directory -Path $outputDir -Force | Out-Null
        }
        $stdout = Join-Path $outputDir "meis_tests_stdout.log"
        $stderr = Join-Path $outputDir "meis_tests_stderr.log"

        $testArgs = @(
            "`"$($ProjectFile.FullName)`"",
            "-Map=/Engine/Maps/Entry",
            "-ExecCmds=`"Automation RunTests MEIS; Quit`"",
            "-unattended", "-nullrhi", "-nop4",
            "-testexit=`"Automation Test Queue Empty`"",
            "-stdout", "-FullStdOutLogOutput"
        )

        Write-Log "Running MEIS automation tests..."
        $proc = Start-Process -FilePath $UEEditorCmd -ArgumentList $testArgs -NoNewWindow -PassThru -RedirectStandardOutput $stdout -RedirectStandardError $stderr
        $waited = $proc | Wait-Process -Timeout $Timeout -ErrorAction SilentlyContinue

        if (-not $waited) {
            Write-Log "Automation tests timed out after ${Timeout}s; terminating UnrealEditor-Cmd" -Level Error
            Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue
            $results += "AutomationTests: FAIL"
            $allPassed = $false
        }
        elseif ($proc.ExitCode -eq 0) {
            Write-Log "Automation Tests PASSED" -Level Success
            $results += "AutomationTests: PASS"
        }
        else {
            Write-Log "Automation Tests FAILED (exit code: $($proc.ExitCode))" -Level Error
            $results += "AutomationTests: FAIL"
            $allPassed = $false
        }
    }

    Write-Host ""
}
else {
    Write-Log "Skipping automation tests (use -RunTests to enable)" -Level Warning
    $results += "AutomationTests: SKIP"
    Write-Host ""
}

Write-Log "════════════════════════════════════════════════════════════════"
Write-Log "TEST SUMMARY"
Write-Log "════════════════════════════════════════════════════════════════"

foreach ($r in $results) {
    if ($r -like "*PASS*") {
        Write-Log $r -Level Success
    }
    elseif ($r -like "*SKIP*") {
        Write-Log $r -Level Warning
    }
    else {
        Write-Log $r -Level Error
    }
}

Write-Log "════════════════════════════════════════════════════════════════"

if ($allPassed) {
    Write-Host "" 
    Write-Host "================================================================" -ForegroundColor Green
    Write-Host "          ALL CHECKS PASSED                                     " -ForegroundColor Green
    Write-Host "================================================================" -ForegroundColor Green
    Write-Host ""
    exit 0
}

Write-Host "" 
Write-Host "================================================================" -ForegroundColor Red
Write-Host "          SOME CHECKS FAILED                                    " -ForegroundColor Red
Write-Host "================================================================" -ForegroundColor Red
Write-Host ""
exit 1
