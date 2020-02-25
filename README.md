# IPP Debayer Project

This is a sample code for ipp debayer. We also use tbb to increase the process.

## Prebuild

1. Downalod IPP and TBB

3. Install IPP and TBB SDK
My Installed Path: 
    * Windows: 
        * ipp: C:\Program Files (x86)\IntelSWTools\compilers_and_libraries\windows\ipp
        * tbb: C:\Program Files (x86)\IntelSWTools\compilers_and_libraries\windows\tbb
    * Linux: 
        * ipp: /opt/intel/ipp
        * tbb: /opt/intel/tbb


## How to build 

1. Create a build folder
```
> mkdir build
```
2. Generate a project
```
> cd build
> cmake ..
```
3. Build
```
> cmake --build .
```

## Reference

> [ipp demosaic document](https://software.intel.com/en-us/ipp-dev-reference-demosaicahd)

> [ipp resize with tbb sample code](https://stackoverflow.com/questions/39874560/resizing-an-image-using-intels-integrated-performance-primitives)

> [find ipp cmake](https://github.com/hanjianwei/cmake-modules/blob/master/FindIPP.cmake)

> [find tbb cmake](https://github.com/justusc/FindTBB/blob/master/FindTBB.cmake)
