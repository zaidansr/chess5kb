param(
    [Parameter(Mandatory = $true)]
    [string]$Fasm,
    [string]$Output = (Join-Path $PSScriptRoot 'bin\chess5kb.exe')
)
$ErrorActionPreference = 'Stop'
$sourcePath = Join-Path $PSScriptRoot 'chess5kb.asm'
$fullPath = Join-Path ([IO.Path]::GetTempPath()) ('chess5kb-' + [Guid]::NewGuid().ToString('N') + '.exe')
& $Fasm $sourcePath $fullPath
if ($LASTEXITCODE -ne 0) { throw 'Assembly failed.' }

# FASM writes full 512-byte file alignment padding. The loader also accepts
# the exact initialized size for this final section. Keep the complete base
# relocation directory, then remove only its trailing alignment padding.
[byte[]]$imageBytes = [IO.File]::ReadAllBytes($fullPath)
Remove-Item -LiteralPath $fullPath
$peOffset = [BitConverter]::ToInt32($imageBytes, 60)
$sectionCount = [BitConverter]::ToUInt16($imageBytes, $peOffset + 6)
$optionalSize = [BitConverter]::ToUInt16($imageBytes, $peOffset + 20)
if ($sectionCount -ne 1 -or $optionalSize -ne 224) {
    throw 'Unexpected executable layout.'
}
$optionalOffset = $peOffset + 24
$sectionOffset = $optionalOffset + $optionalSize
$sectionRva = [BitConverter]::ToUInt32($imageBytes, $sectionOffset + 12)
$rawOffset = [BitConverter]::ToUInt32($imageBytes, $sectionOffset + 20)
$relocRva = [BitConverter]::ToUInt32($imageBytes, $optionalOffset + 136)
$relocSize = [BitConverter]::ToUInt32($imageBytes, $optionalOffset + 140)
if ($relocRva -eq 0 -or $relocSize -eq 0) {
    throw 'The executable must retain its relocation table.'
}
$fileSize = [int]($rawOffset + $relocRva + $relocSize - $sectionRva)
if ($fileSize -ge 5000) { throw "Executable is $fileSize bytes; limit is under 5,000." }
for ($index = $fileSize; $index -lt $imageBytes.Length; $index++) {
    if ($imageBytes[$index] -ne 0) { throw 'Refusing to discard initialized data.' }
}
[uint32]$rawSize = $fileSize - $rawOffset
[BitConverter]::GetBytes($rawSize).CopyTo($imageBytes, $sectionOffset + 16)
# Deterministic desktop builds; no image checksum is required for this app.
[Array]::Clear($imageBytes, $peOffset + 8, 4)
[Array]::Clear($imageBytes, $optionalOffset + 64, 4)
[byte[]]$resultBytes = New-Object byte[] $fileSize
[Array]::Copy($imageBytes, $resultBytes, $fileSize)
[IO.File]::WriteAllBytes($Output, $resultBytes)
[pscustomobject]@{
    Executable = [IO.Path]::GetFullPath($Output)
    Bytes = $fileSize
    Under5000Bytes = ($fileSize -lt 5000)
    RelocationBytes = $relocSize
    SHA256 = (Get-FileHash -LiteralPath $Output -Algorithm SHA256).Hash
}
