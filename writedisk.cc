#include <string>
#include <stdlib.h>

#include "disksystem.h"


void usage() 
{
  cerr << "usage: writedisk filestem blocknum numblocks < data\n";
}

int main(int argc, char *argv[])
{
  if (argc<4) { 
    usage();
    exit(-1);
  }
  SIZE_T blocknum=atoi(argv[2]);
  SIZE_T numblocks=atoi(argv[3]);
  double reqtime;

  DiskSystem disk(argv[1]);
  SIZE_T blocksize = disk.GetBlockSize();

  vector<Block> b;

  for (unsigned i=0;i<numblocks;i++) { 
    Block block(blocksize);
    for (unsigned j=0;j<blocksize;j++) { 
      cin >> block.data[j];
    }
    b.push_back(block);
  }


  ERROR_T rc= disk.Write(blocknum, numblocks, b, reqtime);

  if (rc!=ERROR_NOERROR) { 
    cerr << "Error "<< rc << " occured.\n";
    return -1;
  } else {
    cerr << "Your data was successfully written.\n";
  }
  return 0;
}
