#define INCL_PM
#define INCL_DOS
#define  INCL_WINSYS
#define INCL_DOSDEVIOCTL

#include <os2.h>
#include "protype.h"
#include "sliddlg.h"

#define RETURN_SUCCESS      0            /* successful return in DosExit */
#define RETURN_ERROR        1            /* error return in DosExit      */
#define BEEP_WARN_FREQ      60           /* frequency of warning beep    */
#define BEEP_WARN_DUR      100           /* duration of warning beep     */

CHAR PrintBuf[80];

HAB		   hab;
HMQ		   hmq;
HWND		   hwndMainFrame;
HWND		   hwndMain;
HWND		   hwndDlg;
QMSG		   qmsg;

HFILE         HandleComm;

CHAR szClientClass1[MAXNAMEL];

ULONG flFrameFlags = FCF_TITLEBAR | FCF_SYSMENU	  |
                     FCF_SIZEBORDER | FCF_MINMAX |
                     FCF_SHELLPOSITION | FCF_TASKLIST |
							FCF_MENU;

/****************************************************************************
					  M A I N
****************************************************************************/

int main(VOID)
{
   ULONG ulCtlData;      /* frame control data */

   hab = WinInitialize(0);
   if(!hab)
   {
      DosBeep(BEEP_WARN_FREQ, BEEP_WARN_DUR);
      return(RETURN_ERROR);
   }

   hmq = WinCreateMsgQueue(hab, 0);
   if(!hmq)
   {
      DosBeep(BEEP_WARN_FREQ, BEEP_WARN_DUR);
      WinTerminate(hab);
      return(RETURN_ERROR);
   }


    if ( DosExitList(EXLST_ADD, (PFNEXITLIST)ExitProc) )
		{
      MessageBox(HWND_DESKTOP,
                   IDMSG_CANNOTLOADEXITLIST,
                   MB_OK | MB_ERROR,
                   TRUE);
      
		DosExit(EXIT_PROCESS, RETURN_ERROR);
		}


	if( !WinLoadString(hab, (HMODULE)0, IDS_CLIENT1CLASS,
		                      MAXNAMEL, szClientClass1))
	   {
      MessageBox(HWND_DESKTOP,
                   IDMSG_TEST,
                   MB_OK | MB_ERROR,
                   TRUE);

	   return (0);
	}
	

	if ( !WinRegisterClass ( hab,
                    (PSZ)szClientClass1,
                    (PFNWP)ClientWndProc,
                    CS_SIZEREDRAW | CS_CLIPCHILDREN,
                    0) )
	   {
      MessageBox(HWND_DESKTOP,
                   IDMSG_TEST,
                   MB_OK | MB_ERROR,
                   TRUE);

	   return (0);
	   }
   
   ulCtlData = FCF_STANDARD;


	hwndMainFrame = WinCreateStdWindow( HWND_DESKTOP,
                                   WS_VISIBLE,
                                   (PVOID)&ulCtlData,
                                   (PSZ)szClientClass1,
                                   (PSZ)NULL,
                                   WS_VISIBLE,
                                   (HMODULE)NULL,
                                   IDR_MAIN_MENU_WIN1,
                                   (PHWND)&hwndMain);
	if (hwndMainFrame == 0)
		{
	
      MessageBox(HWND_DESKTOP,
                   IDMSG_TEST,
                   MB_OK | MB_ERROR,
                   TRUE);
		return(0);
		}
   
   WinSetWindowText(hwndMainFrame, szClientClass1);

   HandleComm = 0;

   InitTermDriver();

   while(WinGetMsg(hmq, (PQMSG)&qmsg, 0, 0, 0))
       WinDispatchMsg(hmq, (PQMSG)&qmsg);

	WinDestroyMsgQueue(hmq);
	WinTerminate(hab);
	return 0;

}

/****************************************************************************

****************************************************************************/
MRESULT EXPENTRY ClientWndProc (HWND hwnd,USHORT msg, MPARAM mp1 ,MPARAM mp2)
{

	switch (msg)
		{
		case WM_PAINT:
			   MainPaint(CLR_BLUE,hwnd);
			   break;
		
      case WM_COMMAND:
            MainCommand(mp1, mp2);
			   break;

		default:
            return(WinDefWindowProc(hwnd, msg, mp1, mp2));
            break;

   }
   return (MRESULT)0;
}

