#ifndef _btree_ds
#define _btree_ds

#include <iostream>
#include "global.h"
#include "block.h"

using namespace std;

// Types of nodes
#define BTREE_UNALLOCATED_BLOCK 0
#define BTREE_SUPERBLOCK 1
#define BTREE_ROOT_NODE 2
#define BTREE_INTERIOR_NODE 3
#define BTREE_LEAF_NODE 4


typedef Block Buffer;
typedef Buffer KeyOrValue;
typedef KeyOrValue KEY_T;
typedef KeyOrValue VALUE_T;


class BufferCache;
struct KeyValuePair;

struct NodeMetadata {
  int nodetype;
  SIZE_T keysize; 
  SIZE_T valuesize;
  SIZE_T blocksize;
  SIZE_T rootnode; //meaningful only for superblock
  SIZE_T freelist; //meaningful only for superblock or a free block
  SIZE_T numkeys;

  SIZE_T GetNumDataBytes() const;
  SIZE_T GetNumSlotsAsInterior() const;
  SIZE_T GetNumSlotsAsLeaf() const;

  ostream &Print(ostream &rhs) const;
			  
};


inline ostream & operator<< (ostream &os, const NodeMetadata &node) { return node.Print(os); }



//
// Interior node:
//
// PTR KEY PTR KEY PTR KEY PTR
//
// Leaf:
//
// PTR* KEY VALUE KEY VALUE KEY VALUE
//
// *Here this pointer is not used


struct BTreeNode {
  NodeMetadata  info;
  char         *data;
  //
  // unallocated or superblock => blank
  // interior => array of keys
  // leaf => array of key/value pairs


  BTreeNode();
  //
  // Note: This destructor is INTENTIONALLY left non-virtual
  //       This class must NOT have a vtable pointer
  //         because we will serialize it directly to disk
  //
  ~BTreeNode();
  BTreeNode(int node_type, SIZE_T key_size, SIZE_T value_size, SIZE_T block_size);
  BTreeNode(const BTreeNode &rhs);
  BTreeNode & operator=(const BTreeNode &rhs);
  
  ERROR_T Serialize(BufferCache *b, const SIZE_T block) const;
  ERROR_T Unserialize(BufferCache *b, const SIZE_T block);

  char *ResolveKey(const SIZE_T offset) const; // Gives a pointer to the ith key  (interior or leaf)
  char *ResolvePtr(const SIZE_T offset) const; // Gives a pointer to the ith pointer (interior)
  char *ResolveVal(const SIZE_T offset) const; // Gives a pointer to the ith value (leaf)
  char *ResolveKeyVal(const SIZE_T offset) const ; // Gives a pointer to the ith keyvalue pair (leaf)

  ERROR_T GetKey(const SIZE_T offset, KEY_T &k) const ; // Gives the ith key  (interior or leaf)
  ERROR_T GetPtr(const SIZE_T offset, SIZE_T &p) const ;   // Gives the ith pointer (interior)
  ERROR_T GetVal(const SIZE_T offset, VALUE_T &v) const ; // Gives  the ith value (leaf)
  ERROR_T GetKeyVal(const SIZE_T offset, KeyValuePair &p) const; // Gives  the ith key value pair (leaf)


  ERROR_T SetKey(const SIZE_T offset, const KEY_T &k); // Writesthe ith key  (interior or leaf)
  ERROR_T SetPtr(const SIZE_T offset, const SIZE_T &p);   // Writes the ith pointer (interior)
  ERROR_T SetVal(const SIZE_T offset, const VALUE_T &v); // Writes the ith value (leaf)
  ERROR_T SetKeyVal(const SIZE_T offset, const KeyValuePair &p); // Writes the ith key value pair (leaf)

  ostream &Print(ostream &rhs) const;
};


inline ostream & operator<<(ostream &os, const BTreeNode &node) { return node.Print(os); }






#endif
