# Surface Reconstruction
This project is a 3D surface reconstruction application that reads contour data from a file and renders it using OpenGL. The camera can be controlled using the mouse.

## Prerequisites
Install the required libraries using the command below on Linux terminal.
```sh
sudo apt-get update
sudo apt-get install libglew-dev libglfw3-dev libglm-dev libglu1-mesa-dev libcgal-dev freeglut3-dev
```

## Instructions
- Build: `cmake ..` in the `build` directory
- Make: `make` to generate object files and compile them in a single execuatble
- Run: `./SurfaceReconstruction`