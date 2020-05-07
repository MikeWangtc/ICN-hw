#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <utility>
#include <climits>
#include <tuple>

using namespace std;

#ifndef VERTEX_H
#define VERTEX_H

class Vertex{
    public:
        friend ostream& operator << (ostream& os, const Vertex& v);

        Vertex(unsigned id): _id(id), _cost(UINT_MAX), _visit(false), _from(0){}
        // ~Vertex();
        unsigned                            getID()             const;
        unsigned                            getCost()           const;
        bool                                getVisit()          const;
        int                                 getWeight(Vertex*)  const;
        Vertex*                             getFromVertex()     const;     
        vector<tuple<Vertex*, int> >        getOutEdge()        const;

        void                                setCost      (unsigned);
        void                                setVisit     (bool);
        void                                setFromVertex(Vertex*);

        bool                                isVoid();

        void                                buildOutEdge (Vertex*, int);
        void                                removeOutEdge(Vertex*);
        vector<Vertex*>                     relax        ();
        
    private:
        unsigned                            _id;
        unsigned                            _cost;              // cost to arrive the vertex
        bool                                _visit;             // whether the vertex is in shortest tree
        Vertex*                             _from;              // previous vertex    
        vector<tuple<Vertex*, int> >        _outEdge;
};

#endif