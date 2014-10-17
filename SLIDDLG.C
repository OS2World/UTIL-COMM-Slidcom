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



typedef struct
{
   HFILE  CommHandle;
   char   CommName[10];
} COMMSTRUCT;

COMMSTRUCT  CommStruct[4] =   {0,"COM1",
                               0,"COM2",
                               0,"COM3",
                               0,"COM4" };

LINECONTROL DataPacket;

CHAR PrintBuf[80];


#define TOTALSPINVALS   11
static PSZ   SpinSelectVals[] = {"1200", "1800", "2000", "2400","3600",
                     "4800", "7200", "9600", "19200", "38400",
                     "57600"};

#define MaximumTicks      100
#define MinorTickSpacing  1
#define MinorTickSize     4
#define MajorTickSpacing  5 
#define MajorTickSize     8
#define DetentSpacing     10
#define TextSpacing       10

static USHORT LastVal;

/****************************************************************************

****************************************************************************/
MRESULT EXPENTRY ComSelectDlgProc( HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2)
{

   APIRET RetCode;

   switch(msg)
   {
      //*********************************
      case WM_INITDLG:
         RetCode = WmInitComSelect(  hwnd);
         return (MRESULT)1;


      //*********************************
      case WM_COMMAND:
         switch( SHORT1FROMMP( mp1 ) )     /* Extract the command value    */
         {
            case DID_OK:
               RetCode = OpenComPort(  hwnd);
               if (RetCode != 0)
                  break;

            case DID_CANCEL: 
               WinDismissDlg( hwnd, TRUE ); 
               return (MRESULT)FALSE;

           default:
               break;
         }
         break;


      //*********************************
      default:
         return(WinDefDlgProc(hwnd, msg, mp1, mp2));
   }
   return (MRESULT)0;
}

/****************************************************************************

****************************************************************************/
MRESULT EXPENTRY ComInitDlgProc( HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2)
{

   APIRET RetCode;

   switch(msg)
   {
      //*********************************
      case WM_INITDLG:
         
         if ( CommPortOpen(hwnd) == FALSE)
            {
            WinDismissDlg( hwnd, TRUE );  
            return (MRESULT)FALSE;
            }

         InitLineBits(hwnd);
         InitializeSpinButton( hwnd);

         return (MRESULT)1;


      //*********************************
      case WM_COMMAND:
         switch( SHORT1FROMMP( mp1 ) )     /* Extract the command value    */
         {
            case DID_OK:
               RetCode = SetLineBitVals( hwnd);
               if (RetCode != 0)
                  break;
               RetCode = SetBiteRate( hwnd);
               if (RetCode != 0)
                  break;
            
               
            case DID_CANCEL: 
               WinDismissDlg( hwnd, TRUE ); 
               return (MRESULT)FALSE;

           default:
               break;
         }
         break;


      //*********************************
      default:
         return(WinDefDlgProc(hwnd, msg, mp1, mp2));
   }
   return (MRESULT)0;
}


