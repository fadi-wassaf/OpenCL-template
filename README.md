# OpenCL-template
Template for an OpenCL program with the traditional vector addition example. Platform and device selection also included.

## Installation
Tested with MSYS2 on Windows. If using a similar setup, you can just install the following:
```bash
pacman -S mingw-w64-x86_64-opencl-headers mingw-w64-x86_64-opencl-clhpp mingw-w64-x86_64-opencl-icd 
```

You can build the program using CMake.
```bash
# Clone the repository
git clone https://github.com/fadi-wassaf/OpenCL-template
cd OpenCL-template

# Create and go to the build directory
mkdir build
cd build

# Run cmake with preferred settings (e.g. for MSYS2 add -G "MinGW Makefiles")
cmake ..

# Compile
make
```
## Running The Program
You can then run the program in its different modes while in the `build` directory. Examples with output:
```
> ./OpenCL-template.exe list_platforms
Available Platforms:
   [0] - AMD Accelerated Parallel Processing

> ./OpenCL-template.exe list_devices 0
Selected Platform: [0] - AMD Accelerated Parallel Processing
Available Devices:
   [0] - gfx1030
    | Device Type = 4
    | Device Version = OpenCL 2.0 AMD-APP (3444.0)
    | Max Work Group Size = 256

> ./OpenCL-template.exe compute 0 0
Selected Platform: [0] - AMD Accelerated Parallel Processing
Selected Device: [0] - gfx1030
Write vector 'a' contents to 'b_a': 0.00304 ms
Write vector 'b' contents to 'b_b': 0.0026 ms
Add elements ('b_a' + 'b_b'): 0.0022 ms
Read 'b_c' contents to vector 'c': 0.0026 ms
```
Note that if you want to move the executable somewhere else, you will need to move your `.cl` file. Some ways around this include hard coding the kernel source into the program or generating an include file with `xxd -i add.cl`.
