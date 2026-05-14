# Check if argument is provided (新增machineType和customerName参数，总参数数=3)
if ($args.Count -ne 3) {
    Write-Host "Usage: resource_package.ps1 <zip_file> <machineType> <customerName>"
    exit 1
}

# Get the parameters (顺序：zip文件、设备型号、客户名称)
$zipFile = $args[0]
$machineType = $args[1]
$customerName = $args[2]

# Check if file exists
if (!(Test-Path $zipFile)) {
    Write-Host "File not found!"
    exit 1
}

# Calculate MD5 hash of the zip file
$md5 = (Get-FileHash $zipFile -Algorithm MD5).Hash.ToLower()

# Get the filename
$filename = Split-Path $zipFile -Leaf

Write-Host "Filename: $filename"
Write-Host "MD5 Value: $md5"

# Extract version from filename (e.g. V1.0.6)
$versionMatch = [regex]::Match($filename, 'V\d+\.\d+(\.\d+)?')
$version = if ($versionMatch.Success) { $versionMatch.Value } else { "" }

Write-Host "Version: $version"

# Read the original resource.json content to preserve formatting including line endings and encoding
$originalContent = [System.IO.File]::ReadAllText("resource.json", [System.Text.Encoding]::UTF8)

# Update the JSON properties using regex to preserve original formatting
$updatedContent = $originalContent -replace '(?m)^(\s*"fileName":\s*")[^"]*(".*)$', "`${1}$filename`${2}"
$updatedContent = $updatedContent -replace '(?m)^(\s*"md5":\s*")[^"]*(".*)$', "`${1}$md5`${2}"
if ($version) {
    $updatedContent = $updatedContent -replace '(?m)^(\s*"version":\s*")[^"]*(".*)$', "`${1}$version`${2}"
}
# 新增：更新machineType字段
$updatedContent = $updatedContent -replace '(?m)^(\s*"machineType":\s*")[^"]*(".*)$', "`${1}$machineType`${2}"
# 新增：更新customerName字段
$updatedContent = $updatedContent -replace '(?m)^(\s*"customerName":\s*")[^"]*(".*)$', "`${1}$customerName`${2}"

# Write the updated content back to the file, preserving original UTF-8 encoding without BOM
$utf8WithoutBom = New-Object System.Text.UTF8Encoding($false)
[System.IO.File]::WriteAllText("resource.json", $updatedContent, $utf8WithoutBom)

# Read the updated JSON to extract values for the zip naming
$json = ConvertFrom-Json $updatedContent
$packageType = $json.packageType
# 从参数获取（替代原有从JSON读取），确保使用最新传入值
$machineType = $json.machineType
$customerName = $json.customerName

# Set MQ as fixed value since moduleType field is not in JSON anymore
$mqType = "MQ"

# Build new zip file name
$newZipName = "${packageType}_${machineType}-${mqType}_${customerName}_${version}.zip"
Write-Host "New package name: $newZipName"

# Create new zip file containing resource.json and the source zip file
Compress-Archive -Path @("resource.json", $zipFile) -DestinationPath $newZipName -Force

Write-Host "MD5 hash, version, machineType, customerName and file information updated successfully in resource.json"
Write-Host "New package created: $newZipName"