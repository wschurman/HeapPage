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
	if(freeSpace < length) {
		return DONE;
	}

	bool emptySlot = false; 
	Slot *slotPointer = GetFirstSlotPointer();
	int currSlot= 1;
	while(currSlot <= numOfSlots) { 
		if((slotPointer->offset == NULL)||(SlotIsEmpty(slotPointer))) { 
			emptySlot = true; 
			break;       
		}
		//slotPointer = slotPointer - sizeof(Slot); 
		slotPointer--;
		currSlot=currSlot+1;
    }
	
	if(!emptySlot) {
		numOfSlots++;
		freeSpace = freeSpace - sizeof(Slot);
	}

	FillSlot(slotPointer, freePtr, length);
	freePtr = freePtr + length;

	memcpy(&data[freePtr], recPtr, length);

	rid.pageNo = PageNo();
	rid.slotNo = currSlot;
	numRecords++;
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
	return FAIL;
	if (rid.slotNo > this->numOfSlots) return FAIL;
	Slot* cur = GetFirstSlotPointer() + rid.slotNo;
	cur->offset = -1; //probably wrong
	//need to remove actual record from page and rearrange to make new space according to book.
	//needs to move them, update freeSpace field, etc...
	//this->numRecords--;
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
	rid.slotNo = 1;
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
	cout << "Got Next from curSlotNo: " << curRid.slotNo << endl;
	cout << "Total Num of Slots: " << this->numOfSlots << endl; 
	int newslot = curRid.slotNo + 1;
	if (newslot > this->numOfSlots) return DONE;
	nextRid.pageNo = this->PageNo();
	nextRid.slotNo = newslot;
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
	int slotno = rid.slotNo;
	Slot* f = this->GetFirstSlotPointer() + slotno;
	if (slotno > this->numOfSlots || f->offset < 0) return FAIL;
	memcpy(recPtr, &data[f->offset], f->length);
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
	Slot* f = this->GetFirstSlotPointer() + slotno;
	if (slotno > this->numOfSlots || f->offset < 0) return FAIL;
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
	return this->freeSpace;
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
	return this->numRecords == 0;
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
	return this->numRecords;
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
