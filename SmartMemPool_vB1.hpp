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
    //for use of constructor TODO(junzhe)
    //r_idx(r),d_idx(d),size(s),offset(o),Occupied(oc)
};

///class mem-pool SmartMemPool
class SmartMemPool{
public:
    SmartMemPool(); //constructor
    //TODO(junzhe) change all size related int to size_t
    //TODO(junzhe) in Singa, void Malloc( void**, size_t); change to cudaMalloc and cudaFree.
    void* Malloc(size_t size);
    void Free(void* ptr);
    ~SmartMemPool();
    size_t getMaxLoad(string loadFlag);
private:
    int mallocFlag =0; //0 for cudaMalloc, 1 for coloringMalloc
    int gc =0; //global counter each time Malloc/Free, add 1.
    int globeCounter=-1;
    void* ptrPool = NULL;
    int idxRange = 0;
    size_t offset = 0;
    size_t maxload = 0;
    int maxLen =0;
    int location=0;
    vector<string> vec;
    map<int,int>Table_r2d; //full duration info, cross-iteration duration.
    map<int,int>Table_d2r;
    map<int,lookUpElement>Table_r2Ver;
    map<int, pair<size_t,size_t>>Table_load; //gc, <cudaLoad, colorLoad>
    map<void*,size_t>Table_p2s; //For tracking load. add when allocate, delete when deallocate.
    map<void*,int>Table_p2r; //ptr for arrival idx, for look up Table during free
};


#endif /* SmartMemPool_hpp */



