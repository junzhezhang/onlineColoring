#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <stdlib.h>     /* malloc, free, rand */

using namespace std;

///vertex of the graph.
class Vertex {
public: //TODO(junzhe) can change some to private type
    int name;
    int size;
    int r; //arrive
    int d; //depart
    Vertex(int,int,int,int);
    pair<int, int> colorRange;
    vector<pair<int, int>> colorOccupied;
};
Vertex::Vertex(int varName, int s, int startIdx, int endIdx){
    name =varName;
    size = s;
    r = startIdx;
    d = endIdx;
}//end of class Vertex


///Section for structs and respective sorting function:
// onePieceMsg, onePairMsg, oneIterMsg, version 11/30 3pm
//below are syntax for customized sorting.
//https://stackoverflow.com/questions/1380463/sorting-a-vector-of-custom-objects
//https://stackoverflow.com/questions/3574680/sort-based-on-multiple-things-in-c
struct onePieceMsg{
    /*
     members: [ptr, size, MallocFree, idx]
     */
    string ptr;
    int size;
    int MallocFree;
    int idx;
};


struct less_than_ptrIdx{
    /*
     sort onePieceMsg by ptr and then idx.
     */
    inline bool operator() (const onePieceMsg& struct1, const onePieceMsg& struct2)
    {
        return ((struct1.ptr<struct2.ptr)||((struct1.ptr==struct2.ptr)&&(struct1.idx<struct2.idx)));
    }
};

struct oneIterMsg{
    /*
     members: [idx, MallocFree, size_delta]
     */
    int size_delta;
    int MallocFree;
    int idx;
};


struct less_than_iterIdx{
    /*
     sort oneIterMsg by Idx.
     */
    inline bool operator() (const oneIterMsg& struct1, const oneIterMsg& struct2)
    {
        return (struct1.idx<struct2.idx);
    }
};

//TODO(junzhe) to replace vertex with onePairMsg, try combine other structs as well.
struct onePairMsg{
    /*
     members: [name (r_idx), size, r_idx, d_idx]
     */
    int name;
    int size;
    int r_idx;
    int d_idx;
    char condition; //verifySection, for debug purpose only
    //pair<int, int> colorRange;
    //vector<pair<int, int>> colorOccupied;
};


struct less_than_size{
    /*
     sort onePairMsg by descending size.
     */
    inline bool operator() (const onePairMsg& struct1, const onePairMsg& struct2)
    {
        return (struct1.size>struct2.size);
    }
};

struct lookUpElement{
    /*
     for memory pool Malloc look-up table.
     */
    int r_idx;
    int d_idx;
    int size;
    int offset;
    int Occupied; //0 is free, 1 is occupied.
};

struct less_than_lookupIdx{
    /*
     sort lookUpElement by idx.
     */
    inline bool operator() (const lookUpElement& struct1, const lookUpElement& struct2)
    {
        return (struct1.r_idx<struct2.r_idx);
    }
};


/// string delimiter
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

///Section of converting text file -->vector of Sring --> pieceMsg -->pairMsg -->iterMsg
//vector of pairMsg is used in run.
//vector of iterMsg is used in test.
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


vector<onePieceMsg> strVec_2_pieceMsgVec(vector<string> vec, int &idxRange){
   /*
    convert vector of string into vector of onePieceMsg, sorted by ptr and then idx.
    */
    vector<onePieceMsg>onePieceMsgVec_;
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

    sort(onePieceMsgVec_.begin(),onePieceMsgVec_.end(),less_than_ptrIdx());
    idxRange = static_cast<int>(onePieceMsgVec_.size());
//   //verifySection
//            for (int i=0;i<onePieceMsgVec_.size();i++) {
//                cout<<onePieceMsgVec_[i].ptr<<' '<<onePieceMsgVec_[i].size<<' '<<onePieceMsgVec_[i].MallocFree<<' '<<onePieceMsgVec_[i].idx<<' '<<endl;
//            }
    return onePieceMsgVec_;
}// end of strVec_2_pieceMsgVec function


