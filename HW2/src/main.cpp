#include "bits/stdc++.h"
using namespace std;
const int maxn=2e5+10;
struct Cell{
    int size,isleft=0,gain=0,ismoved=0;
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
void init_partition(){
    for(int i:icells){
        if(cells[i].isleft)continue;
        for(int j:cells[i].inets){
            if(nets[j].cntL==nets[j].icells.size())continue;
            for(int cidx:nets[j].icells){
                if(cells[cidx].isleft)continue;
                if(sumsizeL*2<sumsize){
                    cells[cidx].isleft=1;
                    sumsizeL+=cells[cidx].size;
                    for(int nidx:cells[cidx].inets){
                        ++nets[nidx].cntL;
                    }
                }
                else{
                    goto loop;
                }
            }
        }
    }
    loop:;
    //*/
    //-----compute cutsize------
    for(int i:inets){
        int cntL=nets[i].cntL,cntR=nets[i].icells.size()-nets[i].cntL;
        cutsize+=cntL&&cntR;
    }
    for(int cidx:icells){
        for(int nidx:cells[cidx].inets){
            int cntL=nets[nidx].cntL,cntR=nets[nidx].icells.size()-nets[nidx].cntL;
            if(cells[cidx].isleft&&cntL==1||!cells[cidx].isleft&&cntR==1)++cells[cidx].gain;
            if(cells[cidx].isleft&&cntR==0||!cells[cidx].isleft&&cntL==0)--cells[cidx].gain;
        }
        if(cells[cidx].gain>0)maxGain.emplace(cidx,cells[cidx].gain);
    }
    //------------
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
        int &cntL=nets[nidx].cntL;
        cntL+=(isleft<<1)-1;
        int cntR=nets[nidx].icells.size()-cntL;
        if(cntL==0||cntR==0){
            for(int tcidx:nets[nidx].icells){
                --cells[tcidx].gain;
                gain_changed.insert(tcidx);
            }
        }
        if(cntL==1&&isleft||cntR==1&&!isleft){
            for(int tcidx:nets[nidx].icells){
                ++cells[tcidx].gain;
                gain_changed.insert(tcidx);
            }
        }
        if(!(cntL==1&&!isleft||cntR==1&&isleft||cntL==2&&isleft||cntR==2&&!isleft))continue;
        vector<int>L,R;
        for(int ci:nets[nidx].icells){
            if(ci==cidx)continue;
            if(cells[ci].isleft)L.push_back(ci);
            else R.push_back(ci);
        }
        if(cntL==1&&!isleft||cntR==1&&isleft){
            int tcidx=isleft?R[0]:L[0];
            ++cells[tcidx].gain;
            gain_changed.insert(tcidx);
        }
        if(cntL==2&&isleft||cntR==2&&!isleft){
            int tcidx=isleft?L[0]:R[0];
            --cells[tcidx].gain;
            gain_changed.insert(tcidx);
        }
    }
    cells[cidx].gain=cidxgain;
    for(int ci:gain_changed){
        if(cells[ci].ismoved!=dep+1){
            if(cells[ci].gain>0)maxGain.emplace(ci,cells[ci].gain);
        }
    }
    return true;
}
void process(){
    init_partition();
    for(dep=0;dep<30000&&maxGain.size();++dep){
        stack<int> chosen;
        for(;maxGain.size();){
            while(chosen.size()){
                int cidx=chosen.top();chosen.pop();
                if(cells[cidx].gain>0)maxGain.emplace(cidx,cells[cidx].gain);
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
        chosen={};
        maxGain={};
        for(int maxMove=0;maxGain.empty()&&maxMove<10;++maxMove){
            for(int cidx:icells){
                if(cells[cidx].gain==0){
                    moveCell(cidx);
                }
            }
        }
        //cout<<dep<<' '<<maxGain.size()<<' '<<cutsize<<endl;
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