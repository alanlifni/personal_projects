// File: ece650-a4.cpp (Implementation file)

// Include necessary headers
#include <iostream>
#include <stack>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>

#include "minisat/core/Solver.h"


// Include necessary headers
using namespace std;

// Class declarations (if any)
class MyGraph
{
    int vertex_num = 0;
    vector <vector<int>> edge_list;
    bool vertex_edge_programmed = false;

public:
    /* Below Handler is invoked when vertex number is provided */
    void V_cmd(string &arg);
    /* Below Handler is invoked when edges are provided */
    void E_cmd(string &arg);

};

// Global initializations
MyGraph graph_obj;

bool VE_set = false;

// extern declarions
extern MyGraph graph_obj;

// Public member function definitions
void MyGraph::V_cmd( string &arg ) 
{ 
    // Erase the vertex and edge list if any
    graph_obj.vertex_num = 0;
    graph_obj.edge_list.clear(); 

    // Parse the vertex value from the line 
    int i = 0;

    for (char c : arg) {
        if (c >= '0' && c <= '9') {
            i = i * 10 + (c - '0');
        }
    }

    if (i <= 1){
        cerr<<"Error:  [a4] Vertex number cannot be one or lesser"<<endl;
        return;

    }

    graph_obj.vertex_num = i;
    graph_obj.vertex_edge_programmed = false;

}

void MyGraph::E_cmd( string &arg) 
{ 
    // If the vertex number is not set or is already programmed, don't get the edge list
    if (graph_obj.vertex_edge_programmed == true) {
        cerr<<"Error:  [a4] Number of vertex must be provided first"<<endl;
        return; 
    }

    //Clear the edge list, just in case
    graph_obj.edge_list.clear(); 

    // Parse the input line and gather the edges
    int i = 0; 
    vector <int> num_stream;
    
    for (char c : arg) {
        
        if (c >= '0' && c <= '9') {
            i = i * 10 + (c - '0');
            if(c == '0' && i == 0) { // zero provided as vertex
                cerr<<"Error:  [a4] Zero is not a valid vertex"<<endl;
                return;
            }
            }
        if (i != 0 && (c < '0' || c > '9')){
            if (i>graph_obj.vertex_num || i<=0){ 
                cerr<<"Error:  [a4] Non existent vertex provided"<<endl;
                return;
            }
            num_stream.push_back(i);
            i = 0;
            }
            
        }

    // Update the parsed values in the master variables
    vector<int>::iterator iter = num_stream.begin();

    for (iter = num_stream.begin() ; iter < num_stream.end(); iter++){

        vector<int> edge_pair;
        edge_pair.push_back(*iter);
        iter++;
        edge_pair.push_back(*(iter));
        // If an edge starts and ends at the same vertex, throw an error
        if (edge_pair[0] == edge_pair[1]){
            cerr<<"Error:  [a4] In an undirected graph, start and end vertex of an edge cannot be the same"<<endl;
            return;
        }
        graph_obj.edge_list.push_back(edge_pair);

    }

    graph_obj.vertex_edge_programmed = true; 

    // Do the minisat thing

    bool SAT = false;
    unsigned int curr_vertex = 1;
    std::vector<int> cover;

    graph_obj.vertex_num++;

    while( SAT == false && curr_vertex <= (unsigned int) graph_obj.vertex_num){

        cover.clear();

        Minisat::Solver solver;

        std::vector<std::vector<Minisat::Lit>> Vertices(graph_obj.vertex_num);

        // Add the literals

        for (unsigned int i = 0; i < (unsigned int) graph_obj.vertex_num ; i++)
            for (unsigned int  j = 0; j < curr_vertex; j++){
                Minisat::Lit literal = Minisat::mkLit(solver.newVar());
                Vertices[i].push_back(literal);
            }

        // Add the clauses
        
        for(unsigned int  i = 0; i < curr_vertex; ++i)
            for(unsigned int  j = 0; j < (unsigned int) (graph_obj.vertex_num -1); ++j)
                for(unsigned int  k = j+1; k < (unsigned int) graph_obj.vertex_num ; ++k) solver.addClause(~Vertices[j][i],~Vertices[k][i]);

        for(unsigned int  i = 0; i < curr_vertex; ++i){
            Minisat::vec<Minisat::Lit> Literals_num;
            for(unsigned int  j = 0; j < (unsigned int) graph_obj.vertex_num ; ++j) Literals_num.push(Vertices[j][i]);
            solver.addClause(Literals_num);
            Literals_num.clear();
        }

        for(unsigned int  i = 0; i < (unsigned int) graph_obj.vertex_num ; ++i)
            for(unsigned int  j = 0; j < (curr_vertex-1); ++j)
                for(unsigned int  k = j+1; k < curr_vertex; ++k) solver.addClause(~Vertices[i][j],~Vertices[i][k]);


        for(unsigned int  i = 0; i < (unsigned int)  (graph_obj.edge_list.size()); i++){
            Minisat::vec<Minisat::Lit> Literals_num;
            for(unsigned int  j = 0; j < curr_vertex; ++j){
                Literals_num.push(Vertices[graph_obj.edge_list[i][0]][j]);
                Literals_num.push(Vertices[graph_obj.edge_list[i][1]][j]);
            }
            solver.addClause(Literals_num);
            Literals_num.clear();
        }

        int solution = solver.solve();        

        if(solution){
            for(unsigned int  i = 0; i < (unsigned int) graph_obj.vertex_num ; ++i)
                for(unsigned int  j = 0; j < curr_vertex; ++j){
                    unsigned int result = Minisat::toInt(solver.modelValue(Vertices[i][j]));
                    if(result == 0) cover.push_back(i);                  
                }           
            SAT = true;
        }
        else{
            SAT = false;
            curr_vertex++;
        }

    }
    sort(cover.begin(),cover.end());
    for(unsigned int  i = 0; i < cover.size(); ++i)  std::cout << cover[i] << " ";
    std::cout << std::endl;
   }

// Main function 
int main(int argc, char** argv) {

// read from stdin until EOF
    while (!std::cin.eof()) {
        // read a line of input until EOL and store in a string
        std::string line;
        std::getline(std::cin, line);

        // if nothing was read, go to top of the while to check for eof
        if (line.size() == 0) {
            continue;
        }
      
        switch(line[0])
        {
            case 'V':
            graph_obj.V_cmd(line);
            break;

            case 'E':
            graph_obj.E_cmd(line);
            break;

            default:
            cerr<<"Error: [a4] Invalid Input"<<endl;

        }        
    }
}
