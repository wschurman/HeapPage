#include <iostream>
#include <stdlib.h>
#include <memory.h>

#include "heappage.h"
#include "heapfile.h"
#include "db.h"

using namespace std;

//------------------------------------------------------------------
// HeapPage::Init
//
// Input     : Page ID.
// Output    : None.
// Purpose   : Inialize the page with given PageID.
//------------------------------------------------------------------

void HeapPage::Init(PageID pageNo)
{
	nextPage = INVALID_PAGE;     
	prevPage = INVALID_PAGE;     
	numOfSlots = 0;              
	pid = pageNo;               
	freePtr = 0;                // start pointer at beginning of data array
	freeSpace = HEAPPAGE_DATA_SIZE;  // free space starts as full data array
}


//------------------------------------------------------------------
// HeapPage::InsertRecord
//
// Input     : Pointer to the record and the record's length.
// Output    : Record ID of the record inserted.
// Purpose   : Insert a record into the page.
// Return    : OK if everything went OK, DONE if sufficient space 
//             does not exist.
//------------------------------------------------------------------

Status HeapPage::InsertRecord(const char *recPtr, int length, RecordID& rid)
{
	if(freeSpace < length) return DONE;			 // if there's not enough free space in array to fit record

	bool emptySlot = false;                      // boolean to store whether there's an empty slot available
	Slot *slotPointer = GetFirstSlotPointer();   // start looking in first slot
	int currSlot = 0;                            // counter for slots 
	while(currSlot < numOfSlots) {               // loop through all slots in numSlots
		if(SlotIsEmpty(slotPointer)) {
			// empty slot found, insert here
			emptySlot = true;
			//currSlot++;
			break;
		}
		slotPointer--; //current pointer not to empty so try next one
		currSlot++;
    }

	if(!emptySlot) {                              //no empty slot found
		if(freeSpace < length + sizeof(Slot)) return DONE; // if there's not enough space for slot + record, quit
		numOfSlots++;                             // increment number of slots for new slot
		freeSpace -= sizeof(Slot);                // adjust free space for new slot space
	} 

	FillSlot(slotPointer, freePtr, length);       // call FillSlot to create new slot at slotPointer's location

	memcpy(&(data[freePtr]), recPtr, length);     // move new record data to free pointer location    
	freeSpace -= length;                      // remove record size from freespace
	freePtr += length;                        // move free pointer forward by record size

	rid.pageNo = PageNo();                    // store current page in record ID
	rid.slotNo = currSlot;                    // store current slot No in record ID
	return OK;
}


//------------------------------------------------------------------
// HeapPage::DeleteRecord 
//
// Input    : Record ID.
// Output   : None.
// Purpose  : Delete a record from the page.
// Return   : OK if successful, FAIL otherwise.
//------------------------------------------------------------------ 

Status HeapPage::DeleteRecord(RecordID rid)
{
	if (rid.slotNo >= numOfSlots) return FAIL;          // if the slotNo is greater than the number of slots, fail
	Slot* cur = GetFirstSlotPointer() - rid.slotNo;     // make a slot pointer to slot that record is in 
	if (SlotIsEmpty(cur)) return FAIL;                  // if that slot is actually empty, fail

	//move data that starts after record and ends at free pointer up to the location where the record to 
	//be deleted starts, to free memory space. 
	memmove(&(data[cur->offset]), &(data[cur->offset + cur->length]), freePtr-(cur->offset + cur->length));

	Slot* slotPointer = GetFirstSlotPointer();
	int currSlot = 0;
	while(currSlot < numOfSlots) {                  // loop through all slots
		if(slotPointer->offset > cur->offset) {     // if slot is to an offset higher than slot with deletion
			slotPointer->offset -= cur->length;        // decrease the offset by length of removed record
		}
		slotPointer--;                              // move to next slot
		currSlot++;
	}

	freeSpace += cur->length;                       // increase free space by size of record removed
	freePtr -= cur->length;                         // move free space pointer back by size of record removed
	SetSlotEmpty(cur);                              // set slot that had record deleted to empty
	
	if (rid.slotNo == numOfSlots - 1) {             // if slot was last slot, we can delete the slot as well
		numOfSlots--;                               // decrease number of slots
		freeSpace += sizeof(Slot);                  // increase freespace by size of one slot
	}
	return OK;
}


//------------------------------------------------------------------
// HeapPage::FirstRecord
//
// Input    : None.
// Output   : Record id of the first record on a page.
// Purpose  : To find the first record on a page.
// Return   : OK if successful, DONE otherwise.
//------------------------------------------------------------------

Status HeapPage::FirstRecord(RecordID& rid)
{
	if (IsEmpty()) return DONE;           // if empty, unsucessful
	Slot* f = GetFirstSlotPointer();      // set pointer to first slot
	rid.pageNo = PageNo();                // store pageNo in rid
	rid.slotNo = 0;                       // store slotNo in rid
	return OK;
}


//------------------------------------------------------------------
// HeapPage::NextRecord
//
// Input    : ID of the current record.
// Output   : ID of the next record.
// Purpose  : To find the next record on a page.
// Return   : Return DONE if no more records exist on the page; 
//            otherwise OK.
//------------------------------------------------------------------