pair<vector<onePairMsg>,vector<onePairMsg>> pieceMsgVec_2_pairOfPairMsgVec(vector<onePieceMsg>onePieceMsgVec_, int idxRange){

    //pairMsg is grouped into 1. normal blocks 2. cross-iteration blocks.
    vector<onePairMsg>onePairMsgVec_1;
    vector<onePairMsg>onePairMsgVec_2;
    int i=0;
    int checker =0; //verifySection just to verify if goes in any of the condition;

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
            onePairMsgVec_1.push_back(tempPair);
            i+=2; checker = 1;
        }
        // condition C: start with Malloc, no free.
        if ((onePieceMsgVec_[i].MallocFree==1)&&(onePieceMsgVec_[i].ptr!=onePieceMsgVec_[i+1].ptr)){
            onePairMsg tempPair;
            tempPair.name = onePieceMsgVec_[i].idx;
            tempPair.size = onePieceMsgVec_[i].size;
            tempPair.r_idx = onePieceMsgVec_[i].idx;
            tempPair.d_idx = idxRange;
            tempPair.condition='C';
            onePairMsgVec_2.push_back(tempPair);
            i+=1; checker = 1;
        }
    }//end of while
    //condition D: if still left with the last item
    if ((i<onePieceMsgVec_.size())&&(onePieceMsgVec_[i+1].MallocFree==1)){
        onePairMsg tempPair;
        tempPair.name = onePieceMsgVec_[i].idx;
        tempPair.size = onePieceMsgVec_[i].size;
        tempPair.r_idx = onePieceMsgVec_[i].idx;
        tempPair.d_idx = idxRange;
        tempPair.condition='D';
        onePairMsgVec_2.push_back(tempPair);
        i+=1; checker = 1;
    }
    if (checker==0){
        cout<<"error with the onePairMsg processing"<<endl;
    }
    //sort both pairVec
    sort(onePairMsgVec_1.begin(),onePairMsgVec_1.end(),less_than_size());
    sort(onePairMsgVec_2.begin(),onePairMsgVec_2.end(),less_than_size());
    pair<vector<onePairMsg>,vector<onePairMsg>>pairOfPairMsgVec_(onePairMsgVec_1,onePairMsgVec_2);
    
    return pairOfPairMsgVec_;
}//end of pieceMsgVec_2_pairOfPairMsgVec function

///Section of coloring algorithm. mergeSeg and then FFallocation when building edges of the graph.
vector<pair<int, int>>  mergeSeg(vector<pair<int, int>> colorOccupied){
    /*
     version 11/29 3pm
     input:the collection of color ranges that is once occupied by some block during a block's life time.
     function: merge consecutive/overlapping segments of colorOccupied
     output: merged segments in ascending order.
     */
    sort(colorOccupied.begin(), colorOccupied.end());
    //    //verifySection
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
        //cout<<"in the while loop, m is "<<m<<endl;  //verifySection
        if (colorOccupied[m].second - colorOccupied[m+1].first >=-1){
            pair<int,int>tempItem(colorOccupied[m].first,max(colorOccupied[m].second,colorOccupied[m+1].second));
            //remove m+1 and m
            colorOccupied.erase(colorOccupied.begin()+m+1);
            colorOccupied.erase(colorOccupied.begin()+m);
            //insert the combined range
            colorOccupied.insert(colorOccupied.begin()+m,tempItem);
            //            //verifySection
            //            cout<<"print colorOccupied after this iterartion"<<endl;
            //            for(auto item : colorOccupied) {
            //                cout << item.first<<' '<<item.second << endl;
            //            }
        }else{
            m+=1;
        }
    }//end of while loop
    
    if (colorOccupied[m].second - colorOccupied[m+1].first >=-1){
        pair<int, int>tempItem(colorOccupied[m].first,max(colorOccupied[m+1].second,colorOccupied[m+1].second));
        //remove m+1 and m
        colorOccupied.erase(colorOccupied.begin()+m+1);
        colorOccupied.erase(colorOccupied.begin()+m);
        //insert the combined one
        colorOccupied.insert(colorOccupied.begin()+m,tempItem);
    }
    
    //    //verifySection
    //    cout<<"print after mergeSeg, len of "<<colorOccupied.size()<<endl;
    //    for (int i=0;i<colorOccupied.size();i++){
    //        cout<<"("<<colorOccupied[i].first<<","<<colorOccupied[i].second<<")";
    //    }
    //    cout<<endl;
    
    return colorOccupied;
}//end of mergeSeg function


