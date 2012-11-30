#include <string>
#include <stdlib.h>

#include "buffercache.h"


void usage() 
{
  cerr << "usage: writebuffer cachesize filestem blocknum numblocks < data\n";
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

  SIZE_T blocksize = disk.GetBlockSize();

  cache.Attach();

  for (unsigned i=blocknum;i<(blocknum+numblocks);i++) { 
    Block block(blocksize);
    ERROR_T rc;
    for (unsigned j=0;j<blocksize;j++) { 
      cin >> block.data[j];
    }
    rc=cache.NotifyAllocateBlock(i);
    if (rc!=ERROR_NOERROR) { 
      cerr << "Error " << rc <<" occured when notifying cache of allocation of block "<< i << endl;
      return -1;
    }
    rc=cache.WriteBlock(i,block);
    if (rc!=ERROR_NOERROR) { 
      cerr << "Error " << rc <<" occured when writing block "<< i << endl;
      return -1;
    }
  }

  cache.Detach();

  cerr << "Your data was successfully written.\n";

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
