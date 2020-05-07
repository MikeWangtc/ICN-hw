#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include "graph.h"

using namespace std;

vector<vector<int> > readfile(string);

int main(int argc, char* argv[]){
    Graph* graph;

    string cmd;
    string filename;

    cout << "RouterMgr>> ";

    while(getline(cin, cmd)){
        stringstream ss(cmd);
        string oprt, arg;
        ss >> oprt >> arg;

        if(oprt == "lf"){
            delete graph;       // avoid memory leaking

            filename = arg.substr(0, arg.length()-4);
            vector<vector<int> > cost_table = readfile(arg);
            graph = new Graph(cost_table.size(), cost_table);
            graph -> buildForwardTables();
            graph -> print(); 
        }
        else if(oprt == "of"){
            if(graph == NULL){
                cout << "Please load file first !\n";
            }
            else{
                string outfilename = filename + "_out1.txt";
                graph -> writeResult(outfilename);
            }
        }
        else if(oprt == "rm"){
            if(graph == NULL){
                cout << "Please load file first !\n";
            }
            else{
                int deleteRouterID = stoi(arg.substr(1)) - 1;
                graph -> deleteRouter(deleteRouterID);
                graph -> buildForwardTables();
                graph -> print();
            }
        }
        else{
            cout << "Invalid command !!!\n";
            delete graph;
            break;
        }
        cout << "RouterMgr>> ";
    }
    return 0;
}

vector<vector<int> > readfile(string filename){
    ifstream file(filename);
    vector<vector<int> > rtn;
    if( file.is_open() ){
        cout << "Successfully read in " << filename << endl;
        int n_vertex;
        file >> n_vertex;
        rtn.reserve(n_vertex);
        for( int i = 0 ; i < n_vertex ; ++i ){
            vector<int> tmp;
            tmp.reserve(n_vertex);
            for( int j = 0 ; j < n_vertex ; ++j ){
                int cost;
                file >> cost;
                tmp.push_back(cost);
            }
            rtn.push_back(tmp);
        }
        file.close();
    }
    else{
        cout << "Faild to open file\n";
    }
    return rtn;
}
