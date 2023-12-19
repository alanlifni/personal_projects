// File: ece650-prj.cpp (Implementation file)

// Include necessary headers
#include <iostream>
#include <stack>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <unordered_map>
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stack>
#include <unordered_set>
#include <utility>
#include "minisat/core/Solver.h"
#include <set>
#include <map>

using namespace std;

// Include necessary typedefs
typedef std::pair<std::vector<std::vector<int>>, std::unordered_map<int, int>> Pair;

// Class declarations (if any)
class MyGraph
{

public:

    int vertex_num = 0;
    vector <vector<int>> edge_list;
    bool vertex_edge_programmed = false;

    /* Below Handler is invoked when vertex number is provided */
    void V_cmd(string &arg);
    /* Below Handler is invoked when edges are provided */
    void E_cmd(string &arg);  
  
};

// Global initializations
MyGraph graph_obj;
vector <int > cnf_sat_vc_res;
vector <int > approx_vc_1_res;
vector <int > approx_vc_2_res;

bool VE_set = false;
bool timeout_scenario = false;

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

    for (char c : arg) 
        if (c >= '0' && c <= '9') i = i * 10 + (c - '0');       

    if (i <= 1){
        cerr<<"Error:  [prj] Vertex number cannot be one or lesser"<<endl;
        return;
    }

    graph_obj.vertex_num = i;
    graph_obj.vertex_edge_programmed = false;
}

void MyGraph::E_cmd( string &arg) 
{ 
    // If the vertex number is not set or is already programmed, don't get the edge list
    if (graph_obj.vertex_edge_programmed == true) {
        cerr<<"Error:  [prj] Number of vertex must be provided first"<<endl;
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
                cerr<<"Error:  [prj] Zero is not a valid vertex"<<endl;
                return;
            }
            }
        if (i != 0 && (c < '0' || c > '9')){
            if (i>graph_obj.vertex_num || i<=0){ 
                cerr<<"Error:  [prj] Non existent vertex provided"<<endl;
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
            cerr<<"Error:  [prj] In an undirected graph, start and end vertex of an edge cannot be the same"<<endl;
            return;
        }
        graph_obj.edge_list.push_back(edge_pair);

    }

    graph_obj.vertex_edge_programmed = true; 

   }


std::vector<int> singleVertFinder(std::vector<vector<int>> Edges)
{
    //Find Vertices with just one edge.
    std::vector<int> singleVerts;
    if(Edges.size() == 0)  return singleVerts;           //Assuming that all verts got covered..

    std::unordered_map<int,int> VertsFreq; //To store the vertex frequency.

    for (const auto& vector : Edges) 
        for (int num : vector) 
            VertsFreq[num]++;
        
    //Now we have a list of verts with their frequency.
    ///Extract the ones with just a single occurence [Means they were only in a single edge.]

    for(const auto& element : VertsFreq) 
        if(element.second == 1) singleVerts.push_back(element.first);

    return singleVerts;
}

std::vector<int> removeDuplicates(std::vector<int> vec){

    sort(vec.begin(),vec.end());

    for(unsigned int i=0; i<vec.size()-1; i++)
        if(vec[i]==vec[i+1]) vec.erase(vec.begin() + i);
        
    return vec; 
}

//Notice that we dont have call by reference here..
pair<std::vector<std::vector<int>>, std::unordered_map<int,int>> Mapper(std::vector<std::vector<int>> optimizedEdges)
{
    std::unordered_map<int,int> vertEncoder; //This stores a key for every vertex present as a map.
    int counter = 1;
    std::unordered_set<int> visitedNodes;
    std::vector<vector<int>> encodedOptimizedEdges; //Store the values who have been encoded..
    for(const auto& vertVector : optimizedEdges)
    {
        std::vector<int> tempEdge;
        for(const auto& vertex : vertVector)
        {
            //If the node is not visited..Add that node a special key  
            if(visitedNodes.find(vertex) == visitedNodes.end())
            {
                visitedNodes.insert(vertex);  
                vertEncoder[vertex] = counter ; //Giving the vertex a special key.. So lets say 47 is first value, then 47 -> 1 ; 48 -> 2 and so on..
                tempEdge.push_back(counter);    //Push that as a part of the edge..
                counter++;                      //Increment it
            }

            //If the node exists, simply gets its encoding an create the edge..
            else tempEdge.push_back(vertEncoder[vertex]);
        
        }
        //A pakaged edge is ready, push it onto our optimized list of edges.
        encodedOptimizedEdges.push_back(tempEdge);
    }

    return std::make_pair(encodedOptimizedEdges, vertEncoder);
}

//Function to remove single edge vertices. Returns a part of the cover thats retrieved from this optimization.
std::vector<int> TentacleRemover(std::vector<std::vector<int>> &optimizedEdges)
{
    std::vector<int> appendToCover;
    std::vector<int> temp;
    std::unordered_set<int> visitedStack;
    
    //This gives us the vertices that are only in one edge.    
    temp = singleVertFinder(optimizedEdges);
    std::stack<int> incompetentVerts;

    for(const auto& ele : temp)
        if(visitedStack.find(ele) == visitedStack.end())
        {
          incompetentVerts.push(ele);
          visitedStack.insert(ele);
        }
    
    //Head back to your edges list and extract the edges that these are a part of..
    //Add the other vertex into the SAT Cover and remove this vert and edge.
    
    std::vector<int> vertsToDelete;
 
    while(!incompetentVerts.empty())
    {
        int vertex = incompetentVerts.top();
        incompetentVerts.pop();
        
        //The usage of const auto& in your range-based for loop is perfectly fine, 
        //and you can still modify the container (in this case, graph_obj.edge_list) within the loop body. 
        //The const in const auto& refers to the fact that you can't modify the loop variable (edge in this case), not the container itself.
 
        for(const auto &edge : optimizedEdges)
        {
            //Container to store the vertices which need to be removed from the edges database..
            //Find the edge, Add the other vertex of that edge in our Cover and remove the edge from the edgelist
            if(edge[0] == vertex)
            {
                vertsToDelete.push_back(edge[1]);
                appendToCover.push_back(edge[1]);
            }
            if(edge[1] == vertex)
            {
                vertsToDelete.push_back(edge[0]);
                appendToCover.push_back(edge[0]);
            }

        }
    
        appendToCover = removeDuplicates(appendToCover);
        vertsToDelete = removeDuplicates(vertsToDelete);

        //Remove all the edges that are connected with the key, which is now added in the SAT Cover.
        //std::remove_if is used in combination with the erase function. 
        //The lambda function passed to remove_if checks if the valueToFind exists in each vector. 
        //If it does, the vector is removed.
        
        if(optimizedEdges.size() != 0){
        for (auto &vertToRemove : vertsToDelete) {

           auto edgesToDelete = [vertToRemove](const std::vector<int>& Edge) {
            return std::find(Edge.begin(), Edge.end(), vertToRemove) != Edge.end();
           };

           auto newEnd = std::remove_if(optimizedEdges.begin(), optimizedEdges.end(), edgesToDelete);
           
           optimizedEdges.erase(newEnd, optimizedEdges.end());
        
         }
        }
        
        //After removing all the edges connected to the verts which are now present in the SAT Cover,
        //Recompute the number of single edged vertices and fill the stack. If it returns size 0, then we move out of the loop
        
        if(optimizedEdges.size() != 0 && incompetentVerts.size() == 0){
          std::vector<int> temp = singleVertFinder(optimizedEdges);
          if(temp.size() == 0) break; 
          for(auto &t : temp) { 
            if(visitedStack.find(t) == visitedStack.end()) {
            visitedStack.insert(t);
            incompetentVerts.push(t);
          }
        }
      }
    }
    
    return appendToCover;

}

std::vector<int> TriangleOptimizer(vector<vector<int>> &optimizedEdges)
{
    std::vector<int> appendToCover; 

    bool triangle_found = false;

    set <int> neighbour_node_a, neighbour_node_b, neighbour_node_c;
    bool a_intersection = false, b_intersection = false, c_intersection = false;
    int node_a,node_b,node_c;

    for (const auto &i : optimizedEdges){

        // In this edge get neighbour nodes of vertex 1 (a) and vertex 2 (b)

        neighbour_node_a.clear();
        neighbour_node_b.clear();
        neighbour_node_c.clear();
        a_intersection = false;
        b_intersection = false;
        c_intersection = false;
        node_c = 0;
        node_a = i[0];
        node_b = i[1];

        for (const auto &j : optimizedEdges){
            if ((i[0] == j [0]) && (i[1] == j [1])) continue;

            // check for node a          
            if (j[0] == i[0]) neighbour_node_a.insert(j[1]);
            if (j[1] == i[0]) neighbour_node_a.insert(j[0]);

            // check for node b           
            if (j[0] == i[1]) neighbour_node_b.insert(j[1]);
            if (j[1] == i[1]) neighbour_node_b.insert(j[0]);
         
        }

        if (neighbour_node_a.size() > 1) a_intersection = true;
        if (neighbour_node_b.size() > 1) b_intersection = true;

        // if there are common elements between neighbour_node_a and neighbour_node_b, then we have a triangle
        set<int> intersect;

        set_intersection(neighbour_node_a.begin(), neighbour_node_a.end(), neighbour_node_b.begin(), neighbour_node_b.end(),
                std::inserter(intersect, intersect.begin()));

        for (auto p: intersect){
            // Check if this node is an intersection, so get the neighbour nodes of intersections
            neighbour_node_c.clear();
            node_c = p;
            for (const auto &k : optimizedEdges){
                if (p == k[0]) neighbour_node_c.insert(k[1]);
                if (p == k[1]) neighbour_node_c.insert(k[0]);
            }

            if (neighbour_node_c.size() > 2) c_intersection = true;

            // check if all a b c are not intersections, if yes we can use these vertices for optimization

            if (a_intersection && b_intersection && c_intersection) continue;
            else {
                triangle_found = true;
                break;
            };

            }

            if (triangle_found) break;
    
    }

    if (triangle_found == false) return appendToCover;


    // identify the type of triangle here
    set <int> triangle_vertex;
    set<int>::iterator it;  

    triangle_vertex.insert(node_a);
    triangle_vertex.insert(node_b);
    triangle_vertex.insert(node_c);

    if (neighbour_node_a.size() > 1){
        appendToCover.push_back(node_a);
        it=triangle_vertex.find(node_a);  
        triangle_vertex.erase (*it); 
    }

    if (neighbour_node_b.size() > 1){
        appendToCover.push_back(node_b);
        it=triangle_vertex.find(node_b);  
        triangle_vertex.erase (*it); 
    }

    if (neighbour_node_c.size() > 2){
        appendToCover.push_back(node_c);
        it=triangle_vertex.find(node_c);  
        triangle_vertex.erase (*it); 
    }    
    
    if (triangle_vertex.size() == 3){
        // means an isolated triangle, here we are free to add any two vertex to the vertex cover
        appendToCover.push_back(node_a);
        appendToCover.push_back(node_b);

    }
    else if (triangle_vertex.size() == 2){
        // only one vertex here is an intersection, here we can pick any of the other two nodes for vertex cover
        appendToCover.push_back(*triangle_vertex.begin());
    }

    // Time to optimize the edge list
    vector <int> vertsToDelete;
    vertsToDelete.push_back(node_a);
    vertsToDelete.push_back(node_b);
    vertsToDelete.push_back(node_c);

    if(optimizedEdges.size() != 0){
    for (auto &vertToRemove : vertsToDelete) {
        auto edgesToDelete = [vertToRemove](const std::vector<int>& Edge) {
        return std::find(Edge.begin(), Edge.end(), vertToRemove) != Edge.end();
        };
        auto newEnd = std::remove_if(optimizedEdges.begin(), optimizedEdges.end(), edgesToDelete);      
        optimizedEdges.erase(newEnd, optimizedEdges.end());  
        }
    }
 
    return appendToCover;
}

std::vector<int> LineOptimizer(vector<vector<int>> &optimizedEdges)
{
    std::vector<int> appendToCover; 
    //return appendToCover; // For test purpose
    bool line_found = false;

    //create a map and store the vertices in the edge list

    map <int, int > mymap;

    for(const auto &i : optimizedEdges){
        mymap[i[0]]++;
        mymap[i[1]]++;
    }

    map<int, int>::iterator it;
    int neigh_node_a = 0, neigh_node_b = 0;
    int node_a = 0, node_b = 0;
    int subject_node = 0;

    for (it = mymap.begin(); it != mymap.end(); it++){
        // Find the middle node
        if (it->second != 2) continue;
        subject_node = it->first;

        // Find the adjacent nodes of this node 
        vector <int> temp;
        for (const auto &i : optimizedEdges){
            
            if (i[0] == subject_node) temp.push_back(i[1]);
            if (i[1] == subject_node) temp.push_back(i[0]);
        }
        if (temp.size()!= 2) continue;
        else{
            node_a = temp[0];
            node_b = temp[1];
        }

        // Make sure adjacent nodes are not intersections
        if ((mymap[node_a] > 2) || (mymap[node_b] > 2) ) continue;

        // Now find the neighbouring nodes of this adjacent nodes.
        for (const auto &i : optimizedEdges){
            if (i[0] == node_a && i[1] != subject_node) neigh_node_a = i[1];
            if (i[1] == node_a && i[0] != subject_node) neigh_node_a = i[0];
            if (i[0] == node_b && i[1] != subject_node) neigh_node_b = i[1];
            if (i[1] == node_b && i[0] != subject_node) neigh_node_b = i[0];
        }
        
        // Make sure the neighbouring nodes are intersections and they are connected directly

        bool neigh_nodes_linked = false;

        for (const auto &i : optimizedEdges)
            if ((i[1] == neigh_node_a && i[0] == neigh_node_b) || (i[0] == neigh_node_a && i[1] == neigh_node_b) ) neigh_nodes_linked = true;

        //if ((mymap[neigh_node_a] > 2) && (mymap[neigh_node_b] > 2) )  {

        if (neigh_nodes_linked){
            line_found = true;
            break;
        }
        else continue;

    }

    if (line_found){
        // Add the valid vertices
        appendToCover.push_back(subject_node);
        appendToCover.push_back(neigh_node_a);
        appendToCover.push_back(neigh_node_b);

    }
    else return appendToCover;

    // Time to optimize the edge list
    vector <int> vertsToDelete;
    vertsToDelete.push_back(subject_node);
    vertsToDelete.push_back(node_a);
    vertsToDelete.push_back(node_b);
    vertsToDelete.push_back(neigh_node_a);
    vertsToDelete.push_back(neigh_node_b);

    if(optimizedEdges.size() != 0){
    for (auto &vertToRemove : vertsToDelete) {
        auto edgesToDelete = [vertToRemove](const std::vector<int>& Edge) {
        return std::find(Edge.begin(), Edge.end(), vertToRemove) != Edge.end();
        };
        auto newEnd = std::remove_if(optimizedEdges.begin(), optimizedEdges.end(), edgesToDelete);      
        optimizedEdges.erase(newEnd, optimizedEdges.end());  
        }
    }

    return appendToCover;
}

void* cnf_sat_vc (void *data){

    // Do the minisat thing
    unsigned int curr_vertex = 1;
    std::vector<vector<int>> OptimizedEdges = graph_obj.edge_list;
    std::vector<int> masterOptimizedCover; //Keep getting both optimized covers and add it in this. 
    std::vector<int> cover1;  //For Tentacles..
    std::vector<int> cover2;  //Comes after optimizing as Triangles..
    std::vector<int> cover3;  //For line optimizer

    while (1){

        // First clear all the tentacles
        cover1.clear();
        cover1 = TentacleRemover(OptimizedEdges);
        if(cover1.size() != 0) masterOptimizedCover.insert(masterOptimizedCover.end(), cover1.begin(), cover1.end());
        if (cover1.size() != 0) continue;

        // At this point all tentacles and line redundancies are removed ... Now run triangle optimizer
        cover2.clear();
        cover2 = TriangleOptimizer(OptimizedEdges); 
        if(cover2.size() != 0) masterOptimizedCover.insert(masterOptimizedCover.end(), cover2.begin(), cover2.end());
        if (cover2.size() != 0) continue;
        
        // Introduce line optimizer here
        cover3.clear();
        cover3 = LineOptimizer(OptimizedEdges);
        if(cover3.size() != 0) masterOptimizedCover.insert(masterOptimizedCover.end(), cover3.begin(), cover3.end());
        if (cover3.size() != 0) continue;
        else break;

    }
 
    //Means all the graph nodes were taken care by the optimizations.
    if(OptimizedEdges.size() == 0)
    {
        cnf_sat_vc_res = masterOptimizedCover;
        return 0;
    }
    
    //Mapping optimization:

    Pair mapperUtils = Mapper(OptimizedEdges);

    std::vector<std::vector<int>> encodedOptimizedEdges = mapperUtils.first;
    std::unordered_map<int,int> vertEncoder = mapperUtils.second;
    
    //Till this point, Optimized edges is ready..

    std::vector<int> tempResult;
    std::vector<int> tempVector;
    bool SAT = false;

    int vertex_num = vertEncoder.size() + 1;

    if(vertEncoder.size() > (int)15){ //Timeout, in this scenario
        cnf_sat_vc_res.clear();
        timeout_scenario = true;
        return 0;
     }
     
    curr_vertex = 1;
    while( SAT == false && curr_vertex <= (unsigned int) vertex_num){

        tempVector.clear();
        cnf_sat_vc_res.clear();

        Minisat::Solver solver;

        std::vector<std::vector<Minisat::Lit>> Vertices(vertex_num);

        // Add the literals

        for (unsigned int i = 0; i < (unsigned int) vertex_num ; i++)
            for (unsigned int  j = 0; j < curr_vertex; j++){
                Minisat::Lit literal = Minisat::mkLit(solver.newVar());
                Vertices[i].push_back(literal);
            }

        // Add the clauses
        
        for(unsigned int  i = 0; i < curr_vertex; ++i){
            Minisat::vec<Minisat::Lit> Literals_num;
            for(unsigned int  j = 0; j < (unsigned int) vertex_num ; ++j) Literals_num.push(Vertices[j][i]);
            solver.addClause(Literals_num);
            Literals_num.clear();
        }

        for(unsigned int  i = 0; i < (unsigned int) vertex_num ; ++i)
            for(unsigned int  j = 0; j < (curr_vertex-1); ++j)
                for(unsigned int  k = j+1; k < curr_vertex; ++k) solver.addClause(~Vertices[i][j],~Vertices[i][k]);

        for(unsigned int  i = 0; i < curr_vertex; ++i)
            for(unsigned int  j = 0; j < (unsigned int) (vertex_num -1); ++j)
                for(unsigned int  k = j+1; k < (unsigned int) vertex_num ; ++k) solver.addClause(~Vertices[j][i],~Vertices[k][i]);
        
        for(unsigned int  i = 0; i < (unsigned int)  (encodedOptimizedEdges.size()); i++){
            Minisat::vec<Minisat::Lit> Literals_num;
            for(unsigned int  j = 0; j < curr_vertex; ++j){
                Literals_num.push(Vertices[encodedOptimizedEdges[i][0]][j]);
                Literals_num.push(Vertices[encodedOptimizedEdges[i][1]][j]);
            }

            solver.addClause(Literals_num);
            Literals_num.clear();
        }

        int solution = solver.solve();        

        if(solution){
            for(unsigned int  i = 0; i < (unsigned int) vertex_num ; ++i)
                {
                    for(unsigned int  j = 0; j < curr_vertex; ++j){
                    unsigned int result = Minisat::toInt(solver.modelValue(Vertices[i][j]));
                    if(result == 0) tempVector.push_back(i);                  
                }
            }
            tempResult = tempVector;
            SAT = true;

        }
        else{
            SAT = false;
            curr_vertex++;
        }

    }

    //Unmap all the vertices back to their original values..
    tempVector.clear();

    for( const auto& vertex : tempResult)
    {
        auto it = std::find_if(vertEncoder.begin(), vertEncoder.end(),
                           [&vertex](const std::pair<int, int>& pair) {
                               return pair.second == vertex;
                           });

        tempVector.push_back(it->first);    
    }

    //Clear the cnf_sat_vc_res and copy tempvector to it..
    cnf_sat_vc_res.clear();

    tempVector.insert(tempVector.end(), masterOptimizedCover.begin(), masterOptimizedCover.end()); //Appending the Cover vertices got before passing the problem to Minisat.
    sort(tempVector.begin(),tempVector.end());

    if(tempVector[0] == 0 && tempVector.size() == 1) tempVector.clear();
    
    //Use std::unique to remove consecutive duplicates and push it to the end..
    auto newEnd = std::unique(tempVector.begin(), tempVector.end());
    
    //Removing all duplicates that are pushed beyond the end...
    tempVector.resize(std::distance(tempVector.begin(),newEnd));

    cnf_sat_vc_res = tempVector;

    return 0;
}

void* approx_vc_1 (void *data){

    // Scan the edge list and hash the occurences of each vertex
    int hash[graph_obj.vertex_num + 1] = {0};

    while(true){

        //clear the hash table
        for (int j = 1; j <= graph_obj.vertex_num ; j++ ) if (hash[j] != -1) hash[j] = 0;

        for (unsigned int i = 0 ; i < graph_obj.edge_list.size(); i++){
            if (hash[graph_obj.edge_list[i][0]] != -1 && hash[graph_obj.edge_list[i][1]] != -1){
                hash[graph_obj.edge_list[i][0]]++;
                hash[graph_obj.edge_list[i][1]]++;
            }
        }

        int max = -1, curr_vertex = 0;
        for (int j = 1; j <= graph_obj.vertex_num ; j++ ){
            if (hash[j] > max) {
                max = hash[j];
                curr_vertex = j;
        }

    }
        if (max <= 0) break;
        hash[curr_vertex] = -1;
        approx_vc_1_res.push_back(curr_vertex);

    }

    sort(approx_vc_1_res.begin(),approx_vc_1_res.end());

    return 0;
}

