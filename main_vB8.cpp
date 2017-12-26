//
//  main.cpp
//  smartMemPool
//
//  Created by Junzhe Zhang on 3/12/17.
//  Copyright © 2017 Junzhe Zhang. All rights reserved.
//

#include <iostream>
#include <chrono>
#include "SmartMemPool.hpp"

vector<string> file_2_strVec(string fileName){
    /*
     convert from file.text to vector of strings
     */
    std::ifstream infile(fileName);
    if(!infile) {
        cout << "error: Cannot open input file.\n";
        //return 1;
    }
    char str[255];
    vector<string>vec;
    
    while(infile) {
        infile.getline(str, 255);  // delim defaults to '\n'
        vec.push_back(str);
    }
    vec.pop_back(); //pop_back the last empty line.
    infile.close();
    //cout<<"done all, size of vec: "<<vec.size()<<endl;
    return vec;
}//end of file_2_strVec function

// string delimiter
vector<string> split_main(string s, string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    string token;
    vector<string> res;
    while ((pos_end = s.find(delimiter, pos_start)) != string::npos) {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }
    res.push_back(s.substr(pos_start));
    return res;
}


///main() simulation of how exactly singa will run online using running info of 20 iterations.
int main() {
    
    int64_t tic, toc;
    /// test a full loop.
    SmartMemPool pool("FF");

    // get the simulation input from text file, select one for testing.
    //string fileName ="memInfo_alex_20itr_100size.text";
    //string fileName ="memInfo_vgg_20itr_100size.text";
    string fileName ="memInfo_resnet_20itr_100size.text";
    vector<string>vec = file_2_strVec(fileName);
    cout<<"size of vec: "<<vec.size()<<endl;
    cout<<"example of one vec string"<<endl;
    cout<<vec[1]<<endl;
    cout<<vec[45]<<endl;

    int testNum = static_cast<int>(vec.size());
    map<string,void*>mapFptr_Rptr; //map fake to real address, for simulation purpose.
    
    chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();
    toc=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    tic =toc;
    //for loop, execute Malloc and Free based on the simulation input.
    for (int i=0; i<testNum;i++){
        vector<string> v = split_main(vec[i], " ");
        //cout<<"============"<<v[0]<<' '<<i<<"============"<<endl;
//        if(i%100==0){
//            int64_t delta = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()-tic;
//            tic = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
//            //cout<<"i = "<<i<<' '<<delta<<endl;
//        }
        if (v[0]=="Malloc"){
            //covert size from str to int.
            int result;
            stringstream convert(v[2]);
            if (!(convert>>result)){
                result =-1;
                cout<<"error for converting size from str to int 2."<<endl;
            }
            void* ptr=nullptr;//verify if this is changed or not.
            //cout<<ptr<<endl;
            pool.Malloc(&ptr,result);
            //cout<<ptr<<endl;
            mapFptr_Rptr[v[1]]= ptr;
            //"hello hello how are you?";
        }else{
            if (!(mapFptr_Rptr.find(v[1])==mapFptr_Rptr.end())){
                pool.Free(mapFptr_Rptr.find(v[1])->second);
                mapFptr_Rptr.erase(v[1]);
            }else{
                cout<<"error, ptr to free not found."<<endl;
            }
        }
    }// end of for loop of test
    //cout<<"total time is "<<tic-toc<<endl;
    chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>( t2 - t1 ).count();
    cout<<"total time is "<<duration<<" (in micronseconds)"<<endl;
    return 0;
}

