# Creates a Desktop shortcut for portable Graber (run once after unzip).
$ErrorActionPreference = "Stop"
$here = Split-Path -Parent $MyInvocation.MyCommand.Path
$exe = Join-Path $here "graber.exe"
if (-not (Test-Path $exe)) {
    Write-Error "graber.exe not found next to this script: $exe"
    exit 1
}
$desktop = [Environment]::GetFolderPath("Desktop")
$lnkPath = Join-Path $desktop "Clipboard Graber.lnk"
$wsh = New-Object -ComObject WScript.Shell
$sc = $wsh.CreateShortcut($lnkPath)
$sc.TargetPath = $exe
$sc.WorkingDirectory = $here
$sc.WindowStyle = 1
$sc.Description = "Clipboard Graber — capture notes from the clipboard"
$ico = Join-Path $here "graber.ico"
if (Test-Path $ico) { $sc.IconLocation = $ico }
$sc.Save()
Write-Host "Desktop shortcut created: $lnkPath"