Status HeapPage::NextRecord(RecordID curRid, RecordID& nextRid)
{
	Slot * slotPointer = GetFirstSlotPointer() - curRid.slotNo;  // set slot pointer to slot of given record
	int currSlot = curRid.slotNo;                                // store the slot number of given record
	while(currSlot < numOfSlots) {                               // loop through slots
		slotPointer--;                                           
		currSlot++;                                              
		if (currSlot == numOfSlots) return DONE;           // if gone through all remaining slots, DONE
		if (!SlotIsEmpty(slotPointer)) break;              // if found a non-empty slot, break loop and use it
	}

	nextRid.pageNo = PageNo();       // store pageNo of currSlot (which is next non-empty slot)
	nextRid.slotNo = currSlot;       // store slotNo of currSlot, which is currSlot
	return OK;
}


//------------------------------------------------------------------
// HeapPage::GetRecord
//
// Input    : rid - Record ID. len - the length of allocated memory.
// Output   : Records length and a copy of the record itself.
// Purpose  : To retrieve a COPY of a record with ID rid from a page.
// Return   : OK if successful, FAIL otherwise.
//------------------------------------------------------------------

Status HeapPage::GetRecord(RecordID rid, char *recPtr, int& len)
{
	int slotno = rid.slotNo;                           //store slotNo of given rid
    if (slotno >= numOfSlots) return FAIL;             //if slotNo is too high, fail
	Slot* f = GetFirstSlotPointer() - slotno;          //set pointer to that slot
	if (SlotIsEmpty(f)) return FAIL;                   //if slot is empty, fail
	memcpy(recPtr, &(data[f->offset]), f->length);     //store record data in recPtr location
	len = f->length;                                   //store length of record in len
	return OK;
}


//------------------------------------------------------------------
// HeapPage::ReturnRecord
//
// Input    : Record ID.
// Output   : Pointer to the record, record's length.
// Purpose  : To retrieve a POINTER to the record.
// Return   : OK if successful, FAIL otherwise.
//------------------------------------------------------------------

Status HeapPage::ReturnRecord(RecordID rid, char*& recPtr, int& len)
{
	int slotno = rid.slotNo;                      // store slotNo of rid
	if (slotno >= numOfSlots) return FAIL;        // if slotNo too high, fail
	Slot* f = GetFirstSlotPointer() - slotno;     // set pointer to slot of rid
	if (SlotIsEmpty(f)) return FAIL;              // if slot is emplty, faul
	recPtr = &data[f->offset];                    // store pointer in given variable
	len = f->length;                              // stpre length of record in len
	return OK;
}


//------------------------------------------------------------------
// HeapPage::AvailableSpace
//
// Input    : None.
// Output   : None.
// Purpose  : To return the amount of available space.
// Return   : The amount of available space on the heap file page.
//------------------------------------------------------------------

int HeapPage::AvailableSpace()
{
	return freeSpace;    // return freespace variable
}


//------------------------------------------------------------------
// HeapPage::IsEmpty
// 
// Input    : None.
// Output   : None.
// Purpose  : Check if there is any record in the page.
// Return   : true if the HeapPage is empty, and false otherwise.
//------------------------------------------------------------------

bool HeapPage::IsEmpty()
{
	return GetNumOfRecords() == 0;   // return if number of records variable is 0
}

//------------------------------------------------------------------
// HeapPage::GetNumOfRecords
// 
// Input    : None.
// Output   : None.
// Purpose  : Counts the number of records in the page.
// Return   : Number of records in the page.
//------------------------------------------------------------------

int HeapPage::GetNumOfRecords()
{
	int count = 0;        // initialize count to 0

	Slot *slotPointer = GetFirstSlotPointer();  //get pointer to first slot
	int currSlot = 0;                           //start slot count at 0
	while(currSlot < numOfSlots) {              //loop through all slots
		if (!SlotIsEmpty(slotPointer)) {    //if the current slot is not empty
			count++;                        //increment count
		}
		slotPointer--;                      //move slot pointer and counter
		currSlot++;
	}

	return count;   //return count variable
}


//------------------------------------------------------------------
// HeapPage::SetNextPage
// 
// Input    : The PageID for next page.
// Output   : None.
// Purpose  : Set the PageID for next page.
// Return   : None.
//------------------------------------------------------------------
void HeapPage::SetNextPage(PageID pageNo)
{
	nextPage = pageNo;     // set nextPage variable to given pageNo
}

//------------------------------------------------------------------
// HeapPage::SetNextPage
// 
// Input    : The PageID for previous page.
// Output   : None.
// Purpose  : Set the PageID for previous page.
// Return   : None.
//------------------------------------------------------------------
void HeapPage::SetPrevPage(PageID pageNo)
{
	prevPage = pageNo;     // set prevPage to given pageNo
}

//------------------------------------------------------------------
// HeapPage::GetNextPage
// 
// Input    : None.
// Output   : None.
// Purpose  : Get the PageID of next page.
// Return   : The PageID of next page.
//------------------------------------------------------------------
PageID HeapPage::GetNextPage()
{
	return nextPage;    // return nextPage variable
}

//------------------------------------------------------------------
// HeapPage::SetPrevPage
// 
// Input    : The PageID for previous page.
// Output   : None.
// Purpose  : Get the PageID of previous page.
// Return   : The PageID of previous page.
//------------------------------------------------------------------
PageID HeapPage::GetPrevPage()
{
	return prevPage;    // return prevPage variable
}

//------------------------------------------------------------------
// HeapPage::PageNo
// 
// Input    : None.
// Output   : None.
// Purpose  : Get the PageID of this page.
// Return   : The PageID of this page.
//------------------------------------------------------------------
PageID HeapPage::PageNo() 
{
	return pid;     // return pid variable
}   