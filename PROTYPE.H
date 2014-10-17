/****************************************************************************
This structure is used for inter-thread communications. When a buffer is 
sub-alloacted from the heap, pbBase is the Heap id or base, BuffPtr is 
a pointer to the buffer sub-allocated from the Heap, ulSize is the size
of the buffer.  BUFFWEFORMAT is the template assigned to BuffPtr.

N O T E: The sending thread must fill in the first 3 fields. This is because
the receiving thread "de-allocates" the buffer using those fields as
parmaters to DosSubFreeMem.

****************************************************************************/
typedef struct
{
	PBYTE pbBase;  //Base ptr for the Heap
	void * BuffPtr; //Ptr for the block allocated from the heap
	ULONG ulSize;  //Size of the block allocated
	ULONG id;      //Who sent the message
	ULONG CharCnt; //Number of chars in Buff[]
	UCHAR Buff[1];
} BUFFERFORMAT;



MRESULT EXPENTRY ClientWndProc (HWND hwnd, USHORT msg ,MPARAM mp1, MPARAM mp2);
VOID MainPaint(ULONG color, HWND hwnd);
VOID APIENTRY ExitProc(USHORT usTermCode);
VOID WinDebugMsg(char *DebugMsg,HWND hwnd);
BOOL CommPortOpen(HWND hwnd);
SHORT MessageBox(HWND hwndOwner, SHORT idMsg, SHORT fsStyle, BOOL fBeep);
VOID MainCommand(MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY ComSelectDlgProc( HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY ComInitDlgProc( HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY SliderDlgProc( HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2);
void	Allocate_Buffer_Memory(void);
void APIENTRY Terminate_Session(void);
void Term_Write_Thread(void * Parm1);
void Term_Read_Thread(void * Parm1);
void Create_Threads(void);
void Set_Baud_Rate(void);
void Set_Line_Characteristics(void);
void Set_DCB_Parms(void);
void Open_Comm_Port(void);
void CreateTermWrtQueue(void);
void PrintTerm(void *ptr);
BUFFERFORMAT * AllocateMsgBuffer(ULONG BuffSize);
void InitTermDriver(void);
APIRET InitLineBits(HWND hwnd);
APIRET SetLineBitVals( HWND hwnd);
BOOL InitializeSpinButton(HWND hwnd);
USHORT SetBiteRate( HWND hwnd);
void Set_IOCTL_Bit_Rate(USHORT BitRate,HWND hwnd);
USHORT Get_IOCTL_Bit_Rate(HWND hwnd);
APIRET OpenComPort( HWND hwnd);
APIRET WmInitComSelect( HWND hwnd);
VOID InitSlider ( HWND    hwndSlider,             
                  ULONG   idSlider,               
                  USHORT  usMaximumTicks,         
                  USHORT  usMinorTickSpacing,     
                  USHORT  usMinorTickSize,        
                  USHORT  usMajorTickSpacing,     
                  USHORT  usMajorTickSize,        
                  USHORT  usDetentSpacing,        
                  USHORT  usTextSpacing,          
                  PSZ     pszFont);               










