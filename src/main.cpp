#include <stdlib.h>
#include <iostream>

#include "heaptest.h"

using namespace std;

int MINIBASE_RESTART_FLAG = 0;

int main(int argc, char **argv)
{
   HeapDriver hd;
   Status dbstatus;

   dbstatus = hd.RunTests();

   // Check if the create database has succeeded
   if (dbstatus != OK) {       
      cout << "Error encountered during hfpage tests: " << endl;
      minibase_errors.show_errors();

	  cin.get();
      return(1);
   }

   cin.get();
   return(0);
}


