# IBM Developer's Workframe/2 Make File Creation run at 19:51:43 on 02/02/93

# Make File Creation run in directory:
#   C:\OSII\SLIDCOM;

.SUFFIXES:

.SUFFIXES: .c .rc

ALL: SLIDCOM.EXE \
     SLIDCOM.RES

slidcom.exe:  \
  PMCOMDRV.OBJ \
  SLIDCOM.OBJ \
  SLIDCOM.RES \
  SLIDDLG.OBJ \
  slidcom.def \
  SLIDCOM.MAK
   @REM @<<SLIDCOM.@0
     /CO /ST:25000 /NOL /PM:PM +
     PMCOMDRV.OBJ +
     SLIDCOM.OBJ +
     SLIDDLG.OBJ
     slidcom.exe
     
     dde4mbs + os2386
     slidcom.def;
<<
   LINK386.EXE @SLIDCOM.@0
   RC SLIDCOM.RES slidcom.exe

{.}.rc.res:
   RC -r .\$*.RC

{.}.c.obj:
   ICC.EXE /Ss /Q /Ls /Ti /N200 /Gm /C .\$*.c

!include SLIDCOM.DEP
