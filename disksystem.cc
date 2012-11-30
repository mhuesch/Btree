#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <string.h>
#include <stdio.h>

#include <math.h>

#include "disksystem.h"


static SIZE_T mywrite(FILE *f, const SIZE_T off, const BYTE_T *buf, const int len)
{
  SIZE_T left=len;
  SIZE_T sent;

  fseek(f,off,SEEK_SET);
  while (left>0) {
    sent=fwrite(&(buf[len-left]),1,left,f);
    if (sent<0) {	
      return 0;
    } else if (sent==0) {
      break;
    } else {
      left-=sent;
    }
  }
  return len-left;
}

static SIZE_T myread(FILE *f, const SIZE_T off, BYTE_T *buf, const int len, bool trunconeof=true)
{
  SIZE_T left=len;
  SIZE_T sent;

  fseek(f,off,SEEK_SET);
  while (left>0) {
    sent=fread(&(buf[len-left]),1,left,f);
    if (sent<0) {	
      return 0;
    } else if (sent==0) {
      // if we reached this point, the likely cause is that we
      // are trying to read a block which has not been allocated yet
      // Hence, we will try to ftruncate to this size and then retry the
      // read.  However, we don't want to loop forever doing this,
      // hence the trunconeof parameter
      if (!feof(f)) { 
	// OK, the end of file is not the problem
	break;
      } else {
	// EOF case... should we truncate and try again?
	if (!trunconeof) { 
	  break;
	} else {
	  // yes!
	  if (ftruncate(fileno(f),off+len)) { 
	    // uh oh, something weird is going on
	    break;
	  } else {
	    // OK, now retry, but don't truncate a second time
	    return myread(f,off,buf,len,false);
	  }
	}
      }
    } else {
      left-=sent;
    }
  }
  return len-left;
}


DiskSystem::DiskSystem(const string &filestem,
		       const bool   create,
		       const SIZE_T offset,
		       const SIZE_T blcks,
		       const SIZE_T blcksize,
		       const SIZE_T heads,
		       const SIZE_T blckspertrack,
		       const SIZE_T tracks,
		       const double avgseek,
		       const double trackseek,
		       const double rotlat) :
  bitmap(0),
  datafilefd(0),
  configfilefd(0),
  bitmapfilefd(0),
  diskfilestem(filestem), 
  offset(offset),
  numblocks(blcks),
  blocksize(blcksize),
  numheads(heads),
  blockspertrack(blckspertrack),
  numtracks(tracks),
  last_track(0),
  last_sector(0),
  averageseeklatency(avgseek),
  trackseeklatency(trackseek),
  rotationallatency(rotlat)
{
  if (create) { 
    // Only in this case are the parameters used:
    InitFromInMemoryConfig();
  } else {
    InitFromConfigFile();
  }
}

DiskSystem::~DiskSystem()
{
  WriteConfig();
  WriteBitMap();
  fclose(configfilefd);
  fclose(bitmapfilefd);
  fclose(datafilefd);
  delete [] bitmap;
}

ERROR_T DiskSystem::SanityCheckConfig()
{
  if (averageseeklatency<=0 || trackseeklatency<=0 || rotationallatency<=0) { 
    cerr << "Impossible performance.\n";
    return ERROR_BADCONFIG;
  }
  if (numblocks != (numheads*blockspertrack*numtracks)) {
    cerr << "Geometry mismatch.\n";
    return ERROR_BADCONFIG;
  }

  return ERROR_NOERROR;
}


ERROR_T DiskSystem::WriteConfig()
{
  ftruncate(fileno(configfilefd),0);
  rewind(configfilefd);
  fprintf(configfilefd,"# disksystem config file version 0.9\n");
  fprintf(configfilefd,"# filestem\n");
  fprintf(configfilefd,"%s\n",diskfilestem.c_str());
  fprintf(configfilefd,"# offset\n");
  fprintf(configfilefd,"%u\n",offset);
  fprintf(configfilefd,"# numblocks\n");
  fprintf(configfilefd,"%u\n",numblocks);
  fprintf(configfilefd,"# blocksize\n");
  fprintf(configfilefd,"%u\n",blocksize);
  fprintf(configfilefd,"# numheads\n");
  fprintf(configfilefd,"%u\n",numheads);
  fprintf(configfilefd,"# blockspertrack\n");
  fprintf(configfilefd,"%u\n",blockspertrack);
  fprintf(configfilefd,"# numtracks\n");
  fprintf(configfilefd,"%u\n",numtracks);
  fprintf(configfilefd,"# averageseeklatency\n");
  fprintf(configfilefd,"%lf\n",averageseeklatency);
  fprintf(configfilefd,"# trackseeklatency\n");
  fprintf(configfilefd,"%lf\n",trackseeklatency);
  fprintf(configfilefd,"# rotationalatency\n");
  fprintf(configfilefd,"%lf\n",rotationallatency);
  fflush(configfilefd);

  return ERROR_NOERROR;
}



