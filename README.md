# Example of error handling

## Build
```shell
conan install . --output-folder=cmake-build-release/conan --build=missing -s build_type=Release`
conan install . --output-folder=cmake-build-debug/conan --build=missing -s build_type=Debug
```