/****************************************************************************

First time through HandleComm = 0, no port initially selected

On "exit" either no port was selected or an initial port was selected or
a different port was selected.


****************************************************************************/
APIRET OpenComPort( HWND hwnd)
{
   MRESULT QueryButtonCheck;
   short ButtonIndex;
   APIRET rc;
   ULONG ulAction;
   CHAR PrintBuf[100];
   int i;


   if (HandleComm != 0) //HandleComm does = 0 first time through
      {                 //find open port and close, so can open new selection
      for (i=0; i < 4; i++)
         {
         if (CommStruct[i].CommHandle == HandleComm)
            {
            rc = DosClose(CommStruct[i].CommHandle);
            CommStruct[i].CommHandle = 0;
            if (rc)
               {
               sprintf(PrintBuf,"RC=%u Line=%u\nFile: %s",rc,__LINE__,__FILE__);
               WinDebugMsg(PrintBuf,hwnd);
               }
            break;
            }
         }
      }

   
   //Determine which port selected by user if any
   
   for (ButtonIndex=0; ButtonIndex < 4; ButtonIndex++)
      {
      QueryButtonCheck = WinSendDlgItemMsg(hwnd,
         IDC_COMM1_SELECT+ButtonIndex,
         BM_QUERYCHECK,
         (MPARAM) 0,
         (MPARAM) 0);

      if (SHORT1FROMMP(QueryButtonCheck) == 1)
         {
         //Open the selected comm port
         rc = DosOpen(CommStruct[ButtonIndex].CommName,
                     &CommStruct[ButtonIndex].CommHandle,
                     &ulAction,
                     0,
                     FILE_NORMAL,
                     FILE_OPEN,
                     OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE,
                     (PEAOP2) NULL);

         if (rc)
            {
            sprintf(PrintBuf,"RC=%u Line=%u\nFile: %s",rc,__LINE__,__FILE__);
            WinDebugMsg(PrintBuf,hwnd);
            WinSetFocus (HWND_DESKTOP,
                           WinWindowFromID(hwnd,DID_OK));
            HandleComm = 0;
            return 1;
            }

         // Set the global variable
         HandleComm = CommStruct[ButtonIndex].CommHandle;

         return 0;

         }
      }

   return 0;

}
/****************************************************************************

****************************************************************************/
APIRET WmInitComSelect( HWND hwnd)
{

   int i;

   if (HandleComm == 0) //no comm port open, uncheck all buttons
      {
      for (i=0; i < 4; i++)
         {
          WinSendDlgItemMsg(hwnd,
                        IDC_COMM1_SELECT+i,
                        BM_SETCHECK,
                        MPFROM2SHORT(FALSE,0),
                        NULL);
         WinSetFocus (HWND_DESKTOP,
            WinWindowFromID(hwnd,IDC_COMM1_SELECT+i));
         }
         WinSetFocus (HWND_DESKTOP,
            WinWindowFromID(hwnd,DID_OK));
      return 0;
      }

   //otherwise, search for the open comm port and check corresponding button  

   for (i=0; i < 4; i++)
      {
      if (CommStruct[i].CommHandle == HandleComm)
         {
          WinSendDlgItemMsg(hwnd,
                        IDC_COMM1_SELECT+i,
                        BM_SETCHECK,
                        MPFROM2SHORT(TRUE,0),
                        NULL);
         WinSetFocus (HWND_DESKTOP,
            WinWindowFromID(hwnd,IDC_COMM1_SELECT+i));
         }
      }
      return 0;

}


