#include <string>
#include <stdlib.h>

#include "disksystem.h"


void usage() 
{
  cerr << "usage: readdisk filestem blocknum numblocks > data\n";
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

  vector<Block> b;

  ERROR_T rc= disk.Read(blocknum, numblocks, b, reqtime);

  if (rc!=ERROR_NOERROR) { 
    cerr << "Error "<< rc << " occured.\n";
    return -1;
  } else {
    cerr << "Read took "<<reqtime<<" milliseconds\n";
    for (vector<Block>::const_iterator i=b.begin();i!=b.end();++i) {
      // cerr << (*i);
      for (SIZE_T j=0;j<(*i).length;j++) { 
	cout << (*i).data[j];
      }
    }
  }
  return 0;
}
