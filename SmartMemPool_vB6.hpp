//
//  SmartMemPool.hpp
//  smartMemPool
//
//  Created by Junzhe Zhang on 3/12/17.
//  Copyright © 2017 Junzhe Zhang. All rights reserved.
//

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
private:
    string colorMethod;
    int mallocFlag =0; //0 for cudaMalloc, 1 for coloringMalloc
    int gc =0; //global counter each time Malloc/Free, add 1.
    int globeCounter=-1;
    int loadLogFlag =1; //record when its 1.
    void* ptrPool = NULL;
    int idxRange = 0;
    size_t offset = 0;
    int maxLen =0;
    int location=0;
    vector<string> vec;
    map<int,int>Table_r2d; //full duration info, cross-iteration duration.
    map<int,int>Table_d2r;
    map<int,lookUpElement>Table_r2Ver;
    vector<pair<int,lookUpElement>>Vec_r2Ver; //b.
    map<int, pair<size_t,size_t>>Table_load; //gc, <cudaLoad, colorLoad>
    map<void*,size_t>Table_p2s; //For tracking load in Free. add when allocate, delete when deallocate.
    map<void*,int>Table_p2r; //ptr for arrival idx, for look up Table during free
    int checkPoint=300; //for reduce number of test.
};


#endif /* SmartMemPool_hpp */



