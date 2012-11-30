#ifndef _btree
#define _btree

#include <iostream>
#include <string>

#include "global.h"
#include "block.h"
#include "disksystem.h"
#include "buffercache.h"

#include "btree_ds.h"

using namespace std;

// To simplify our lives, we will just treat a Key or Value as being
// identical to a block

typedef Block Buffer;

typedef Buffer KeyOrValue;

// A key is a sequence of bytes that is used to search for a value
// A value is a sequence of bytes 
typedef KeyOrValue KEY_T;
typedef KeyOrValue VALUE_T;

struct KeyValuePair {
  KEY_T key;
  VALUE_T value;

  KeyValuePair();
  KeyValuePair(const KEY_T &key, const VALUE_T &value);
  KeyValuePair(const KeyValuePair &rhs);
  virtual ~KeyValuePair();
  KeyValuePair & operator=(const KeyValuePair &rhs);

};

enum BTreeOp {BTREE_OP_INSERT, BTREE_OP_DELETE, BTREE_OP_UPDATE,BTREE_OP_LOOKUP};

enum BTreeDisplayType {BTREE_DEPTH, BTREE_DEPTH_DOT, BTREE_SORTED_KEYVAL};

class BTreeIndex {
 private:
  BufferCache *buffercache;
  SIZE_T       superblock_index;
  BTreeNode    superblock;

 protected:

  ERROR_T      AllocateNode(SIZE_T &node);

  ERROR_T      DeallocateNode(const SIZE_T &node);

  ERROR_T      LookupOrUpdateInternal(const SIZE_T &Node,
				      const BTreeOp op, 
				      const KEY_T &key,
				      VALUE_T &val);
  

  ERROR_T      DisplayInternal(const SIZE_T &node,
			       ostream &o, 
			       const BTreeDisplayType display_type=BTREE_DEPTH) const;
public:
  //
  // keysize and valueszie should be stored in the 
  // superblock.  They are included in the constructor
  // so that it is possible to create a new index by 
  // constructing one with the right key and value sizes
  // and then doing an Attach(initialblock,true) to create it
  // and actually write the data in the superblock.
  // otherwise, the expectation is that keysize and valuesize
  // will be zero and will be read when Attach(initialblock,false) is 
  // invoked
  BTreeIndex(SIZE_T keysize, 
	     SIZE_T valuesize,
	     BufferCache *cache,
	     bool unique=true);   // true if a  key maps to a single value


  BTreeIndex();
  BTreeIndex(const BTreeIndex &rhs);
  virtual ~BTreeIndex();
  BTreeIndex & operator=(const BTreeIndex &rhs);
  

  // This is called before any inserts, updates, or deletes happen
  // If create=true, then initblock is meaningless
  // If create=false, than the index already exists and we are telling you
  // the block that the last detach returned
  // This should be your superblock, which contains the information 
  // you need to find the elements of the tree.
  // return zero on success or ERROR_NOTANINDEX if we are
  // giving you an incorrect block to start with
  ERROR_T Attach(const SIZE_T initblock, const bool create=false );
  
  // This is called after all inserts, updates, or deletes are done.
  // We expect you to tell us the number of your superblock, which
  // we will return to you on the next attach
  ERROR_T Detach(SIZE_T &initblock);
  
  // return zero on success
  // return ERROR_NOSPACE if you run out of disk space
  // return ERROR_SIZE if the key or value are the wrong size for this index
  // return ERROR_CONFLICT if the key already exists and it's a unique index
  ERROR_T Insert(const KEY_T &key, const VALUE_T &value);
  
  // return zero on success
  // return ERROR_NONEXISTENT  if the key doesn't exist
  // return ERROR_SIZE if the key or value are the wrong size for this index
  ERROR_T Update(const KEY_T &key, const VALUE_T &value);
  
  // return zero on success
  // return ERROR_NONEXISTENT  if the key doesn't exist
  // return ERROR_SIZE if the key or value are the wrong size for this index
  ERROR_T Delete(const KEY_T &key);
  
  // return zero on success
  // return ERROR_NONEXISTENT  if the key doesn't exist
  ERROR_T Lookup(const KEY_T &key, VALUE_T &value);

  // Here you should figure out if your index makes sense
  // Is it a tree?  Is it in order?  Is it balanced?  Does each node have
  // a valid use ratio?
  ERROR_T SanityCheck() const;

  // Display tree
  // BTREE_DEPTH means to do a depth first traversal of 
  // the tree, printing each node
  // BTREE_DEPTH_DOT means to do the same way, but print
  // the tree in a Graphviz/dot-compatible way (nodes and edges)
  // BTREE_SORTED_KEYVAL means to do a depth first traversal,
  // like the previous two, but to only print the
  // key/value pairs in the leaves, one "(key, value)" tuple
  // per line.  This will be the keys and values in the tree
  // sorted in order of keys.
  ERROR_T Display(ostream &o, BTreeDisplayType display_type=BTREE_DEPTH) const;
  
  ostream & Print(ostream &os) const;
  
};


inline ostream & operator<<(ostream &os, const BTreeIndex &b) { return b.Print(os);}

#endif
