
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <stdlib.h>
#include <map>
using namespace std;

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
    double t;
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
     format of onePieceMsg [ptr, size/-1, flag, idx, timestamp]
     flag: 1 for malloc, -1 for free, 2 for mutable/read, 3 for layer
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
            double tempTime;
            stringstream convert2(v[3]);
            convert2>>tempTime;
            tempMsg.t =tempTime;
            onePieceMsgVec_.push_back(tempMsg);
        }else if (v[0]=="Free"){
            onePieceMsg tempMsg(v[1],-1, -1, i);
            double tempTime;
            stringstream convert2(v[2]);
            convert2>>tempTime;
            tempMsg.t =tempTime;
            onePieceMsgVec_.push_back(tempMsg);
        }else if (v[0]=="Mutable" or v[0]=="Read"){
            onePieceMsg tempMsg(v[1],-1, 2, i);
            double tempTime;
            stringstream convert2(v[2]);
            convert2>>tempTime;
            tempMsg.t =tempTime;
            onePieceMsgVec_.push_back(tempMsg);
        }else if (v[0]=="Layer"){
            onePieceMsg tempMsg(v[1],-1, 3, i);
            double tempTime;
            stringstream convert2(v[2]);
            convert2>>tempTime;
            tempMsg.t =tempTime;
            onePieceMsgVec_.push_back(tempMsg);
        } else{
            cout<<"error for process the onePriceMsg."<<endl;
            cout<<vec[i]<<endl;
        }
        //cout<<onePieceMsgVec_[i].t<<" time if in double"<<endl;
//        if (i>0){
//            cout<<onePieceMsgVec_[i].t-onePieceMsgVec_[i-1].t<<" delta"<<endl;
//        }
    }
    
    sort(onePieceMsgVec_.begin(),onePieceMsgVec_.end(),less_than_ptrIdx());
    idxRange = static_cast<int>(onePieceMsgVec_.size());
    ///add size for non-malloc block info
    size_t tempSize=0;
    for (int i=0;i<onePieceMsgVec_.size();i++){
        if (onePieceMsgVec_[i].MallocFree==1){
            tempSize = onePieceMsgVec_[i].size;
        } else {
            onePieceMsgVec_[i].size = tempSize;
        }
    }

    return onePieceMsgVec_;
}// end of strVec_2_pieceMsgVec function


vector<size_t> Swap_piece2rep (vector<onePieceMsg>onePieceMsgVec_){
    vector<oneIterMsg>oneIterMsgVec_;
    string tempStr;
    int tempIdx=0;
    for (int i=0;i<onePieceMsgVec_.size();i++){
        if (onePieceMsgVec_[i].MallocFree==1){
            //update tempStr and idx.
            tempStr = onePieceMsgVec_[i].ptr;
            tempIdx = onePieceMsgVec_[i].idx;
            oneIterMsg tempMsg(onePieceMsgVec_[i].size,1,onePieceMsgVec_[i].idx);
            oneIterMsgVec_.push_back(tempMsg);
        } else {
            oneIterMsg tempMsg(onePieceMsgVec_[i].idx-tempIdx,onePieceMsgVec_[i].MallocFree,onePieceMsgVec_[i].idx);
            tempIdx = onePieceMsgVec_[i].idx;
            oneIterMsgVec_.push_back(tempMsg);
        }
        //cout<<oneIterMsgVec_[i].size_delta<<' '<<oneIterMsgVec_[i].MallocFree<<' '<<oneIterMsgVec_[i].idx<<endl;
    }
    
    sort(oneIterMsgVec_.begin(),oneIterMsgVec_.end(),less_than_iterIdx());
    //only after sort then can create rep.
    vector<size_t>rep; // vector of size_delta, name it as rep for simlisity.
    for (int i =0; i<oneIterMsgVec_.size(); i++){
        rep.push_back(oneIterMsgVec_[i].size_delta);
        //cout<<rep[i]<<endl;
    }
    cout<<"rep size: "<<rep.size()<<endl;
    return rep;
}
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
                // cout<<"maxLen increased, lcoation and maxLen: ("<<location<<","<<maxLen<<")"<<endl;
            }
        }
    }
}// end of repPatternDetector

struct less_than_Idx{
    /*
     sort onePieceMsg by ptr and then idx.
     */
    inline bool operator() (const onePieceMsg& struct1, const onePieceMsg& struct2)
    {
        return (struct1.idx<struct2.idx);
    }
};

