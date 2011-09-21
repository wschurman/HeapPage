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
	freePtr = 0;
	freeSpace = HEAPPAGE_DATA_SIZE;
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
	//cout << "Insert Record called" << endl;
	if(freeSpace < length + sizeof(Slot)) {
		return DONE;
	}

	bool emptySlot = false;
	Slot *slotPointer = GetFirstSlotPointer();
	int currSlot = 0;
	while(currSlot < numOfSlots) { 
		if(SlotIsEmpty(slotPointer)) {
			emptySlot = true;
			currSlot++;
			break;
		}
		slotPointer--;
		currSlot++;
    }
	
	if(!emptySlot) {
		numOfSlots++;
		freeSpace -= sizeof(Slot);
	}

	FillSlot(slotPointer, freePtr, length);
	
	memcpy(&(data[freePtr]), recPtr, length);
	assert(slotPointer->offset == freePtr);
	freeSpace -= length;
	freePtr += length;

	rid.pageNo = PageNo();
	rid.slotNo = currSlot;
	//numRecords++;
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
	//cout << "Deleting slotNo: " << rid.slotNo << endl;
	if (rid.slotNo >= numOfSlots) return FAIL;
	Slot* cur = GetFirstSlotPointer() - rid.slotNo;
	if (SlotIsEmpty(cur)) return FAIL;

	memmove(&(data[cur->offset]), &(data[cur->offset + cur->length]), freePtr-(cur->offset + cur->length));

	Slot *slotPointer = GetFirstSlotPointer();
	int currSlot = 0;
	while(currSlot < numOfSlots) { 
		if(slotPointer->offset > cur->offset) {
			slotPointer->offset -= cur->length;
		}
		slotPointer--;
		currSlot++;
	}
	freeSpace += cur->length;
	freePtr -= cur->length;
	SetSlotEmpty(cur);
	if (rid.slotNo == numOfSlots - 1) {
		numOfSlots--;
		freeSpace += sizeof(Slot);
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
	if (IsEmpty()) return DONE;
	Slot* f = GetFirstSlotPointer();
	rid.pageNo = PageNo();
	rid.slotNo = 0;
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
	//cout << "Got Next from curSlotNo: " << curRid.slotNo << endl;
	//cout << "Total Num of Slots: " << numOfSlots << endl; 
	
	Slot * slotPointer = GetFirstSlotPointer() - curRid.slotNo;
	int currSlot = curRid.slotNo;
	while(currSlot < numOfSlots) {
		slotPointer--;
		currSlot++;
		if (currSlot == numOfSlots) return DONE;
		if (!SlotIsEmpty(slotPointer)) break;
	}
	
	nextRid.pageNo = pid;
	nextRid.slotNo = currSlot;
	//cout << "Got the " << nextRid.slotNo << " slot. " << endl;
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
	//cout << "Get Record called" << endl;
	int slotno = rid.slotNo;
	//cout << "Trying to get slotNo: " << slotno << endl;
	Slot* f = GetFirstSlotPointer() - slotno;
	if (SlotIsEmpty(f)) {
		//cout << "Got Empty Slot Ahhhhhh" << endl;
		return FAIL;
	}
	if (slotno >= numOfSlots) return FAIL; // || f->offset < 0
	memcpy(recPtr, &(data[f->offset]), f->length);
	len = f->length;
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
	int slotno = rid.slotNo;
	Slot* f = GetFirstSlotPointer() - slotno;
	if (SlotIsEmpty(f)) return FAIL;
	if (slotno >= numOfSlots) return FAIL;
	recPtr = &data[f->offset];
	len = f->length;
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
	return freeSpace;
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
	return GetNumOfRecords() == 0;
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
	int count = 0;

	Slot *slotPointer = GetFirstSlotPointer();
	int currSlot = 0;
	while(currSlot < numOfSlots) { 
		if (!SlotIsEmpty(slotPointer)) {
			count++;
		}
		slotPointer--;
		currSlot++;
	}

	return count;
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
	nextPage = pageNo;
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
	prevPage = pageNo;
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
	return nextPage;
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
	return prevPage;
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
	return pid;
}   
