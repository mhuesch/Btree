#include <string>
#include <stdlib.h>

#include "disksystem.h"


void usage() 
{
  cerr << "usage: readdisk filestem\n";
}

int main(int argc, char *argv[])
{
#if 0
  if (argc<2) { 
    usage();
    exit(-1);
  }
#endif

  DiskSystem disk(argv[1]);
  
  cerr << "Disk is as follows.\n" << disk << "\n";

  cerr << "Done.\n";

  return 0;
}
