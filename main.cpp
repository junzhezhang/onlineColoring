#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>

using namespace std;

vector<pair<int, int>> colorOccupied;
//TODO(junzhe) to replace vertex with onePairMsg
class Vertex {
public: //TODO(junzhe) can change some to private type
    int name; //TODO(junzhe) confirm the type //same name as pair msg, int
    int size;
    int r; //arrive
    int d; //depart
    // old plan list of list
    //std::list<std::list<int>>colorRange;
    //std::list<std::list<int>>colorOccupied;
    // new plan, vetor of pairs
    Vertex(int,int,int,int);
    pair<int, int> colorRange;
    vector<pair<int, int>> colorOccupied;
    //colorFlag no use as of now TODO(junzhe)
};
Vertex::Vertex(int varName, int s, int startIdx, int endIdx){
    name =varName;
    size = s;
    r = startIdx;
    d = endIdx;
}

///function mergeSeg, merge colorOccupied, which is a vector of pairs.  version 11.29 3pm
vector<pair<int, int>>  mergeSeg(vector<pair<int, int>> colorOccupied){
    sort(colorOccupied.begin(), colorOccupied.end());
    
//    cout<<"print before mergeSeg, len of "<<colorOccupied.size()<<endl;
//    for (int i=0;i<colorOccupied.size();i++){
//        cout<<"("<<colorOccupied[i].first<<","<<colorOccupied[i].second<<")";
//    }
//    cout<<endl;
    
    if(colorOccupied.size()<=1){
        //cout<<"no need to merge"<<endl;
        return colorOccupied;
    }
    
    int m = 0;
    while (m<(colorOccupied.size()-2)){
        //cout<<"in the while loop, m is "<<m<<endl;
        if (colorOccupied[m].second - colorOccupied[m+1].first >=-1){
            //cout<<"got intersection"<<endl;
            //cout<<colorOccupied[m].second<<' '<<colorOccupied[m+1].first<<endl;
            pair<int,int>tempItem(colorOccupied[m].first,max(colorOccupied[m].second,colorOccupied[m+1].second));
            //cout<<tempItem.first<<' '<<tempItem.second<<endl;
            //remove m+1 and m
            colorOccupied.erase(colorOccupied.begin()+m+1);
            colorOccupied.erase(colorOccupied.begin()+m);
            //insert the combined one
            colorOccupied.insert(colorOccupied.begin()+m,tempItem);
            //cout<<"print colorOccupied after this iterartion"<<endl;
            //for(auto item : colorOccupied) {
              //  cout << item.first<<' '<<item.second << endl;
                //}
        }else{
            m+=1;
        }
    }
    if (colorOccupied[m].second - colorOccupied[m+1].first >=-1){
        pair<int, int>tempItem(colorOccupied[m].first,max(colorOccupied[m+1].second,colorOccupied[m+1].second));
        //remove m+1 and m
        colorOccupied.erase(colorOccupied.begin()+m+1);
        colorOccupied.erase(colorOccupied.begin()+m);
        //insert the combined one
        colorOccupied.insert(colorOccupied.begin()+m,tempItem);
    }
    
//    cout<<"print after mergeSeg, len of "<<colorOccupied.size()<<endl;
//    for (int i=0;i<colorOccupied.size();i++){
//        cout<<"("<<colorOccupied[i].first<<","<<colorOccupied[i].second<<")";
//    }
//    cout<<endl;
    
    return colorOccupied;
}

