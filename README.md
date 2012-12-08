Btree
=====

B-Tree project for EECS 339

Michael Hueschen, Scott Neaves, & Glenn Fellman
mah635, stn147, gvf706

  Our B-Tree implements the functions intialize, attach, insert, update, lookup, sanitycheck, and display.
  
  The version of insert we implemented splits nodes as needed and does new root creation.
  Tests indicate that this function is stable barring disk overflow. 
  
  Insert as well as intialize, attach, update, and lookup are all implemented to be independant of key and value size.
  
  Debugging tools sanitycheck and display are functional as well. Sanity check is run in sim's read/execute loop, by typing "SANE" 
