# store the current location
$originalLocation = Get-Location

try {
    # change the location to the OpenOCD scripts directory
    Set-Location -Path "C:\Program Files\Raspberry Pi\Pico SDK v1.5.0\openocd\scripts"

    # run OpenOCD
    ../openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000"
}
finally {
    # return to the original location
    Set-Location -Path $originalLocation
}