/****************************************************************************

****************************************************************************/
VOID MainPaint(ULONG color, HWND hwnd)
{
    RECTL rclUpdate;
    HPS hps;

    hps = WinBeginPaint(hwnd, NULLHANDLE, (PRECTL)&rclUpdate);

    WinFillRect(hps, (PRECTL)&rclUpdate, color);

    WinEndPaint(hps);
} 


/*****************************************************************************

*****************************************************************************/

VOID APIENTRY ExitProc(USHORT usTermCode)
{

    WinDestroyMsgQueue(hmq);

    WinTerminate(hab);

    DosExitList(EXLST_EXIT, (PFNEXITLIST)ExitProc);

    usTermCode;
} 


/*****************************************************************************
*****************************************************************************/
SHORT MessageBox(HWND hwndOwner, SHORT idMsg, SHORT fsStyle, BOOL fBeep)
{
   CHAR szText[MESSAGELEN];

   if(!WinLoadMessage(hab, (HMODULE)NULL, idMsg, MESSAGELEN, (PSZ)szText))
   {
      WinAlarm(HWND_DESKTOP, WA_ERROR);
      return MBID_ERROR;
   }

   if(fBeep)
      WinAlarm(HWND_DESKTOP, WA_ERROR);

   return(WinMessageBox(HWND_DESKTOP, hwndOwner, szText, (PSZ)NULL,
          IDD_MSGBOX, fsStyle));

} 

/****************************************************************************

****************************************************************************/

VOID WinDebugMsg(char *DebugMsg,HWND hwnd)
{

   char *StrBuf;

   int end_of_line,insert_pos,start_of_filename;

   StrBuf = DebugMsg;

   end_of_line = 0;
   while (StrBuf[end_of_line++] != '\0');
   end_of_line--;

   insert_pos = 0;
   while (StrBuf[insert_pos++] != '\\');
   insert_pos -= 3;

   start_of_filename = end_of_line;
   while (StrBuf[start_of_filename--] != '\\');
   start_of_filename++;

   while (StrBuf[start_of_filename++] != '\0')
      {
      StrBuf[insert_pos++] = StrBuf[start_of_filename];
      }

   StrBuf[insert_pos] = '\0';

   WinMessageBox(HWND_DESKTOP, hwnd, DebugMsg, "Debug Message",
                     0, MB_NOICON | MB_OK  );

}

/****************************************************************************

****************************************************************************/
BOOL CommPortOpen(HWND hwnd)
{

   if (HandleComm == 0)
      {
       MessageBox(HWND_DESKTOP,
                   IDMSG_NOCOMMPORTSELECTED,
                   MB_OK | MB_ERROR,
                   TRUE);

      return FALSE;
      }
   else
      return TRUE;

}

/****************************************************************************

****************************************************************************/
VOID MainCommand(MPARAM mp1, MPARAM mp2)
{
   static SHORT sIdDlg;
   SHORT sOptions, sIdText;
   APIRET  rc;

   switch(SHORT1FROMMP(mp1))
      {
		case IDM_PORT_SELECTION:      //Menu selection ID
            sIdDlg = IDD_PORT_SELECT;  //Dialog selection ID
			   rc = WinDlgBox(HWND_DESKTOP, hwndMain,
							(PFNWP)ComSelectDlgProc,
							(HMODULE)0, sIdDlg, 
                     (PVOID)&sIdDlg); 
			   break;

		case IDM_PORT_INIT:      //Menu selection ID
            sIdDlg = IDD_PORT_INIT;  //Dialog selection ID
			   rc = WinDlgBox(HWND_DESKTOP, hwndMain,
							(PFNWP)ComInitDlgProc,
							(HMODULE)0, sIdDlg, 
                     (PVOID)&sIdDlg); 
			   break;

		case IDM_SLIDER_CONTROL:      //Menu selection ID
            sIdDlg = IDD_SLIDER_CONTROL;  //Dialog selection ID
			   rc = WinDlgBox(HWND_DESKTOP, hwndMain,
							(PFNWP)SliderDlgProc,
							(HMODULE)0, sIdDlg, 
                     (PVOID)&sIdDlg); 
			   break;

	   default:
            break;
   }
} 


