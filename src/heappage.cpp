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
	this->pid = pageNo;
	this->prevPage = INVALID_PAGE;
	this->nextPage = INVALID_PAGE;
	this->numOfSlots = 0; // not sure what this is used for, could be the same as the lastSlot variable I defined.
	this->freePtr = 0;
	this->freeSpace = sizeof(data);
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
	if (this->freeSpace < length) return DONE;
	memcpy(&this->data[this->freePtr], recPtr, length);
	Slot* f = this->GetFirstSlotPointer();
	int currSlot = 0;
	while (f->offset == NULL || f->offset > 0) {
		// go to first free slot, use if free
		currSlot++;
		f++;
	}
	this->FillSlot(f, this->freePtr, length);
	this->freePtr += length;
	this->freeSpace -= length;
	rid.pageNo = this->PageNo();
	rid.slotNo = currSlot;
	if (currSlot > this->lastSlot) this->lastSlot++;
	this->numRecords++;
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
	if (rid.slotNo > this->lastSlot) return FAIL;
	Slot* cur = this->GetFirstSlotPointer() + rid.slotNo;
	cur->offset = -1; //probably wrong
	//need to remove actual record from page and rearrange to make new space according to book.
	//needs to move them, update freeSpace field, etc...
	this->numRecords--;
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
	if (this->IsEmpty()) return DONE;
	Slot* f = this->GetFirstSlotPointer();
	rid.pageNo = this->PageNo();
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
	int newslot = curRid.slotNo + 1;
	if (newslot > this->numOfSlots) return DONE;
	nextRid.pageNo = this->PageNo();
	nextRid.slotNo = newslot;
	return DONE;
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
	memcpy(recPtr, &f->offset, f->length);
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
	recPtr = &this->data[f->offset];
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