/****************************************************************************

****************************************************************************/
APIRET InitLineBits(HWND hwnd)
{

   APIRET rc;
   ULONG DataPacketLen;



   rc = DosDevIOCtl (HandleComm,
                     IOCTL_ASYNC,
                     ASYNC_GETLINECTRL,
                     NULL,                  //no parameter packet
                     0,                  // = 0
                     NULL,                  // length of parm packet
                     (PULONG) &DataPacket,
                     sizeof(DataPacket),
                     &DataPacketLen);

   rc = DosDevIOCtl (HandleComm,
                     IOCTL_ASYNC,
                     ASYNC_GETLINECTRL,
                     NULL,                  //no parameter packet
                     0,                  // = 0
                     NULL,                  // length of parm packet
                     (PULONG) &DataPacket,
                     sizeof(DataPacket),
                     &DataPacketLen);

   if (rc)
      {
      sprintf(PrintBuf,"RC=%u Line=%u\nFile: %s",rc,__LINE__,__FILE__);
      WinDebugMsg(PrintBuf,hwnd);
      }


   WinSendDlgItemMsg(hwnd,
                     (ULONG) IDC_DATA_BIT5 + (DataPacket.bDataBits - 5),
                     (ULONG) BM_SETCHECK,
                     MPFROM2SHORT(TRUE,0),
                     NULL);

   WinSendDlgItemMsg(hwnd,IDC_PARITY_NO + DataPacket.bParity,
                        BM_SETCHECK,
                        MPFROM2SHORT(TRUE,0),
                        NULL);

   WinSendDlgItemMsg(hwnd,IDC_STOP_BIT1 + DataPacket.bStopBits,
                        BM_SETCHECK,
                        MPFROM2SHORT(TRUE,0),
                        NULL);




   WinSetFocus (HWND_DESKTOP, WinWindowFromID(hwnd,DID_OK));


   return 0;
}
/****************************************************************************

****************************************************************************/
APIRET SetLineBitVals( HWND hwnd)
{


   APIRET rc;
   ULONG DataPacketLen;

   MRESULT QueryButtonCheck;
   short ButtonIndex;

   
   for (ButtonIndex=0; ButtonIndex < 4; ButtonIndex++)
      {
      QueryButtonCheck = WinSendDlgItemMsg(hwnd,
         IDC_DATA_BIT5+ButtonIndex,
         BM_QUERYCHECK,
         (MPARAM) 0,
         (MPARAM) 0);

      if (SHORT1FROMMP(QueryButtonCheck) == 1)
         {
         DataPacket.bDataBits = ButtonIndex + 5;
         }
      }

   for (ButtonIndex=0; ButtonIndex < 5; ButtonIndex++)
      {
      QueryButtonCheck = WinSendDlgItemMsg(hwnd,
         IDC_PARITY_NO+ButtonIndex,
         BM_QUERYCHECK,
         (MPARAM) 0,
         (MPARAM) 0);

      if (SHORT1FROMMP(QueryButtonCheck) == 1)
         {
         DataPacket.bParity = ButtonIndex;
         }
      }
   for (ButtonIndex=0; ButtonIndex < 3; ButtonIndex++)
      {
      QueryButtonCheck = WinSendDlgItemMsg(hwnd,
         IDC_STOP_BIT1+ButtonIndex,
         BM_QUERYCHECK,
         (MPARAM) 0,
         (MPARAM) 0);

      if (SHORT1FROMMP(QueryButtonCheck) == 1)
         {
         DataPacket.bStopBits = ButtonIndex;
         }
      }


   rc = DosDevIOCtl (HandleComm,
                     IOCTL_ASYNC,
                     ASYNC_SETLINECTRL,
                     (PULONG) &DataPacket,
                     sizeof(DataPacket),
                     &DataPacketLen,
                     NULL,              
                     0,                 
                     NULL);


   rc = DosDevIOCtl (HandleComm,
                     IOCTL_ASYNC,
                     ASYNC_SETLINECTRL,
                     (PULONG) &DataPacket,
                     sizeof(DataPacket),
                     &DataPacketLen,
                     NULL,              
                     0,                 
                     NULL);


   if (rc)
      {
      sprintf(PrintBuf,"RC=%u Line=%u\nFile: %s",rc,__LINE__,__FILE__);
      WinDebugMsg(PrintBuf,hwnd);
      }

   return 0;

}


/*****************************************************************************
*****************************************************************************/
BOOL InitializeSpinButton(HWND hwnd)
{

   USHORT usBitRate;
   USHORT ArrayIndex;
   char buffer[35];
   char *p;

   if ( !WinSendDlgItemMsg( hwnd, IDC_SET_BIT_RATE, SPBM_SETARRAY, SpinSelectVals,
             MPFROMLONG(TOTALSPINVALS)))
      return FALSE;


   usBitRate = Get_IOCTL_Bit_Rate(hwnd);


   p = _itoa(usBitRate,buffer,10);

   for (ArrayIndex=0; ArrayIndex<TOTALSPINVALS; ArrayIndex++)
      {
      if ( strcmp( (const char * )SpinSelectVals[ArrayIndex],&buffer[0] ) == 0 )
         {
         WinSendDlgItemMsg( hwnd, IDC_SET_BIT_RATE, SPBM_SETCURRENTVALUE,
             MPFROMLONG(ArrayIndex), NULL);
         return TRUE;
         }
      }

   WinSetFocus (HWND_DESKTOP, WinWindowFromID(hwnd,DID_OK));

   return TRUE;
} 

