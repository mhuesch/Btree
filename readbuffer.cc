#include <string>
#include <stdlib.h>

#include "buffercache.h"


void usage() 
{
  cerr << "usage: readbuffer cachesize filestem blocknum numblocks > data\n";
}

int main(int argc, char *argv[])
{
  if (argc<5) { 
    usage();
    exit(-1);
  }
  SIZE_T cachesize=atoi(argv[1]);
  SIZE_T blocknum=atoi(argv[3]);
  SIZE_T numblocks=atoi(argv[4]);

  DiskSystem disk(argv[2]);
  BufferCache cache(&disk,cachesize);

  SIZE_T blocksize = disk.GetBlockSize();

  cache.Attach();

  for (unsigned i=blocknum;i<(blocknum+numblocks);i++) { 
    Block block(blocksize);
    ERROR_T rc;
    rc=cache.ReadBlock(i,block);
    if (rc!=ERROR_NOERROR) { 
      cerr << "Error " << rc <<" occured when reading block "<< i << endl;
      return -1;
    }
    for (SIZE_T j=0;j<block.length;j++) { 
      cout << block.data[j];
    }
  }

  cache.Detach();

  cerr << "Your data was successfully read.\n";

  cerr << "numallocs       = "<<cache.GetNumAllocs()<<endl;
  cerr << "numdeallocs     = "<<cache.GetNumDeallocs()<<endl;
  cerr << "numreads        = "<<cache.GetNumReads()<<endl;
  cerr << "numdiskreads    = "<<cache.GetNumDiskReads()<<endl;
  cerr << "numwrites       = "<<cache.GetNumWrites()<<endl;
  cerr << "numdiskwrites   = "<<cache.GetNumDiskWrites()<<endl;
  cerr << endl;

  cerr << "total time      = "<<cache.GetCurrentTime()<<endl;

  return 0;
}
