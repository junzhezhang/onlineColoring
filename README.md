# onlineColoring
onlineColoring memory pool in C++

main_v5.cpp simulates how exactly singa will run online, using the singa log of 20-iteration Alex.<br />
Diferently, this file running on CPU uses malloc instead of cudaMalloc, arguments and returned variable is slightly different.<br />
Pls read by starting from major APIs Malloc(), Free(), run() and test(). <br />
test() is to obtain the repeated blocks and decide when to switch from cudaMalloc to "color-malloc".<br />
run() is the coloring algorithm after obtained the  repeated blocks.<br />
Pls use /// to navigate through major sections.<br />
