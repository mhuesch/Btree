#include <string>
#include <stdlib.h>

#include "buffercache.h"


void usage() 
{
  cerr << "usage: freebuffer cachesize filestem blocknum numblocks\n";
}

int main(int argc, char *argv[])
{
  if (argc<5) { 
    usage();
    exit(-1);
  }
  SIZE_T cachesize=atoi(argv[2]);
  SIZE_T blocknum=atoi(argv[3]);
  SIZE_T numblocks=atoi(argv[4]);

  DiskSystem disk(argv[1]);
  BufferCache cache(&disk,cachesize);

  cache.Attach();

  for (unsigned i=blocknum;i<(blocknum+numblocks);i++) { 
    ERROR_T rc=cache.NotifyDeallocateBlock(i);
    if (rc!=ERROR_NOERROR) { 
      cerr << "Error " << rc <<" occured when notifying cache of allocation of block "<< i << endl;
      return -1;
    }
  }

  cache.Detach();

  cerr << "Deallocation done\n";

  return 0;
}