pair<int,int> FFallocation(vector<pair<int,int>> colorMerged,int size,int offset){
    /*
     version 12/2 4pm
     First Fit weighted coloring
     return a pair standing for colorRange.
     offset shifts the returned colorRange, allowing multiple runs.
     */
    
    // condition A: if no occupied
    if (colorMerged.size()==0){
        return pair<int,int>(0+offset,size-1+offset);
    }
    
    // condtion B: able to fit before first block
    if (size<(colorMerged[0].first+1)){
        return pair<int,int>(0+offset,size-1+offset);
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
        }//end of while loop.
        // condition D: allocate after the last block.
        if (location == -1){
            location = colorMerged[colorMerged.size()-1].second+1;
        }
    }// end of if loop, conditon C and D.
    
    // condition E: colorMeger len =1, allocate after the last block.
    if (colorMerged.size()==1){
        location = colorMerged[0].second+1;
    }
    if (location==-1){
        cout<<"error in FFallocation!!!"<<endl;
    }
    return pair<int,int>(location+offset,location+size-1+offset);
}//end of FFallocation function


vector<Vertex> colorSomeVertices(vector<onePairMsg> pairMsgVec_, int &offset){
    /*
     color all or 1/2 vertices based on mergeSeg() and FFallocation(), with update offset.
     time complexity: O(n^2).
     */
    int local_offset = offset; //to be feed to FFallocation, shall never change.
    int m = static_cast<int>(pairMsgVec_.size());
    //init all vertices
    vector<Vertex>vertices;
    for (int i=0; i<m;i++){
        Vertex tempVertex(pairMsgVec_[i].name,pairMsgVec_[i].size,pairMsgVec_[i].r_idx,pairMsgVec_[i].d_idx);
        vertices.push_back(tempVertex);
        //cout<<vertices[i].name<<' '<<vertices[i].size<<' '<<vertices[i].r<<' '<<vertices[i].d<<endl;
    }
    //cout<<"test size of pair Msg and vertices: "<<m<<' '<<vertices.size()<<endl;
    
    //syntax refer to http://www.sanfoundry.com/cpp-program-implement-adjacency-matrix/
    int **adj;
    adj = new int*[m]; //TODO(junzhe) should be deleted somewhere.
    // build edges with values 1 and 0; combine with mergeSeg and FFallocation in the loop.
    for (int i=0; i<m;i++){
        adj[i] = new int[m];
        for (int j=0; j<m;j++){
            if ((max(vertices[i].r,vertices[j].r))<(min(vertices[i].d,vertices[j].d))){
                adj[i][j]=1;
                if (vertices[j].colorRange.second){ //as second never be 0, if not empty.
                    vertices[i].colorOccupied.push_back(vertices[j].colorRange);
                }
            }
            else { adj[i][j]=0; }
        }
        vector<pair<int, int>>colorMerged = mergeSeg(vertices[i].colorOccupied);
        vertices[i].colorRange = FFallocation(colorMerged,vertices[i].size, local_offset);
        if (vertices[i].colorRange.second >offset){
            offset = vertices[i].colorRange.second; //offset, (largest memory footprint -1) as well.
        }
    }//end of for loop.
    return vertices;
}

/// main run funtion
vector<Vertex> run(vector<string>vec, int &idxRange, int &offset,int &maxload){
    /*
     run function, input vector of strings, return colored vertices,
     update idxRange, offset, and maxload
     */
    vector<onePieceMsg>onePieceMsgVec_ = strVec_2_pieceMsgVec(vec,idxRange);
    pair<vector<onePairMsg>,vector<onePairMsg>>pairOfPairMsgVec_=pieceMsgVec_2_pairOfPairMsgVec(onePieceMsgVec_,idxRange);
    vector<onePairMsg>pairMsgVec_1 = pairOfPairMsgVec_.first;
    vector<onePairMsg>pairMsgVec_2 = pairOfPairMsgVec_.second;
    vector<Vertex>vertices_2 = colorSomeVertices(pairMsgVec_2,offset);
    int load_part1=offset;
    vector<Vertex>vertices = colorSomeVertices(pairMsgVec_1,offset);
    vector<Vertex>vertices_1 =vertices; //for computing maxload.
    vertices.insert(vertices.end(),vertices_2.begin(),vertices_2.end());
    
    ///below is doing maxload given 2 group of vertices. for 1-iteration epoch, it should be
    // returning the same value as py version maxload, as group 2 vertices none.
    vector<int>load(idxRange,0);
    for (int i=0; i<idxRange;i++){
        for (int j=0; j<vertices_1.size();j++){
            if ((i>=vertices_1[j].r) && (i<=vertices_1[j].d)){
                load[i]+=vertices_1[j].size;
            }
        }
    }
    maxload =*max_element(load.begin(),load.end())+load_part1;

    cout<<"testing, size of Pair 1: "<<pairMsgVec_1.size()<<endl;
    cout<<"testing, size of Pair 2: "<<pairMsgVec_2.size()<<endl;
    cout<<"size of new vertices: "<<vertices.size()<<endl;
    cout<<"idxRange is: "<<idxRange<<endl;
    cout<<"new offset, aka max memory foot print: "<<offset+1<<endl;
    cout<<"maxload is: "<<maxload<<endl;
    
    return vertices;
}


