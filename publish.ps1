# 设置项目目录和安装目录
$projectDir = Get-Location
$buildDir = Join-Path $projectDir "build"
$installDir = Join-Path $projectDir "bin"

# 生成构建文件
# cmake -S . -B build
cmake -S $projectDir -B $buildDir -DCMAKE_INSTALL_PREFIX="$installDir"

# 构建项目
cmake --build $buildDir --config Release

# 安装项目
cmake --install $buildDir

Write-Host "Build and installation complete."
Write-Host "Files installed to: $installDir"
