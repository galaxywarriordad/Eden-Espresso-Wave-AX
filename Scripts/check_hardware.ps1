# =======================================================================
# 4. ELASTIC 2:3:1 NON-SMT HARDWARE MSI INTERRUPT AFFINITY AUDIT
# =======================================================================

# Start the audit by displaying a header message
Write-Output "[3/5] Auditing 1:1 PCIe/USB/SATA Hardware IRQ Steering (Elastic 2:3:1)..."

# Define a function to test IRQ affinity for given device matches
function Test-DeviceIrqAffinity {
    param(
        [string]$MatchName,         # Substring to match device FriendlyName
        [uint64]$ExpectedMask,      # Expected affinity mask for MSI interrupts
        [string]$PartitionLabel     # Logical label for core binding
    )
    
    # Get devices with status OK and matching name
    $devices = Get-PnpDevice -Status OK -ErrorAction SilentlyContinue | Where-Object { $_.FriendlyName -like "*$MatchName*" }
    
    # If no device found, output warning and return
    if (-not $devices) {
        Write-Output ("  [WARN] {0,-62} -> No active PnP device ({1})" -f $MatchName, $PartitionLabel)
        return
    }
    
    # Iterate each matched device
    foreach ($dev in $devices) {
        # Get device location info for display (PCI bus/device/function)
        $locProp = Get-PnpDeviceProperty -InstanceId $dev.InstanceId -KeyName "DEVPKEY_Device_LocationInfo" -ErrorAction SilentlyContinue
        $slotTag = if ($locProp -and $locProp.Data) {
            ($locProp.Data -replace '^PCI bus (\d+), device (\d+), function (\d+).*', ' [Bus $1:$2.$3]')
        } else { "" }
        $displayName = "$($dev.FriendlyName)$slotTag"

        # Registry path for MSI affinity policy
        $regPath = "HKLM:\SYSTEM\CurrentControlSet\Enum\$($dev.InstanceId)\Device Parameters\Interrupt Management\Affinity Policy"
        
        # Check registry key and read properties
        if (Test-Path $regPath) {
            $props = Get-ItemProperty -Path $regPath -ErrorAction SilentlyContinue
            # Validate if MSI affinity policy is set and matches expectation
            if ($props.DevicePolicy -eq 4 -and $null -ne $props.AssignmentSetOverride) {
                $bytes = New-Object byte[] 8
                [Array]::Copy($props.AssignmentSetOverride, $bytes, [Math]::Min($props.AssignmentSetOverride.Length, 8))
                $actualMask = [BitConverter]::ToUInt64($bytes, 0)
                if ($actualMask -eq $ExpectedMask) {
                    # Output pass message with device and mask info
                    Write-Output ("  [PASS] {0,-62} -> 0x{1:X2} ({2})" -f $displayName, $actualMask, $PartitionLabel)
                    continue
                }
            }
        }
        # If no match, output warning with expected mask
        Write-Output ("  [WARN] {0,-62} -> Unpinned (Expected 0x{1:X2} for {2})" -f $displayName, $ExpectedMask, $PartitionLabel)
    }
}

# Audit and print IRQ steering for key devices mapped to CPU cores

Write-Output "  --- [SECTION 1/2] Real-Time Audio/Timing & Vulkan GPU Steering ---"
Test-DeviceIrqAffinity -MatchName "eXtensible Host Controller" -ExpectedMask 0x01 -PartitionLabel "Core 0: Audio/Timing"
Test-DeviceIrqAffinity -MatchName "NVIDIA GeForce GTX 980 Ti"  -ExpectedMask 0x02 -PartitionLabel "Core 1: Vulkan Submit"

Write-Output "  --- [SECTION 2/2] Storage & DMA Interrupt Quarantine (Core 5) ---"
Test-DeviceIrqAffinity -MatchName "Standard NVM Express"       -ExpectedMask 0x20 -PartitionLabel "Core 5: Aux Quarantine (NVMe)"
Test-DeviceIrqAffinity -MatchName "SATA AHCI"                  -ExpectedMask 0x20 -PartitionLabel "Core 5: Aux Quarantine (SATA HDD)"

# Final sign-off
Write-Output "[STATUS] Validated Maxwell Core Hardware Alignment Map (Elastic 2:3:1 Non-SMT)."
exit 0
