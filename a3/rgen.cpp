// adapted from www.gnu.org/software/libc/manual/html_node/Example-of-Getopt.html

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <map>
#include <math.h>
#include <signal.h>
#include <unistd.h>

using namespace std;

int s_val = 10, n_val = 5, l_val = 5, c_val = 20, rand_wait_time = 0;

unsigned int street_name_cnt = 0;
unsigned int start_street_name = 0;

int get_rand_int(int from, int to, unsigned int raw_rand){
  return  round( ((double(raw_rand)/UINT32_MAX )* abs( to - from) ) + from) ;
}

bool find_intersect( int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, float m1, float def1, float m2, float def2){

  if (def1 != def2) return false;
  if (def1 == 1 && def2 == 1 && m1 != m2) return false;


  if (def1 == 0 && def2 == 0){ // vertical lines
  // calculate y intercept;
    if (x1 != x3) return false;

  }
  else if (def1 == 1 && def2 == 1){ 

    if (m1 != m2) return false;
    if (m1 == m2 && m1 == 0) // horizontal lines
    {
      if (y1 != y3) return false;
    }
    else{ // lines with valid slopes

    //find c
    float c1 = 0, c2 = 0;
    c1 = y1 - (m1 * x1);
    c2 = y3 - (m2 * x3);
    if (int(c1 * 10) != int(c2 * 10)) return false;
    }
  }

  if (((x3 >= x1 and x3 <= x2) or (x3 <= x1 and x3 >= x2)) && ((y3 >= y1 and y3 <= y2) or (y3 <= y1 and y3 >= y2))) return true;
  if (((x4 >= x1 and x4 <= x2) or (x4 <= x1 and x4 >= x2)) && ((y4 >= y1 and y4 <= y2) or (y4 <= y1 and y4 >= y2))) return true;

  return false;
}

bool gen_input(){

  //flush the stdin
  fflush(stdin);

  // clear the past inputs
  for (unsigned int i = start_street_name; i<street_name_cnt; i++){
    std::cout<<"rm \""<<i<<"\""<<endl;
  }

  // Store the start of the street
  start_street_name = street_name_cnt;

  // open the urandom file
  
  ifstream MyReadFile("/dev/urandom");

    // check that it did not fail
    if (MyReadFile.fail()) {
      std::cerr << "Error: [rgen] cannot open /dev/urandom"<< endl;
      kill(getppid(), SIGUSR2); // inform a3 that rgen terminated
      return 1;
    }

  // Generate the required random inputs
  unsigned int raw_rand = 0, num_streets = 0, num_line_seg = 0;

  MyReadFile.read((char *)&raw_rand, sizeof(int));
  num_streets = get_rand_int (2, s_val, raw_rand);

  MyReadFile.read((char *)&raw_rand, sizeof(int));
  rand_wait_time = get_rand_int (5, l_val, raw_rand);

  map < array<int, 4> , array<float, 2> > edge_list;

  int px = 0, py = 0;

  // Print the streets
  for (int i = 0; i< int(num_streets); i++ ){

    map < array<int, 2> , bool> coor_list;

      MyReadFile.read((char *)&raw_rand, sizeof(int));
      num_line_seg = get_rand_int (1, n_val, raw_rand);

    for (unsigned int j = 0; j <= num_line_seg; j++){

      int x_axis, y_axis;

      bool coor_uniq = 0;

      char attempt = 25;

      while(coor_uniq == 0){

        MyReadFile.read((char *)&raw_rand, sizeof(unsigned int));
        x_axis = get_rand_int ((c_val * - 1), c_val, raw_rand);

        MyReadFile.read((char *)&raw_rand, sizeof(unsigned int));
        y_axis = get_rand_int ((c_val * - 1), c_val, raw_rand);

        array <int, 2> coor = {x_axis, y_axis};
        array <int, 4> edge = {px, py, x_axis, y_axis};
        bool valid_edge = true;

        if (edge_list.empty() == true) valid_edge = true;

        if (i!=0){ 
        // Check if the generated edge is valid
        // calculate slope
        float m = 0, def = 0;
        if (px - x_axis == 0) def = 0;
        else{
          m = (float(py)-float(y_axis))/float((px)-float(x_axis));
          m = float((int)(double(m) * 1000 + .5))/1000;
          def = 1;
        }

        for (map < array<int, 4> , array<float, 2> > ::  iterator it = edge_list.begin(); it !=edge_list.end() ; it++){
          if (find_intersect(it->first[0],it->first[1],it->first[2],it->first[3], px, py, x_axis, y_axis, m, def, it->second[0], it->second[1]) == true) break;
        }  

          if ( valid_edge == false || coor_list[coor] != 0){
            attempt--;
            if (attempt <= 0) {
              std::cerr  <<"Error: [rgen] failed to generate valid input for 25 simultaneous attempts"<<endl;
              kill(getppid(), SIGUSR2); // inform a3 that rgen terminated

              close(STDOUT_FILENO);
              // Close the file
              MyReadFile.close();
              exit(-1);
              return -1;
              }
            continue;
          }
          else {
             coor_list[coor] = 1;
             px = x_axis;
             py = y_axis;

             array <float, 2> slope = {m, def};
          
             edge_list[edge] = slope;
             coor_uniq = 1;
          }
        }
        else {
          coor_list[coor] = 1;
          px = x_axis;
          py = y_axis;
          coor_uniq = 1;
        }              
        }
    }
    
    if (coor_list.size() < 2 ){
      i--;
      continue;
    }

    std::cout <<"add \""<<street_name_cnt<<"\" ";
    street_name_cnt++;

    for (map < array<int, 2> , bool> ::  iterator it = coor_list.begin(); it !=coor_list.end() ; it++){
      std::cout << "(" << it->first[0] <<"," << it->first[1] <<")";
    }

    std::cout<<endl;
    
  }

  // Issue gg cmd
  std::cout << "gg"<<endl;

  // Close the file
  MyReadFile.close();

  return 0;
}

int main(int argc, char **argv) {

  // Process the command line arguments
  char c;
  string cvalue;

  // expected options are '-a', '-b', and '-c value'
  while ((c = getopt(argc, argv, "s:n:l:c:")) != -1){
    switch (c) {
    case 's':
      cvalue = optarg;
      s_val = atoi(cvalue.c_str());
      break;
    case 'n':
      cvalue = optarg;
      n_val = atoi(cvalue.c_str());
      break;
    case 'l':
      cvalue = optarg;
      l_val = atoi(cvalue.c_str());
      break;
    case 'c':
      cvalue = optarg;
      c_val = atoi(cvalue.c_str());
      break;
    case '?':
        std::cerr << "Error: [rgen] unknown option: " << optopt << std::endl;
      return 1;
    }

  }

  // validate the command line arguments
  if (s_val < 2 || n_val < 1 || l_val < 5 || c_val < 1 ){
    std::cerr  << "Error: [rgen] Invalid command line arguments" << endl;
    return 1;
  }

  while (true){
    if (gen_input () != 0) break;
    else sleep(rand_wait_time);
  }

  close(STDOUT_FILENO);
  return 0;

}
