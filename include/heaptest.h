#ifndef _HEAP_DRIVER_H_
#define _HEAP_DRIVER_H_

#include "test.h"

class HeapDriver : public TestDriver
{
public:

    HeapDriver();
   ~HeapDriver();

    Status RunTests();

private:
    int choice;

    bool Test1();
    bool Test2();
    bool Test3();
    bool Test4();
    bool Test5();
    bool Test6();

    Status RunAllTests();
    const char* TestName();
};

#endif
