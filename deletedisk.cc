#include <string>
#include <stdlib.h>
#include <stdio.h>

#include "disksystem.h"


void usage() 
{
  cerr << "usage: deletedisk filestem\n"; 
}

int main(int argc, char *argv[])
{
  if (argc<2) { 
    usage();
    exit(-1);
  }

  remove((string(argv[1])+".data").c_str());
  remove((string(argv[1])+".bitmap").c_str());
  remove((string(argv[1])+".config").c_str());

  cerr << "Done.\n";

  return 0;
}
