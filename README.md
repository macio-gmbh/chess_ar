# chess_ar

Chess AR is an application to detect a chess board with its figures, get a suggested move from a chess API and present the suggested move to the user.
It needs a Webcam to capture the current chess board. It's currently designed that the camera needs to be above the chess board and look down at the chess board.

The application was initially created as part of a project work at the [Karlsruhe University of Applied Sciences](https://www.hs-karlsruhe.de/home/) in the 
summer semester 2017. Further information can be found [here](https://github.com/macio-gmbh/chess_ar/wiki) (currently in German only).

It consists of 4 different modules which are communicating with each other via the Message Broker Rabbit MQ:
* Controller: Manages the application, receives the current chess setting from the eye, sends and receives suggested moves to the chess engine
and forwards the information to the gui.
* Eye: Receives the camera image and takes care of the computer vision processing to detect the chess board with its figures.
* GUI: Prints the chess board and the next suggested move.
* ChessEngine: External server for the suggested move (we're using currently the stockfish-engine, running on the local system).


## Building the code

The code was tested on Linux (also Raspbian on Raspberry Pi 3) and Windows 10 (Using Cmake and MinGW-w64, untested with Visual Studio).
You also need to set some extra Paths for Windows (like BOOST_ROOT and OpenCV_DIR) in the Cmake cache file which should not necessary on Linux. 

### Requirements

* [Cmake 3.6 or better](https://cmake.org/)
* [Boost >1.50.0 ](http://www.boost.org/)
* [rabbitmq-c](https://github.com/alanxz/rabbitmq-c)
* [OpenCV >3.2](http://opencv.org/)

Also if you want to run the code (seperate modules communicate via a Message broker):
* [RabbitMQ Server](https://www.rabbitmq.com/download.html)

### How to build the code:

```
mkdir build && cd build
cmake ..
cmake --build .
make
```

## Usage
Every module needs to be started to be fully operational.


## Project structure
* `src` Contains all the src files with seperate CMake Modules.
* `train` Contains training data (images) for the eye module training.

