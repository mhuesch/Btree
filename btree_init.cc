#include <stdio.h>
#include <stdlib.h>
#include "btree.h"

void usage() 
{
  cerr << "usage: btree_init filestem cachesize keysize valuesize\n";
}


int main(int argc, char **argv)
{
  char *filestem;
  SIZE_T cachesize, keysize, valuesize;
  SIZE_T superblocknum;

  if (argc!=5) { 
    usage();
    return -1;
  }

  filestem=argv[1];
  cachesize=atoi(argv[2]);
  keysize=atoi(argv[3]);
  valuesize=atoi(argv[4]);

  DiskSystem disk(filestem);
  BufferCache cache(&disk,cachesize);
  BTreeIndex btree(keysize,valuesize,&cache);
  
  ERROR_T rc;

  if ((rc=cache.Attach())!=ERROR_NOERROR) { 
    cerr << "Can't attach buffer cache due to error"<<rc<<endl;
    return -1;
  }

  if ((rc=btree.Attach(0,true))!=ERROR_NOERROR) { 
    cerr << "Can't attach to index with creation due to error "<<rc<<endl;
    return -1;
  } else {
    cerr << "Index created!"<<endl;
    if ((rc=btree.Detach(superblocknum))!=ERROR_NOERROR) { 
      cerr <<"Can't detach from index due to error "<<rc<<endl;
      return -1;
    }
    if ((rc=cache.Detach())!=ERROR_NOERROR) { 
      cerr <<"Can't detach from cache due to error "<<rc<<endl;
      return -1;
    }
    cerr << "Performance statistics:\n";
    
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
}
  

  
