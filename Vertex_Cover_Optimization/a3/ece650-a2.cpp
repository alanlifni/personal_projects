// File: ece650-a2.cpp (Implementation file)

// Include necessary headers
#include <iostream>
#include <stack>
#include <string>
#include <vector>
#include <signal.h>
#include <unistd.h>

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
    /* Below Handler is invoked when start and end vertices are provided */
    void s_cmd(string &arg);
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
        cerr<<"Error:  [a2] Vertex number cannot be one or lesser"<<endl;
    }

    graph_obj.vertex_num = i;
    graph_obj.vertex_edge_programmed = false;

}

void MyGraph::E_cmd( string &arg) 
{ 
    // If the vertex number is not set or is already programmed, don't get the edge list
    if (graph_obj.vertex_edge_programmed == true) {
        cerr<<"Error:  [a2] Number of vertex must be provided first"<<endl;
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
                cerr<<"Error:  [a2] Zero is not a valid vertex"<<endl;
                return;
            }
            }
        if (i != 0 && (c < '0' || c > '9')){
            if (i>graph_obj.vertex_num || i<=0){ 
                cerr<<"Error:  [a2] Non existent vertex provided"<<endl;
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
            cerr<<"Error:  [a2] In an undirected graph, start and end vertex of an edge cannot be the same"<<endl;
            return;
        }
        graph_obj.edge_list.push_back(edge_pair);

    }

    graph_obj.vertex_edge_programmed = true; 

    // Print the vertex and edges to the console
    cout << "V "<< graph_obj.vertex_num <<endl;
    cout << arg << endl;
    }


void MyGraph::s_cmd( string &arg) 
{ 
    
    if (graph_obj.edge_list.empty()){
        cerr<<"Error: [a2] Provide inputs for the edges first"<<endl;
        return;
    }
    
    // Parse the input line and gather start and end vertex
    vector <int> from_to_vertex;

    int i = 0, start_vertex = 0, end_vertex = 0; 

    for (char c : arg) {
        
        if (c >= '0' && c <= '9') {
            i = i * 10 + (c - '0');
            }
        if (i != 0 && (c < '0' || c > '9')){
            if (i>graph_obj.vertex_num){ 
                cerr<<"Error:  [a2] Given nodes are not in vertex list"<<endl;
                return;
            }
            if (start_vertex == 0)
            start_vertex = i; // Store the start vertex
            else
            break;
            
            i = 0;
            }
            
        }

    if (i>graph_obj.vertex_num){ 
        cerr<<"Error:  [a2] Given nodes are not in vertex list"<<endl;
        return;
    }
    end_vertex = i; // Store the end vertex

    if (start_vertex <=0 || end_vertex <= 0) {
        cerr<<"Error:  [a2] Given nodes are not in vertex list"<<endl;
        return;
    }

    // If the start and end vertex are same, the shortest path between them is themselves.
    if(start_vertex == end_vertex) {
        cout <<start_vertex<<endl;
        return;
    }
    
    /*Okay, let's do the core logic now ... I'm going to use Dijkstra's Algo ... Let's see! */
    bool *visited_vertex = new bool[graph_obj.vertex_num + 1];
    bool all_nodes_visited = false;

    int **dijkstra_tbl = new int*[graph_obj.vertex_num + 1];

    for (int i = 1 ; i <= graph_obj.vertex_num; i++){
        visited_vertex[i] = 0;
        dijkstra_tbl[i] = new int[2];
        dijkstra_tbl[i][0] = 0xFFFE;
        dijkstra_tbl[i][1] = 0;

    }   
    
    //Fix the dijkstra table for the start vertex
    dijkstra_tbl[start_vertex][0]= 0;
    dijkstra_tbl[start_vertex][1]= 0;

    int local_min = 0xFFFF;
    int local_min_node = 0;

    while(all_nodes_visited != true){
        // Traverse the dijkstra_tbl and find the unvisted node with the lowest shortest distance from start vertex
        local_min = 0xFFFF;
        local_min_node = 0;

        all_nodes_visited = true;
        for (int i = 1 ; i <= graph_obj.vertex_num; i++){
            if (dijkstra_tbl[i][0] < local_min && visited_vertex[i] == false){
                local_min = dijkstra_tbl[i][0];
                local_min_node = i;
            }
        }
        if(visited_vertex[start_vertex] == false) local_min_node = start_vertex; // start from start vertex
        
        // Find the neigbours of this node
        vector <int> neighbour_nodes;
        vector <vector<int>>::iterator iter;

        for (iter = graph_obj.edge_list.begin(); iter< graph_obj.edge_list.end(); iter++){
            int from = 0 , to = 0;
            for (int i : *iter){
                if (from == 0) from = i;
                if (from != 0) to  = i;

            }
            if (from == local_min_node ) neighbour_nodes.push_back(to);
            if (to == local_min_node ) neighbour_nodes.push_back(from);
        }

        // Update the shortest distance field for the neighbour notes if they are unvisited

        for (int i: neighbour_nodes){
            if (visited_vertex[i] == false ){
                // Update the distance to reach this neibouring noden if this route is better
                if (dijkstra_tbl[local_min_node][0] + 1 <dijkstra_tbl[i][0]){
                    dijkstra_tbl[i][0] = dijkstra_tbl[local_min_node][0] + 1;
                    dijkstra_tbl[i][1] = local_min_node;
                }
            }      
        neighbour_nodes.clear(); 
        }
        
        // Check if all nodes are visited, if not set the flag to false
        for (int i = 0; i<= graph_obj.vertex_num; i++) if (visited_vertex[i] == false) all_nodes_visited = false;

        // Remove the current local node form the visited list
        visited_vertex[local_min_node] = true;    

    } 

    //finally, let's print stuff
    int current_node = end_vertex;
    stack <int> printing_list;

    while(current_node != start_vertex){

        if(current_node != end_vertex) printing_list.push(current_node);

        for  (int i = 1 ; i <= graph_obj.vertex_num; i++)
        {
            if (i == current_node){
                current_node =dijkstra_tbl[i][1];
                break;
            }
        }
        if (current_node == 0) {
            cerr<<"Error:  [a2] No path exists"<<endl;
            /* Deallocate the dynammic memory since we don't need it now */
            for (int i = 1 ; i <= graph_obj.vertex_num; i++) delete [] dijkstra_tbl[i]; 
            delete  []  visited_vertex;
            delete  [] dijkstra_tbl;  
            return;
        }
    }

    printing_list.push(current_node);

    while(!printing_list.empty()) {
        cout<<printing_list.top()<<"-";
        printing_list.pop();
    }
    
    cout<<end_vertex<<endl;

    /* Deallocate the dynammic memory since we don't need it now */
    for (int i = 1 ; i <= graph_obj.vertex_num; i++) delete [] dijkstra_tbl[i]; 
    delete  []  visited_vertex;
    delete  [] dijkstra_tbl;  
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

            if (VE_set == false){
            kill(getppid(), SIGUSR1); // let a3 know that we now have a valid V and E
            VE_set = true;
            }

            break;

            case 's':
            graph_obj.s_cmd(line);
            break;

            default:
            cerr<<"Error: [a2] Invalid Input"<<endl;

        }        
    }
}