/****************************************************************************

****************************************************************************/
USHORT SetBiteRate( HWND hwnd)
{

   ULONG ArrayIndex=0;
   USHORT usBitRate;

   WinSendDlgItemMsg(hwnd,
                        IDC_SET_BIT_RATE,
                        SPBM_QUERYVALUE,
                        &ArrayIndex,
                        MPFROM2SHORT(0,0));
     

   usBitRate = atoi( SpinSelectVals[ArrayIndex] );
   Set_IOCTL_Bit_Rate(usBitRate,hwnd );

   return 0;

   
}

/***************************************************************************/
/*                                                                         */
/***************************************************************************/
void Set_IOCTL_Bit_Rate(USHORT BitRate,HWND hwnd)
{
   APIRET rc;
   ULONG ParmPacketLen;
   ParmPacketLen = 2;  //size of Parmpacket

   rc = DosDevIOCtl (HandleComm,
                     IOCTL_ASYNC,
                     ASYNC_SETBAUDRATE,
                     (PULONG) &BitRate, 
                     sizeof(BitRate),    //size input parm packet
                     &ParmPacketLen,         // length of parm packet
                     NULL,
                     0,
                     NULL);

   rc = DosDevIOCtl (HandleComm,
                     IOCTL_ASYNC,
                     ASYNC_SETBAUDRATE,
                     (PULONG) &BitRate, 
                     sizeof(BitRate),    //size input parm packet
                     &ParmPacketLen,         // length of parm packet
                     NULL,
                     0,
                     NULL);


   if (rc)
      {
      sprintf(PrintBuf,"RC=%u Line=%u\nFile: %s",rc,__LINE__,__FILE__);
      WinDebugMsg(PrintBuf,hwnd);
      }

}
/***************************************************************************/
/*                                                                         */
/***************************************************************************/
USHORT Get_IOCTL_Bit_Rate(HWND hwnd)
{
   APIRET rc;
   USHORT DataPacket=0;
   ULONG DataPacketLen;
   DataPacket = 0;

   rc = DosDevIOCtl ( (HFILE) HandleComm,
                     (ULONG) IOCTL_ASYNC,
                     (ULONG) ASYNC_GETBAUDRATE,
                     (PVOID) NULL,                  //no parameter packet
                     (ULONG) NULL,                  // = 0
                     (PULONG) NULL,                  // length of parm packet
                     (PVOID) &DataPacket,
                     (ULONG) sizeof(DataPacket),
                     (PULONG) &DataPacketLen);

   rc = DosDevIOCtl ( (HFILE) HandleComm,
                     (ULONG) IOCTL_ASYNC,
                     (ULONG) ASYNC_GETBAUDRATE,
                     (PVOID) NULL,                  //no parameter packet
                     (ULONG) NULL,                  // = 0
                     (PULONG) NULL,                  // length of parm packet
                     (PVOID) &DataPacket,
                     (ULONG) sizeof(DataPacket),
                     (PULONG) &DataPacketLen);


   if (rc)
      {
      sprintf(PrintBuf,"RC=%u Line=%u\nFile: %s",rc,__LINE__,__FILE__);
      WinDebugMsg(PrintBuf,hwnd);
      }

      return DataPacket;

}

