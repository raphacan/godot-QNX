# QNX platform port

This folder contains the adaptations to run godot on QNX.
It is based on :
- the SDP7.1 (porting to SDP8 will be done later and should not be a problem) 
- the Godot release 4.4.


## Documentation

### Compiling for QNX

#### Requirements
Compiling for QNX, the following is required:
- [Python 3.8+](https://www.python.org/downloads/macos/).
- [SCons 4.0+](https://scons.org/pages/download.html) build system.
- [QNX SDP 7.1+](https://https://blackberry.qnx.com/) QNX Software Development Platform

#### Setting up the build system
- Set the environment variables `QNX_HOST` and  `QNX_TARGET` to point to the appropriate locations in the QNX SDP.
    - The QNX SDP typically contains a `qnxsdp-env.sh` for setting this up. If available, run source on this script inside your QNX SDP installation:
    ```
    source qnxsdp-env.sh
    ```
#### Building the export templates
To build QNX export templates, run the build system with the following parameters:
- (32 bits)
```
scons platform=qnx target=template_release arch=arm32
scons platform=qnx target=template_debug arch=arm32
```
- (64 bis)
```
scons platform=qnx target=template_release arch=arm64
scons platform=qnx target=template_debug arch=arm64
```
