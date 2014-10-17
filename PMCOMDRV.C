#define INCL_DOS
#define INCL_ERRORS
#define INCL_DOSDEVIOCTL
#define INCL_DOSDEVICES
#define INCL_PM
#include <os2.h>
#include <process.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>


#include "protype.h"
#include "sliddlg.h"
#include "extern.h"

#define PRTBUFSZE  50

PBYTE pbBase;
HQUEUE hqTermWrt;

ULONG AcutalWritten;


#define STACK_SIZE         8096
#define FOURKBLKSIZE       16000
#define TERM_WRITE_QUE     "\\queues\\termwrt.que"



/***************************************************************************/
/***************************************************************************/
void InitTermDriver(void)
{

   CreateTermWrtQueue();

   Create_Threads();

   Allocate_Buffer_Memory();


}   


/***************************************************************************/
/*                                                                          */
/***************************************************************************/
void   Allocate_Buffer_Memory(void)
{
   APIRET rc;
   UCHAR   PrintBuf[PRTBUFSZE];

   //Get a chuck of memory off the heap
   rc = DosAllocMem ( (PVOID *) &pbBase, FOURKBLKSIZE, fALLOC);
   if (rc)
      {
      sprintf(PrintBuf,"RC=%u Line=%u\nFile: %s",rc,__LINE__,__FILE__);
      WinDebugMsg(PrintBuf,hwndMain);
      }
   //Set this memory so we can sub alloc any size block
   rc = DosSubSetMem (pbBase, DOSSUB_INIT, FOURKBLKSIZE);
   if (rc)
      {
      sprintf(PrintBuf,"RC=%u Line=%u\nFile: %s",rc,__LINE__,__FILE__);
      WinDebugMsg(PrintBuf,hwndMain);
      }
}

/***************************************************************************/
/*                                                                         */
/***************************************************************************/
void Term_Write_Thread(void * Parm1)
{

   UCHAR   PrintBuf[PRTBUFSZE];

   APIRET rc;
   REQUESTDATA Request;
   ULONG    DataLength;
   PVOID    DataAddress;
   ULONG      ElementCode;
   BOOL32    NoWait;
   BYTE       ElemPriority;
   HEV      SemHandle;

   HQUEUE   QueueHandle;
   PID      OwnerPID;


   ULONG BytesWritten;

   BUFFERFORMAT * BuffPtr;

   ElementCode = 0;
   NoWait = 0;
   SemHandle = 0;

   rc = DosOpenQueue (&OwnerPID,   &QueueHandle,   TERM_WRITE_QUE);
   if (rc)
      {
      sprintf(PrintBuf,"RC=%u Line=%u\nFile: %s",rc,__LINE__,__FILE__);
      WinDebugMsg(PrintBuf,hwndMain);
      }


   while (1)
      {
      rc = DosReadQueue(QueueHandle,
                        &Request,
                        &DataLength,
                        &DataAddress,
                        ElementCode,
                        DCWW_WAIT,
                        &ElemPriority,
                        SemHandle);
      
      if (rc)
         {
         sprintf(PrintBuf,"RC=%u Line=%u\nFile: %s",rc,__LINE__,__FILE__);
         WinDebugMsg(PrintBuf,hwndMain);
         }

      BuffPtr = (BUFFERFORMAT *) DataAddress;

      while (HandleComm == 0)
         {
         DosSleep(500);
         }


      rc = DosWrite(HandleComm,
                           BuffPtr->Buff,
                           strlen(BuffPtr->Buff),
                           &BytesWritten);
      if (rc)
         {
         sprintf(PrintBuf,"RC=%u Line=%u\nFile: %s",rc,__LINE__,__FILE__);
         WinDebugMsg(PrintBuf,hwndMain);
         }
      
      rc = DosSubFreeMem( BuffPtr->pbBase,
                          BuffPtr->BuffPtr,
                          BuffPtr->ulSize);
      if (rc)
         {
           sprintf(PrintBuf,"RC=%u Line=%u\nFile: %s",rc,__LINE__,__FILE__);
           WinDebugMsg(PrintBuf,hwndMain);
         }
      }
}



