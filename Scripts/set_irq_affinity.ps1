# scripts/set_irq_affinity.ps1 — 1:1 Hardware MSI Interrupt Steering (i5-9600K Elastic 2:3:1 + Storage Quarantine)
#Requires -RunAsAdministrator

function Set-DeviceIrqAffinity {
    param(
        [string]$MatchName,
        [uint64]$Mask,
        [string]$Label
    )
    $devices = Get-PnpDevice -Status OK | Where-Object { $_.FriendlyName -like "*$MatchName*" }
    if (-not $devices) {
        Write-Host "[WARN] No PnP device matched '$MatchName'" -ForegroundColor Yellow
        return
    }
    foreach ($dev in $devices) {
        $regPath = "HKLM:\SYSTEM\CurrentControlSet\Enum\$($dev.InstanceId)\Device Parameters\Interrupt Management\Affinity Policy"
        if (-not (Test-Path $regPath)) {
            New-Item -Path $regPath -Force | Out-Null
        }
        # DevicePolicy = 4 (IrqPolicySpecifiedProcessors)
        New-ItemProperty -Path $regPath -Name "DevicePolicy" -Value 4 -PropertyType DWord -Force | Out-Null
        $bytes = [BitConverter]::GetBytes($Mask)
        New-ItemProperty -Path $regPath -Name "AssignmentSetOverride" -Value $bytes -PropertyType Binary -Force | Out-Null
        Write-Host "[OK] Steered '$($dev.FriendlyName)' ($Label) -> Mask 0x$($Mask.ToString('X2'))" -ForegroundColor Green
    }
}

# 1. Intel USB 3.0/3.10 eXtensible Host Controller (HiBy FC3 DAC) -> Core 0 (0x01)
Set-DeviceIrqAffinity -MatchName "eXtensible Host Controller" -Mask 0x01 -Label "Core 0: Audio/Timing"

# 2. NVIDIA GTX 980 Ti -> Core 1 (0x02)
Set-DeviceIrqAffinity -MatchName "NVIDIA GeForce GTX 980 Ti"  -Mask 0x02 -Label "Core 1: Vulkan Submit"

# 3. Standard NVM Express Controllers (NVMe #1 OS/Eden + NVMe #2 Updates/DLCs) -> Core 5 (0x20)
Set-DeviceIrqAffinity -MatchName "Standard NVM Express"       -Mask 0x20 -Label "Core 5: Aux Quarantine (NVMe)"

# 4. SATA AHCI Controller (7200 RPM Level-18 .chd HDD) -> Core 5 (0x20)
Set-DeviceIrqAffinity -MatchName "SATA AHCI"                  -Mask 0x20 -Label "Core 5: Aux Quarantine (SATA HDD)"

Write-Host "`nReboot Windows to apply PCIe/USB/SATA MSI interrupt affinity masks." -ForegroundColor Cyan
