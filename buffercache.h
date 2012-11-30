#ifndef _buffercache
#define _buffercache

#include <iostream>
#include <map>

#include "global.h"
#include "block.h"
#include "disksystem.h"

using namespace std;

struct cache_compare_lessthan {
  bool operator()(const SIZE_T s1, const SIZE_T s2) const {
    return s1<s2;
  }
};


//
// LRU block cache with single step prefetch
//
// Write Back
// Write Allocate
class BufferCache {
 private:
  DiskSystem *disk;
  SIZE_T cachesize;
  map<SIZE_T, Block, cache_compare_lessthan> blockmap;
  double curtime;
  SIZE_T allocs, deallocs, reads, writes, diskreads, diskwrites;
 protected:
  ERROR_T CheckDeleteOldest();
 public:
  // Cache size is in number of blocks
  BufferCache(DiskSystem *disk,
	      const SIZE_T cachesize);
  BufferCache() { throw 0; }
  BufferCache(const BufferCache &rhs) { throw 0; } 
  BufferCache & operator=(const BufferCache &rhs) { throw 0; return *this; } 
  ~BufferCache();

  // Call Attach before your first read or write
  // Call Detach after your last read or write
  ERROR_T Attach();
  ERROR_T Detach();

  // Number of blocks in the cache
  SIZE_T GetCacheSize() const;
  // Number of bytes per block
  SIZE_T GetBlockSize() const;
  // Number of blocks in the underlying device
  SIZE_T GetNumBlocks() const;
  // Current time in the simulation (starts at zero)
  double GetCurrentTime() const;

  // outblocknum is the number of the block that we just allocated
  // if the error return is nonzero
  ERROR_T NotifyAllocateBlock(const SIZE_T outblocknum);
  // inblocknum is the block that we just deallocated
  ERROR_T NotifyDeallocateBlock(const SIZE_T inblocknum);
  // check to see if we think the block was allocated
  bool  IsBlockAllocated(const SIZE_T inblocknum);
  
  // returns one of ERROR_NOERROR  (zero)
  // ERROR_NOSUCHBLOCK or other nonzero error codes
  ERROR_T ReadBlock(const SIZE_T inblocknum, Block &outblock);
  
  // returns one of ERROR_NOERROR  (zero)
  // ERROR_NOSUCHBLOCK
  // ERROR_WRONGSIZEBLOCK or other nonzero error codes
  ERROR_T WriteBlock(const SIZE_T inblocknum, const Block &inblock);
  
  // Request that a block be read into the cache
  // This returns immediately.
  // ERROR_NOFETCH means that there is no room currently
  // to prefetch the block and it was not prefetched.
  ERROR_T PrefetchBlock (const SIZE_T blocknum);
  
  // Request that a block be flushed to disk
  // Note that this blocks until the block is finished.
  ERROR_T FlushBlock(const SIZE_T blocknum);
  
 
  SIZE_T GetNumAllocs() const { return allocs; }
  SIZE_T GetNumDeallocs() const { return deallocs; }
  SIZE_T GetNumReads() const { return reads;}
  SIZE_T GetNumWrites() const { return writes;}
  SIZE_T GetNumDiskReads() const { return diskreads;}
  SIZE_T GetNumDiskWrites() const { return diskwrites;}

  ostream & Print(ostream &os) const;
  
};


inline ostream & operator<< (ostream &os, const BufferCache &b) { return b.Print(os);}


#endif
