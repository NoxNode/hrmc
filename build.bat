powershell -Command "(New-Object Net.WebClient).DownloadFile('https://raw.githubusercontent.com/NoxNode/hrmc/refs/heads/main/hrmc.dmp', './hrmc.dmp')"
powershell -Command "& { $hex_string = Get-Content -Path 'hrmc.dmp' ; $hex_string = $hex_string -replace '#.*', '' ; $hex_string = $hex_string -join '' ; $hex_string = $hex_string -replace '\s+', ''  ; $hex_string_spaced = $hex_string -replace '..', '0x$& ' ; $byte_array = [byte[]] -split $hex_string_spaced ; Set-Content -Path 'hrmc.exe' -Value $byte_array -Encoding Byte } "

