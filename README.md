# Colony Simulator

A simulator of colonizing the world

## How to run

- Download the latest release
- Unzip it
- Run ColonySimulator or ColonySimulator.exe depending on your system

## How to build

1. If on Linux, install raylib's dependencies following [this tutorial](https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux#dependencies)
1. You'll need a C/C++ compiler and CMake
1. Run 
```bash
git clone https://github.com/SemkiShow/ColonySimulator
cd ColonySimulator
git submodule update --init --recursive --depth 1 --jobs 8
chmod +x run.sh
./run.sh
```
