# Cpp_video_collect-MacOS
use cpp and opencv to capture video
## Prerequisite
```bash
brew install opencv
brew install cmake
```
Then add the path of opencv library `eg:"/opt/homebrew/Cellar/opencv/4.9.0_7/include/opencv4"` to the incluePath in file `./vscode/c_cpp_properties.json`
## Usage
```bash
cmake -D CMAKE_OSX_DEPLOYMENT_TARGET=14.0 ./build 
cd build
make
```
!test your camera id
```bash
./video <camera_id> <fps_set> <duration_time(seconds)>
```
The exec file will create a folder named as the start collect time. And put jpg images with a name of unix timestamp under the folder.
Up to now test 1080p 30HZ 180s, no problem.
If want to collect more time, may change the N defined in `./capture_video.cpp`     