ERROR_T DiskSystem::ReadConfig()
{
  char buf[80];

#define GETNEXTVAL do { fgets(buf,80,configfilefd); } while (buf[0]=='#')  
#define PARSEUNSIGNED(x) do { sscanf(buf,"%u",x); } while (0)
#define PARSEDOUBLE(x) do { sscanf(buf,"%lf",x); } while (0)

  rewind(configfilefd);
  GETNEXTVAL;
  if (buf[strlen(buf)-1]=='\n') { 
    buf[strlen(buf)-1]=0;
  }
  diskfilestem = string(buf);
  GETNEXTVAL;
  PARSEUNSIGNED(&offset);
  GETNEXTVAL;
  PARSEUNSIGNED(&numblocks);
  GETNEXTVAL;
  PARSEUNSIGNED(&blocksize);
  GETNEXTVAL;
  PARSEUNSIGNED(&numheads);
  GETNEXTVAL;
  PARSEUNSIGNED(&blockspertrack);
  GETNEXTVAL;
  PARSEUNSIGNED(&numtracks);
  GETNEXTVAL;
  PARSEDOUBLE(&averageseeklatency);
  GETNEXTVAL;
  PARSEDOUBLE(&trackseeklatency);
  GETNEXTVAL;
  PARSEDOUBLE(&rotationallatency);

  return ERROR_NOERROR;
}


ERROR_T DiskSystem::WriteBitMap()
{
  rewind(bitmapfilefd);
  
  SIZE_T numbitmapbytes = numblocks / 8 + (numblocks%8 != 0); 

  if (mywrite(bitmapfilefd,0,bitmap,numbitmapbytes)!=numbitmapbytes) { 
    cerr << "Can't write bitmap file\n";
    return ERROR_IMPLBUG;
  }
  return ERROR_NOERROR;
}

ERROR_T DiskSystem::ReadBitMap()
{
  rewind(bitmapfilefd);
  
  SIZE_T numbitmapbytes = numblocks / 8 + (numblocks%8 != 0); 

  if (bitmap) { delete [] bitmap; } ;

  bitmap = new BYTE_T [numbitmapbytes];

  if (myread(bitmapfilefd,0,bitmap,numbitmapbytes,false)!=numbitmapbytes) { 
    cerr << "Can't read bitmap file\n";
    return ERROR_IMPLBUG;
  }
  return ERROR_NOERROR;
}



ERROR_T DiskSystem::InitFromConfigFile()
{
  string configname = diskfilestem + ".config";
  string dataname = diskfilestem + ".data";
  string bitmapname = diskfilestem + ".bitmap";
  
  if (configfilefd) { fclose(configfilefd); }
  
  if ((configfilefd = fopen(configname.c_str(),"r+"))==0) { 
    return ERROR_NOFILE;
  }

  int rc = ReadConfig();
  
  if (rc) { 
    return rc;
  }

  rc=SanityCheckConfig();

  if (rc) { 
    return rc;
  }

  if (datafilefd) { fclose(datafilefd);}

  if ((datafilefd = fopen(dataname.c_str(),"r+"))==0) { 
    return ERROR_NOFILE;
  }


  if (bitmapfilefd) { fclose(bitmapfilefd);}

  if ((bitmapfilefd = fopen(bitmapname.c_str(),"r+"))==0) { 
    return ERROR_NOFILE;
  }
  
  rc = ReadBitMap();

  if (rc) { 
    return rc;
  }

  return ERROR_NOERROR;
}


