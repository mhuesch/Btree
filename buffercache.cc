#include "buffercache.h"

ERROR_T BufferCache::CheckDeleteOldest()
{
  // In a real buffer cache, we would use a priority queue to make this O(1)

  map<SIZE_T, Block, cache_compare_lessthan>::iterator oldestptr=blockmap.end();
  double oldest = curtime+1;

  // Only delete if the cache is full
  if (blockmap.size() < cachesize) {
    return ERROR_NOERROR;
  }

  // Find oldest

  for (map<SIZE_T, Block, cache_compare_lessthan>::iterator i=blockmap.begin();
	 i!=blockmap.end();
	 ++i) {
       if ((*i).second.lastaccessed<oldest) { 
	 oldestptr=i;
	 oldest=(*i).second.lastaccessed;
       }
  }
  
  // write and delete it if it exists
 
  if (oldestptr!=blockmap.end()) { 
    if ((*oldestptr).second.dirty) {
      double reqtime;
      int rc=disk->Write((*oldestptr).first,
			 (*oldestptr).second,
			 reqtime);
      curtime+=reqtime;
      diskwrites++;
      if (rc!=ERROR_NOERROR) { 
	return rc;
      }
    }
    blockmap.erase(oldestptr);
  }
  return ERROR_NOERROR;
}

BufferCache::BufferCache(DiskSystem *d,
			 SIZE_T cs) : 
   disk(d), cachesize(cs), curtime(0),
   allocs(0), deallocs(0), reads(0), writes(0),
   diskreads(0), diskwrites(0)
{}


BufferCache::~BufferCache()
{
  if (disk) { 
    Detach();
  }
  disk=0; cachesize=0; curtime=0;
}

ERROR_T BufferCache::Attach()
{
  blockmap.clear();
  return ERROR_NOERROR;
}

ERROR_T BufferCache::Detach()
{
  // write out all of our data and then throw it away

  for (map<SIZE_T, Block, cache_compare_lessthan>::iterator i=blockmap.begin();
	 i!=blockmap.end();
	 ++i) {
    if ((*i).second.dirty) { 
      double reqtime;
      int rc=disk->Write((*i).first,
			 (*i).second,
			 reqtime);
      curtime+=reqtime;
      diskwrites++;
      if (rc!=ERROR_NOERROR) { 
	return rc;
      }
    }
  }
  blockmap.clear();
  return ERROR_NOERROR;
}


SIZE_T BufferCache::GetCacheSize() const
{
  return cachesize;
}


SIZE_T BufferCache::GetBlockSize() const
{
  return disk->GetBlockSize();
}

SIZE_T BufferCache::GetNumBlocks() const
{
  return disk->GetNumBlocks();
}

double BufferCache::GetCurrentTime() const
{
  return curtime;
}

ERROR_T BufferCache::NotifyAllocateBlock(const SIZE_T outblocknum)
{
  allocs++;
  return disk->NotifyAllocateBlocks(outblocknum,1);
}

ERROR_T BufferCache::NotifyDeallocateBlock(const SIZE_T inblocknum)
{
  deallocs++;
  return disk->NotifyDeallocateBlocks(inblocknum,1);
}


bool  BufferCache::IsBlockAllocated(const SIZE_T inblocknum)
{
  return disk->IsBlockAllocated(inblocknum);
}


ERROR_T BufferCache::ReadBlock(const SIZE_T inblocknum, Block &outblock) 
{
  map<SIZE_T, Block, cache_compare_lessthan>::iterator b;

  b = blockmap.find(inblocknum);

  if (b!=blockmap.end()) {
    // It's in  cache, just update its lastaccessed and return it
    outblock=(*b).second;
    (*b).second.lastaccessed=curtime;
    reads++;
    return ERROR_NOERROR;
  } else {
    // It's not in cache, so time to allocate it
    CheckDeleteOldest();
    // read it from disk
    if (!(disk->IsBlockAllocated(inblocknum))) { 
      if (PRINT_BUFFERCACHE_ALLOCATION_ERRORS) {
	cerr << "BufferCache::ReadBlock: Attempt to read unallocated block " << inblocknum<<endl;
      }
    }
    double reqtime;
    int rc = disk->Read(inblocknum,
			outblock,
			reqtime);
    curtime+=reqtime;
    diskreads++;
    if (rc!=ERROR_NOERROR) { 
      return rc;
    } else {
      outblock.lastaccessed=curtime;
      outblock.dirty=false;
      blockmap[inblocknum]=outblock;
      reads++;
      return ERROR_NOERROR;
    }
  }
} 
 
ERROR_T BufferCache::WriteBlock(const SIZE_T inblocknum, const Block &inblock)
{
  map<SIZE_T, Block, cache_compare_lessthan>::iterator b;
  
  b = blockmap.find(inblocknum);

  if (b!=blockmap.end()) {
    // It's in  cache, so just replace the block
    (*b).second=inblock;
    (*b).second.lastaccessed=curtime;
    (*b).second.dirty=true;
    writes++;
    return ERROR_NOERROR;
  } else {
    // It's not in cache, so time to allocate it
    CheckDeleteOldest();
    if (!(disk->IsBlockAllocated(inblocknum))) { 
      if (PRINT_BUFFERCACHE_ALLOCATION_ERRORS) {
	cerr << "BufferCache::WriteBlock: Attempt to write unallocated block " << inblocknum << endl;
      }
    }
    Block myblock=inblock;
    myblock.lastaccessed=curtime;
    myblock.dirty=true;
    blockmap[inblocknum]=myblock;
    writes++;
    return ERROR_NOERROR;
  }
}
  
ERROR_T BufferCache::PrefetchBlock (const SIZE_T blocknum)
{
  // Not implemented yet
  return ERROR_IMPLBUG;
}
  
ERROR_T BufferCache::FlushBlock(const SIZE_T blocknum)
{
  map<SIZE_T, Block, cache_compare_lessthan>::iterator b;
  
  b = blockmap.find(blocknum);

  if (b==blockmap.end()) { 
    return ERROR_NOERROR;
  } else {
    if ((*b).second.dirty) { 
      double reqtime;
      int rc;
      rc=disk->Write((*b).first,
		     (*b).second,
		     reqtime);
      diskwrites++;
      curtime+=reqtime;
      if (rc!=ERROR_NOERROR) { 
	return rc;
      }
    }
    blockmap.erase(b);
    return ERROR_NOERROR;
  }
}
  
ostream & BufferCache::Print(ostream &os) const
{
  os << "BufferCache(cachesize="<<cachesize
     << ", blocksize="<<GetBlockSize()
     << ", curtime="<<curtime
     << ", allocs="<<allocs
     << ", deallocs="<<deallocs
     << ", reads="<<reads
     << ", writes="<<writes
     << ", diskreads="<<diskreads
     << ", diskwrites="<<diskwrites
     << ", blocks = {";

  
  for (map<SIZE_T, Block, cache_compare_lessthan>::const_iterator b=blockmap.begin(); 
       b!=blockmap.end(); 
       ++b) {
    if (b!=blockmap.begin()) { 
      os << ", ";
    }
    os << (*b).first << ((*b).second.dirty ? "(dirty)" : "");
  }
  os << "}, disk="<<*disk<<")";
  
  return os;
}
  
