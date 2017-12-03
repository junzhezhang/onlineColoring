# onlineColoring
onlineColoring memory pool in C++

This file simulates how exactly singa will run online, using the singa log of 20-iteration Alex.
Diferently, this file running on CPU uses malloc instead of cudaMalloc, arguments and returned variable is slightly different.
Pls read from major APIs Malloc(), Free(), run() and test(). 
test() is to obtain the repeated blocks and decide when to switch from cudaMalloc to "color-malloc".
run() is the coloring algorithm after obtained the  repeated blocks.
Pls use /// to navigate through major sections.