ERROR_T DiskSystem::InitFromInMemoryConfig()
{
  string configname = diskfilestem + ".config";
  string dataname = diskfilestem + ".data";
  string bitmapname = diskfilestem + ".bitmap";

  int rc=SanityCheckConfig();

  if (rc) { 
    return rc;
  }

  // it should be the case that none of the files exist
  // except for the data file, since we may be using a chunk of it
  // ie, think parition.

  struct stat s;
  
  if (stat(configname.c_str(),&s)!=-1 ||
      stat(bitmapname.c_str(),&s)!=-1) { 
    cerr << "Configuration or bitmap files exist for this name!\n";
    return ERROR_BADCONFIG;
  }

  
  // OK, now we'll create the files.


  // config
  if (configfilefd) { fclose(configfilefd); }
  
  if ((configfilefd = fopen(configname.c_str(),"w+"))==0) { 
    return ERROR_NOFILE;
  }

  rc = WriteConfig();
  
  if (rc) { 
    return rc;
  }


  // allocate in-memory bitmap

  SIZE_T numbitmapbytes = numblocks / 8 + (numblocks%8 != 0); 
  
  bitmap = new BYTE_T [numbitmapbytes];

  memset(bitmap,0,numbitmapbytes);

  // create the bitmap file and write out the bitmap

  if (bitmapfilefd) { fclose(bitmapfilefd); }

  if ((bitmapfilefd = fopen(bitmapname.c_str(),"w+"))==0) { 
    return ERROR_NOFILE;
  }

  rc = WriteBitMap();
  
  if (rc) { 
    return rc;
  }

  // Now we'll open the data file
  // notice that we will REUSE an existing data file if it exists
  // The idea is that we will write only from offset to offset+blocksize*numblocks

  if (datafilefd) { fclose(datafilefd);}

  if (stat(dataname.c_str(),&s)!=-1) { 
    // reuse existing datafile
    if ((datafilefd = fopen(dataname.c_str(),"r+"))==0) { 
      return ERROR_NOFILE;
    }
  } else {
    // create new data file
    if ((datafilefd = fopen(dataname.c_str(),"w+"))==0) { 
      return ERROR_NOFILE;
    }
  }

  return ERROR_NOERROR;
}



    

//
// Note, this assumes disk is kept continously busy
// or that time does not advance except during a disk op
//
double DiskSystem::ModelAccess(const SIZE_T offblock, const SIZE_T numblock) 
{

  SIZE_T req_trackstart = (offblock) / (numheads*blockspertrack);
  SIZE_T req_sectorstart=  (offblock) % (numheads*blockspertrack);

  SIZE_T req_trackend = (offblock+numblock-1) / (numheads*blockspertrack);
  SIZE_T req_sectorend=  (offblock+numblock-1) % (numheads*blockspertrack);

  SIZE_T trackhop = (SIZE_T) fabs((double)req_trackstart-(double)last_track);
  double trackhopfrac = (double)trackhop/(double)numtracks;

  // This is a simplistic model.  
  double trackbytracktime = trackhop*trackseeklatency;
  double longseektime = (trackhopfrac/(0.5))*averageseeklatency;
  double timeinseek = trackbytracktime<longseektime ? trackbytracktime : longseektime;

  // Now we are on the first track and we need to wait for the first
  // sector to show up

  SIZE_T sectorhop = (req_sectorstart >= last_sector) ? (req_sectorstart-last_sector) : (blockspertrack - (last_sector - req_sectorstart));
  double sectorhopfrac = (double)sectorhop/(double)blockspertrack;
  double timeinrotation=rotationallatency*sectorhopfrac;

  // Now we've got to read numblockelements

  // The number of side by side tracks we'll deal with:
  SIZE_T numtrackbytrackhops = req_trackend-req_trackstart;
  double timeintrackbytrackhops = numtrackbytrackhops*trackseeklatency;

  // The total number of sectors read
  double timeinreadsectors = rotationallatency*((double)numblock/(double)blockspertrack);

  last_track=req_trackend;
  last_sector=req_sectorend;

  return timeinseek+timeinrotation+timeintrackbytrackhops+timeinreadsectors;
}


ERROR_T DiskSystem::Read(const SIZE_T   inoffblock,
			 const SIZE_T   numblock,
			 vector<Block> &blocks,
			 double        &reqtime)
{
  reqtime=0;

  if (inoffblock+numblock > numblocks) { 
    cerr << "DiskSystem::Read: Attempt to read blocks "<<inoffblock<<" to "<<(inoffblock+numblock-1)<<", but maxmimum block is only "<<(numblocks-1)<<endl;
    return ERROR_NOSPACE;
  }

  reqtime=ModelAccess(inoffblock,numblock);

  for (SIZE_T i=0;i<numblock;i++) { 
    Block b(blocksize);
    if (!IsBlockAllocated(inoffblock+i)) { 
      if (PRINT_DISKSYSTEM_ALLOCATION_ERRORS) {
	cerr <<"DiskSystem::Read: reading unallocated block "<<(i+inoffblock)<<endl;
      }
    }
    if (myread(datafilefd,offset+(inoffblock+i)*blocksize,b.data,blocksize,true)!=blocksize) { 
      cerr << "DiskSystem::Read: myread has failed"<<endl;
      return ERROR_IMPLBUG;
    }
    blocks.push_back(b);
  }

  return ERROR_NOERROR;
}

