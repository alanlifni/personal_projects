#include <iostream>
#include <fstream>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <unistd.h>

using namespace std;

int s_val = 10, n_val = 5, l_val = 5, c_val = 20;

bool valid_VE = false;

bool rgen_active = true;

void ve_handler(int signum)
{
    if (signum == SIGUSR1) valid_VE = true;
    return;
}

void rgen_handler(int signum)
{
    if (signum == SIGUSR2) rgen_active = false;
    return;
}


int main (int argc, char **argv) {

  signal(SIGUSR1, ve_handler);
  signal(SIGUSR2, rgen_handler);

 // Process the command line arguments
  int c;
  string s_str = "10", n_str = "5", l_str = "5", c_str = "20";

  // expected options are '-a', '-b', and '-c value'
  while ((c = getopt(argc, argv, "s:n:l:c:")) != -1){
    switch (c) {
    case 's':
      s_str = optarg;
      s_val = atoi(s_str.c_str());
      break;
    case 'n':
      n_str = optarg;
      n_val = atoi(n_str.c_str());
      break;
    case 'l':
      l_str = optarg;
      l_val = atoi(l_str.c_str());
      break;
    case 'c':
      c_str = optarg;
      c_val = atoi(c_str.c_str());
      break;
    case '?':
        std::cerr << "Error: [a3] unknown option: " << optopt << std::endl;
      return 1;
    }

  }

  // validate the command line arguments
  if (s_val < 2 || n_val < 1 || l_val < 5 || c_val < 1 ){
    std::cerr  << "Error: [a3] Invalid command line arguments" << endl;
    return 1;
  }

    // Create a pipe between rgen and a1
    int rgen_to_a1[2];
    pipe(rgen_to_a1);

    // Create a pipe between a1 and a2
    int a1_to_a2[2];
    pipe(a1_to_a2);

    // Create a pipe between a2 and a3
    int a2_to_a3[2];
    pipe(a2_to_a3);

    //Connect a3 output to driver output

    dup2(STDOUT_FILENO, a2_to_a3[0]);
    close(a2_to_a3[0]);
    close(a2_to_a3[1]);

    // Spawn rgen
    pid_t proc_rgen;
    proc_rgen = fork();

    if (proc_rgen == 0){ // Invoking rgen
        dup2(rgen_to_a1[1], STDOUT_FILENO); //Connect rgen output to a1 input
        close(rgen_to_a1[0]);
        close(rgen_to_a1[1]);

        char* argv[] = {(char *)"rgen", (char *)"-s", (char *)s_str.c_str() ,(char *)"-n", (char *)n_str.c_str() , (char *)"-l", (char *)l_str.c_str(),  (char *)"-c", (char *)c_str.c_str() , NULL};

        execv("rgen", argv); 

        std::cerr << "Error: [a3] could not exec rgen \n";
        return 1;
    }
    else if (proc_rgen < 0) {
    std::cerr << "Error: [a3] could not fork rgen \n";
    return 1;
    }

    // Spawn a1
    pid_t proc_a1;
    proc_a1 = fork();

    if (proc_a1 == 0){ // Invoking a1
        dup2(rgen_to_a1[0], STDIN_FILENO); //Connect rgen output to a1 input
        dup2(a1_to_a2[1], STDOUT_FILENO); //Connect a1 output to a2 input
        close(rgen_to_a1[0]);
        close(rgen_to_a1[1]);
        close(a1_to_a2[0]);
        close(a1_to_a2[1]);

        char* py[3];
        py[0] = (char *)"/usr/bin/python3";
        py[1] = (char *)"ece650-a1.py";
        py[2] = nullptr;

        execv("/usr/bin/python3",py);

        std::cerr << "Error: [a3] could not exec a1 \n";
        return 1;

    }

    // Spawn a2
    pid_t proc_a2;
    proc_a2 = fork();

    if (proc_a2 == 0){ // Invoking a2
        dup2(a1_to_a2[0], STDIN_FILENO); //Connect a1 output to a2 input
        dup2(a2_to_a3[1], STDOUT_FILENO); // Connect 12 output to a3 output

        close(a2_to_a3[0]);
        close(a2_to_a3[1]);

        close(a1_to_a2[0]);
        close(a1_to_a2[1]);

        char* argv[] = {(char *)"ece650-a2",  NULL};

        execv("ece650-a2", argv);

        std::cerr << "Error: [a3] could not exec a2 \n";
        return 1;

    }

    else if (proc_a2 < 0) {
    std::cerr << "Error: [a3] could not fork a2 \n";
    return 1;
    }

    bool stay = 1;

    int status_rgen;

    while ( rgen_active == true )if (valid_VE == false) continue; else break;

    // read from stdin until EOF
    while (stay && !std::cin.eof() ) {

        if (rgen_active == false) break;

        // read a line of input until EOL and store in a string
        std::string line;
        std::getline(std::cin, line);

        sleep(0);

        // if nothing was read, go to top of the while to check for eof
        if (line.size() == 0) {
            continue;
        }
        
        switch(line[0])
        {
            case 's':
            fflush(stdout);   
            dup2(a1_to_a2[1], STDOUT_FILENO); // Connect a3 output to a2 input
            cout <<line << endl;
            dup2(a1_to_a2[1], rgen_to_a1[0]); // Connect a1 output to a2 input   
            break;

            case 'p':
            stay = 0;
            break;

            default:
            cout<<"Error: [a3] Invalid Input"<<endl;

        }        
    }
   
  dup2(STDOUT_FILENO, 1); // Connect a3 output to std out

  sleep(2);

  int status_a2;
  kill(proc_a2, SIGTERM);
  waitpid(proc_a2, &status_a2, 0);

  int status_a1;
  kill(proc_a1, SIGTERM);
  waitpid(proc_a1, &status_a1, 0);
  
  kill(proc_rgen, SIGTERM);
  waitpid(proc_rgen, &status_rgen, 0);

  return 0;
}
