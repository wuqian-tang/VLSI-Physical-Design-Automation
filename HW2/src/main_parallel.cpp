#include "bits/stdc++.h"
#include "atomic"
#include "thread"
#include "mutex"
using namespace std;
const int maxn=2e5+10;
struct Cell{
    int size,isleft=0,ismoved=0;
    atomic<int> gain=0;
    vector<int> inets;
}cells[maxn];
int sumsize=0,sumsizeL=0;
struct Net{
    int cntL=0;
    vector<int> icells;
}nets[maxn];
int cutsize=0;
vector<int> icells,inets;
struct cmp{
    bool operator()(const pair<int,int>&p1,const pair<int,int> &p2){
        return p1.second<p2.second;
    }
};
priority_queue<pair<int,int>,deque<pair<int,int>>,cmp> maxGain;
int dep;
void input(const string &netpath,const string &cellpath){
    ifstream fin(cellpath);
    char c;
    for(int cidx,csize;fin>>c>>cidx>>csize;){
        icells.push_back(cidx);
        cells[cidx].size=csize;
        sumsize+=csize;
    }
    fin.close();
    fin.open(netpath);
    for(int nidx,cidx;fin>>c>>c>>c>>c>>nidx;){
        fin>>c;
        for(;fin>>c&&c=='c';){
            fin>>cidx;
            cells[cidx].inets.push_back(nidx);
            nets[nidx].icells.push_back(cidx);
        }
        if(nets[nidx].icells.size()>1)inets.push_back(nidx);
    }
    fin.close();
}
mutex mtxMaxGain;
void init_partition(){
    srand(time(0));
    random_shuffle(icells.begin(),icells.end());
    for(int cidx:icells){
        if(sumsizeL*2<sumsize){
            cells[cidx].isleft=1;
            sumsizeL+=cells[cidx].size;
            for(int nidx:cells[cidx].inets){
                ++nets[nidx].cntL;
            }
        }
    }
    //*/
    //-----compute cutsize------
    for(int i:inets){
        int cntL=nets[i].cntL,cntR=nets[i].icells.size()-nets[i].cntL;
        cutsize+=cntL&&cntR;
    }
    //---compute gain----
    function calcGain=[](int cidx){
        for(int nidx:cells[cidx].inets){
            int cntL=nets[nidx].cntL,cntR=nets[nidx].icells.size()-nets[nidx].cntL;
            if(cells[cidx].isleft&&cntL==1||!cells[cidx].isleft&&cntR==1)++cells[cidx].gain;
            if(cells[cidx].isleft&&cntR==0||!cells[cidx].isleft&&cntL==0)--cells[cidx].gain;        
        }
        mtxMaxGain.lock();
        if(cells[cidx].gain>=0)maxGain.emplace(cidx,cells[cidx].gain);
        mtxMaxGain.unlock();
    };
    for(int cidx:icells){
        thread t(calcGain,cidx);
        t.join();
    }
    //------------
}
mutex mtx;
void updateGain(int nidx,int isleft,set<int> &gain_changed,int cidx){
    vector<int> ggain_changed;
    int &cntL=nets[nidx].cntL;
    cntL+=(isleft<<1)-1;
    int cntR=nets[nidx].icells.size()-cntL;
    if(cntL==0||cntR==0){
        for(int tcidx:nets[nidx].icells){
            --cells[tcidx].gain;
            ggain_changed.push_back(tcidx);
        }
    }
    if(cntL==1&&isleft||cntR==1&&!isleft){
        for(int tcidx:nets[nidx].icells){
            ++cells[tcidx].gain;
            ggain_changed.push_back(tcidx);
        }
    }
    if(!(cntL==1&&!isleft||cntR==1&&isleft||cntL==2&&isleft||cntR==2&&!isleft))return;
    vector<int>L,R;
    for(int ci:nets[nidx].icells){
        if(ci==cidx)continue;
        if(cells[ci].isleft)L.push_back(ci);
        else R.push_back(ci);
    }
    if(cntL==1&&!isleft||cntR==1&&isleft){
        int tcidx=isleft?R[0]:L[0];
        ++cells[tcidx].gain;
        ggain_changed.push_back(tcidx);
    }
    if(cntL==2&&isleft||cntR==2&&!isleft){
        int tcidx=isleft?L[0]:R[0];
        --cells[tcidx].gain;
        ggain_changed.push_back(tcidx);
    }
    mtx.lock();
    for(int i:ggain_changed)gain_changed.insert(i);
    mtx.unlock();
}
bool moveCell(int cidx){
    int sumL=sumsizeL;
    sumL-=((cells[cidx].isleft<<1)-1)*cells[cidx].size;
    int sumR=sumsize-sumL;
    if(!(abs(sumL-sumR)*10<sumsize))return false;
    int &isleft=cells[cidx].isleft;
    isleft^=1;
    cells[cidx].ismoved=dep+1;
    int cidxgain=-cells[cidx].gain;
    sumsizeL=sumL;
    cutsize-=cells[cidx].gain;
    set<int> gain_changed;
    for(int nidx:cells[cidx].inets){
        thread t(updateGain,nidx,isleft,ref(gain_changed),cidx);
        t.join();
    }
    cells[cidx].gain=cidxgain;
    for(int ci:gain_changed){
        if(cells[ci].ismoved!=dep+1){
            if(cells[ci].gain>=0)maxGain.emplace(ci,cells[ci].gain);
        }
    }
    return true;
}
void process(){
    init_partition();
    stack<int> chosen;
    for(;maxGain.size();){
        while(chosen.size()){
            int cidx=chosen.top();chosen.pop();
            if(cells[cidx].gain>=0)maxGain.emplace(cidx,cells[cidx].gain);
        }
        bool ismoved;
        do{
            if(maxGain.empty())break;
            auto[cidx,gain]=maxGain.top();maxGain.pop();
            if(gain!=cells[cidx].gain)continue;
            ismoved=moveCell(cidx);
            if(!ismoved)chosen.push(cidx);
        }while(!ismoved);
    }
}
void output(const string &path){
    vector<int>L,R;
    for(int i:icells){
        if(cells[i].isleft)L.push_back(i);
        else R.push_back(i);
    }
    ofstream fout(path);
    fout<<"cut_size "<<cutsize<<endl;
    fout<<"A "<<L.size()<<endl;
    for(int i:L)fout<<'c'<<i<<endl;
    fout<<"B "<<R.size()<<endl;
    for(int i:R)fout<<'c'<<i<<endl;
}
int main(int argc,char **argv){
    ios::sync_with_stdio(0),cin.tie(0);
    clock_t beg=clock();
    input(argv[1],argv[2]);
    float itime=float(clock()-beg)/1000000;
    beg=clock();
    process();
    float ptime=float(clock()-beg)/1000000;
    beg=clock();
    output(argv[3]);
    float otime=float(clock()-beg)/1000000;
    cout<<"min cutsize: "<<cutsize<<endl;
    cout<<"I/O time: "<<itime+otime<<endl;
    cout<<"computation time: "<<ptime<<endl;
}