///function FFallocation version 11.29 4pm
pair<int,int> FFallocation(vector<pair<int,int>> colorMerged,int size){
    /*
     First Feed weighted coloring
     return a pair stands for colorRange, rather than the first location.
     */
    // condition A: if no occupied
    if (colorMerged.size()==0){
        return pair<int,int>(0,size-1);
    }
    // condtion B: able to fit before first block
    if (size<(colorMerged[0].first+1)){
        return pair<int,int>(0,size-1);
    }
    int location= -1;
    if (colorMerged.size()>1) {
        //int location = -1;
        int n = 0;
        while (n<(colorMerged.size()-1)){
            // condition C: able to fit in between middle blocks.
            if ((colorMerged[n+1].first-colorMerged[n].second-1)>=size){
                location = colorMerged[n].second+1;
                break;
            }
            n+=1;
        }
        // condition D: allocate after the last block.
        if (location == -1){
            location = colorMerged[colorMerged.size()-1].second+1;
        }
    }
    // condition E: colorMeger len =1, allocate after the last block.
    if (colorMerged.size()==1){
        location = colorMerged[0].second+1;
    }
    return pair<int,int>(location,location+size-1);
    //TODO(junzhe) conditions verified, but can put assert function for other unknown conditions.
    //TODO(junzhe) to further verify the value of location and its scope.
}

///below shows in memInfo_2_sortedDict
///onePieceMsg [ptr, size, flag 1/-1, idx] version 11.30 3pm
struct onePieceMsg{
    // [ptr, size, flag 1/-1, idx]
    string ptr;
    int size;
    int MallocFree;
    int idx;
};

/// sort by ptr and then idx.
//https://stackoverflow.com/questions/1380463/sorting-a-vector-of-custom-objects
//https://stackoverflow.com/questions/3574680/sort-based-on-multiple-things-in-c
struct less_than_ptrIdx{
    inline bool operator() (const onePieceMsg& struct1, const onePieceMsg& struct2)
    {
        return ((struct1.ptr<struct2.ptr)||((struct1.ptr==struct2.ptr)&&(struct1.idx<struct2.idx)));
    }
};

struct oneIterMsg{
    // [idx, flag 1/-1, size or duration]
    //string ptr;
    int size_delta;
    int MallocFree;
    int idx;
};

struct less_than_iterIdx{
    inline bool operator() (const oneIterMsg& struct1, const oneIterMsg& struct2)
    {
        return (struct1.idx<struct2.idx);
    }
};


struct onePairMsg{
    // [start idx as name, size, start idx, free idx]
    int name;
    int size;
    int r_idx;
    int d_idx;
    char condition; //for debug purpose TODO(junzhe)
    //pair<int, int> colorRange;  //TODO(junzhe) to figure out why adding this 2 line the result is changed. below 3 line test.
    //vector<pair<int, int>> colorOccupied;
//    cout<<"before "<<pairMsgVec_[0].name<<endl;
//    pairMsgVec_[0].name=10;
//    cout<<"after "<<pairMsgVec_[0].name<<endl;
};
///sort by size, descending size
struct less_than_size{
    inline bool operator() (const onePairMsg& struct1, const onePairMsg& struct2)
    {
        return (struct1.size>struct2.size);
    }
};

