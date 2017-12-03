# onlineColoring
onlineColoring memory pool in C++

This file simulates how exactly singa will run online, using the singa log of 20-iteration Alex.\n
Diferently, this file running on CPU uses malloc instead of cudaMalloc, arguments and returned variable is slightly different.\n
Pls read from major APIs Malloc(), Free(), run() and test(). \n
test() is to obtain the repeated blocks and decide when to switch from cudaMalloc to "color-malloc".\n
run() is the coloring algorithm after obtained the  repeated blocks.\n
Pls use /// to navigate through major sections.\n
