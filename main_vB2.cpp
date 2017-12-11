//
//  main.cpp
//  smartMemPool
//
//  Created by Junzhe Zhang on 3/12/17.
//  Copyright © 2017 Junzhe Zhang. All rights reserved.
//

#include <iostream>
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

///// string delimiter
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
//    pair<int, int>tempP(-1,-1);
//    cout<<tempP.first<<' '<<tempP.second<<endl;
//    tempP.first =3;
//    cout<<tempP.first<<' '<<tempP.second<<endl;
    
    
    /// test a full loop.
    SmartMemPool pool("FF");

    // get the simulation input from text file
    //string fileName ="memInfo_alex_20itr_100size.text";
    //string fileName ="memInfo_vgg_20itr_100size.text";
    string fileName ="memInfo_resnet_20itr_100size.text";
    vector<string>vec = file_2_strVec(fileName);
    cout<<"example of one vec string"<<endl;
    cout<<vec[1]<<endl;
    cout<<vec[45]<<endl;
    cout<<"806 "<<vec[806]<<endl;
    cout<<"811 "<<vec[811]<<endl;
    cout<<"812 "<<vec[812]<<endl;
    cout<<"813 "<<vec[813]<<endl;
    int testNum = static_cast<int>(vec.size());
    //testNum =812;
    map<string,void*>mapFptr_Rptr; //map fake to real address, for simulation purpose.

    //for loop, execute Malloc and Free based on the simulation input.
    for (int i=0; i<testNum;i++){

        vector<string> v = split_main(vec[i], " ");

        cout<<"============"<<v[0]<<' '<<i<<"============"<<endl;
//        if (i==800){
//            pool.getMaxLoad("printVertices");
//        }
        if (v[0]=="Malloc"){
            //covert size from str to int.
            int result;
            stringstream convert(v[2]);
            if (!(convert>>result)){
                result =-1;
                cout<<"error for converting size from str to int 2."<<endl;
            }
            mapFptr_Rptr[v[1]]= pool.Malloc(result);
            //            if(i>805){
            //                cout<<"marker "<<i<<' '<<v[1]<<' '<<mapFptr_Rptr.find(v[1])->second<<endl;
            //            }
            //cout<<mapF_R.find(v[1])->first<<' '<<mapF_R.find(v[1])->second<<endl;
        }else{
            if (!(mapFptr_Rptr.find(v[1])==mapFptr_Rptr.end())){
                pool.Free(mapFptr_Rptr.find(v[1])->second);

                //                if(i>805){
                //                    cout<<"marker "<<i<<' '<<v[1]<<' '<<mapFptr_Rptr.find(v[1])->second<<endl;
                //                }
                mapFptr_Rptr.erase(v[1]);
            }else{
                cout<<"error, ptr to free not found."<<endl;
            }
        }

    }// end of for loop of test
    cout<<"=====================load================="<<endl;
    pool.getMaxLoad("cuda");
    pool.getMaxLoad("memPool");
    //pool.getMaxLoad("printVertices");
        //TODO(junzhe)  destruct it

    //print size of each block, no issue.
//        cout<<"=====================size================="<<endl;
//        for (int i=0; i<testNum;i++){
//            vector<string> v = split_main(vec[i], " ");
//            if(v[0]=="Malloc"){
//                int result;
//                stringstream convert(v[2]);
//                if (!(convert>>result)){
//                    result =-1;
//                    cout<<"error for converting size from str to int 2."<<endl;
//                }
//                cout<<i<<' '<<result/1000<<endl;}
//        }
    return 0;
}

