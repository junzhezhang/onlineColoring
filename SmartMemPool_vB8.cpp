
#include "SmartMemPool.hpp"
#include<iostream>
using namespace std;

///vertex of the graph.
class Vertex {
public:
    int name;
    size_t size;
    int r; //arrive
    int d; //depart
    int crossItr =0; //r.
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

struct less_than_size_rIdx{
    /*
     sort onePairMsg by descending size and r_idx
     */
    inline bool operator() (const onePairMsg& struct1, const onePairMsg& struct2)
    {
        return ((struct1.size>struct2.size)||((struct1.size==struct2.size)&&(struct1.r_idx<struct2.r_idx)));
    }
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
                cout<<"error for converting size from str to int."<<endl;
            }
            onePieceMsg tempMsg(v[1],result, 1, i);
            onePieceMsgVec_.push_back(tempMsg);
        }else if (v[0]=="Free"){
            onePieceMsg tempMsg(v[1],-1, -1, i);
            onePieceMsgVec_.push_back(tempMsg);
        }else {
            cout<<"error for process the onePriceMsg."<<endl;
            cout<<vec[i]<<endl;
        }
    }
    
    sort(onePieceMsgVec_.begin(),onePieceMsgVec_.end(),less_than_ptrIdx());
    idxRange = static_cast<int>(onePieceMsgVec_.size());

    return onePieceMsgVec_;
}// end of strVec_2_pieceMsgVec function


pair<vector<onePairMsg>,vector<onePairMsg>> pieceMsgVec_2_pairOfPairMsgVec(vector<onePieceMsg>onePieceMsgVec_, int idxRange){
    /*
     pairMsg is grouped into 1. normal blocks 2. cross-iteration blocks.
     */
    vector<onePairMsg>onePairMsgVec_1;
    vector<onePairMsg>onePairMsgVec_2;
    int i=0;
    
    //while loop processes a pair at each time, if got a pair.
    while (i<(onePieceMsgVec_.size()-1)){
        //condition A: start with free. do nothing.
        if (onePieceMsgVec_[i].MallocFree==-1){
            i+=1;
        }
        //condition B: start with Malloc, next item same ptr and is free.
        if ((onePieceMsgVec_[i].MallocFree==1)&& (onePieceMsgVec_[i+1].MallocFree==-1)&&((onePieceMsgVec_[i].ptr==onePieceMsgVec_[i+1].ptr))){
            onePairMsg tempPair(onePieceMsgVec_[i].idx,onePieceMsgVec_[i].size,onePieceMsgVec_[i].idx,onePieceMsgVec_[i+1].idx);
            onePairMsgVec_1.push_back(tempPair);
            i+=2;
        }
        // condition C: start with Malloc, no free.
        if ((onePieceMsgVec_[i].MallocFree==1)&&(onePieceMsgVec_[i].ptr!=onePieceMsgVec_[i+1].ptr)){
            onePairMsg tempPair(onePieceMsgVec_[i].idx,onePieceMsgVec_[i].size,onePieceMsgVec_[i].idx,idxRange);
            onePairMsgVec_2.push_back(tempPair);
            i+=1;
        }
    }//end of while
    //condition D: if still left with the last item
    if ((i<onePieceMsgVec_.size())&&(onePieceMsgVec_[i+1].MallocFree==1)){
        onePairMsg tempPair(onePieceMsgVec_[i].idx,onePieceMsgVec_[i].size,onePieceMsgVec_[i].idx,idxRange);
        onePairMsgVec_2.push_back(tempPair);
        i+=1;
    }

    //sort both pairVec
    sort(onePairMsgVec_1.begin(),onePairMsgVec_1.end(),less_than_size_rIdx());
    sort(onePairMsgVec_2.begin(),onePairMsgVec_2.end(),less_than_size_rIdx());
    pair<vector<onePairMsg>,vector<onePairMsg>>pairOfPairMsgVec_(onePairMsgVec_1,onePairMsgVec_2);
    
    return pairOfPairMsgVec_;
}//end of function

///Section of coloring algorithm. mergeSeg and then FFallocation when building edges of the graph.vec
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
    
    //verify if mergeSeg is completed. O(n^2) optional
