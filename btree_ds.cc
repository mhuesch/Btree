#include <new>
#include <iostream>
#include <assert.h>
#include <string.h>

#include "btree_ds.h"
#include "buffercache.h"

#include "btree.h"

using namespace std;

SIZE_T NodeMetadata::GetNumDataBytes() const
{
  SIZE_T n=blocksize-sizeof(*this);
  return n;
}


SIZE_T NodeMetadata::GetNumSlotsAsInterior() const
{
  return (GetNumDataBytes()-sizeof(SIZE_T))/(keysize+sizeof(SIZE_T));  // floor intended
}

SIZE_T NodeMetadata::GetNumSlotsAsLeaf() const
{
  return (GetNumDataBytes()-sizeof(SIZE_T))/(keysize+valuesize);  // floor intended
}


ostream & NodeMetadata::Print(ostream &os) const 
{
  os << "NodeMetaData(nodetype="<<(nodetype==BTREE_UNALLOCATED_BLOCK ? "UNALLOCATED_BLOCK" :
				   nodetype==BTREE_SUPERBLOCK ? "SUPERBLOCK" :
				   nodetype==BTREE_ROOT_NODE ? "ROOT_NODE" :
				   nodetype==BTREE_INTERIOR_NODE ? "INTERIOR_NODE" :
				   nodetype==BTREE_LEAF_NODE ? "LEAF_NODE" : "UNKNOWN_TYPE")
     << ", keysize="<<keysize<<", valuesize="<<valuesize<<", blocksize="<<blocksize
     << ", rootnode="<<rootnode<<", freelist="<<freelist<<", numkeys="<<numkeys<<")";
  return os;
}

BTreeNode::BTreeNode() 
{
  info.nodetype=BTREE_UNALLOCATED_BLOCK;
  data=0;
}

BTreeNode::~BTreeNode()
{
  if (data) { 
    delete [] data;
  }
  data=0;
  info.nodetype=BTREE_UNALLOCATED_BLOCK;
}


BTreeNode::BTreeNode(int node_type, SIZE_T key_size, SIZE_T value_size, SIZE_T block_size)
{
  info.nodetype=node_type;
  info.keysize=key_size;
  info.valuesize=value_size;
  info.blocksize=block_size;
  info.rootnode=0;
  info.freelist=0;
  info.numkeys=0;				       
  data=0;
  if (info.nodetype!=BTREE_UNALLOCATED_BLOCK && info.nodetype!=BTREE_SUPERBLOCK) {
    data = new char [info.GetNumDataBytes()];
    memset(data,0,info.GetNumDataBytes());
  }
}

BTreeNode::BTreeNode(const BTreeNode &rhs) 
{
  info.nodetype=rhs.info.nodetype;
  info.keysize=rhs.info.keysize;
  info.valuesize=rhs.info.valuesize;
  info.blocksize=rhs.info.blocksize;
  info.rootnode=rhs.info.rootnode;
  info.freelist=rhs.info.freelist;
  info.numkeys=rhs.info.numkeys;				       
  data=0;
  if (rhs.data) { 
   data=new char [info.GetNumDataBytes()];
    memcpy(data,rhs.data,info.GetNumDataBytes());
  }
}


BTreeNode & BTreeNode::operator=(const BTreeNode &rhs) 
{
  return *(new (this) BTreeNode(rhs));
}


ERROR_T BTreeNode::Serialize(BufferCache *b, const SIZE_T blocknum) const
{
  assert((unsigned)info.blocksize==b->GetBlockSize());

  Block block(sizeof(info)+info.GetNumDataBytes());

  memcpy(block.data,&info,sizeof(info));
  if (info.nodetype!=BTREE_UNALLOCATED_BLOCK && info.nodetype!=BTREE_SUPERBLOCK) { 
    memcpy(block.data+sizeof(info),data,info.GetNumDataBytes());
  }

  return b->WriteBlock(blocknum,block);
}


ERROR_T  BTreeNode::Unserialize(BufferCache *b, const SIZE_T blocknum)
{
  Block block;

  ERROR_T rc;

  rc=b->ReadBlock(blocknum,block);

  if (rc!=ERROR_NOERROR) {
    return rc;
  }

  memcpy(&info,block.data,sizeof(info));
  
  if (data) { 
    delete [] data;
    data=0;
  }

  assert(b->GetBlockSize()==(unsigned)info.blocksize);

  if (info.nodetype!=BTREE_UNALLOCATED_BLOCK && info.nodetype!=BTREE_SUPERBLOCK) {
    data = new char [info.GetNumDataBytes()];
    memcpy(data,block.data+sizeof(info),info.GetNumDataBytes());
  }
  
  return ERROR_NOERROR;
}


char * BTreeNode::ResolveKey(const SIZE_T offset) const
{
  switch (info.nodetype) { 
  case BTREE_INTERIOR_NODE:
  case BTREE_ROOT_NODE:
    assert(offset<info.numkeys);
    return data+sizeof(SIZE_T)+offset*(sizeof(SIZE_T)+info.keysize);
    break;
  case BTREE_LEAF_NODE:
    assert(offset<info.numkeys);
    return data+sizeof(SIZE_T)+offset*(info.keysize+info.valuesize);
    break;
  default:
    return 0;
  }
}