/// for string delimiter
vector<string> split(string s, string delimiter) {
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
//"cudaMalloc_memInfo_alexnet_1000_mid2.text"
///below shows in memInfo_2_sortedDict
vector<onePairMsg> memInfo_2_sortedPair(string fileName){
    std::ifstream infile(fileName);
    
    if(!infile) {
        cout << "Cannot open input file.\n";
        //return 1;
    }
    char str[255];
    vector<string>vec;
    vector<onePieceMsg>onePieceMsgVec_;
    
    while(infile) {
        infile.getline(str, 255);  // delim defaults to '\n'
        //if(infile) cout << str << endl;
        vec.push_back(str);
        //cout<<"done with one iteration"<<endl;
    }
    //cout<<"done all, size of vec: "<<vec.size()<<endl;
    vec.pop_back();
    infile.close();
    //cout<<"done all, size of vec: "<<vec.size()<<endl;
    ///below to spilit each vector. TODO(junzhe)
    //cout<<"below printed splits"<<endl;
    for (int i=0;i<vec.size();i++) {
        vector<string> v = split(vec[i], " ");
        if (v[0]=="Malloc"){
            onePieceMsg tempMsg;
            tempMsg.ptr =v[1];
            //change v[2] from str to int
            int result;
            stringstream convert(v[2]);
            if (!(convert>>result)){
                result =-1;
                cout<<"error for converting size from str to int."<<endl;
            }
            tempMsg.size =result;
            tempMsg.MallocFree = 1;
            tempMsg.idx = i;
            onePieceMsgVec_.push_back(tempMsg);
        }else if (v[0]=="Free"){
            onePieceMsg tempMsg;
            tempMsg.ptr =v[1];
            tempMsg.size = -1;
            tempMsg.MallocFree = -1;
            tempMsg.idx = i;
            onePieceMsgVec_.push_back(tempMsg);
        }else {
            cout<<"error for process the onePriceMsg."<<endl;
        }
    }
    //cout<<"now print onePieceMsgVec_"<<endl;
    // sort based on ptr and ind:
    sort(onePieceMsgVec_.begin(),onePieceMsgVec_.end(),less_than_ptrIdx());
    //below prints onePieceMsgVec_
//        for (int i=0;i<onePieceMsgVec_.size();i++) {
//            cout<<onePieceMsgVec_[i].ptr<<' '<<onePieceMsgVec_[i].size<<' '<<onePieceMsgVec_[i].MallocFree<<' '<<onePieceMsgVec_[i].idx<<' '<<endl;
//        }
    
    // this section construct the onePairMsgVec_
    int maxIdx = static_cast<int>(onePieceMsgVec_.size())+2;
    //TODO(junzhe) optional to show percentage/size of the 3 group blocks.
    vector<onePairMsg>onePairMsgVec_;
    int i=0;
    int checker =0; //just to verify if goes in any of the condition; TODO(junzhe) refine
    //cout<<"print value of i before while loop "<<i<<endl;
    //while loop processes a pair at each time, if got a pair.
    while (i<(onePieceMsgVec_.size()-1)){
        //condition A: no matter if i=0, start with free. do nothing.
        if (onePieceMsgVec_[i].MallocFree==-1){
            i+=1; checker = 1;
        }
        //condition B: start with Malloc, next item same ptr and is free.
        if ((onePieceMsgVec_[i].MallocFree==1)&& (onePieceMsgVec_[i+1].MallocFree==-1)&&((onePieceMsgVec_[i].ptr==onePieceMsgVec_[i+1].ptr))){
            onePairMsg tempPair;
            tempPair.name = onePieceMsgVec_[i].idx;
            tempPair.size = onePieceMsgVec_[i].size;
            tempPair.r_idx = onePieceMsgVec_[i].idx;
            tempPair.d_idx = onePieceMsgVec_[i+1].idx;
            tempPair.condition='B';
            onePairMsgVec_.push_back(tempPair);
            i+=2; checker = 1;
        }
        // condition C: start with Malloc, no free.
        if ((onePieceMsgVec_[i].MallocFree==1)&&(onePieceMsgVec_[i].ptr!=onePieceMsgVec_[i+1].ptr)){
            onePairMsg tempPair;
            tempPair.name = onePieceMsgVec_[i].idx;
            tempPair.size = onePieceMsgVec_[i].size;
            tempPair.r_idx = onePieceMsgVec_[i].idx;
            tempPair.d_idx = maxIdx;
            tempPair.condition='C';
            onePairMsgVec_.push_back(tempPair);
            i+=1; checker = 1;
        }
    }//TODO(junzhe) verify if this is end of while
    //cout<<"print value of i after while loop "<<i<<endl;
    //condition D: if still left with the last item
    if ((i<onePieceMsgVec_.size())&&(onePieceMsgVec_[i+1].MallocFree==1)){
        onePairMsg tempPair;
        tempPair.name = onePieceMsgVec_[i].idx;
        tempPair.size = onePieceMsgVec_[i].size;
        tempPair.r_idx = onePieceMsgVec_[i].idx;
        tempPair.d_idx = maxIdx;
        tempPair.condition='D';
        onePairMsgVec_.push_back(tempPair);
        i+=1; checker = 1;
    }
    if (checker==0){
        cout<<"error with the onePairMsg processing"<<endl;
    }
    //this section is to sort
    sort(onePairMsgVec_.begin(),onePairMsgVec_.end(),less_than_size());
    //below prints sorted onePairMsgVec_
//    for (int i=0;i<onePairMsgVec_.size();i++) {
//        cout<<onePairMsgVec_[i].name<<' '<<onePairMsgVec_[i].size<<' '<<onePairMsgVec_[i].r_idx<<' '<<onePairMsgVec_[i].d_idx<<' '<<onePairMsgVec_[i].condition<<endl;
//    }
    //cout<<"size of onePriceMsgVec_ "<<onePieceMsgVec_.size()<<endl;
    //cout<<"end of test so far"<<endl;
    return onePairMsgVec_;
}

vector<Vertex> run(vector<onePairMsg> pairMsgVec_){
    //cout<<"done with function memInfo_2_sortedPair."<<endl;
    int m = static_cast<int>(pairMsgVec_.size());
    /// below is the run function
    //init all vertices
    vector<Vertex>vertices;
    for (int i=0; i<m;i++){
        Vertex tempVertex(pairMsgVec_[i].name,pairMsgVec_[i].size,pairMsgVec_[i].r_idx,pairMsgVec_[i].d_idx);
        vertices.push_back(tempVertex);
        //cout<<vertices[i].name<<' '<<vertices[i].size<<' '<<vertices[i].r<<' '<<vertices[i].d<<endl;
    }
    cout<<"test size of pair Msg and vertices: "<<m<<' '<<vertices.size()<<endl; //TODO(junzhe) double confirm if any case it is 95 instead of 94.
    // build edges with values 1 and 0; combine with colorOccupied and coloring at the same step.
    int **adj; //refer to http://www.sanfoundry.com/cpp-program-implement-adjacency-matrix/ TODO(junzhe) firgue out what.
    adj = new int*[m];
    for (int i=0; i<m;i++){
        //cout<<"===============no doing item "<<i<<" ================"<<endl;
        
        adj[i] = new int[m];
        for (int j=0; j<m;j++){
            if ((max(vertices[i].r,vertices[j].r))<(min(vertices[i].d,vertices[j].d))){
                adj[i][j]=1;
                if (vertices[j].colorRange.second){ //as second never be 0, if not empty.
                    // TODO(junzhe) how to implement below function
                    //cout<<"get into colorRange"<<endl; //TODO(junzhe) verify here, print comment
                    vertices[i].colorOccupied.push_back(vertices[j].colorRange);
                }
            }
            else { adj[i][j]=0; }
        }
        vector<pair<int, int>>colorMerged = mergeSeg(vertices[i].colorOccupied);
        vertices[i].colorRange = FFallocation(colorMerged,vertices[i].size);
//        cout<<"colored size of "<<vertices[i].size<<" at <"<<vertices[i].colorRange.first<<"," <<vertices[i].colorRange.second<<">"<<endl;
    }
    //cout<<vertices[1].colorRange.second<<endl;
    //if (vertices[1].colorRange.second+10){ cout<<"test size of emtpy pair "<<endl;}
    //cout<<adj[0][0]<<adj[0][1]<<adj[0][2]<<adj[0][3]<<adj[0][4]<<endl; //verifyed 5 results.
    return vertices;
}


int main() {
//    colorOccupied.push_back( pair<int, int>(2,4));
//    colorOccupied.push_back( pair<int, int>(2,4));
//    colorOccupied.push_back( pair<int, int>(5,11));
//    colorOccupied.push_back( pair<int, int>(7,10));
//    colorOccupied.push_back( pair<int, int>(13,13));
//    //colorOccupied.push_back( pair<int, int>(6,8));
//    //colorOccupied.push_back( pair<int, int>(9,10));
//    for(auto item : colorOccupied) {
//        cout << item.first<<' '<<item.second << endl;
//    }
//    vector<pair<int,int>>temp=mergeSeg(colorOccupied);
//    cout<<"============above is for testing mergeSeg==============="<<endl;
//    cout<<"below is merged "<<endl;
//    for(auto item : temp) {
//        cout << item.first<<' '<<item.second << endl;
//    }
//    pair<int,int>temp2 = FFallocation(temp, 1);
//    pair<int,int>temp3 = FFallocation(temp, 2);
//    pair<int,int>temp4 = FFallocation(temp, 3);
//    cout<<"below are FFallocation for size =1, 2, and 3: "<<endl;
//    cout<<temp2.first<<' '<<temp2.second<<endl;
//    cout<<temp3.first<<' '<<temp3.second<<endl;
//    cout<<temp4.first<<' '<<temp4.second<<endl;
    
    string fileName ="cudaMalloc_memInfo_alexnet_1000_mid2.text";
    vector<onePairMsg>pairMsgVec_ = memInfo_2_sortedPair(fileName);
    //cout<<"done with function memInfo_2_sortedPair."<<endl;
    vector<Vertex>vertices = run(pairMsgVec_);
    //cout<<vertices[10].size<<' '<<vertices[10].colorRange.second-vertices[10].colorRange.first+1<<endl;
    
    ///below is doing maxload given vertices
    // time complexity O(n^2), to be optimzied.
    //TODO(junzhe) taking max
    int maxTime =188;
    vector<int>load(maxTime,0);
//    int load[maxTime];
//    memset(load,0,sizeof(load));
    for (int i=0; i<maxTime;i++){
        for (int j=0; j<vertices.size();j++){
            if ((i>=vertices[j].r) && (i<=vertices[j].d)){
                load[i]+=vertices[j].size;
            }
        }
    }
    //cout<<load[0]<<' '<<load[1]<<' '<<load[2]<<' '<<load[3]<<' '<<load[4]<<endl;
    //print load
//    cout<<vertices.size()<<' '<<load.size()<<endl;
//    cout<<"======================print load========================="<<endl;
//    for(int i=0; i<load.size();i++)
//        cout<<load[i]<<endl;

    int maxload =*max_element(load.begin(),load.end());
    cout<<"maxload is "<<maxload<<endl;
    
    //now maxMemPrint
    int maxMemPrint=0;
    for (int j=0; j<vertices.size();j++){
        if (maxMemPrint<=(vertices[j].colorRange.second+1)){
            maxMemPrint=vertices[j].colorRange.second+1;
        }
    }
    cout<<"maxMemPrint is "<<maxMemPrint<<endl;
    cout<<"====================== below oneIterMsg ========================="<<endl;
    string fileName2 ="memInfo_vgg_20itr_100size.text";
    std::ifstream infile(fileName2);
    
    if(!infile) {
        cout << "Cannot open input file.\n";
        //return 1;
    }
    char str[255];
    vector<string>vec;
    vector<onePieceMsg>onePieceMsgVec_;
    
    while(infile) {
        infile.getline(str, 255);  // delim defaults to '\n'
        //if(infile) cout << str << endl;
        vec.push_back(str);
        //cout<<"done with one iteration"<<endl;
    }
    //cout<<"done all, size of vec: "<<vec.size()<<endl;
    vec.pop_back();
    infile.close();
    cout<<"done all, size of vec: "<<vec.size()<<endl;
    ///below to spilit each vector. TODO(junzhe)
    //cout<<"below printed splits"<<endl;
    //TODO(junzhe) below decide how long to test. Important!
    int strVecSize = static_cast<int>(vec.size());
    //int strVecSize =1000;
    cout<<"strVecSize is: "<<strVecSize<<endl;
    for (int i=0;i<strVecSize;i++) {
        vector<string> v = split(vec[i], " ");
        if (v[0]=="Malloc"){
            onePieceMsg tempMsg;
            tempMsg.ptr =v[1];
            //change v[2] from str to int
            int result;
            stringstream convert(v[2]);
            if (!(convert>>result)){
                result =-1;
                cout<<"error for converting size from str to int."<<endl;
            }
            tempMsg.size =result;
            tempMsg.MallocFree = 1;
            tempMsg.idx = i;
            onePieceMsgVec_.push_back(tempMsg);
        }else if (v[0]=="Free"){
            onePieceMsg tempMsg;
            tempMsg.ptr =v[1];
            tempMsg.size = -1;
            tempMsg.MallocFree = -1;
            tempMsg.idx = i;
            onePieceMsgVec_.push_back(tempMsg);
        }else {
            cout<<"error for process the onePriceMsg."<<endl;
        }
    }
    cout<<"done all, size of onePieceMsgVec_: "<<onePieceMsgVec_.size()<<endl;

    sort(onePieceMsgVec_.begin(),onePieceMsgVec_.end(),less_than_ptrIdx());

    //    //below prints onePieceMsgVec_
//        for (int i=0;i<onePieceMsgVec_.size();i++) {
//            cout<<onePieceMsgVec_[i].ptr<<' '<<onePieceMsgVec_[i].size<<' '<<onePieceMsgVec_[i].MallocFree<<' '<<onePieceMsgVec_[i].idx<<' '<<endl;
//        }
//
    // this section construct the onePairMsgVec_
    int maxIdx = static_cast<int>(onePieceMsgVec_.size());
    //TODO(junzhe) optional to show percentage/size of the 3 group blocks.
    vector<onePairMsg>onePairMsgVec_;
    int i=0;
    int checker =0; //just to verify if goes in any of the condition; TODO(junzhe) refine
    //cout<<"print value of i before while loop "<<i<<endl;
    //while loop processes a pair at each time, if got a pair.
    
    while (i<(onePieceMsgVec_.size()-1)){ //TODO(junzhe) not full size to create come only variables.
        //condition A: no matter if i=0, start with free. do nothing.
        if (onePieceMsgVec_[i].MallocFree==-1){
            i+=1; checker = 1;
        }
        //condition B: start with Malloc, next item same ptr and is free.
        if ((onePieceMsgVec_[i].MallocFree==1)&& (onePieceMsgVec_[i+1].MallocFree==-1)&&((onePieceMsgVec_[i].ptr==onePieceMsgVec_[i+1].ptr))){
            onePairMsg tempPair;
            tempPair.name = onePieceMsgVec_[i].idx;
            tempPair.size = onePieceMsgVec_[i].size;
            tempPair.r_idx = onePieceMsgVec_[i].idx;
            tempPair.d_idx = onePieceMsgVec_[i+1].idx;
            tempPair.condition='B';
            onePairMsgVec_.push_back(tempPair);
            i+=2; checker = 1;
        }
        // condition C: start with Malloc, no free.
        if ((onePieceMsgVec_[i].MallocFree==1)&&(onePieceMsgVec_[i].ptr!=onePieceMsgVec_[i+1].ptr)){
            onePairMsg tempPair;
            tempPair.name = onePieceMsgVec_[i].idx;
            tempPair.size = onePieceMsgVec_[i].size;
            tempPair.r_idx = onePieceMsgVec_[i].idx;
            tempPair.d_idx = maxIdx;
            tempPair.condition='C';
            onePairMsgVec_.push_back(tempPair);
            i+=1; checker = 1;
        }
    }//TODO(junzhe) verify if this is end of while
    //cout<<"print value of i after while loop "<<i<<endl;
    //condition D: if still left with the last item
    if ((i<onePieceMsgVec_.size())&&(onePieceMsgVec_[i+1].MallocFree==1)){
        onePairMsg tempPair;
        tempPair.name = onePieceMsgVec_[i].idx;
        tempPair.size = onePieceMsgVec_[i].size;
        tempPair.r_idx = onePieceMsgVec_[i].idx;
        tempPair.d_idx = maxIdx;
        tempPair.condition='D';
        onePairMsgVec_.push_back(tempPair);
        i+=1; checker = 1;
    }
    if (checker==0){
        cout<<"error with the onePairMsg processing"<<endl;
    }
    //this section is to sort
    sort(onePairMsgVec_.begin(),onePairMsgVec_.end(),less_than_size());
    cout<<"done all, size of onePairMsgVec_: "<<onePairMsgVec_.size()<<endl;
    
    // below is to get oneIterMsg
    int idxReplacer = maxIdx; //TODO(junzhe) for get come-only pairs
    vector<oneIterMsg>oneIterMsgVec_;
    //TODO(junzhe) below m should be size of oneIterMsgvec_
    int counter1=0; int counter2=0; int counter3=0;
    for (int i =0; i<onePairMsgVec_.size(); i++){
        oneIterMsg tempIterM;
        tempIterM.idx = onePairMsgVec_[i].r_idx;
        tempIterM.MallocFree=1;
        tempIterM.size_delta = onePairMsgVec_[i].size;
        oneIterMsgVec_.push_back(tempIterM);
        counter1++;
        if (onePairMsgVec_[i].d_idx<idxReplacer){
            oneIterMsg tempIterF;
            tempIterF.idx = onePairMsgVec_[i].d_idx;
            tempIterF.MallocFree=-1;
            tempIterF.size_delta = onePairMsgVec_[i].d_idx-onePairMsgVec_[i].r_idx;
            oneIterMsgVec_.push_back(tempIterF);
            counter2++;
        }
        if (onePairMsgVec_[i].d_idx==idxReplacer){
            counter3++;
        }
    }
    sort(oneIterMsgVec_.begin(),oneIterMsgVec_.end(),less_than_iterIdx());
    vector<int>rep; // vector of size_delta, name it as rep for simlisity.
    for (int i =0; i<oneIterMsgVec_.size(); i++){
        rep.push_back(oneIterMsgVec_[i].size_delta);
        //cout<<rep[i]<<endl;
    }
    cout<<"done all, size of size_delta: "<<rep.size()<<endl;
    //TODO(junzhe) delete these counters
//    int counter4=0;
//    int counter5=0;
//    vector<char>counter6;
//     for (int i =0; i<onePairMsgVec_.size(); i++){
//         if (onePairMsgVec_[i].condition=='C'){
//             counter4++;
//             cout<<"a condition C's idx is: "<<onePairMsgVec_[i].name<<endl;
//             cout<<"it's string is "<<vec[10]<<endl;
//             break;
//         }
//         if (onePairMsgVec_[i].d_idx==maxIdx){
//             counter6.push_back(onePairMsgVec_[i].condition);
//             counter5++;
//         }
//     }
    cout<<"counter1 and counter2, counter3 are: "<<counter1<<' '<< counter2<<' '<< counter3<<endl;
//    cout<<"counter4 is: "<<counter4<<endl;
//    cout<<"counter5 is: "<<counter5<<endl;
    cout<<"done all, size of oneIterMesgVec_: "<<oneIterMsgVec_.size()<<endl;
    
    ///Below for substring
//    cout<<"=============================below print size_delta============================="<<endl;
//    for (int i =0; i<size_delta.size(); i++){
//        cout<<size_delta[i]<<',';
//    }
//    cout<<endl;

    if(equal(rep.begin()+201,rep.begin()+200+188,rep.begin()+201+188)) {
        cout<<"test rep itself..."<<endl;
    }
    //now brtue force method using uqual
    int maxLen =0;
    for (int i=0; i<strVecSize;i++){
        for (int len=1; len<(strVecSize-i);len++){
            if((equal(rep.begin()+i,rep.begin()+i-1+len,rep.begin()+i+len))&&(maxLen<len)) {
                maxLen =len;
                cout<<"maxLen increased, lcoation and maxLen: ("<<i<<","<<maxLen<<")"<<endl;
            }
        }
    }
    cout<<"end of test"<<endl;
    cout<<"below test why some 2nd iteration cannot make it"<<endl;
    
    
//    //figure out which location it is
//    for (int i=0; i<(strVecSize-res_length);i++){
//        cout<<"print i only "<<i<<endl;
//        if(equal(rep.begin()+i,rep.begin()+i-1+res_length,res.begin())) {
//            cout<<"yes, it is "<<i<<endl;
//        }
//    }
////    cout<<"above test location"<<endl;
//    if(equal(rep.begin()+389,rep.begin()+389+187,res.begin()+389+188)) {
//        cout<<"yes, it is===== "<<endl;
//    }
//
//    for (int i=0; i<(strVecSize-res_length);i++){
//        if(equal(rep.begin()+i,rep.begin()+i+res_length,res.begin())) {
//            cout<<"yes "<<i<<endl;
//        }
//    }
    
    return 0;
}