/****************************************************************************

****************************************************************************/
MRESULT EXPENTRY SliderDlgProc( HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2)
{

   CHAR      acBuffer[10];
   CHAR      *cData;
   USHORT  usNewPctDone;
   SHORT ulValue;
   static SHORT sPtVal=0;
   APIRET rc;

    switch(msg)
    {
       //*********************************
       case WM_INITDLG:

         if ( CommPortOpen(hwnd) == FALSE)
            {
            WinDismissDlg( hwnd, TRUE );  
            return (MRESULT)FALSE;
            }
         hwndDlg = hwnd; //used in read task to send term input
         
         InitSlider (hwnd, IDC_SLIDER_CONTROL,MaximumTicks,
                         MinorTickSpacing,MinorTickSize, 
                         MajorTickSpacing,MajorTickSize, 
                         DetentSpacing, TextSpacing,    
                        "10.Courier");

        //set the limit of the entry field    
         WinSendDlgItemMsg(hwnd,
                     IDC_SLIDER_DATA,
                     EM_SETTEXTLIMIT,
                     (MPARAM)3,
                     (MPARAM)0);
         
         WinStartTimer( hab, hwnd,
                     TMID_QUERY_TIMER,
                     750);  //every 750 msec
         LastVal = 0;        
    
         return 0;

      //*********************************
      case WM_TERMINAL_MSG:
         //sent from the DOS read thread
         usNewPctDone    =    (USHORT) mp1;
         if ( (usNewPctDone >= 0)  && (usNewPctDone <= MaximumTicks) )
            {

            WinSendDlgItemMsg (hwnd, IDC_SLIDER_CONTROL,
                        SLM_SETSLIDERINFO,
                        MPFROM2SHORT (SMA_SLIDERARMPOSITION,
                                        SMA_INCREMENTVALUE),
                        MPFROMSHORT (usNewPctDone));
             LastVal = usNewPctDone;
             }

            return 0;

      //*********************************
      case WM_TIMER:
         
            switch(SHORT1FROMMP(mp1))
               {
               case TMID_QUERY_TIMER:
                 {
                 ulValue = (SHORT) WinSendDlgItemMsg(hwnd, IDC_SLIDER_CONTROL,
                     SLM_QUERYSLIDERINFO,
                  MPFROM2SHORT(SMA_SLIDERARMPOSITION,SMA_INCREMENTVALUE),
                  NULL);
                 
                  if ( ulValue != LastVal)
                     {
                     LastVal = ulValue;
                     cData = _ltoa(ulValue,acBuffer,10);
                     WinSetDlgItemText(hwnd,IDC_SLIDER_DATA, cData);
                     strcat( acBuffer, " ");
                     PrintTerm(acBuffer);
                     }
                  break;
                  }
               default: break;
               }
         break;

      //*********************************
      case WM_CONTROL:
         switch(SHORT2FROMMP(mp1))
         {
            case SLN_SLIDERTRACK:
            case  SLN_CHANGE:
               {
               ulValue = (SHORT) WinSendDlgItemMsg(hwnd, IDC_SLIDER_CONTROL,
                  SLM_QUERYSLIDERINFO,
                  MPFROM2SHORT(SMA_SLIDERARMPOSITION,SMA_INCREMENTVALUE),
                  NULL);
               cData = _ltoa(ulValue,acBuffer,10);
               WinSetDlgItemText(hwnd,IDC_SLIDER_DATA, cData);
               break;
               }
         
         default:
            break;
         
         }

        break;

        case WM_COMMAND:
            switch (SHORT1FROMMP(mp1))
            {
                case DID_OK:
                  WinStopTimer( hab,  hwnd, TMID_QUERY_TIMER);
                  WinDismissDlg (hwnd, FALSE);
                  return 0;

                case IDC_MID_POINT:
                
                      WinSendDlgItemMsg (hwnd, IDC_SLIDER_CONTROL,
                                  SLM_SETSLIDERINFO,
                                  MPFROM2SHORT (SMA_SLIDERARMPOSITION,
                                                  SMA_INCREMENTVALUE),
                                  MPFROMSHORT (MaximumTicks/2));
                     return 0;
            }
         return 0;

        default:
            return (WinDefDlgProc (hwnd, msg, mp1, mp2));
    }
    return FALSE;
}
/****************************************************************************

****************************************************************************/