char * BTreeNode::ResolvePtr(const SIZE_T offset) const
{
  switch (info.nodetype) { 
  case BTREE_INTERIOR_NODE:
  case BTREE_ROOT_NODE:
    assert(offset<=info.numkeys);
    return data+offset*(sizeof(SIZE_T)+info.keysize);
    break;
  case BTREE_LEAF_NODE:
    assert(offset==0);
    return data;
    break;
  default:
    return 0;
  }
}



char * BTreeNode::ResolveVal(const SIZE_T offset) const
{
  switch (info.nodetype) { 
  case BTREE_LEAF_NODE:
    assert(offset<info.numkeys);
    return data+sizeof(SIZE_T)+offset*(info.keysize+info.valuesize)+info.keysize;
    break;
  default:
    return 0;
  }
}



char * BTreeNode::ResolveKeyVal(const SIZE_T offset) const
{
  return ResolveKey(offset);
}

ERROR_T BTreeNode::GetKey(const SIZE_T offset, KEY_T &k) const
{
  char *p=ResolveKey(offset);

  if (p==0) { 
    return ERROR_NOMEM;
  }
  
  k.Resize(info.keysize,false);
  memcpy(k.data,p,info.keysize);
  return ERROR_NOERROR;
}

ERROR_T BTreeNode::GetPtr(const SIZE_T offset, SIZE_T &ptr) const
{
  char *p=ResolvePtr(offset);

  if (p==0) { 
    return ERROR_NOMEM;
  }
  
  memcpy(&ptr,p,sizeof(SIZE_T));
  return ERROR_NOERROR;
}

ERROR_T BTreeNode::GetVal(const SIZE_T offset, VALUE_T &v) const
{
  char *p=ResolveVal(offset);

  if (p==0) { 
    return ERROR_NOMEM;
  }
  
  v.Resize(info.valuesize,false);
  memcpy(v.data,p,info.valuesize);
  return ERROR_NOERROR;
}


ERROR_T BTreeNode::GetKeyVal(const SIZE_T offset, KeyValuePair &p) const
{
  ERROR_T rc= GetKey(offset,p.key);

  if (rc!=ERROR_NOERROR) { 
    return rc; 
  } else {
    return GetVal(offset,p.value);
  }
}


ERROR_T BTreeNode::SetKey(const SIZE_T offset, const KEY_T &k)
{
  char *p=ResolveKey(offset);

  if (p==0) { 
    return ERROR_NOMEM;
  }

  memcpy(p,k.data,info.keysize);

  return ERROR_NOERROR;
}


ERROR_T BTreeNode::SetPtr(const SIZE_T offset, const SIZE_T &ptr)
{
  char *p=ResolvePtr(offset);

  if (p==0) { 
    return ERROR_NOMEM;
  }

  memcpy(p,&ptr,sizeof(SIZE_T));

  return ERROR_NOERROR;
}



ERROR_T BTreeNode::SetVal(const SIZE_T offset, const VALUE_T &v)
{
  char *p=ResolveVal(offset);
  
  if (p==0) { 
    return ERROR_NOMEM;
  }
  
  memcpy(p,v.data,info.valuesize);
  
  return ERROR_NOERROR;
}


ERROR_T BTreeNode::SetKeyVal(const SIZE_T offset, const KeyValuePair &p)
{
  ERROR_T rc=SetKey(offset,p.key);

  if (rc!=ERROR_NOERROR) { 
    return rc;
  } else {
    return SetVal(offset,p.value);
  }
}




ostream & BTreeNode::Print(ostream &os) const 
{
  os << "BTreeNode(info="<<info;
  if (info.nodetype!=BTREE_UNALLOCATED_BLOCK && info.nodetype!=BTREE_SUPERBLOCK) { 
    os <<", ";
    if (info.nodetype==BTREE_INTERIOR_NODE || info.nodetype==BTREE_ROOT_NODE) {
      SIZE_T ptr;
      KEY_T key;
      os << "pointers_and_values=(";
      if (info.numkeys>0) { // ==0 implies an empty root node
	for (SIZE_T i=0;i<info.numkeys;i++) {
	  GetPtr(i,ptr);
	  os<<ptr<<", ";
	  GetKey(i,key);
	  os<<key<<", ";
	}
	GetPtr(info.numkeys,ptr);
	os <<ptr;
      } 
      os << ")";
	
    }
    if (info.nodetype==BTREE_LEAF_NODE) { 
      KEY_T key;
      VALUE_T val;
      os << "keys_and_values=(";
      for (SIZE_T i=0;i<info.numkeys;i++) {
	if (i>0) { 
	  os<<", ";
	}
	GetKey(i,key);
	os<<key<<", ";
	GetVal(i,val);
	os<<val;
      }
      os <<")";
    }
  }
  os <<")";
  return os;
}