/***************************************************************************/
/*                                                                         */
/***************************************************************************/
void Term_Read_Thread(void * Parm1)
{
   UCHAR   PrintBuf[PRTBUFSZE];
   
   UCHAR Buf[10];
   ULONG BytesRead, BytesWritten,BufferLength;

   APIRET rc;

   LONG   val;
   char *stopstring;
   static UCHAR ValBuf[20];
   static int idx;

   idx = 0;
   while(1)
      {
       while (HandleComm == 0)
         {
         DosSleep(500);
         }
      BufferLength = 1;
      rc = DosRead(HandleComm, Buf, BufferLength, &BytesRead);
      if (rc)
            {
            sprintf(PrintBuf,"RC=%u Line=%u\nFile: %s",rc,__LINE__,__FILE__);
            WinDebugMsg(PrintBuf,hwndMain);
            }

      if (BytesRead != 0) //got a char from termial
         {
         if (Buf[0] == '\r')  //got a carriage return
            {
            if ( idx == 0)  //only a carriage return, ignore
               continue;
            ValBuf[idx]= '\0'; //null terminate buffer
            idx = 0;
            val  = strtol(ValBuf,&stopstring ,10);
            WinPostMsg(hwndDlg, WM_TERMINAL_MSG, (MPARAM) val ,(MPARAM) 0 );
            }
         else
            {
            ValBuf[idx++] = Buf[0];
            }
         }
      }
}



/***************************************************************************/
/*                                                                         */
/***************************************************************************/
void Create_Threads(void)
{
   

   
    _beginthread( Term_Read_Thread,
                           NULL,
                           STACK_SIZE,
                           NULL);

    _beginthread( Term_Write_Thread,
                           NULL,
                           STACK_SIZE,
                           NULL);



}


/***************************************************************************/
/*                                                                         */
/***************************************************************************/
void CreateTermWrtQueue(void)
{
   APIRET rc;
   UCHAR   PrintBuf[PRTBUFSZE];

   rc  = DosCreateQueue(&hqTermWrt, QUE_FIFO | QUE_CONVERT_ADDRESS, TERM_WRITE_QUE);
   if (rc)
      {
      sprintf(PrintBuf,"RC=%u Line=%u\nFile: %s",rc,__LINE__,__FILE__);
      WinDebugMsg(PrintBuf,hwndMain);
      }

}
/***************************************************************************/
/*                                                                         */
/***************************************************************************/
BUFFERFORMAT * AllocateMsgBuffer(ULONG BuffSize)
{
   
   UCHAR   PrintBuf[PRTBUFSZE];
   APIRET  rc;
   ULONG i = 1;
   BUFFERFORMAT * BuffPtr;
   
   rc = DosSubAllocMem(pbBase,
                      (PVOID *) &BuffPtr,
                      BuffSize +(sizeof(BUFFERFORMAT)) );
   if (rc)
         {
         sprintf(PrintBuf,"RC=%u Line=%u\nFile: %s",rc,__LINE__,__FILE__);
         WinDebugMsg(PrintBuf,hwndMain);
         }

   BuffPtr->pbBase = pbBase;
   BuffPtr->BuffPtr = (PBYTE) BuffPtr;
   BuffPtr->ulSize = BuffSize +(sizeof(BUFFERFORMAT)) ;

   for (i=0; i < BuffSize; i++)
         {
         BuffPtr->Buff[i] = '\0';
         }

   return(BuffPtr);
}

/****************************************************************************
INPUT: void *ptr must be NULL terminated

ACTION:

   1.   Allocates a buffer
   2.   Copies the data over   to buffer from calling routine
   3.   Post buffer to write terminal thread
****************************************************************************/
void PrintTerm(void *ptr)
{
   APIRET  rc;
   ULONG   Request;
   ULONG   ElemPriority;
   ULONG   BytesRead;
   UCHAR   PrintBuf[PRTBUFSZE];

   BUFFERFORMAT * BuffPtr;
   ULONG BuffSize;

   BuffSize = strlen(ptr)+1;
   
   rc = DosSubAllocMem(pbBase, (PVOID *) &BuffPtr, BuffSize);
   if (rc)
      {
      sprintf(PrintBuf,"RC=%u Line=%u\nFile: %s",rc,__LINE__,__FILE__);
      WinDebugMsg(PrintBuf,hwndMain);
      return;
      }

   BuffPtr->pbBase = pbBase;
   BuffPtr->BuffPtr = (PBYTE) BuffPtr;
   BuffPtr->ulSize = BuffSize;


   strcpy(BuffPtr->Buff, ptr);

   BytesRead =  strlen(BuffPtr->Buff);
   BytesRead += sizeof(BUFFERFORMAT);

   do
      {
      ElemPriority = 0;

      rc = DosWriteQueue(hqTermWrt,
                     Request,
                     BytesRead,
                     BuffPtr,
                     ElemPriority);

      if (rc == ERROR_QUE_NO_MEMORY)
         {
         DosSleep(1000);
         }
      } while (rc == ERROR_QUE_NO_MEMORY);
   
   if (rc)
      {
      sprintf(PrintBuf,"RC=%u Line=%u\nFile: %s",rc,__LINE__,__FILE__);
      WinDebugMsg(PrintBuf,hwndMain);
      }
}


