#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <tuple>
#include "vertex.h"

using namespace std;

#ifndef GRAPH_H
#define GRAPH_H

class Graph{
    public:
        friend ostream& operator << (ostream&, const Graph&);

        Graph(int n_vertex, vector<vector<int> > matrix): _n_vertex(n_vertex), _tmp_src_next_vertex(n_vertex, -2){
            _graph.reserve(n_vertex);
            _forward_table.reserve(n_vertex);
            buildGraph(matrix);
        }
        void print();
        void buildGraph(vector<vector<int> >);
        void writeResult(string);
        void deleteRouter(unsigned);
        void buildForwardTables();

        // Auxiliary function for forward tables building
        void shortest_path(unsigned);
        void buildForwardTable(unsigned);
        unsigned recursiveFindPrevVert(unsigned, unsigned);

        void setVertexToDefault();
        void setGraphToDefault();
        
    private:
        vector<Vertex>                      _graph;
        vector<vector<tuple<int, int> >>    _forward_table;
        vector<int>                         _tmp_src_next_vertex;
        int                                 _n_vertex;
};
#endif