//    if(colorOccupied.size()>1){
//        for (int i=0;i<(colorOccupied.size()-1);i++){
//            if(colorOccupied[i].second>=colorOccupied[i+1].first){
//                cout<<"error in mergeSeg"<<endl;
//            }
//        }
//    }
    
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
    
    // condition B: able to fit before first block, after the local_offset
    if ((size+local_offset)<(colorMerged[0].first+1)){
        return pair<size_t,size_t>(0+local_offset,size-1+local_offset);
    }
    
    size_t yLocation= -1;
    if (colorMerged.size()>1) {
        int n = 0;
        while (n<(colorMerged.size()-1)){
            // condition C: able to fit in between middle blocks.
            if ((colorMerged[n+1].first-colorMerged[n].second-1)>=size){
                yLocation = colorMerged[n].second+1;
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


pair<size_t,size_t> BFallocation(vector<pair<size_t,size_t>> colorMerged,size_t size, size_t local_offset){
    /*
     version 12/11 1pm
     Best Fit allocation, input and output same as FFallocation
     */
    // condition A: if no occupied, put after the local_offset
    if (colorMerged.size()==0){
        return pair<size_t,size_t>(0+local_offset,size-1+local_offset);
    }
    //condition B: if size=1, able to fit before the first block
    if ((colorMerged.size()==1)&&((size+local_offset)<(colorMerged[0].first+1))){
        return pair<size_t,size_t>(0+local_offset,size-1+local_offset);
    }
    
    //condition C: else of B
    if ((colorMerged.size()==1)&&((size+local_offset)>=(colorMerged[0].first+1))){
        return pair<size_t,size_t>(colorMerged[0].second+1,colorMerged[0].second+size);
    }
    
    //condition D and E:
    size_t yLocation=-1;
    pair<int, size_t>tempHole(-1,-1); // n, hole size between n and n+1
    if (colorMerged.size()>1) {
        int n = 0;
        while (n<(colorMerged.size()-1)){
            // condition C: able to fit in between middle blocks. select smallest.
            if (((colorMerged[n+1].first-colorMerged[n].second-1)>=size)&&((colorMerged[n+1].first-colorMerged[n].second-1)<tempHole.second)){
                tempHole.first=n;
                tempHole.second=colorMerged[n+1].first-colorMerged[n].second-1;
            }
            n+=1;
        }//end of while loop.
        
        if(tempHole.first==-1){
            // condition D: allocate after the last block.
            yLocation = colorMerged[colorMerged.size()-1].second+1;
        }else{
            //condition E: best fit in the smallest hole.
            yLocation = colorMerged[tempHole.first].second+1;
            
        }
    }// end of if loop, conditon D and E.
    
    if (yLocation==-1){
        cout<<"error in BFallocation!"<<endl;
    }
    
    return pair<size_t,size_t>(yLocation,yLocation+size-1);
}

vector<Vertex> colorSomeVertices(vector<onePairMsg> pairMsgVec_, size_t &offset,string colorMethod){
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

    }

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
       
        if(colorMethod=="FF"){
            vertices[i].colorRange = FFallocation(colorMerged,vertices[i].size, local_offset);
            
        }else{ //BF
            vertices[i].colorRange = BFallocation(colorMerged,vertices[i].size, local_offset);
        }

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
    
    map<int,int>Table_r2d; //full duration info, cross-iteration duration.
    map<int,int>Table_d2r;
    for (int i=0;i<pairOfPairMsgVec_2.first.size();i++){
        if(pairOfPairMsgVec_2.first[i].r_idx<maxLen){
            Table_r2d[pairOfPairMsgVec_2.first[i].r_idx] =pairOfPairMsgVec_2.first[i].d_idx%maxLen;
            Table_d2r[pairOfPairMsgVec_2.first[i].d_idx%maxLen]=pairOfPairMsgVec_2.first[i].r_idx;
        }
    }
    
    return pair<map<int,int>,map<int,int>>(Table_r2d,Table_d2r);
}

/// main run funtion
vector<Vertex> run(vector<string>vec, int &idxRange, size_t &offset_normal, size_t &offset_cross,string colorMethod){ //c.
    /*
     run function, input vector of strings, return colored vertices,
     update idxRange, offset.
     time complexity: O(n^2) where n is maxLen.
     */
    vector<onePieceMsg>onePieceMsgVec_ = strVec_2_pieceMsgVec(vec,idxRange);
    pair<vector<onePairMsg>,vector<onePairMsg>>pairOfPairMsgVec_=pieceMsgVec_2_pairOfPairMsgVec(onePieceMsgVec_,idxRange);
    //TODO(junzhe) counter verify above here.
    //1. normal blocks 2. cross-iteration blocks.
    vector<onePairMsg>pairMsgVec_normal = pairOfPairMsgVec_.first;
    vector<onePairMsg>pairMsgVec_cross = pairOfPairMsgVec_.second;
    //cout<<onePieceMsgVec_.size()<<' '<<pairMsgVec_normal.size()<<' '<<pairMsgVec_cross.size()<<endl;
    vector<Vertex>vertices = colorSomeVertices(pairMsgVec_normal,offset_normal,colorMethod);
    //if got this group
    if (pairMsgVec_cross.size()!=0){
        vector<Vertex>vertices_cross = colorSomeVertices(pairMsgVec_cross,offset_cross,colorMethod);
        for (int i=0; i<vertices_cross.size();i++){
            vertices_cross[i].crossItr = 1;
        }
        offset_cross = offset_cross*2;
        //merge
        vertices.insert(vertices.end(),vertices_cross.begin(),vertices_cross.end());
    }
    
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
    }

    return rep;
}//end of pairOfPairMsgVec_2_repSeq function
void repPatternDetector(vector<size_t>rep, int &maxLen, int &location){
    int idxRange = (int)rep.size();
    int threshold =50;
    vector<pair<int,int>>maxLen_location;
    
    for (int i=0; i<idxRange;i++){
        if (maxLen>threshold){
            break;
        }
        for (int len=1; len<(idxRange-i);len++){
            if (maxLen>threshold){
                break;
            }
            if((equal(rep.begin()+i,rep.begin()+i-1+len,rep.begin()+i+len))&&(maxLen<len)) {
                maxLen = len;
                location = i;
                maxLen_location.push_back(make_pair(maxLen,location));
//                cout<<"maxLen increased, lcoation and maxLen: ("<<location<<","<<maxLen<<")"<<endl;
            }
        }
    }
}// end of repPatternDetector

//vector<size_t> maxRepeatedSeg(vector<size_t>rep, int idxRange, int &maxLen, int &location){
//    /*
//     get max repeated non-overlapping Seg of a vector, return the repeated segment,
//     update maxLen, and location of where Seg starts to repeat.
//     brtue force method using equal()
//     time complexity O(n^2)
//     */
//    for (int i=0; i<idxRange;i++){
//        for (int len=1; len<(idxRange-i);len++){
//            if((equal(rep.begin()+i,rep.begin()+i-1+len,rep.begin()+i+len))&&(maxLen<len)) {
//                maxLen = len;
//                location = i;
//                cout<<"maxLen increased, lcoation and maxLen: ("<<location<<","<<maxLen<<")"<<endl;
//            }
//        }
//    }

//    vector<size_t>subSeq(&rep[location],&rep[location+maxLen]);
//    if(!(equal(rep.begin()+location,rep.begin()+maxLen-1+location,subSeq.begin()) && equal(rep.begin()+location+maxLen,rep.begin()+2*maxLen-1+location,subSeq.begin()))){
//        cout<<"error in get the maxRep"<<endl;
//    }
//    return subSeq;
//}


//void verifyAndCut (vector<size_t>subSeq, int &maxLen, int &location){
//    /*
//     to cut, in case the repeated Seg contains multiple iterations.
//     */
//    int tempMaxLen=0;
//    int tempLocation =0;
//    int tempIdxRange = maxLen;
//    int threshold =50;
//    vector<int>maxLenVec;
//    cout<<" running verify cut"<<endl;
//    vector<size_t>tempSubSeq = maxRepeatedSeg(subSeq,tempIdxRange,tempMaxLen, tempLocation);
//    maxLenVec.push_back(tempMaxLen);
//    cout<<"tempMaxLen is "<<tempMaxLen<<endl;
//
//    if (maxLenVec[-1] !=maxLen){
//        if (tempMaxLen>threshold){
//            maxLen = tempMaxLen;
//            location += tempLocation;
//            cout<<"max length get cut"<<endl;
//        }
//        vector<size_t>seq4test (&tempSubSeq[0],&tempSubSeq[tempMaxLen]);
//        tempSubSeq = maxRepeatedSeg(seq4test,tempIdxRange,tempMaxLen, tempLocation);
//        maxLenVec.push_back(tempMaxLen);
//        cout<<"in this loop, tempMaxLen and maxLen "<<tempMaxLen<<' '<<maxLen<<' '<<maxLenVec[-1]<<endl;
//    }
//    cout<<"new maxLen:"<<maxLen<<endl;
//}


//main function of test
int test(vector<string>vec3, int &maxLen, int &location){
    /*
     main function of test, returns globeCounter, which is when flag shall be switched,
     update maxLen and location of where the repeated Seg starts.
     */
    cout<<"====================== test ========================="<<endl;
    int idxRange3=0;
    vector<onePieceMsg>onePieceMsgVec_3 =strVec_2_pieceMsgVec(vec3,idxRange3);
    pair<vector<onePairMsg>,vector<onePairMsg>>pairOfPairMsgVec_=pieceMsgVec_2_pairOfPairMsgVec(onePieceMsgVec_3,idxRange3);
    vector<size_t>rep=pairOfPairMsgVec_2_repSeq(pairOfPairMsgVec_);
    
    repPatternDetector(rep,maxLen,location);
    //get repeated sub vector.
    int globeCounter=-1;
    if (maxLen>50){ //TODO(junzhe) tunable threshold.
        cout<<"new location and maxLen: "<<location<<' '<<maxLen<<endl;
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


SmartMemPool::SmartMemPool(string meth){
    colorMethod = meth;
}

///Malloc
void SmartMemPool::Malloc(void** ptr, size_t size){
    /*
     1. switch flag when gc == globeCounter, construct lookup table and malloc the whole pool.
     2. if flag=0, malloc/cudaMalloc, collect vec string
     3. if flag=1, look up table, malloc/cudaMalloc if not in the Table
     4. test repeated sequence every 100 blocks, update globeCounter.
     */
    
    void* allocatedPtr = NULL; //ptr to be returned
    
    if (gc == globeCounter){
        /// 1. switch flag when gc == globeCounter, construct lookup table and malloc the whole pool.

        mallocFlag=1;
        offset_normal=0;
        offset_cross=0;
        //cout<<gc<<" switched to color-malloc:  "<<endl;
        //cout<<"vec len here "<<vec.size()<<endl;
        //cout<<"location and gc_start_count"<<location<<' '<<gc_start_count<<endl;
        vector<string>vec_run(&vec[location-gc_start_count],&vec[location-gc_start_count+maxLen]);
        //cout<<"vec_run len "<<vec_run.size()<<' '<<location-gc_start_count<<' '<<location-gc_start_count+maxLen<<endl;
        vector<Vertex>vertices = run(vec_run, idxRange,offset_normal,offset_cross, colorMethod); //c.
        //cout<<"run done "<<idxRange<<' '<<offset_normal<<' '<<offset_cross<<endl;
        
        //verify if got overlapping, not work anymore.
        //overlap_test(vertices);

        //obtain the cross-iteration duration info, TODO verify for test itr.
        int doubleRange=0;
        vector<string>vec_double(&vec[location-gc_start_count],&vec[location-gc_start_count+2*maxLen]);
        pair<map<int,int>,map<int,int>>pairs =cross_itr_durations(vec_double, location,maxLen,doubleRange);
        Table_r2d = pairs.first;
        Table_d2r = pairs.second;

        //update ptrPool
        //only once
        if (ptrPool_cross==NULL){
            ptrPool_cross = malloc(offset_cross);
        }
        //every train/test
        if (ptrPool_normal!=NULL){
            free(ptrPool_normal);
            ptrPool_normal=NULL;
        }
        ptrPool_normal = malloc(offset_normal); //poolSize or memory foot print  offset.
        
        //b.  below 2 loops: vec_r2Ver to replace Table_r2Ver
        for (int i=0; i<idxRange; i++){
            lookUpElement tempElement;
            Vec_r2Ver.push_back(make_pair(i,tempElement));
        }
        //cout<<"now print Vec_r2Ver"<<endl;
        //cout<<"size "<<vertices.size()<<endl;
        for (int i=0; i<vertices.size(); i++){
            lookUpElement temp;
            temp.r_idx =vertices[i].r;
            temp.d_idx =Table_r2d.find(vertices[i].r)->second;
            temp.size =vertices[i].size;
            temp.offset=vertices[i].colorRange.first;
            temp.Occupied =0;
            temp.crossItr = vertices[i].crossItr; //c.
            temp.Occupied_backup =0; //c.
            if (temp.crossItr==0){
                temp.ptr = (void*)((char*)ptrPool_normal+temp.offset*sizeof(char));
            }else{
                temp.ptr = (void*)((char*)ptrPool_cross+temp.offset*sizeof(char));
            }
            //build tables for lookup.
            Vec_r2Ver[vertices[i].r].second= temp;
            //cout<<vertices[i].r<<' '<<Vec_r2Ver[vertices[i].r].second.r_idx<<' '<<Vec_r2Ver[vertices[i].r].second.d_idx<<' '<<vertices[i].size<<' '<<Vec_r2Ver[vertices[i].r].second.crossItr<<endl;
        }
        //cout<<gc<<" switched to color-malloc:  completes"<<endl;
    }
    
    if(mallocFlag==0){
        ///  2. if flag=0, malloc/cudaMalloc
        allocatedPtr = malloc(size);

        //update load
        if(loadLogFlag==1){
            if (gc>0){
                Table_load[gc]=make_pair(Table_load.find(gc-1)->second.first+size,Table_load.find(gc-1)->second.second);
            }else{ //very first block
                Table_load[gc]=make_pair(size,0);
            }
        }
        
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
        //cout<<gc<<" condition M1 "<<endl;
    }else{
        /// 3. if flag=1, look up table
        int lookupIdx = (gc-location)%maxLen;
//        if (gc==globeCounter){
//            cout<<"just done GC"<<endl;
//            cout<<Vec_r2Ver[lookupIdx].second.size<<' '<<size<< ' '<<Vec_r2Ver[lookupIdx].second.Occupied<< ' '<<Vec_r2Ver[lookupIdx].second.Occupied_backup<<endl;
//        }

        if ((Vec_r2Ver[lookupIdx].second.size ==size)&&(Vec_r2Ver[lookupIdx].second.Occupied*Vec_r2Ver[lookupIdx].second.Occupied_backup==0)){
            if (Vec_r2Ver[lookupIdx].second.Occupied==0){
                allocatedPtr = Vec_r2Ver[lookupIdx].second.ptr;
                Vec_r2Ver[lookupIdx].second.Occupied= 1;
                Table_p2r[allocatedPtr]=lookupIdx;
                //cout<<gc<<" condition M2 "<<allocatedPtr<<endl;
                //update load
                if(loadLogFlag==1){
                    Table_load[gc]=make_pair(Table_load.find(gc-1)->second.first,Table_load.find(gc-1)->second.second+size);
                }
            }else{ // here must be crossItr, occupied_backup =0
                //if ((Vec_r2Ver[lookupIdx].second.crossItr==1) &&          (Vec_r2Ver[lookupIdx].second.Occupied==1) && (Vec_r2Ver[lookupIdx].second.Occupied_backup ==0))
                allocatedPtr = (void*)((char*)Vec_r2Ver[lookupIdx].second.ptr+offset_cross*sizeof(char));
                Vec_r2Ver[lookupIdx].second.Occupied_backup=1;
                Table_p2r[allocatedPtr]=lookupIdx;
                //cout<<gc<<" condition M4 "<<allocatedPtr<<endl;
                //update load
                if(loadLogFlag==1){
                    Table_load[gc]=make_pair(Table_load.find(gc-1)->second.first,Table_load.find(gc-1)->second.second+size);
                }
            }
        } else{
            // size not proper or both occupied,normal train iterations
            // never falls in here.
            // falling in here means test iterations.
            mallocFlag=0;
            allocatedPtr = malloc(size);
            //update load
            if(loadLogFlag==1){
                Table_load[gc]=make_pair(Table_load.find(gc-1)->second.first+size,Table_load.find(gc-1)->second.second);
            }
            //cout<<gc<<" condition M3"<<endl;
            
            // clear Tables and vectors for next table build up.
            //cout<<"size of Table_p2r: "<<Table_p2r.size()<<endl;
            //Table_p2r no need to clear.
            vec.clear();
            Vec_r2Ver.clear();
            Table_r2d.clear();
            Table_d2r.clear();
            Table_p2s.clear();
            old_location =location;
            old_maxLen =maxLen;
            maxLen =0; location = 0;
            idxRange=0; //TODO(junzhe) verify if needed here.
            globeCounter = -1;
            gc_start_count =gc+1;
            
        }
    } //c.
    
    ///4. test repeated sequence every 100 blocks, update globeCounter.
    if (((gc+1)%300==0) && (mallocFlag==0) && (globeCounter==-1)&&(gc+2>checkPoint)&&(vec.size()>100)){
        Test();
    }
    
    ///get load info, after gc == GC+2maxLen TODO(junzhe) to make it only once
    if (gc==(globeCounter+2*maxLen)&& (globeCounter>0)){
        getMaxLoad();
        loadLogFlag=0;
    }
    
    gc++;
    Table_p2s[allocatedPtr]=size;
    
    *ptr = allocatedPtr;
}


///Free
void SmartMemPool::Free(void* ptr){

    size_t deallocatedSize = Table_p2s.find(ptr)->second;
    if (mallocFlag==0){
        //push_back the string for later test and run.
        string tempStr1 ="Free ";
        stringstream strm2;
        strm2<<ptr;
        string tempStr2 = strm2.str();
        string temp = tempStr1+tempStr2;
        vec.push_back(temp);
        
        
        //update load before free
        if(loadLogFlag==1){
            Table_load[gc]=make_pair(Table_load.find(gc-1)->second.first-deallocatedSize,Table_load.find(gc-1)->second.second);
        }
        if (!(Table_p2r.find(ptr)==Table_p2r.end())){ //via Table
            Table_p2r.erase(ptr);
            //cout<<gc<<" condition mixed, before mallocFlag change."<<endl;
        }else {
            /// before flag switch, for sure all free shall be done by free()
            free(ptr);
            //cout<<gc<<" condition F1 "<<endl;
        }
    }else{ //free from Table
        if (!(Table_p2r.find(ptr)==Table_p2r.end())){ //via Table
            int resp_rIdx = Table_p2r.find(ptr)->second;
            Table_p2r.erase(ptr);
      
            if (ptr == Vec_r2Ver[resp_rIdx].second.ptr){
                Vec_r2Ver[resp_rIdx].second.Occupied =0; //freed, able to allocate again.
                //cout<<gc<<" condition F2 : primary location"<<endl;
            }else if (ptr == (void*)((char*)Vec_r2Ver[resp_rIdx].second.ptr+offset_cross*sizeof(char))){
                Vec_r2Ver[resp_rIdx].second.Occupied_backup =0; //freed, able to allocate again.
                //cout<<gc<<" condition F4 : backup location"<<endl;
         
            }else {
                if (((float)((char*)ptr-((char*)ptrPool_cross+offset_cross*sizeof(char)))>0) && ((float)((char*)ptr-((char*)ptrPool_cross+2*offset_cross*sizeof(char)))<0)){
                    Vec_r2Ver[resp_rIdx].second.Occupied_backup =0;
                }else{
                    Vec_r2Ver[resp_rIdx].second.Occupied =0;
                }
            }
            //update load
            if(loadLogFlag==1){
                Table_load[gc]=make_pair(Table_load.find(gc-1)->second.first,Table_load.find(gc-1)->second.second-deallocatedSize);
            }
        }else{ // not via Table.
            //update load
            if(loadLogFlag==1){
                Table_load[gc]=make_pair(Table_load.find(gc-1)->second.first-deallocatedSize,Table_load.find(gc-1)->second.second);
            }
            free(ptr);
            
            //cout<<gc<<" condition F3 "<<endl;
        }
    }
    
    if (((gc+1)%300==0) && (mallocFlag==0) && (globeCounter==-1)&&(gc+2>checkPoint)&&(vec.size()>100)){
        Test();
    }
    
    gc++;
    
}


SmartMemPool::~SmartMemPool(){
    free(ptrPool_normal);
    free(ptrPool_cross);
}

void SmartMemPool::getMaxLoad(){
    
    vector<size_t>cudaLoadLog;
    for (int i=0; i<Table_load.size();i++){
        cudaLoadLog.push_back(Table_load.find(i)->second.first);
    }
    size_t maxCudaLoad = *max_element(cudaLoadLog.begin(),cudaLoadLog.end());
    int idxMaxCudaLoad = static_cast<int>(distance(cudaLoadLog.begin(),max_element(cudaLoadLog.begin(),cudaLoadLog.end())));
    
    vector<size_t>colorLoadLog;
    for (int i=0; i<Table_load.size();i++){
        colorLoadLog.push_back(Table_load.find(i)->second.second);
    }
    size_t maxColorLoad = *max_element(colorLoadLog.begin(),colorLoadLog.end());
    int idxMaxColorLoad = static_cast<int>(distance(colorLoadLog.begin(),max_element(colorLoadLog.begin(),colorLoadLog.end())));
    size_t offsetCudaLoad = Table_load.find(idxMaxColorLoad)->second.first;
    
    size_t maxTotalLoad = max(maxCudaLoad,maxColorLoad+offsetCudaLoad);
    size_t maxMemUsage = max(maxCudaLoad,offset_normal+offset_cross+offsetCudaLoad);
    float memRatio = (float)maxMemUsage/(float)maxTotalLoad;
    
    cout<<"=============================memory usage stats print: ================================"<<endl;
    cout<<"maxColorLoad vs memPoolSize: (at idx "<<idxMaxColorLoad<<")"<<endl;
    cout<<maxColorLoad<<endl;
    cout<<offset_cross+offset_normal<<endl;
    cout<<"maxTotalLoad vs maxCudaLoad(at idx "<<idxMaxCudaLoad<<") maxMemUsage"<<endl;
    cout<<maxTotalLoad<<endl;
    cout<<maxCudaLoad<<endl;
    cout<<maxMemUsage<<endl;
    cout<<"memRatio: "<<memRatio<<endl;
    
}

void SmartMemPool::Test(){
    int idxRange3=0; //rename TODO
    vector<onePieceMsg>onePieceMsgVec_3 =strVec_2_pieceMsgVec(vec,idxRange3);
    pair<vector<onePairMsg>,vector<onePairMsg>>pairOfPairMsgVec_=pieceMsgVec_2_pairOfPairMsgVec(onePieceMsgVec_3,idxRange3);
    vector<size_t>rep=pairOfPairMsgVec_2_repSeq(pairOfPairMsgVec_);
    repPatternDetector(rep,maxLen,location);
    if (maxLen>50){
        if (first_location == 1){//location, GC reliable at first test.
            first_location=0;
        }else{
            location = gc_start_count+(maxLen-(gc_start_count-old_location)%old_maxLen);
        }
        globeCounter =maxLen-(gc+1-location)%maxLen+gc+1;
    }

    //cout<<"test done: gc, GC, gc_start_count location, maxLen, vec len,"<<endl;
    //cout<<gc<<' '<< globeCounter<<' '<<gc_start_count<<' '<<location<<' '<< maxLen<<' '<<vec.size()<<endl;
    //checkPoint=checkPoint*2;//d. do it every 300 TODO(junzhe)
}
