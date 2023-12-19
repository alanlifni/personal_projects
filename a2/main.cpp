// File: main.cpp (Main file)

// Include necessary headers
#include <iostream>
#include "ece650-a2.hpp"

// Global initializations
MyGraph graph_obj;

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

            case 's':
            graph_obj.s_cmd(line);
            break;

            default:
            cout<<"Error: Invalid Input"<<endl;

        }        
    }
}