ERROR_T DiskSystem::Write(const SIZE_T   inoffblock,
			  const SIZE_T   numblock,
			  const vector<Block> &blocks,
			  double        &reqtime)
{
  reqtime=0;

  if (inoffblock+numblock > numblocks) { 
    cerr << "DiskSystem::Write: Attempt to write blocks "<<inoffblock<<" to "<<(inoffblock+numblock-1)<<", but maxmimum block is only "<<(numblocks-1)<<endl;
    return ERROR_NOSPACE;
  }

  reqtime=ModelAccess(inoffblock,numblock);

  for (SIZE_T i=0;i<numblock;i++) { 
    if (!IsBlockAllocated(inoffblock+i)) { 
      if (PRINT_DISKSYSTEM_ALLOCATION_ERRORS) {
	cerr <<"DiskSystem::Write: writing unallocated block "<<(i+inoffblock)<<endl;
      }
    }
    if (mywrite(datafilefd,offset+(inoffblock+i)*blocksize,blocks[i].data,blocksize)!=blocksize) {  
      cerr << "DiskSystem::Write: mywrite has failed"<<endl;
      return ERROR_IMPLBUG;
    }
  }

  return ERROR_NOERROR;
}


ERROR_T DiskSystem::Read(const SIZE_T inoffblock, Block &blocks, double &reqtime)
{
  vector<Block> bl;

  ERROR_T rc = Read(inoffblock,1,bl,reqtime);

  if (rc!=ERROR_NOERROR) { 
    return rc;
  }

  blocks = bl[0];

  return ERROR_NOERROR;
}

ERROR_T DiskSystem::Write(const SIZE_T inoffblock, const Block &blocks, double &reqtime)
{
  vector<Block> bl;

  bl.push_back(blocks);

  ERROR_T rc = Write(inoffblock,1,bl,reqtime);

  if (rc!=ERROR_NOERROR) { 
    return rc;
  }

  return ERROR_NOERROR;
}


SIZE_T DiskSystem::GetBlockSize() const
{
  return blocksize;
}

SIZE_T DiskSystem::GetNumBlocks() const
{
  return numblocks;
}



#define GETBIT(x) ((bitmap[(x)/8] >> (7-((x)%8))) & 0x1)
#define SETBIT(x) do { bitmap[(x)/8] |= 0x1 << (7-((x)%8)); } while (0)
#define CLEARBIT(x) do { bitmap[(x)/8] &= ~(0x1 << (7-((x)%8))); } while (0)


bool DiskSystem::IsBlockAllocated(const SIZE_T block)
{
  return GETBIT(block);
}


ERROR_T DiskSystem::NotifyAllocateBlocks(const SIZE_T offset, const SIZE_T innumblocks)
{
  if (offset+innumblocks > numblocks) { 
    cerr << "Disksystem: NotifyAllocateBlocks: Attempt to allocate"<<offset<<" to "<<(offset+innumblocks-1)<<" but maximum block is "<<(numblocks-1)<<endl;
    return ERROR_NOSUCHBLOCK;
  }


  for (SIZE_T i=offset; i<(offset+innumblocks); i++) { 
    if (IsBlockAllocated(i)) {
      if (PRINT_DISKSYSTEM_ALLOCATION_ERRORS) {
	cerr << "Disksystem: NotifyAllocateBlocks: Block "<<i<<" is being allocated, but it's already allocated!"<<endl;
      }
    }
    SETBIT(i);
  }

  return ERROR_NOERROR;
}

ERROR_T DiskSystem::NotifyDeallocateBlocks(const SIZE_T offset,const SIZE_T innumblocks)
{
  if (offset+innumblocks > numblocks) { 
    cerr << "Disksystem: NotifyDeallocateBlocks: Attempt to deallocate"<<offset<<" to "<<(offset+innumblocks-1)<<" but maximum block is "<<(numblocks-1)<<endl;
    return ERROR_NOSUCHBLOCK;
  }


  for (SIZE_T i=offset; i<(offset+innumblocks); i++) { 
    if (!IsBlockAllocated(i)) {
      if (PRINT_DISKSYSTEM_ALLOCATION_ERRORS) {
	cerr << "Disksystem: NotifyDeallocateBlocks: Block "<<i<<" is being deallocated, but it's already deallocated!"<<endl;
      }
    }
    CLEARBIT(i);
  }

  return ERROR_NOERROR;
}


ostream & DiskSystem::Print(ostream &os) const
{
  os << "DiskSystem(diskfilestem="<<diskfilestem
     << ", offset="<<offset
     << ", numblocks="<<numblocks
     << ", blocksize="<<blocksize
     << ", numheads="<<numheads
     << ", blockspertrack="<<blockspertrack
     << ", numtracks="<<numtracks
     << ", last_track="<<last_track
     << ", last_sector="<<last_sector
     << ", averageseeklatency="<<averageseeklatency
     << ", trackseeklatency="<<trackseeklatency
     << ", rotationallatency="<<rotationallatency
     << ", bitmap=";

  for (SIZE_T i=0;i<numblocks;i++) { 
    if (GETBIT(i)) { 
      os <<"*";
    } else {
      os <<".";
    }
  }

  os <<")";
  return os;
}

  


