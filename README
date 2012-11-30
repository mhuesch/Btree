Welcome to BTree 

(c) 2004 Peter A. Dinda, pdinda@northwestern.edu

In this lab, you will write a tiny implementation of a B-Tree that
will be run on top of a buffer cache that runs on top of a simulated
disk system.  The simulator will accept a sequence of requests and
print out the total time needed to complete the requests with your
B-Tree implementation.  The simulator will also print out the errors
that occur.

This is instructional code to support EECS 339, Introduction to
Database Systems in the EECS Department at Northwestern University.

Requirements
------------

You must have the following software running:

   GCC 3+ - we are using gcc 4.4.6 (Red Hat)
   Perl 5.8+

You must have enough disk space for the virtual disk
and the test sequences and outputs.

In order to draw Btrees you need to have the Graphviz system (dot) 
installed.

Contents
--------

   README

   Makefile 

   global.h        Global defines
   block.*         Disk block abstraction
   disksystem.*    Simulated disk system with a few extra components
   buffercache.*   LRU buffercache implementation

   btree.h         The required B-Tree interface
   btree.cc        The btree implementation that you will write
                   The handout version contains only stubs

   btree_ds.h
   btree_ds.cc     An implementation of the basic BTree data
                   structures, which you are welcome to use

   makedisk.cc
   infodisk.cc
   readdisk.cc
   writedisk.cc    Tools to create, examine, read, and write virtual
                   disk systems - no allocation is done


   freebuffer,cc
   readbuffer.cc
   writebuffer.cc  Tools to read and write virtual disk systems
                   using a buffer cache.  The results should be 
                   identical to read and writedisk
                   allocation is done here

   btree_init.cc   Initialize the btree structure (like format)
   btree_insert.cc Insert a key,value pair into the btree
   btree_delete.cc Delete a key, value pair from the btree
   btree_update.cc Update a key, value pair in the btree
   btree_lookup.cc Query for the value associated with a tree
   btree_show.cc   Display the btree as (key,value) pairs sorted in key order 
   btree_sane.cc   Sanity Check the btree
                   

   sim.cc          Simulator used to test performance and correctness 
                   of btree implementation

   ref_impl.pl     Reference implementation in Perl for comparison
                   This is correct (when run with bug probability 0)

   test_me.pl      Test the student's implementation (using sim)
 

   test.pl         Test two implementations against each other
   gen_test_sequence.pl
                   Generate a sequence of operations for use in testing
   compare.pl      Compare two outputs resulting from the same test sequence
  


Installing Btree Lab
--------------------

cd your_projects_directory
tar xvfz btree_lab.tgz
cd btree_lab
touch .dependencies
make depend
make clean
make


Understanding Virtual Disk Systems
----------------------------------

You can create a virtual disk using makedisk

$ makedisk mydisk 1024 1024 1 16 64 100 10 .28

This will create a disk which has 1024 blocks, each of length 1024
bytes.  The disk has a single head, and each track has 16 blocks,
leading to 64 blocks total.  The disk has an average seek time of 100
ms, a track-to-track seek time of 10 ms, and a rotational latency of
0.28 ms (it spins at 3600 RPM).  This is for a circa 1979 disk.

The following files are created:

mydisk.config    -   this stores the configuration of the disk
mydisk.data      -   the 1 MB of data in the disk
mydisk.bitmap    -   a bitmap of the allocated blocks of the disk

Notice that real disks do not have allocation bitmaps.  This is a tool
we'll use for debugging.  We'll require that you call the buffer
cache's allocation notification functions whenever you get a new block.

You can now get information about the disk using infodisk, and read
and write blocks using readdisk and writedisk.



Understanding The Buffer Cache
------------------------------

A buffer cache wraps a disk system, providing a similar interface, but
one which does write back, write allocate caching with LRU
replacement.

The read, write, and free buffer programs do allocation and
deallocation, unlike the read and write disk programs.

By exploiting temporal and spatial locality via the buffer cache you 
can improve performance.



Btree
-----

You will implement the Btree class described in btree.h, btree.cc and
the handout.  Once this is implemented, the btree_* tools and sim 
will be functional.

The btree_* tools allow you to manipulate the btree stored on the
virtual disk.  Each tool does exactly one operation.  The btree 
state persists (in the disk files) from operation to operation.  



Testing
-------

To begin with, you should play around with your implementation using
the btree_* tools.  If these crash or leave your btree in a bad state,
there is no point in trying sim.

Sim is a bit different from the btree tools.  Sim takes, from standard
input, a sequence of operations, begining with INIT and ending with
DEINIT.  It runs these operations.  The btree state does not persist
from one run of sim to the next.  

Here is what a stream of operations to sim looks like and what is
done:

INIT keysize valuesize     

  - sim should create a fresh btree and reply "OK"

Any number of the following operations:

INSERT key value           
   
  - sim should insert the pair and reply "OK" if the key does not
    already exists.  If it does already exist, it should insert
    nothing and reply "FAIL".

UPDATE key value           
   
  - sim should update the value associated with the key  and reply 
    "OK" if the key already exists.  If it does not already exist, 
    the btree should not be modified and the reply is "FAIL".

DELETE key
   
  - sim should delete the key and its associated value and reply 
    "OK" if the key already exists.  If it does not already exist, 
    the btree should not be modified and the reply is "FAIL".

LOOKUP key
  - if the key exists, sim replied "OK value", otherwise it replies 
    "FAIL".

Finally, the very last operation is:

DEINIT

  - sim should throw away all state and quit


The reference implementaion, ref_impl.pl shows what sim is supposed to
do.  When test_me.pl is run, a test sequence is generated and run
through both sim and ref_impl.pl.  compare.pl is then used to
determine if there are any differences between the two outputs.


Hand-in
-------

You will be handing in btree.cc and btree.h, and any other files you
add or modify. 

We will test it using test_me.pl, but we won't tell you which random
number seeds we'll use. 