///Section of test functions.
vector<int> pairOfPairMsgVec_2_repSeq(pair<vector<onePairMsg>,vector<onePairMsg>>pairOfPairMsgVec_){
    int counter_1M=0; int counter_1F=0; int counter_2=0;
    vector<onePairMsg>onePairMsgVec_1 = pairOfPairMsgVec_.first;
    vector<onePairMsg>onePairMsgVec_2 = pairOfPairMsgVec_.second;
    vector<oneIterMsg>oneIterMsgVec_;
    for (int i =0; i<onePairMsgVec_1.size(); i++){
        oneIterMsg tempIterM;
        tempIterM.idx = onePairMsgVec_1[i].r_idx;
        tempIterM.MallocFree=1;
        tempIterM.size_delta = onePairMsgVec_1[i].size;
        oneIterMsgVec_.push_back(tempIterM);
        counter_1M++;
        
        oneIterMsg tempIterF;
        tempIterF.idx = onePairMsgVec_1[i].d_idx;
        tempIterF.MallocFree=-1;
        tempIterF.size_delta = onePairMsgVec_1[i].d_idx-onePairMsgVec_1[i].r_idx;
        oneIterMsgVec_.push_back(tempIterF);
        counter_1F++;
    }
    
    for (int i =0; i<onePairMsgVec_2.size(); i++){
        oneIterMsg tempIterM;
        tempIterM.idx = onePairMsgVec_2[i].r_idx;
        tempIterM.MallocFree=1;
        tempIterM.size_delta = onePairMsgVec_2[i].size;
        oneIterMsgVec_.push_back(tempIterM);
        counter_2++;
    }
    
    sort(oneIterMsgVec_.begin(),oneIterMsgVec_.end(),less_than_iterIdx());
    //only after sort then can create rep.
    vector<int>rep; // vector of size_delta, name it as rep for simlisity.
    for (int i =0; i<oneIterMsgVec_.size(); i++){
        rep.push_back(oneIterMsgVec_[i].size_delta);
        //cout<<rep[i]<<endl;
    }
    
    //verifySection
    cout<<"counter1 and counter2, counter3 are: "<<counter_1M<<' '<< counter_1F<<' '<< counter_2<<endl;
    cout<<"done all, size of oneIterMesgVec_: "<<oneIterMsgVec_.size()<<endl;
    cout<<"done all, size of rep: "<<rep.size()<<endl;
    
    return rep;
}//end of pairOfPairMsgVec_2_repSeq function


vector<int> maxRepeatedSeg (vector<int>rep, int idxRange, int &maxLen, int &location){
    /*
     get max repeated non-overlapping Seg of a vector, return the repeated segment,
     update maxLen, and location of where Seg starts to repeat.
     brtue force method using equal()
    */
    for (int i=0; i<idxRange;i++){
        for (int len=1; len<(idxRange-i);len++){
            if((equal(rep.begin()+i,rep.begin()+i-1+len,rep.begin()+i+len))&&(maxLen<len)) {
                maxLen = len;
                location = i;
                cout<<"maxLen increased, lcoation and maxLen: ("<<location<<","<<maxLen<<")"<<endl;
            }
        }
    }
    //TODO(junzhe) verify the subSeq returned.
   vector<int>subSeq(&rep[location],&rep[location+maxLen]);
    if(!(equal(rep.begin()+location,rep.begin()+maxLen-1+location,subSeq.begin()) && equal(rep.begin()+location+maxLen,rep.begin()+2*maxLen-1+location,subSeq.begin()))){
        cout<<"error in get the maxRep"<<endl;
    }

    return subSeq;
}


void verifyAndCut (vector<int>subSeq, int &maxLen, int &location){
    /*
     to cut, in case the repeated Seg contains multiple iterations.
     */
    int tempMaxLen=0;
    int tempLocation =0;
    int tempIdxRange = maxLen;
    
    vector<int>tempSubSeq = maxRepeatedSeg(subSeq,tempIdxRange,tempMaxLen, tempLocation);
    //TODO(junzhe), tunable threshold.
    int threshold =50;
    if (tempMaxLen>threshold){
        maxLen = tempMaxLen;
        location += tempLocation;
        cout<<"max length get cut"<<endl;
    }
    cout<<tempMaxLen<<endl;
}