VOID InitSlider (HWND    hwnd,                    
                 ULONG   idSlider,                
                 USHORT  usMaximumTicks,          
                 USHORT  usMinorTickSpacing,      
                 USHORT  usMinorTickSize,         
                 USHORT  usMajorTickSpacing,      
                 USHORT  usMajorTickSize,         
                 USHORT  usDetentSpacing,         
                 USHORT  usTextSpacing,           
                 PSZ     pszFont)                 
{
    USHORT     i;                          
    CHAR       buffer[20];                 
    USHORT     usIncrements;
    USHORT     usSpacing;
    WNDPARAMS  wprm;                   
    SLDCDATA   sldcd;                  
    HWND       hwndSlider = WinWindowFromID (hwnd, idSlider);

    wprm.fsStatus   = WPM_CTLDATA;      
    wprm.cbCtlData  = sizeof (SLDCDATA);
    wprm.pCtlData   = &sldcd;

   WinSendMsg (hwndSlider, WM_QUERYWINDOWPARAMS,
                     MPFROMP(&wprm), 0);

   //*********************************************************************
   // Set the total-max num of increments for slider
   sldcd.usScale1Increments = usMaximumTicks +1;
   sldcd.usScale1Spacing = 0;  //let slider do spacing

   wprm.fsStatus   = WPM_CTLDATA;  
   wprm.cbCtlData  = sizeof (SLDCDATA);
   wprm.pCtlData   = &sldcd;

   //set these paramaters
   WinSendMsg (hwndSlider, WM_SETWINDOWPARAMS,
                    MPFROMP(&wprm), 0);

   //*********************************************************************
   //Read these parameters back so we can use them
   wprm.fsStatus   = WPM_CTLDATA;      
   wprm.cbCtlData  = sizeof (SLDCDATA);
   wprm.pCtlData   = &sldcd;

   WinSendMsg (hwndSlider, WM_QUERYWINDOWPARAMS,
                     MPFROMP(&wprm), 0);

   usIncrements =  sldcd.usScale1Increments;
   usSpacing    =  sldcd.usScale1Spacing;

    //*********************************************************************
   if (usMinorTickSpacing != 0)
      {
      for (i = 0; i <= usIncrements; i += usMinorTickSpacing)
         {
          WinSendMsg (hwndSlider,     // Set minor tick
                        SLM_SETTICKSIZE,
                        MPFROM2SHORT(i, usMinorTickSize),
                        NULL);
         }
      }

   
    //*********************************************************************
   if (usMajorTickSpacing != 0)
      {
      for (i = 0; i <= usIncrements; i += usMajorTickSpacing)
        {
        WinSendMsg (hwndSlider,     // Set major tick
                        SLM_SETTICKSIZE,
                        MPFROM2SHORT(i, usMajorTickSize),
                        NULL);
        }
      }

    //*********************************************************************
    if (usDetentSpacing != 0)
      {
      for (i = 0; i <= usIncrements; i += usDetentSpacing)
        {
        WinSendMsg (hwndSlider,     // Set the detent
                   SLM_ADDDETENT,
                   MPFROM2SHORT((i*usSpacing), usDetentSpacing),
                   NULL);
        }
      }
    //*********************************************************************
    if (usTextSpacing != 0)
      {
      if (pszFont != NULL)
         {
          WinSetPresParam (hwndSlider, PP_FONTNAMESIZE,
                             (strlen(pszFont)+1),  pszFont);
         }
      for (i = 0; i <= usIncrements; i += usTextSpacing)
        {
        _itoa (i, buffer, 10);  
        WinSendMsg (hwndSlider,SLM_SETSCALETEXT,
                        MPFROMSHORT(i),
                        MPFROMP(buffer));
        }
    }

}
