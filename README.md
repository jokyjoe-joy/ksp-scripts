<p align="center" width="100%">
    <img width="33%" src="https://freepikpsd.com/wp-content/uploads/2019/10/kerbal-space-program-logo-png-1-Transparent-Images.png"> 
</p>

# KSP Scripts

You can find all my scripts used for playing KSP in this repository. While learning C++, I thought what would be a better way to learn it thoroughly than learning while doing something you love. Therefore, I set up a KSP C++ Client using kRPC and this repository is its outcome.

## Installation (for Visual Studio)
1. Clone this repository.
```bash
git clone https://github.com/jokyjoe-joy/ksp-scripts
```
2. Create a Visual Studio solution.
3. Set configuration to Release - x64
4. In the solution's properties, under "C/C++/General" set Additional Include Directories to the following:
```
[your kspscripts dir]\dependencies\asio\include
[your kspscripts dir]\dependencies\protobuf\include
[your kspscripts dir]\dependencies\krpc\include
```
5. Under "Linker/General" set Additional Library Directories to the following:
```
[your kspscripts dir]\dependencies\protobuf\lib
[your kspscripts dir]\dependencies\krpc\lib
```

6. Under "Linker/Input" set Additional Dependencies to the following:
```
krpc.lib
libprotobuf.lib
libprotobuf-lite.lib
libprotoc.lib
```
## Obsolete installation instructions
1. Set up a kRPC C++ client. For setting it up using Visual Studio watch [this video by Saucer Chrome](https://www.youtube.com/watch?v=XE8GB1vOLyI).
2. Clone this repository.
```bash
git clone https://github.com/jokyjoe-joy/ksp-scripts
```