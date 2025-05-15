# GestureDetection
A controller to detect gestures on motion capute, tested on Motion Capture Laboratory at Intituto Tecnologico de Buenos Aires

# Installation
To compile in Windows follow these instructions:
```
cd build
cmake --build . --config Release
```

# Running
To run on windows (from the root directory of the repository)
```
.\build\Release\GestureCaptureController.exe
```

# Clarifications
Please make sure to setup OptiTrack information correctly as follows:
1. OptiTrack server IP: local interface IP
2. OptiTrack local IP: local interface IP

In motive please ensure you have "Multicast" set.

Example:
