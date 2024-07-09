// File: ece650-a2.hpp (Header file)

// Header guards
#ifndef ECE650_A2_HPP_
#define ECE650_A2_HPP_

// Include necessary headers
#include <string>
#include <vector>

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

#endif /* end of ECE650_A2_HPP_ */
