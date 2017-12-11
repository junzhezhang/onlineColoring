#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <stdlib.h>     /* malloc, free, rand */
#include <map>
using namespace std;

///vertex of the graph.
class Vertex {
public: //TODO(junzhe) can change some to private type
    int name;
    size_t size;
    int r; //arrive
    int d; //depart
    Vertex(int,size_t,int,int);
    pair<size_t, size_t> colorRange;
    vector<pair<size_t, size_t>> colorOccupied;
};
Vertex::Vertex(int n, size_t s, int r1, int d1){
    name =n;
    size = s;
    r = r1;
    d = d1;
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
    size_t size;
    int MallocFree;
    int idx;
    onePieceMsg(string p, size_t s, int M, int i):ptr(p),size(s),MallocFree(M),idx(i){}
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
    size_t size_delta;// type as size_t in case size if large.
    int MallocFree;
    int idx;
    oneIterMsg(size_t s, int M, int i):size_delta(s),MallocFree(M),idx(i){}
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
    size_t size;
    int r_idx;
    int d_idx;
    char condition; //verifySection, for debug purpose only
    //pair<int, int> colorRange;
    //vector<pair<int, int>> colorOccupied;
    onePairMsg(int n,size_t s, int r,int d):name(n),size(s),r_idx(r),d_idx(d){}
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
    size_t size;
    size_t offset;
    void* ptr;
    int Occupied; //0 is free, 1 is occupied.
    //for use of constructor TODO(junzhe)
    //r_idx(r),d_idx(d),size(s),offset(o),Occupied(oc)
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
    convert vector of string into vector of onePieceMsg, sorted by ptr and then idx, and update idxRange to pieceMsgVec size.
    */
    vector<onePieceMsg>onePieceMsgVec_;
    for (int i=0;i<vec.size();i++) {
        vector<string> v = split(vec[i], " ");
        if (v[0]=="Malloc"){
            //change v[2] from str to size_t
            size_t result;
            stringstream convert(v[2]);
            if (!(convert>>result)){
                result =-1;
                cout<<"error for converting size from str to int 1."<<endl;
            }
            onePieceMsg tempMsg(v[1],result, 1, i);
            onePieceMsgVec_.push_back(tempMsg);
        }else if (v[0]=="Free"){
            onePieceMsg tempMsg(v[1],-1, -1, i);
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
    /*
     pairMsg is grouped into 1. normal blocks 2. cross-iteration blocks.
     */
    vector<onePairMsg>onePairMsgVec_1;
    vector<onePairMsg>onePairMsgVec_2;
    int i=0;
    int checker =0; //verifySection just to verify if goes in any of the condition;

    //while loop processes a pair at each time, if got a pair.
    while (i<(onePieceMsgVec_.size()-1)){
        //condition A: start with free. do nothing.
        if (onePieceMsgVec_[i].MallocFree==-1){
            i+=1; checker = 1;
        }
        //condition B: start with Malloc, next item same ptr and is free.
        if ((onePieceMsgVec_[i].MallocFree==1)&& (onePieceMsgVec_[i+1].MallocFree==-1)&&((onePieceMsgVec_[i].ptr==onePieceMsgVec_[i+1].ptr))){
            onePairMsg tempPair(onePieceMsgVec_[i].idx,onePieceMsgVec_[i].size,onePieceMsgVec_[i].idx,onePieceMsgVec_[i+1].idx);
            tempPair.condition='B';
            onePairMsgVec_1.push_back(tempPair);
            i+=2; checker = 1;
        }
        // condition C: start with Malloc, no free.
        if ((onePieceMsgVec_[i].MallocFree==1)&&(onePieceMsgVec_[i].ptr!=onePieceMsgVec_[i+1].ptr)){
            onePairMsg tempPair(onePieceMsgVec_[i].idx,onePieceMsgVec_[i].size,onePieceMsgVec_[i].idx,idxRange);
            tempPair.condition='C';
            onePairMsgVec_2.push_back(tempPair);
            i+=1; checker = 1;
        }
    }//end of while
    //condition D: if still left with the last item
    if ((i<onePieceMsgVec_.size())&&(onePieceMsgVec_[i+1].MallocFree==1)){
        onePairMsg tempPair(onePieceMsgVec_[i].idx,onePieceMsgVec_[i].size,onePieceMsgVec_[i].idx,idxRange);
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
vector<pair<size_t, size_t>>  mergeSeg(vector<pair<size_t,size_t>> colorOccupied){
    /*
     version 12/9 11am -- modify to accomodate unsigned int/size_t
     input:the collection of color ranges that is once occupied by some block during a block's life time.
     function: merge consecutive/overlapping segments of colorOccupied
     output: merged segments in ascending order.
     time complexity: O(n) for run, O(n^2) for verify section(optional), where n is size of colorOccupied.
     */
    sort(colorOccupied.begin(), colorOccupied.end());
    
    if(colorOccupied.size()<=1){
        return colorOccupied;
    }
    
    int m = 0;
    while (m<(colorOccupied.size()-1)){

        if ((colorOccupied[m].second +2)> colorOccupied[m+1].first){
            pair<int,int>tempItem(colorOccupied[m].first,max(colorOccupied[m].second,colorOccupied[m+1].second));
            //remove m+1 and m
            colorOccupied.erase(colorOccupied.begin()+m+1);
            colorOccupied.erase(colorOccupied.begin()+m);
            //insert the combined range
            colorOccupied.insert(colorOccupied.begin()+m,tempItem);
        }else{
            m+=1;
        }
    }//end of while loop
    
    //verify if mergeSeg is completed. O(n^2)
    if(colorOccupied.size()>1){
        for (int i=0;i<(colorOccupied.size()-1);i++){
            if(colorOccupied[i].second>=colorOccupied[i+1].first){
            cout<<"error in mergeSeg"<<endl;
            }
        }
    }
    
    return colorOccupied;
}//end of mergeSeg function


pair<size_t,size_t> FFallocation(vector<pair<size_t,size_t>> colorMerged,size_t size, size_t local_offset){
    /*
     version 12/2 4pm
     First Fit weighted coloring
     return a pair standing for colorRange.
     local_offset shifts the returned colorRange, allowing multiple run().
     local_offset not changable, whereas offset is changable.
     */
    // condition A: if no occupied, put after the local_offset
    if (colorMerged.size()==0){
        return pair<size_t,size_t>(0+local_offset,size-1+local_offset);
    }
    
    // condtion B: able to fit before first block, after the local_offset
    if ((size+local_offset)<(colorMerged[0].first+1)){
        return pair<size_t,size_t>(0+local_offset,size-1+local_offset);
    }
    
    size_t yLocation= -1;
    if (colorMerged.size()>1) {
        //int location = -1;
        int n = 0;
        while (n<(colorMerged.size()-1)){
            // condition C: able to fit in between middle blocks.
            if ((colorMerged[n+1].first-colorMerged[n].second-1)>=size){
                yLocation = colorMerged[n].second+1;
                cout<<yLocation<<endl;
                break;
            }
            n+=1;
        }//end of while loop.
        // condition D: allocate after the last block.
        if (yLocation == -1){
            yLocation = colorMerged[colorMerged.size()-1].second+1;
        }
    }// end of if loop, conditon C and D.
    
    // condition E: colorMeger len =1, allocate after the last block.
    if (colorMerged.size()==1){
        yLocation = colorMerged[0].second+1;
    }
    
    if (yLocation==-1){
        cout<<"error in FFallocation!!!"<<endl;
    }
    
    return pair<size_t,size_t>(yLocation,yLocation+size-1);
}//end of FFallocation function


vector<Vertex> colorSomeVertices(vector<onePairMsg> pairMsgVec_, size_t &offset){
    /*
     color all or 1/2 vertices using mergeSeg() and FFallocation(), with update offset.
     time complexity: O(n^2).
     */
    size_t local_offset = offset; //feed into FFallocation, shall never change.
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
        
        vector<pair<size_t,size_t>>colorMerged = mergeSeg(vertices[i].colorOccupied);
        vertices[i].colorRange = FFallocation(colorMerged,vertices[i].size, local_offset);
        //verify
        //cout<<"this is to color r idx: "<<vertices[i].r<<' '<<vertices[i].colorRange .first<<endl;
        
        //update of offset, largest memory footprint as well.
        if (vertices[i].colorRange.second >=offset){
            offset = vertices[i].colorRange.second+1;
        }
    }//end of for loop.
    
    return vertices;
}


///get cross-iteration duration pairs
pair<map<int,int>,map<int,int>> cross_itr_durations(vector<string>vec_double, int location, int maxLen, int &doubleRange){
    
    vector<onePieceMsg>onePieceMsgVec_2 = strVec_2_pieceMsgVec(vec_double,doubleRange);
    pair<vector<onePairMsg>,vector<onePairMsg>>pairOfPairMsgVec_2=pieceMsgVec_2_pairOfPairMsgVec(onePieceMsgVec_2,doubleRange);
    
    //verifySection
    //cout<<"cout doubleRange: "<<doubleRange<<endl;
    //cout<<"cout first pair size: ";
    //cout<<pairOfPairMsgVec_2.first.size()<<' '<<pairOfPairMsgVec_2.second.size()<<endl;
    
    map<int,int>Table_r2d; //full duration info, cross-iteration duration.
    map<int,int>Table_d2r;
    for (int i=0;i<pairOfPairMsgVec_2.first.size();i++){
        if(pairOfPairMsgVec_2.first[i].r_idx<maxLen){
            Table_r2d[pairOfPairMsgVec_2.first[i].r_idx] =pairOfPairMsgVec_2.first[i].d_idx%maxLen;
            Table_d2r[pairOfPairMsgVec_2.first[i].d_idx%maxLen]=pairOfPairMsgVec_2.first[i].r_idx;
        }
    }
    cout<<"size of both Tables "<<Table_r2d.size()<<endl;
    
    return pair<map<int,int>,map<int,int>>(Table_r2d,Table_d2r);
}

/// main run funtion
vector<Vertex> run(vector<string>vec, int &idxRange, size_t &offset,size_t &maxload){
    /*
     run function, input vector of strings, return colored vertices,
     update idxRange, offset, and maxload
     */
    vector<onePieceMsg>onePieceMsgVec_ = strVec_2_pieceMsgVec(vec,idxRange);
    pair<vector<onePairMsg>,vector<onePairMsg>>pairOfPairMsgVec_=pieceMsgVec_2_pairOfPairMsgVec(onePieceMsgVec_,idxRange);
    vector<onePairMsg>pairMsgVec_1 = pairOfPairMsgVec_.first;
    vector<onePairMsg>pairMsgVec_2 = pairOfPairMsgVec_.second;
    vector<Vertex>vertices_2 = colorSomeVertices(pairMsgVec_2,offset);
    size_t load_part1=offset;//verify
    vector<Vertex>vertices = colorSomeVertices(pairMsgVec_1,offset);
    vector<Vertex>vertices_1 =vertices; //for computing maxload.
    vertices.insert(vertices.end(),vertices_2.begin(),vertices_2.end());
    //TODO(junzhe) ready to be removed.
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
vector<size_t> pairOfPairMsgVec_2_repSeq(pair<vector<onePairMsg>,vector<onePairMsg>>pairOfPairMsgVec_){
    int counter_1M=0; int counter_1F=0; int counter_2=0;
    vector<onePairMsg>onePairMsgVec_1 = pairOfPairMsgVec_.first;
    vector<onePairMsg>onePairMsgVec_2 = pairOfPairMsgVec_.second;
    vector<oneIterMsg>oneIterMsgVec_;
    for (int i =0; i<onePairMsgVec_1.size(); i++){
        oneIterMsg tempIterM(onePairMsgVec_1[i].size,1,onePairMsgVec_1[i].r_idx);
        oneIterMsgVec_.push_back(tempIterM);
        counter_1M++;
        
        size_t temp_s_d = static_cast<size_t>(onePairMsgVec_1[i].d_idx-onePairMsgVec_1[i].r_idx);
        oneIterMsg tempIterF(temp_s_d,-1,onePairMsgVec_1[i].d_idx);
        oneIterMsgVec_.push_back(tempIterF);
        counter_1F++;
    }
    
    for (int i =0; i<onePairMsgVec_2.size(); i++){
        oneIterMsg tempIterM(onePairMsgVec_2[i].size,1,onePairMsgVec_2[i].r_idx);
        oneIterMsgVec_.push_back(tempIterM);
        counter_2++;
    }
    
    sort(oneIterMsgVec_.begin(),oneIterMsgVec_.end(),less_than_iterIdx());
    //only after sort then can create rep.
    vector<size_t>rep; // vector of size_delta, name it as rep for simlisity.
    for (int i =0; i<oneIterMsgVec_.size(); i++){
        rep.push_back(oneIterMsgVec_[i].size_delta);
        //cout<<rep[i]<<endl;
    }
    
//    //verifySection
//    cout<<"counter1 and counter2, counter3 are: "<<counter_1M<<' '<< counter_1F<<' '<< counter_2<<endl;
//    cout<<"done all, size of oneIterMesgVec_: "<<oneIterMsgVec_.size()<<endl;
//    cout<<"done all, size of rep: "<<rep.size()<<endl;
    return rep;
}//end of pairOfPairMsgVec_2_repSeq function


vector<size_t> maxRepeatedSeg (vector<size_t>rep, int idxRange, int &maxLen, int &location){
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
   vector<size_t>subSeq(&rep[location],&rep[location+maxLen]);
    if(!(equal(rep.begin()+location,rep.begin()+maxLen-1+location,subSeq.begin()) && equal(rep.begin()+location+maxLen,rep.begin()+2*maxLen-1+location,subSeq.begin()))){
        cout<<"error in get the maxRep"<<endl;
    }
    return subSeq;
}


void verifyAndCut (vector<size_t>subSeq, int &maxLen, int &location){
    /*
     to cut, in case the repeated Seg contains multiple iterations.
     */
    int tempMaxLen=0;
    int tempLocation =0;
    int tempIdxRange = maxLen;
    
    vector<size_t>tempSubSeq = maxRepeatedSeg(subSeq,tempIdxRange,tempMaxLen, tempLocation);
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
    vector<size_t>rep=pairOfPairMsgVec_2_repSeq(pairOfPairMsgVec_);
    
    //get repeated sub vector.
   
    vector<size_t>subSeq = maxRepeatedSeg(rep,idxRange3,maxLen,location);
    cout<<subSeq.size()<<endl;
    verifyAndCut(subSeq, maxLen, location);
    int globeCounter=-1;
    if (maxLen>100){ //TODO(junzhe) tunable threshold.
    cout<<"new location and maxLen: "<<location<<' '<<maxLen<<endl;
    //verifySection
//    cout<<"mod: "<<(idxRange3-location)%maxLen<<endl;
//    cout<<"left: "<<maxLen-(idxRange3-location)%maxLen<<endl;
    globeCounter = idxRange3+maxLen-(idxRange3-location)%maxLen;
    }
    return globeCounter;
}

///verify if coloring got overlapping
void overlap_test(vector<Vertex> vertices){
    size_t s = vertices.size();
    int i,j;
    for (i=0; i<s; i++){
        for (j=i+1; j<s; j++){
            if (((max(vertices[i].r,vertices[j].r))<(min(vertices[i].d,vertices[j].d)))&& ((max(vertices[i].colorRange.first,vertices[j].colorRange.first))<(1+min(vertices[i].colorRange.second,vertices[j].colorRange.second)))){
                cout<<"error overlapping"<<endl;
            }
        }
    }
}


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
SmartMemPool::SmartMemPool(void){
//TODO(junzhe) to figure out what to do here.
}

///Malloc
void* SmartMemPool::Malloc(size_t size){
    /*
     1. switch flag when gc == globeCounter, construct lookup table and malloc the whole pool.
     2. if flag=0, malloc/cudaMalloc, collect vec string
     3. if flag=1, look up table, malloc/cudaMalloc if not in the Table
     4. test repeated sequence every 100 blocks, update globeCounter.
     */
    cout<<"before Malloc: gc  GC idxRange:"<<gc<<' '<<globeCounter<<' '<<idxRange<<endl;
    cout<<"maxLen, location and flag: "<<maxLen<<' '<<location<<' '<<mallocFlag<<endl;
    cout<<" offset and maxload: "<<offset<<" "<<maxload<<endl;
    void* allocatedPtr = NULL; //ptr to be returned
    
    if (gc == globeCounter){
        /// 1. switch flag when gc == globeCounter, construct lookup table and malloc the whole pool.
        
        mallocFlag=1;
        cout<<"switched to color-malloc"<<endl;
        vector<string>vec_run(&vec[location],&vec[location+maxLen]);
        vector<Vertex>vertices = run(vec_run, idxRange,offset,maxload);
        //TODO(junzhe) verify if memory foot print, or input got issue.
        cout<<idxRange<<' '<<offset<<' '<<maxload<<endl;
        cout<<"vertices size: "<<vertices.size()<<endl;
        
        //here to verify if the coloring got overlapping. TODO(junzhe)
        overlap_test(vertices);
        
        //obtain the cross-iteration duration info
        int doubleRange=0;
        vector<string>vec_double(&vec[location],&vec[location+2*maxLen]);
        pair<map<int,int>,map<int,int>>pairs =cross_itr_durations(vec_double, location,maxLen,doubleRange);
        Table_r2d = pairs.first;
        Table_d2r = pairs.second;
        
        //update ptrPool
        ptrPool = malloc(offset); //poolSize or memory foot print  offset.
        cout<<"ptrPool is: "<<ptrPool<<endl;
        //3rd map
        for (int i=0; i<vertices.size(); i++){
            lookUpElement temp;
            temp.r_idx =vertices[i].r;
            temp.d_idx =Table_r2d.find(vertices[i].r)->second;
            temp.size =vertices[i].size;
            temp.offset=vertices[i].colorRange.first;
            temp.ptr = (void*)((char*)ptrPool+temp.offset*sizeof(char));
            //TODO(junzhe) if sizeof(char) needed.
            temp.Occupied =0;
            //build tables for lookup.
            Table_r2Ver[temp.r_idx]= temp;
        }
    }
    
    if(mallocFlag==0){
        ///  2. if flag=0, malloc/cudaMalloc
        allocatedPtr = malloc(size);
        cout<<"condition M1"<<endl;
        
        //update load
        if (gc>0){
            Table_load[gc]=make_pair(Table_load.find(gc-1)->second.first+size,Table_load.find(gc-1)->second.second);
        }else{
            Table_load[gc]=make_pair(size,0);
        }
        
        //push_back the string for later test and run.
        string tempStr1 ="Malloc ";
        stringstream strm2;
        strm2<<allocatedPtr;
        string tempStr2 = strm2.str();
        stringstream strm3;
        strm3<<size; //TODO(junzhe) verify if size_t can be streamed to str or not
        string tempStr3 = strm3.str();
        string temp = tempStr1+tempStr2+" "+tempStr3;
        vec.push_back(temp);
    }else{
        /// 3. if flag=1, look up table, switch back at last iteration.
        
        int lookupIdx = (gc-location)%maxLen;
        if ((Table_r2Ver.find(lookupIdx)->second.size == size)&& (Table_r2Ver.find(lookupIdx)->second.Occupied==0)){
            cout<<"condition M2"<<endl;
            //assign ptr and mark as occupied, and add in ptr2rIdx
            allocatedPtr = Table_r2Ver.find(lookupIdx)->second.ptr;
            Table_r2Ver.find(lookupIdx)->second.Occupied=1;
            
            Table_p2r[allocatedPtr]=lookupIdx;
            cout<<"lookupIdx "<<lookupIdx<<" changed to Occupied "<<Table_r2Ver.find(lookupIdx)->second.Occupied<<endl;//TODO(junzhe) remove
            //update load
            Table_load[gc]=make_pair(Table_load.find(gc-1)->second.first,Table_load.find(gc-1)->second.second+size);
        }else {
            
            cout<<"size not proper or occupied"<<endl;
            cout<<"condition M3"<<endl;//verify
            
            allocatedPtr = malloc(size);
            //update load
            Table_load[gc]=make_pair(Table_load.find(gc-1)->second.first+size,Table_load.find(gc-1)->second.second);
        }
    }
    
    ///4. test repeated sequence every 100 blocks, update globeCounter.
    if (((gc+1)%100==0) && (mallocFlag==0) && (globeCounter==-1)){
        cout<<"gc and GC before test: "<<gc<<' '<<globeCounter<<endl;
        globeCounter = test(vec,maxLen,location);
    }
    
    gc++;
    Table_p2s[allocatedPtr]=size;
    cout<<"to be allocated: "<<allocatedPtr<<endl;
    return allocatedPtr;
}

///Free
void SmartMemPool::Free(void* ptr){
    cout<<"to be freed: "<<ptr<<endl;
    //verify
    cout<<"before Free: gc  GC idxRange:"<<gc<<' '<<globeCounter<<' '<<idxRange<<endl;
    cout<<"maxLen, location and flag: "<<maxLen<<' '<<location<<' '<<mallocFlag<<endl;
    cout<<" offset and maxload: "<<offset<<" "<<maxload<<endl;
    
    size_t deallocatedSize = Table_p2s.find(ptr)->second;
    
    if ((globeCounter==-1)||(gc<globeCounter)){
        
        //push_back the string for later test and run.
        string tempStr1 ="Free ";
        stringstream strm2;
        strm2<<ptr;
        string tempStr2 = strm2.str();
        string temp = tempStr1+tempStr2;
        vec.push_back(temp);
        
        //update load before free
        Table_load[gc]=make_pair(Table_load.find(gc-1)->second.first-deallocatedSize,Table_load.find(gc-1)->second.second);
        
        /// before flag switch, for sure all free shall be done by free()
        free(ptr);
        //verify
        cout<<"condition F1"<<endl; //368 out of 3818.
    }else{
        if (!(Table_p2r.find(ptr)==Table_p2r.end())){
            int resp_rIdx = Table_p2r.find(ptr)->second;
            Table_p2r.erase(ptr);
//            cout<<"resp_rIdx "<<resp_rIdx<<endl;
//            cout<<"idx: "<<(gc-location)%maxLen<<' '<<Table_r2Ver.find(resp_rIdx)->second.d_idx<<endl;
//            cout<<"ptr and ptr "<<ptr<<' '<<Table_r2Ver.find(resp_rIdx)->second.ptr<<endl;
//            cout<<"occupied "<<Table_r2Ver.find(resp_rIdx)->second.Occupied<<endl;
            
            if (((gc-location)%maxLen == Table_r2Ver.find(resp_rIdx)->second.d_idx) && (ptr == Table_r2Ver.find(resp_rIdx)->second.ptr) &&(Table_r2Ver.find(resp_rIdx)->second.Occupied ==1)){
                //update load
                Table_load[gc]=make_pair(Table_load.find(gc-1)->second.first,Table_load.find(gc-1)->second.second-deallocatedSize);
                //deallocate and unmark TODO(junzhe) double check what else to be done.
                Table_r2Ver.find(resp_rIdx)->second.Occupied =1; //freed, able to allocate again.
            }else{
                cout<<"error, in freeing the ptr"<<endl;
            }
        }else{
            cout<<"condition F3"<<endl; //1411 out of 3318
            //update load
            Table_load[gc]=make_pair(Table_load.find(gc-1)->second.first-deallocatedSize,Table_load.find(gc-1)->second.second);
            free(ptr);
            //loadLog[gc]=make_pair(loadLog.find(gc-1)->second.first-sizeDeallocate,loadLog.find(gc-1)->second.second);
        }
    }
    gc++;
 //TODO(junzhe) verify brackets.
}


SmartMemPool::~SmartMemPool(){
    free(ptrPool);
    //TODO(junzhe) verify what else shall be cleaned up.
}

unsigned long SmartMemPool::getMaxLoad(string loadFlag){
    cout<<"get load of "<<loadFlag<<"==========="<<endl;
    unsigned long returned_maxload=0;
    if (loadFlag=="cuda"){
        vector<size_t>cudaLoadLog;
        for (int i=0; i<Table_load.size();i++){
            cudaLoadLog.push_back(Table_load.find(i)->second.first);
        }
        //TODO(junzhe) unsigned long
        //maxload value and idx.
        cout<<*max_element(cudaLoadLog.begin(),cudaLoadLog.end())<<endl;
        cout<<distance(cudaLoadLog.begin(),max_element(cudaLoadLog.begin(),cudaLoadLog.end()) )<<endl;
//        cout<<cudaLoadLog[275]<<endl;
//        cout<<cudaLoadLog[276]<<endl;
//        cout<<cudaLoadLog[277]<<endl;
    }
    
    if (loadFlag=="memPool"){
        vector<size_t>colorLoadLog;
        for (int i=0; i<Table_load.size();i++){
            colorLoadLog.push_back(Table_load.find(i)->second.second);
        }
        cout<<"get mem Pool size"<<endl;
        cout<<*max_element(colorLoadLog.begin(),colorLoadLog.end())<<endl;
        cout<<distance(colorLoadLog.begin(),max_element(colorLoadLog.begin(),colorLoadLog.end()) )<<endl;
    }
    
    if (loadFlag=="printVertices"){
        cout<<"++++++++++++print Vertices ptr+++++++++"<<endl;
        for (int i=0; i<188;i++){
            if(!(Table_r2Ver.find(i)==Table_r2Ver.end())){
                cout<<i<<" "<<Table_r2Ver.find(i)->second.d_idx<<" "<<Table_r2Ver.find(i)->second.size<<' '<<Table_r2Ver.find(i)->second.offset<<' '<<Table_r2Ver.find(i)->second.ptr<<endl;
            }
        }
    }
    cout<<"end of this function"<<endl;
    
    return returned_maxload;
}

///main() simulation of how exactly singa will run online using running info of 20 iterations.
int main() {

    /// test a full loop.
    SmartMemPool pool;
    
    // get the simulation input from text file
    string fileName ="memInfo_alex_20itr_100size.text";
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

        vector<string> v = split(vec[i], " ");

        cout<<"============"<<v[0]<<' '<<i<<"============"<<endl;
        if (i==800){
             pool.getMaxLoad("printVertices");
        }
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
   
//    //TODO(junzhe)  destruct it
    
    //print size of each block, no issue.
//    cout<<"=====================size================="<<endl;
//    for (int i=0; i<testNum;i++){
//        vector<string> v = split(vec[i], " ");
//        if(v[0]=="Malloc"){
//            int result;
//            stringstream convert(v[2]);
//            if (!(convert>>result)){
//                result =-1;
//                cout<<"error for converting size from str to int 2."<<endl;
//            }
//            cout<<i<<' '<<result/1000000<<endl;}
//    }
    return 0;
}