void* approx_vc_2 (void *data){

    // Scan the edge list and hash the occurences of each vertex

    bool hash[graph_obj.vertex_num + 1] = {0};

    while(true){

        bool exit_true = true;

        for (unsigned int i = 0 ; i < graph_obj.edge_list.size(); i++){

            if (hash[graph_obj.edge_list[i][0]] == 0 && hash[graph_obj.edge_list[i][1]] == 0){
                exit_true = false;
                if (hash[graph_obj.edge_list[i][0]] == 0 ) {
                    approx_vc_2_res.push_back(graph_obj.edge_list[i][0]);
                    hash[graph_obj.edge_list[i][0]] = 1;
                }
                if (hash[graph_obj.edge_list[i][1]] == 0 ){
                    approx_vc_2_res.push_back(graph_obj.edge_list[i][1]);
                    hash[graph_obj.edge_list[i][1]] = 1;
                }
                break;
            }

        }

        if (exit_true == true) break;

    }

    sort(approx_vc_2_res.begin(),approx_vc_2_res.end());

    return 0;
}

// Main function 
int main(int argc, char** argv) {

// read from stdin until EOF
    while (!std::cin.eof()) {
        // read a line of input until EOL and store in a string
        std::string line;
        std::getline(std::cin, line);

        // Initialize the thread variables
        int ret = 0;
        pthread_t t1, t2, t3;

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

            if (graph_obj.vertex_edge_programmed == false) break;
 
            //cnf_sat_vc_res = graph_obj.cnf_sat_vc();
            ret = pthread_create(&t1, nullptr, &cnf_sat_vc, nullptr);
            if (ret != 0) cerr <<"Error: Unable to spawn cnf_sat_vc thread"<<endl;

            //approx_vc_1_res = graph_obj.approx_vc_1();
            ret = pthread_create(&t2, nullptr, &approx_vc_1, nullptr);
            if (ret != 0) cerr <<"Error: Unable to spawn approx_vc_1 thread"<<endl;

            //approx_vc_2_res = graph_obj.approx_vc_2();
            ret = pthread_create(&t3, nullptr, &approx_vc_2, nullptr);
            if (ret != 0) cerr <<"Error: Unable to spawn approx_vc_2 thread"<<endl;

            //Join the threads
            ret = pthread_join(t3, nullptr);
            if (ret != 0) cerr <<"Error: Unable to terminate approx_vc_2 thread"<<endl;
            ret = pthread_join(t2, nullptr);
            if (ret != 0) cerr <<"Error: Unable to terminate approx_vc_1 thread"<<endl;
            ret = pthread_join(t1, nullptr);
            if (ret != 0) cerr <<"Error: Unable to terminate cnf_sat_vc thread"<<endl;

            //Print the results
            std::cout << "CNF-SAT-VC: ";
            sort(cnf_sat_vc_res.begin(),cnf_sat_vc_res.end());
            if (timeout_scenario) std::cout<<"timeout";
            else 
            {
                for(unsigned int  i = 0; i < cnf_sat_vc_res.size() - 1 ; ++i)  std::cout << cnf_sat_vc_res[i] << ",";
                std::cout <<cnf_sat_vc_res.back();
            }
            std::cout<<endl;
            timeout_scenario = false;
            
            std::cout << "APPROX-VC-1: ";
            for(unsigned int  i = 0; i < approx_vc_1_res.size() - 1 ; ++i)  std::cout << approx_vc_1_res[i] << ",";       
            std::cout<<approx_vc_1_res.back()<< endl;

            std::cout << "APPROX-VC-2: ";
            for(unsigned int  i = 0; i < approx_vc_2_res.size() - 1 ; ++i)  std::cout << approx_vc_2_res[i] << ",";          
            std::cout<<approx_vc_2_res.back()<< endl;

            //Clear the solution vectors
            cnf_sat_vc_res.clear(); 
            approx_vc_1_res.clear(); 
            approx_vc_2_res.clear(); 

            break;

            default:
            cerr<<"Error: [prj] Invalid Input"<<endl;

        }        
    }
}
