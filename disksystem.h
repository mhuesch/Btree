#ifndef _disksystem
#define _disksystem

#include <string>
#include <iostream>
#include <vector>

#include "global.h"
#include "block.h"

using namespace std;

// Models a single disk with a single outstanding request
//
// Includes storage allocator and free space bitmap to 
// simplify project - REAL DISKS DO NOT HAVE ALLOCATORS OR BITMAPS
//
class DiskSystem {
 private:
  BYTE_T *bitmap;
  FILE*  datafilefd;
  FILE*  configfilefd;
  FILE*  bitmapfilefd;


  //
  //

  string diskfilestem;
  SIZE_T offset;
  SIZE_T numblocks;
  SIZE_T blocksize;
  SIZE_T numheads;
  SIZE_T blockspertrack;
  SIZE_T numtracks;
  SIZE_T last_track;
  SIZE_T last_sector;
    

  double averageseeklatency;
  double trackseeklatency;
  double rotationallatency;

 protected:
  virtual double ModelAccess(const SIZE_T off, const SIZE_T num);

  ERROR_T SanityCheckConfig();
  ERROR_T InitFromConfigFile();
  ERROR_T InitFromInMemoryConfig();
  ERROR_T ReadConfig();
  ERROR_T WriteConfig();
  ERROR_T ReadBitMap();
  ERROR_T WriteBitMap();
  
   
 public:
  // The data is stored in file "filestem.data"
  // The config is stored in file "filestem.config"

  DiskSystem(const string &filestem,
	     const bool create=false,
	     const SIZE_T offset=0,
	     const SIZE_T blocks=0,
	     const SIZE_T blocksize=0,
	     const SIZE_T heads=0,
	     const SIZE_T blockspertrack=0,
	     const SIZE_T tracks=0,
	     const double avgseek=0,
	     const double trackseek=0,
	     const double rotlat=0);
  DiskSystem() { throw GenericException(); } 
  DiskSystem(const DiskSystem &rhs) { throw GenericException();}
  DiskSystem & operator=(const DiskSystem &rhs) { throw GenericException(); return *this;}

  virtual ~DiskSystem();

  // Each returns the number of milliseconds the operation has taken

  ERROR_T Read(const SIZE_T inoffblock,
	       const SIZE_T numblock,
	       vector<Block> &blocks,
	       double &reqtime);

  ERROR_T Read(const SIZE_T inoffblock, 
	       Block &blocks,
	       double &reqtime);

  ERROR_T Write(const SIZE_T inoffblock,
		const SIZE_T numblock,
		const vector<Block> &blocks,
		double &reqtime);

  ERROR_T Write(const SIZE_T inoffblock, 
		const Block &blocks,
		double &reqtime);

  SIZE_T GetBlockSize() const;
  SIZE_T GetNumBlocks() const;

  //
  // These are notification functions that should be called when
  // a block is allocated or deallocated.  They keep the bitmap updated
  // so that we can sanity check blocks
  //
  ERROR_T NotifyAllocateBlocks(const SIZE_T offset,
			       const SIZE_T innumblocks);
  ERROR_T NotifyDeallocateBlocks(const SIZE_T offset,
				 const SIZE_T innumblocks);

  bool    IsBlockAllocated(const SIZE_T offset);


  ostream & Print(ostream &os) const;
};

inline ostream & operator<< (ostream &os, const DiskSystem &rhs) { return rhs.Print(os);}

#endif
