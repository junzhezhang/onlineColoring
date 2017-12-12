# onlineColoring
onlineColoring memory pool in C++

main_v5.cpp simulates how exactly singa will run online, using the singa log of 20-iteration Alex.<br />
Diferently, this file running on CPU uses malloc instead of cudaMalloc, arguments and returned variable is slightly different.<br />
Pls read by starting from major APIs Malloc(), Free(), run() and test(). <br />
test() is to obtain the repeated blocks and decide when to switch from cudaMalloc to "color-malloc".<br />
run() is the coloring algorithm after obtained the  repeated blocks.<br />
Pls use /// to navigate through major sections.<br />


# Summary of changes/improvements till 12/12
1. Changed all size related variables to size_t, enabling potential large memory usage book keeping.<br />
2. Redesigned lookup tables ensuring uniqueness of key and values. (v10)<br />
3. Added more rules in struct sorting, which guarantees uniform performance from time to time. (vB2)<br />
4. load of cudaMalloc and colorMalloc is being tracked from 0->2 iterations upon colorMalloc. Ratio = max(total mem Usage)/max(total mem load) are 1.001 (Alexnet), 1.051 (VGG) and 1.00482 (ResNet). <br />
5. Time complexity analyzed: run is O(n^2) once only, test is O(N^2) multiple times, where n is maxLen and N is gc. test is the bottleneck.<br />
6. Best Fit allocation implemented as well. BF or FF can be chosen via constructor. BF and FF gives the same results under all 3 networks. BF does not significantly add time complexity.<br />
7. In reducing number of test: (vB4)<br />
    Instead of fix interval of 100, now changed to incremental interval starting from 300. Able to reduce ResNet from 7s to 1.6s.<br />
    New algorithm able to cut maxLen in case it contains more than 1 iterations.<br />
    Alternative way, use "average velocity" idea to detect trend of the load, which can theoretically reduce to test up to 1 time only. In practice, however, it shows that there are some false alarm if noise is significant, such as AlexNet.<br />
8. Splitted into header, cpp and main.cpp for test, ready to adapt into Singa for further test.<br />