//main function of test
int test(vector<string>vec3, int &maxLen, int &location){
    /*
     main function of test, returns globeCounter, which is when flag shall be switched,
     update maxLen and location of where the repeated Seg starts.
     */
    cout<<"====================== test ========================="<<endl;
    int idxRange3=0;
    vector<onePieceMsg>onePieceMsgVec_3 =strVec_2_pieceMsgVec(vec3,idxRange3);
    cout<<"idxRange is: "<<idxRange3<<endl;
    pair<vector<onePairMsg>,vector<onePairMsg>>pairOfPairMsgVec_=pieceMsgVec_2_pairOfPairMsgVec(onePieceMsgVec_3,idxRange3);
    vector<int>rep=pairOfPairMsgVec_2_repSeq(pairOfPairMsgVec_);
    
    //get repeated sub vector.
   
    vector<int>subSeq = maxRepeatedSeg(rep,idxRange3,maxLen,location);
    cout<<subSeq.size()<<endl;
    verifyAndCut(subSeq, maxLen, location);
    int globeCounter=-1;
    if (maxLen>100){ //TODO(junzhe) tunable threshold.
    cout<<"new location and maxLen: "<<location<<' '<<maxLen<<endl;
    cout<<"mod: "<<(idxRange3-location)%maxLen<<endl;
    cout<<"left: "<<maxLen-(idxRange3-location)%maxLen<<endl;
    globeCounter = idxRange3+maxLen-(idxRange3-location)%maxLen;
    }
    return globeCounter;
}


///class mem-pool SmartMemPool
class SmartMemPool{
public:
    SmartMemPool(); //constructor
    //TODO(junzhe) change all size related int to size_t
    //TODO(junzhe) in Singa, void Malloc( void**, size_t); change to cudaMalloc and cudaFree.
    void* Malloc(int size);
    void Free(void* ptr);
    ~SmartMemPool();
private:
//    int mallocFlag ; //0 for cudaMalloc, 1 for coloringMalloc
//    int gc ; //global counter each time Malloc/Free, add 1.
//    int globeCounter;
//    void* ptrPool;
//    int idxRange ;
//    int offset ;
//    int maxload ;
//    int maxLen ;
//    int location ;
    int mallocFlag =0; //0 for cudaMalloc, 1 for coloringMalloc
    int gc =0; //global counter each time Malloc/Free, add 1.
    int globeCounter=-1;
    void* ptrPool = NULL;
    int idxRange = 0;
    int offset = 0;
    int maxload = 0;
    int maxLen =0;
    int location=0;
    vector<string> vec;
    vector<pair<int,int>>Table1; //(all_idx, r_idx) to look at Table 2 for Free.
    vector<lookUpElement>Table2;
};
SmartMemPool::SmartMemPool(void){
//    int mallocFlag =0; //0 for cudaMalloc, 1 for coloringMalloc
//    int gc =0; //global counter each time Malloc/Free, add 1.
//    int globeCounter=-1;
//    void* ptrPool = NULL;
//    int idxRange = 0;
//    int offset = 0;
//    int maxload = 0;
//    int maxLen =0;
//    int location=0;
    
//    vector<pair<int,int>>Table1; //(all_idx, r_idx) to look at Table 2 for Free.
//    vector<lookUpElement>Table2;
    cout << "Object is being created" << endl; //verifySection
}

