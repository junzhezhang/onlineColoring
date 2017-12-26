

#ifndef SmartMemPool_hpp
#define SmartMemPool_hpp

#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <stdlib.h>     /* malloc, free, rand */
#include <map>
using namespace std;

struct lookUpElement{
    /*
     for memory pool Malloc look-up table.
     */
    int r_idx;
    int d_idx;
    size_t size;
    size_t offset;
    void* ptr;
    int Occupied; //0 is free, 1 is occupied.
    int crossItr; //c.
    int Occupied_backup; //c.
    int last_Occupied; //c. 1 means primary allocated, 2 means secondary.
};

///class mem-pool SmartMemPool
class SmartMemPool{
public:
    SmartMemPool(string meth); //constructor
    //TODO(junzhe) in Singa, void Malloc( void**, size_t); change to cudaMalloc and cudaFree.
    void Malloc(void** ptr, size_t size);
    void Free(void* ptr);
    ~SmartMemPool();
    void getMaxLoad(void);
    void Test(void);
private:
    string colorMethod;
    int mallocFlag =0; //0 for cudaMalloc, 1 for coloringMalloc
    int gc =0; //global counter each time Malloc/Free, add 1.
    int globeCounter=-1;
    int loadLogFlag =1; //record when its 1.
    void* ptrPool_normal = NULL;
    void* ptrPool_cross =NULL;
    int idxRange = 0;
    size_t offset_normal = 0;
    size_t offset_cross = 0; //c. cross iteration offset.
    int maxLen =0;
    int location=0;
    vector<string> vec;
    map<int,int>Table_r2d; //full duration info, cross-iteration duration.
    map<int,int>Table_d2r;
    vector<pair<int,lookUpElement>>Vec_r2Ver; //b.
    map<int, pair<size_t,size_t>>Table_load; //gc, <cudaLoad, colorLoad>
    map<void*,size_t>Table_p2s; //For tracking load in Free. add when allocate, delete when deallocate.
    map<void*,int>Table_p2r; //ptr for arrival idx, for look up Table during free
    int checkPoint=300; //for reduce number of test.
    int first_location =1 ; // this flag means only the first location for repPatternDetector is reliable.
    int gc_start_count=0; //
    int old_location=0;
    int old_maxLen =0;
};


#endif /* SmartMemPool_hpp */



