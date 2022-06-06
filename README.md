# ray-tracing
A ray tracing project(s) based on Peter Shirley's Ray Tracing In One Weekend series

## Build
The first edition of the project can be found in the in-one-weekend directory.
To compile with g++:
'''
$ g++ -std=c++17 -o main main.cpp
'''

## Run
This project prints the rendered image in .ppm format to stdout. 
An example single-threaded run of this project would be:
'''
$ ./main > example.ppm
'''
where example.ppm would be the user specified name of the desired new output file.

## Multi-Threaded Run
This project supports multi-threading to increase execution speed. Multi-threading features
were added which allows the user to specify N threads to run the rendering process.
These threads are then each given a single line of pixels to render at a time, and thus 
N lines of the image are rendered at a time in batches until the entire photo is rendered.

To specify a multi-threaded run with N threads, the N argument can be passed as an optional commandline arg:
'''
$ ./main 4 > example.ppm
'''
This will generate a render run using 4 threads. 