void compute_peak(vector<onePieceMsg> vec_run, int& max_Idx, size_t& max_Load ){
    //sort by idx
    int maxIdx =0;
    size_t maxLoad =0;
    size_t load=0;
    cout<<"================================ print vec_run idx=========="<<endl;
    for (int i=0; i<vec_run.size(); i++){
        //cout<<vec_run[i].idx<<" "<<vec_run[i].MallocFree<<" "<<vec_run[i].size<<endl;
        if(vec_run[i].MallocFree==1){
            load=load+vec_run[i].size;
        }else if (vec_run[i].MallocFree==-1){
            load=load-vec_run[i].size; //TODO(junzhe) size
        }
        if (maxLoad<load){
            maxLoad =load;
            maxIdx = i;
        }
        //cout<<"load "<<i<<' '<<load<<" mallocFree: "<<vec_run[i].MallocFree<<endl;
    }
    max_Idx = maxIdx;
    max_Load = maxLoad;
}

int SwapInTime(size_t size){
    //yet to get the formula
    int ans =0;
    if (size==0) {ans = 9500;} else {ans = 0.13*size;}
    return ans;
}
int SwapOutTime(size_t size){
    int ans =0;
    if (size==0) {ans = 17000;} else {ans = 0.29*size;}
    return ans;
}


struct onePairMsg_Swap{
    // more attributes, name different, use ptr.
    /*
     members: [name (r_idx), size, r_idx, d_idx]
     */
    string ptr;
    int name;
    size_t size;
    int r_idx; //out idx
    int d_idx; //in idx
    double r_time; // out time
    double d_time; //in time
    double dt_o; //delta t: t2-t1
    double dt; //delta t: t2'-t1'
    double dt_p; //delta t prime, removing latency
    double pri;  //look at here if big enough TODO(junzhe)
    //below as per planned.
    int i1;
    int i1p;
    int i2;
    int i2p;
    double t1;
    double t2;
    double t1p;
    double t2p;
    //onePairMsg(int n,size_t s, int r,int d):name(n),size(s),r_idx(r),d_idx(d){}
    onePairMsg_Swap(string p, size_t s, int i1, int i2, double t1, double t2): ptr(p), size(s), r_idx(i1),d_idx(i2),r_time(t1), d_time(t2) {}
};

struct less_than_dt{
    /*
     sort onePairMsg_Swap by dt, descending
     */
    inline bool operator() (const onePairMsg_Swap& struct1, const onePairMsg_Swap& struct2)
    {
        return (struct1.dt>struct2.dt);
    }
};

struct less_than_pri{
    /*
     sort onePairMsg_Swap by pri, descending
     */
    inline bool operator() (const onePairMsg_Swap& struct1, const onePairMsg_Swap& struct2)
    {
        return (struct1.pri>struct2.pri);
    }
};

struct less_than_Idx_Swap{
    /*
     sort onePieceMsg_Swap by idx.
     */
    inline bool operator() (const onePairMsg_Swap& struct1, const onePairMsg_Swap& struct2)
    {
        return (struct1.r_idx<struct2.r_idx);
    }
};

