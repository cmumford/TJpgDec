
Set-Variable -Name "build_dir" -Value "build"

Remove-Item -r -fo $build_dir

mkdir $build_dir
Push-Location $build_dir

cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .

Pop-Location