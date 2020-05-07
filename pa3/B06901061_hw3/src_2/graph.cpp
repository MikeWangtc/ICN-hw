#include <vector>
#include <algorithm>
#include <string>
#include <tuple>
#include <climits>
#include <cassert>
#include <ctime>
#include <queue> 
#include <fstream>
#include "graph.h"
#include "vertex.h"

using namespace std;
struct Comparator
{
    bool operator()(const Vertex* v1, const Vertex* v2){
        return v1 -> getCost() > v2 -> getCost();
    }
};

void Graph::print(){
    for( int i = 0, n = _graph.size() ; i < n ; ++i ){
        if(!_graph[i].isVoid()){
            // cout << _graph[i] << endl;
            cout << "Routing table of router [" << _graph[i].getID() + 1 << "] " << "\n";
            for( int j = 0 ; j < n ; ++j ){
                if(!_graph[j].isVoid()){
                    cout << get<0>(_forward_table[i][j]) << " " << get<1>(_forward_table[i][j]) + 1 << endl;
                }
            }
        }
    }
}

void Graph:: buildGraph(vector<vector<int> > cost_table){
    assert(cost_table.size() == cost_table[0].size());
    int n_vertex = cost_table.size();
    
    vector<tuple<int, int> > tmp(n_vertex, make_tuple(-1, -2));
    for( int i = 0 ; i < n_vertex ; ++i ){
        _graph.push_back(Vertex(i));
        _forward_table.push_back(tmp);
    }

    for( int i = 0 ; i < n_vertex ; ++i ){
        for( int j = 0 ; j < n_vertex ; ++j ){
            if( cost_table[i][j] != -1 and i != j ){
                _graph[i].buildOutEdge(&_graph[j], cost_table[i][j]);
            } 
        }
    }
}

void Graph:: writeResult(string filename){
    ofstream fout(filename);
    cout << "Write the routing tables to " << filename << endl;
    for( int i = 0, n = _graph.size() ; i < n ; ++i ){
        if(!_graph[i].isVoid()){
            fout << "Routing table of router " << _graph[i].getID() + 1 << ":" << "\n";
            for( int j = 0 ; j < n ; ++j ){
                if(!_graph[j].isVoid()){
                    fout << get<0>(_forward_table[i][j]) << " " << get<1>(_forward_table[i][j]) + 1 << endl;
                }
            }
        }
    }
}

void Graph:: deleteRouter(unsigned id){
    cout << "Delete Target: " << id << endl;

    for( int i = 0, n = _graph.size() ; i < n ; ++i ){
        if(!_graph[i].isVoid()){
            if(_graph[i].getID() == id){
                Vertex* rm = &_graph[i];
                
                for( int j = 0, m = rm -> getOutEdge().size() ; j < m ; ++j ){
                    Vertex* neighbor = get<0>(rm -> getOutEdge()[j]);
                    neighbor -> removeOutEdge(rm);
                }

                _graph[i] = Vertex(UINT_MAX);
                setGraphToDefault();
                break;
            }
        }
    }

}
void Graph:: buildForwardTables(){
    for( int i = 0 ; i < _graph.size() ; ++i ){
        if(!_graph[i].isVoid()){
            shortest_path(i);
            buildForwardTable(i);
            setVertexToDefault();
        }
    }
}


void Graph:: shortest_path(unsigned src){
    priority_queue<Vertex*, vector<Vertex*>, Comparator> pq;

    _graph[src].setCost(0);
    _graph[src].setFromVertex(&_graph[src]);
    pq.push(&_graph[src]);

    while(!pq.empty()){
        Vertex* u = pq.top();
        pq.pop();

        if(u -> getVisit())
            continue;
        
        vector<Vertex*> v_relaxed = u -> relax();
        for( int i = 0, n = v_relaxed.size() ; i < n ; ++i ){
            pq.push(v_relaxed[i]);
        }
        u -> setVisit(true);
    }
}
void Graph:: buildForwardTable(unsigned src){
    for( int i = 0 ; i < _graph.size() ; ++i ){
        if(!_graph[i].isVoid()){
            if(_tmp_src_next_vertex[i] == -2){
                recursiveFindPrevVert(i, src);
            }
        }
    }

    vector<tuple<int, int> > forward_table(_graph.size(), make_tuple(-1, -2));
    for( int i = 0 ; i < _graph.size() ; ++i ){
        if(!_graph[i].isVoid()){
            forward_table[i] = make_tuple(_graph[i].getCost(), _tmp_src_next_vertex[i]);
        }
    }
    _forward_table[src] = forward_table;
}

unsigned Graph:: recursiveFindPrevVert(unsigned id, unsigned src){
    // Exist next step
    if( _tmp_src_next_vertex[id] != -2 ){
        return _tmp_src_next_vertex[id];
    }
    // Next step = itself
    if( id == src ){
        _tmp_src_next_vertex[id] = id;
        return _tmp_src_next_vertex[id];
    }
    // Next step = a neighbor of source, which views source as ancestor
    vector<tuple<Vertex*, int> > outEdge = _graph[src].getOutEdge();
    for( int i = 0 ; i < outEdge.size() ; i++ ){
        unsigned out_id = get<0>(outEdge[i]) -> getID();
        unsigned out_from_id = get<0>(outEdge[i]) -> getFromVertex() -> getID();
        if(out_id == id and out_from_id == src ){
            _tmp_src_next_vertex[id] = id;
            return id;
        }
    }
    // Exist _from vertex
    Vertex* self = &_graph[id];
    if (self -> getFromVertex() != NULL)
        _tmp_src_next_vertex[id] = recursiveFindPrevVert(self -> getFromVertex() -> getID(), src);

    return _tmp_src_next_vertex[id];
}

void Graph:: setVertexToDefault(){
    for( int i = 0; i < _graph.size() ; ++i ){
        _graph[i].setCost(UINT_MAX);
        _graph[i].setVisit(false);
        _graph[i].setFromVertex(0);

        _tmp_src_next_vertex[i] = -2;
    }
}
void Graph:: setGraphToDefault(){
    _forward_table.clear();
    _tmp_src_next_vertex.clear();

    vector<tuple<int, int> > tmp(_graph.size(), make_tuple(-1, -2));
    for( int i = 0 ; i < _graph.size() ; ++i ){
        _forward_table.push_back(tmp);
        _tmp_src_next_vertex.push_back(-2);
    }
}