///Malloc
void* SmartMemPool::Malloc(int size){
    /*
     1. switch flag when gc == globeCounter, construct lookup table and malloc the whole pool.
     2. if flag=0, malloc/cudaMalloc
     3. if flag=1, look up table, switch back at last iteration.
     4. test repeated sequence every 100 blocks, update globeCounter.
     */
    cout<<"current gc is "<<gc<<endl;
    cout<<"GC is "<<globeCounter<<endl;
    void* allocatedPtr = NULL; //ptr to be returned
    
    if (gc == globeCounter){
        /// 1. switch flag when gc == globeCounter, construct lookup table and malloc the whole pool.
        
        mallocFlag=1;
        //TODO(junzhe) verify Vec2 lengh, and starting point.
        vector<string>vec_run(&vec[location],&vec[location+maxLen]);
        vector<Vertex>vertices = run(vec_run, idxRange,offset,maxload);
        
        //construct Table1 and Table2 for later lookup
        for (int i=0; i<vertices.size(); i++){
            //Table1
            pair<int,int>temp1(vertices[i].r,vertices[i].r);
            pair<int,int>temp2(vertices[i].d,vertices[i].r);
            Table1.push_back(temp1);
            Table1.push_back(temp2);
            //Table2
            lookUpElement temp;
            temp.r_idx =vertices[i].r;
            temp.d_idx =vertices[i].d;
            temp.size =vertices[i].size;
            temp.offset=vertices[i].colorRange.first;
            temp.Occupied =0;
            Table2.push_back(temp);
        }
        
        //sort
        sort(Table2.begin(), Table2.end(),less_than_lookupIdx());
        sort(Table1.begin(),Table1.end());
        //verify Table1 and Table2 TODO(junzhe)
        //move redundant Table1 element, but not necessary. idxRange.
        //update ptrPool
        ptrPool = malloc((size_t)(offset+1)); //poolSize or memory foot print  offset+1.
    }
    
    if(mallocFlag==0){
        ///  2. if flag=0, malloc/cudaMalloc
        
        allocatedPtr = malloc(size);
        
        //push_back the string for later test and run.
        string tempStr1 ="Malloc ";
        stringstream strm2;
        strm2<<allocatedPtr;
        string tempStr2 = strm2.str();
        stringstream strm3;
        strm3<<size;
        string tempStr3 = strm3.str();
        string temp = tempStr1+tempStr2+" "+tempStr3;
        vec.push_back(temp);
        
    }else{
        /// 3. if flag=1, look up table, switch back at last iteration.
        
        int lookupIdx = (gc-location)%maxLen;
        if ((Table2[lookupIdx].size == (size_t) size)&& (Table2[lookupIdx].Occupied==0)){
        int allocatedOffset = Table2[lookupIdx].offset;
         allocatedPtr = (void*)((char*)ptrPool+allocatedOffset*sizeof(char));
        }else {
            //last iteration
            mallocFlag = 0;
            allocatedPtr = malloc(size);
        }
        
        //TODO verify method. for the last iteration.
    }
    
    if ((gc>0)&&(gc%100==0) && (mallocFlag==0) && (globeCounter==-1)){
        ///4. test repeated sequence every 100 blocks, update globeCounter.
        cout<<"gc and GC before test: "<<gc<<' '<<globeCounter<<endl;
        globeCounter = test(vec,maxLen,location);
    }
    
    gc++;
    
    return allocatedPtr;
}

///Free
void SmartMemPool::Free(void* ptr){
    cout<<"current gc is "<<gc<<endl;
    if ((globeCounter==-1)||(gc<globeCounter)){
        /// before flag switch, for sure all free shall be by free()
        free(ptr);
    }else{
        int lookupIdx = (gc-location)%maxLen;
        if(Table2[Table1[lookupIdx].second].Occupied ==1){
            /// allocated by Table lookup
            Table2[Table1[lookupIdx].second].Occupied = 0; //freed, able to allocate again.
        }else{
            free(ptr);
        }
    }
    gc++;
}


SmartMemPool::~SmartMemPool(){
    free(ptrPool);
    //TODO(junzhe) verify what else shall be cleaned up.
}


///main() simulation of how exactly singa will run online using running info of 20 iterations.
int main() {
    
//    /// test functionality of Malloc and Free, selected cases.
//    SmartMemPool pool;
//    cout<<"done 1"<<endl;
//    void* tempPtr = pool.Malloc(20);
//    cout<<"done 2"<<endl;
//    cout<<tempPtr<<endl;
//    cout<<"done 3"<<endl;
//    pool.Free(tempPtr);
//    cout<<"done 4"<<endl;
    
    /// test a full loop.
    SmartMemPool pool;
    string fileName ="memInfo_alex_20itr_100size.text";
    vector<string>vec = file_2_strVec(fileName);

    cout<<"example of one vec string"<<endl;
    cout<<vec[1]<<endl;
    int testNum = static_cast<int>(vec.size());
    testNum = 180;
    for (int i=0; i<testNum;i++){
        vector<string> v = split(vec[i], " ");
        if (v[0]=="Malloc"){
            cout<<"============ Malloc "<<i<<"============"<<endl;
            //covert size fromstr to int.
            int result;
            stringstream convert(v[2]);
            if (!(convert>>result)){
                result =-1;
                cout<<"error for converting size from str to int."<<endl;
            }
            void* tempPtr = pool.Malloc(result);
        }else{
            //Free do later
            //pool.Free();
        }
    }
    
    return 0;
}
