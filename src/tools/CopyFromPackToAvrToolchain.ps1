# this script collects the needed files and copies them to 
# their place in the AVR toolchain directory

# The path where the AVR toolchain is unpacked
$atpath = "D:\avr"
# The path the the Microchip Pack for the 
# Microchip AVR-Dx Series Device Support (2.1.152)
$mppath = "D:\mcpack"

# copy the device-specs
Get-ChildItem -Path $mppath"\gcc\dev\*" -Directory |
Foreach-Object {
	Write-Output "REM $_"
	Get-ChildItem $_ |
	Foreach-Object {
		$fullName = $_.FullName
		# device specs or lib/crt ?
		if ($_.Name -eq "device-specs")
		{
			Write-Output "copy $fullName $atpath\lib\gcc\avr\7.3.0\device-specs\"
		} else {
			Write-Output "copy $fullName\*.* $atpath\avr\lib\$_"
		}
	}
}

Write-Output "REM includes"
Write-Output "copy $mppath\include\avr\*.h $atpath\avr\include\avr\"

