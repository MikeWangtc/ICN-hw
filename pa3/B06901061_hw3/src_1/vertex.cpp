#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <utility>
#include <tuple>
#include <algorithm>
#include <cassert>
#include "vertex.h"

ostream& operator << (ostream& os, const Vertex& v){
    vector<tuple<Vertex*, int> > outEdge = v.getOutEdge();
    os << "Router [" << v.getID() + 1 << "] " << "\n";
    for( int j = 0, n = outEdge.size() ; j < n ; ++j ){
        Vertex* out = get< 0 >(outEdge[j]);
        os << out -> getID() + 1 << " " << v.getWeight(out) << "\n";
    }
    return os;
}

//--------------//
//   getter     //
//--------------//
unsigned Vertex:: getID() const{ 
    return _id; 
}
unsigned Vertex:: getCost() const{
    return _cost;
}
bool Vertex:: getVisit() const{
    return _visit;
}
Vertex* Vertex:: getFromVertex() const{
    return _from;
}
int Vertex:: getWeight(Vertex* vptr) const{
    for( int i = 0 ; i < _outEdge.size() ; ++i )
        if( get< 0 >(_outEdge[i]) == vptr) 
            return get< 1 >(_outEdge[i]);
    return INT_MIN;
}

vector<tuple<Vertex*, int> > Vertex:: getOutEdge() const{
    return _outEdge;
}
void Vertex:: setCost(unsigned cost){
    _cost = cost;
}
void Vertex:: setVisit(bool val){
    _visit = val;
}
void Vertex:: setFromVertex(Vertex* vptr){
    _from = vptr;
}
bool Vertex:: isVoid(){
    return _id == UINT_MAX;
}

void Vertex:: buildOutEdge(Vertex* vptr, int w){
    _outEdge.push_back( make_tuple( vptr, w ) );
}

void Vertex:: removeOutEdge(Vertex* vptr){
    vector<tuple<Vertex*, int> >::iterator cur = _outEdge.begin();
    while(cur != _outEdge.end()){
        if(get< 0 >(*cur) == vptr){
            cout << "Successfully remove [" << vptr -> getID() + 1 << "] ";
            cout << "from [" << getID() + 1 << "]\n";
            _outEdge.erase(cur);
            break;
        }
        ++cur;
    }
}

vector<Vertex*> Vertex:: relax(){
    vector<Vertex*> v_relaxed;
    for( int i = 0, n = _outEdge.size(); i < n ; ++i ){
        Vertex* out_vptr = get< 0 >(_outEdge[i]);
        unsigned new_cost = get< 1 >(_outEdge[i]) + getCost();
        
        if(out_vptr -> getCost() > new_cost){
            out_vptr -> setCost(new_cost);
            out_vptr -> setFromVertex(this);
            v_relaxed.push_back(out_vptr);
        }
    }
    return v_relaxed;
}
