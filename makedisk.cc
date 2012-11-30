#include <string>
#include <stdlib.h>

#include "disksystem.h"


void usage() 
{
  cerr << "usage: makedisk filestem blocks blocksize heads blockspertrack tracks avgseek trackseek rotlat\n";
}

int main(int argc, char *argv[])
{
  if (argc<10) { 
    usage();
    exit(-1);
  }

  DiskSystem disk(argv[1],
		  true,
		  0,
		  atoi(argv[2]),
		  atoi(argv[3]),
		  atoi(argv[4]),
		  atoi(argv[5]),
		  atoi(argv[6]),
		  atof(argv[7]),
		  atof(argv[8]),
		  atof(argv[9]));
  
  
  cerr << "Disk is as follows.\n" << disk << "\n";

  cerr << "Done.\n";

  return 0;
}