struct less_than_Idx_Swap_rvs{
    /*
     sort onePieceMsg_Swap by idx. reverse
     */
    inline bool operator() (const onePairMsg_Swap& struct1, const onePairMsg_Swap& struct2)
    {
        return (struct1.d_idx>struct2.d_idx);
    }
};
int main() {
    ///load blockInfo
    string fileName = "blockInfo_6_itr_time_Alex.text";
    vector<string>vec_str = file_2_strVec(fileName);
//    cout<<"example of one vec string"<<endl;
//    cout<<vec[0]<<endl;
//    cout<<vec[1]<<endl;
    
    ///vec_str to vec_pieceMsg, sort by ptr and idx.
    int idxRange =0;
    vector<onePieceMsg> vec_pieceMsg = strVec_2_pieceMsgVec(vec_str,idxRange);
    cout<<"size of vec_pieceMsg and vec_str are: "<<vec_pieceMsg.size()<<' '<<vec_str.size()<<endl;
    //print onePieceMsg - tested, sort by ptr and idx.
//    for (int i =0; i<vec_pieceMsg.size();i++){
//        cout<<vec_pieceMsg[i].ptr<<' '<<vec_pieceMsg[i].idx<<' '<<vec_pieceMsg[i].MallocFree<<' '<<vec_pieceMsg[i].size<<endl;
//    }
    
    ///rep test
    vector<size_t> vec_rep = Swap_piece2rep(vec_pieceMsg);
    //int idxRange3=0; //rename TODO
    int maxLen=0, location =0;
    repPatternDetector(vec_rep,maxLen,location);
    cout<<"maxLen and location are: "<<maxLen<<' '<<location<<endl;
    cout<<"test rep"<<endl;
    int i= 1200, len=596;
    //modify the location
    i = 1213;
    //test rep.
    if(equal(vec_rep.begin()+i,vec_rep.begin()+i-1+len,vec_rep.begin()+i+len)){
        cout<<"------int the loop"<<endl; //test was correct also
    }
    
    ///cut into one iteration,
    sort(vec_pieceMsg.begin(),vec_pieceMsg.end(),less_than_Idx());
    vector<onePieceMsg>vec_run(&vec_pieceMsg[location],&vec_pieceMsg[location+maxLen]);
    cout<<"time for one itr: "<<vec_run[vec_run.size()-1].t-vec_run[0].t<<endl;
    cout<<"time for 1MB out and in: "<<SwapOutTime(1000000)<<' '<<SwapInTime(1000000)<<endl;
    cout<<"time for 3MB out and in: "<<SwapOutTime(3000000)<<' '<<SwapInTime(3000000)<<endl;
    //scale down idx
    for (int i=0; i<maxLen;i++){
        vec_run[i].idx = vec_run[i].idx - location;
    }
    
    ///get peak and idx
    int maxIdx =0;
    size_t maxLoad =0;
    compute_peak(vec_run, maxIdx, maxLoad);
    cout<<"maxIdx and maxLoad are: "<<maxIdx<<' '<<maxLoad<<endl; //144443088
    size_t memUsage = 25000000; //75% of the maxLoad, for example.
    //sort by ptr & idx
    sort(vec_run.begin(),vec_run.end(),less_than_ptrIdx());
    //TODO(junzhe) pay attention, idx is from 1200. in order of idx
    for (int i =1; i<vec_run.size();i++){
//        cout<<vec_run[i].ptr<<' '<<vec_run[i].idx<<' '<<vec_run[i].MallocFree<<' '<<vec_run[i].size<<' '<<vec_run[i].t<<endl;
//        cout<<vec_run[i].t-vec_run[i-1].t<<endl;
        //above shows the timestamps are correct.
    }
    //feed in one iteration
    int L_out = SwapOutTime(0);
    int L_in = SwapInTime(0);
    vector<onePairMsg_Swap>vec_swap;
    size_t sumSizeSwap =0;
    ///formulate swap items.
    for (int i =1; i<vec_run.size(); i++){
        //condition for selecting condidates: 3->2, cross peak
        if ((vec_run[i-1].idx<maxIdx) && (vec_run[i].idx>maxIdx) && (vec_run[i-1].ptr ==vec_run[i].ptr) && (vec_run[i-1].MallocFree==3)&&(vec_run[i].MallocFree==2)){
            onePairMsg_Swap tempSwap(vec_run[i].ptr,vec_run[i].size,vec_run[i-1].idx, vec_run[i].idx, vec_run[i-1].t, vec_run[i].t);
            //TODO(junzhe) time difference issue.
            tempSwap.dt_o = tempSwap.d_time-tempSwap.r_time;
            tempSwap.dt = tempSwap.d_time-tempSwap.r_time-SwapOutTime(tempSwap.size)-SwapOutTime(tempSwap.size);
            tempSwap.dt_p = tempSwap.dt - L_out - L_in;
            tempSwap.pri = tempSwap.dt * tempSwap.size;
            //cout<<"delta idx and delta time: "<<vec_run[i].idx-vec_run[i-1].idx<<' '<<vec_run[i].t-vec_run[i-1].t<<endl;
            //cout<<tempSwap.d_time-tempSwap.r_time<<endl; //TODO(junzhe) verify. t.
            //onePairMsg_Swap(string p, size_t s, int i1, int i2, int t1, int t2): ptr(p), size(s), r_idx(r),d_idx(d),r_time(t1), d_time(t2) {}
            //cout<<tempSwap.size<<' '<<tempSwap.r_idx<<' '<<tempSwap.d_idx<<"||"<<tempSwap.r_time<<' '<<tempSwap.d_time<<"||"<<tempSwap.dt_o<<' '<<tempSwap.dt<<' '<<tempSwap.dt_p<<endl;
            ///print
            cout<<tempSwap.size<<' '<<tempSwap.r_idx<<' '<<tempSwap.d_idx<<"||"<<tempSwap.dt<<' '<<tempSwap.pri<<endl;
            //TODO(junzhe) all positve here.
            vec_swap.push_back(tempSwap);
            sumSizeSwap+=tempSwap.size;
        }
    }
    ///select the top a few that can meet swap load
    //TODO(junzhe) optimize the for loop.
    cout<<"============== select top a few to swap"<<endl;
    sort(vec_swap.begin(),vec_swap.end(),less_than_pri());
    vector<onePairMsg_Swap>vec_swap_selct;
    size_t sumSizeSwap2=0;
    for (int i =0; i<vec_swap.size(); i++){
        if (sumSizeSwap2<memUsage){
            vec_swap_selct.push_back(vec_swap[i]);
            sumSizeSwap2+=vec_swap[i].size;
            //cout<<vec_swap[i].size<<' '<<vec_swap[i].r_idx<<' '<<vec_swap[i].d_idx<<"||"<<vec_swap[i].dt<<' '<<vec_swap[i].pri<<endl;
            //cout<<"size is: "<<sumSizeSwap2<<endl;
        } else {
            cout<<"to break"<<endl;
            break;
        }
    }
    ///planing the swap at point of time.
    cout<<"===========================planning the swap idx"<<endl;
    sort(vec_swap_selct.begin(),vec_swap_selct.end(),less_than_Idx_Swap());
    sort(vec_run.begin(),vec_run.end(),less_than_Idx());
    ///t1 and t1', i1 and i1'
    for (int i =0; i<vec_swap_selct.size(); i++){
        int tempIdx=vec_swap_selct[i].r_idx;
        if ((i>0) and (tempIdx<=vec_swap_selct[i-1].i1p)){
            //last t1' bigger than this t1
            tempIdx = vec_swap_selct[i-1].i1p;
        } else {
            //round to next Malloc/Free
            while ((vec_run[tempIdx].MallocFree!=1) and (vec_run[tempIdx].MallocFree!=-1)){
                tempIdx++;
            }
        }
        //update t1, t1', i1
        vec_swap_selct[i].i1=vec_run[tempIdx].idx;
        vec_swap_selct[i].t1=vec_run[tempIdx].t;
        vec_swap_selct[i].t1p = vec_swap_selct[i].t1+SwapOutTime(vec_swap_selct[i].size);
        //update i1'
        while ((vec_swap_selct[i].t1p>=vec_run[tempIdx].t) or ((vec_run[tempIdx].MallocFree!=1) and (vec_run[tempIdx].MallocFree!=-1))) {
            tempIdx++; //TODO(junzhe) can speed up
        }
        vec_swap_selct[i].i1p = tempIdx;//Note: here i1' is immediately at Malloc/Free.
    } //for loop
    
    ///t2 and t2', i2 and i2'
    sort(vec_swap_selct.begin(),vec_swap_selct.end(),less_than_Idx_Swap_rvs());
    double overhead=0;
    for (int i =0; i<vec_swap_selct.size(); i++){
        int tempIdx=vec_swap_selct[i].d_idx;
        double tempTime;
        //condition
        if ((i>0) and (tempIdx>vec_swap_selct[i-1].i2p)){
            tempIdx = vec_swap_selct[i-1].i2p;
            tempTime = vec_run[tempIdx].t - SwapInTime(vec_swap_selct[i].size);
        } else{
            tempTime = vec_swap_selct[i].d_time - SwapInTime(vec_swap_selct[i].size);
        }
        //update i2p, t2p; not used for i2 and t2.
        while ((tempTime<=vec_run[tempIdx].t) or ((vec_run[tempIdx].MallocFree!=1) and (vec_run[tempIdx].MallocFree!=-1))) {
            tempIdx--; //TODO(junzhe) can speed up
        }
        vec_swap_selct[i].i2p = vec_run[tempIdx].idx;
        vec_swap_selct[i].t2p = vec_run[tempIdx].t;
    }
    
    cout<<"size of Vec_run is: "<<vec_run.size()<<endl;
    cout<<"size of vec_swap is: "<<vec_swap.size()<<endl;
    cout<<"size of sumSwap is: "<<sumSizeSwap<<endl;
    cout<<"size of maxload is: "<<maxLoad<<endl;

    cout<<"done"<<endl;
    

    
    return 0;
}

