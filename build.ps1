$hex_string = Get-Content -Path 'hrmc.dmp'
$hex_string = $hex_string -replace '#.*', '' # remove comments
$hex_string = $hex_string -join ''            # join lines into one string
$hex_string = $hex_string -replace '\s+', ''  # remove whitespace

$hex_string_spaced = $hex_string -replace '..', '0x$& '
$byte_array = [byte[]] -split $hex_string_spaced
Set-Content -Path 'hrmc.exe' -Value $byte_array -Encoding Byte

