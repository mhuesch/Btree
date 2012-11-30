#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <strstream>
#include <fstream>
#include "btree.h"


using namespace std;

void usage()
{
  cerr << "usage: sim filestem cachesize < specfile \n";
}


int main(int argc, char *argv[])
{

  // CONFORMS to the interface of ref_impl.pl

  if (argc != 3){
    usage();
    return 1;
  }

  char *filestem=argv[1];
  SIZE_T cachesize=atoi(argv[2]);
  SIZE_T superblocknum;

  FILE *file; 
  char line[1024];
  int max = 8192;
  ERROR_T rc;
  
  // We'll connect to the btree only once and then
  // run lots of operations
  // so we need to do this outside the loop
  DiskSystem disk(filestem);
  BufferCache cache(&disk,cachesize);
  // will be set on init
  BTreeIndex *btree;


  if ((rc=cache.Attach())!=ERROR_NOERROR) {
    cerr << "Can't attach cache due to error "<<rc<<"\n";
    return -1;
  }
  
  file=stdin;

  //Now simply read each line and call btree functions corresponding to the same
  while (fgets(line, max, file) != NULL){
    // foreach line read we will refer to a case switch statement
    string line2, action, key, value;
    line2 = line;
    istrstream is(line2.c_str(),line2.size());
    is >> action >> key >> value;

    if (action == "INIT") {
      btree = new BTreeIndex(atoi(key.c_str()),atoi(value.c_str()),&cache);
      if ((rc=btree->Attach(0, true))!=ERROR_NOERROR) {
	cerr << "Can't attach btree with initialization due to error "<<rc<<"\n";
	cout << "FAIL\n";
      } else {
	cout << "OK\n";
      }
    } else if (action == "INSERT"){
      if ((rc=btree->Insert(KEY_T(key.c_str()),VALUE_T(value.c_str())))!=ERROR_NOERROR) { 
        cout <<"FAIL"<<endl;
	cerr <<"Can't insert due to error "<<rc<<"\n";
      } else {
        cout <<"OK\n";
      }
    } else if (action == "UPDATE"){
      if ((rc=btree->Update(KEY_T(key.c_str()),VALUE_T(value.c_str())))!=ERROR_NOERROR) { 
        cout <<"FAIL" <<endl;
	cerr <<"Can't update due to error "<<rc<<"\n";
      } else {
        cout <<"OK\n";
      }
    } else if (action == "DELETE"){
      if ((rc=btree->Delete(KEY_T(key.c_str())))!=ERROR_NOERROR) { 
        cout <<"FAIL"<<endl;
	cerr <<"Can't delete due to error "<<rc<<endl;
      } else {
        cout <<"OK\n";
      }
    } else if (action == "LOOKUP"){
      VALUE_T lookup_value;
      if ((rc=btree->Lookup(KEY_T(key.c_str()),lookup_value))!=ERROR_NOERROR) { 
        cout <<"FAIL"<< endl;
	cerr <<"Can't lookup due to error "<<rc<<endl;
      } else {
        cout <<"OK ";
 	for (unsigned int k=0; k<lookup_value.length; k++) {
 	    cout << lookup_value.data[k];
	}
 	cout << endl;
      }
    } else if (action == "DISPLAY") {
      // This should always be OK
      cout <<"OK BEGIN DISPLAY\n";
      btree->Display(cout,BTREE_SORTED_KEYVAL);
      cout <<"OK END DISPLAY\n";
    } else if (action == "DEINIT"){
      if ((rc=btree->Detach(superblocknum))!=ERROR_NOERROR) { 
	cout << "FAIL"<<endl;
	cerr << "Can't detach btree due to error "<<rc<<endl;
      } else {
	if ((rc=cache.Detach())!=ERROR_NOERROR) { 
	  cout <<"FAIL"<<endl;
	  cerr <<"Can't detach cache due to error "<<rc<<endl;
	} else {
	  delete btree;
	  cout << "OK\n";
	}
      }
    }
  }
    
  fclose(file);

  return 0;

}

