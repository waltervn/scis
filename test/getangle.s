#define ORG_X 0
#define ORG_Y 0
#define MIN_DEST_X 0xfff6
#define MAX_DEST_X 10
#define MIN_DEST_Y 0xfff6
#define MAX_DEST_Y 10

#include "symbols.h"
#include "subfuncs.h"

.exports
			&Game_obj

.locals
			0

#if SCI_VERSION < SCI_VERSION_1_1

.object
			$1234
			$0000			; Locals
			10				; Functarea
			4				; # of varselcs
Game_obj:
			; Varselectors

			c_Game			; species
			c_Game			; superclass
			$0				; -info-
			&Game_name		; name

			; Funcselectors
	
			1				; One overridden
			s_play			; play
			0				; dummy
			&Game_play

#else

.dict
Game_methDict:
			1				; Number of functions
			s_play
			&Game_play

.object
Game_obj:
			$1234
			12
			&Game_methDict
			&Game_methDict
			0
			$ffff
			c_Game
			0
			&Game_name
			0
			1
			1

#endif

#if SCI_VERSION >= SCI_VERSION_2

.dict
Str_methDict:
			0				; Number of functions

.object
			$1234
			12
			&Str_methDict
			&Str_methDict
			0
			$ffff
			c_Str
			0
			&Str_name
			0
			0
			0

#endif

.code
Game_play:
#define HANDLE 0
#define X1 1
#define Y1 2
#define X2 3
#define Y2 4
#define ANGLE 5
#define STR 6
#if SCI_VERSION < SCI_VERSION_2
#define BUF 7				; 128-word string buffer
			link 134
			lea 4 BUF
			sat STR
#else
			link 7
			push2
			push0			; New
			pushi 256		; 256-byte string
			callk k_String 4
			sat STR
#endif

#if SCI_VERSION < SCI_VERSION_01
			push2
			lofss &Outfile
			push2			; Truncate
			callk k_FOpen 4
			sat HANDLE
#else
			pushi 3
			pushi f_FileIO_Open
			lofss &Outfile
			push2			; Truncate
			callk k_FileIO 6
			sat HANDLE
#endif

			ldi ORG_X
			sat X1
			ldi ORG_Y
			sat Y1

			ldi MIN_DEST_Y
			sat Y2
loopY:
			ldi MIN_DEST_X
			sat X2

loopX:
			pushi 4
			lst X1
			lst Y1
			lst X2
			lst Y2
			callk k_GetAngle 8
			sat ANGLE

#if SCI_VERSION < SCI_VERSION_2
			pushi 7
			lst STR
			lofss &Format
			lst X1
			lst Y1
			lst X2
			lst Y2
			lst ANGLE
			callk k_Format 14
#else
			pushi 8
			pushi 12		; FormatBuf
			lst STR
			lofss &Format
			lst X1
			lst Y1
			lst X2
			lst Y2
			lst ANGLE
			callk k_String 16
#endif

#if SCI_VERSION < SCI_VERSION_01
			push2
			lst HANDLE
			lst STR
			callk k_FPuts 4
#else
			pushi 3
			pushi f_FileIO_WriteString
			lst HANDLE
			lst STR
			callk k_FileIO 6
#endif

			+at X2
			pushi MAX_DEST_X
			ge?
			bt &loopX

			+at Y2
			pushi MAX_DEST_Y
			ge?
			bt &loopY

#if SCI_VERSION < SCI_VERSION_01
			push1
			lst HANDLE
			callk k_FClose 2
#else
			push2
			pushi f_FileIO_Close
			lst HANDLE
			callk k_FileIO 4
#endif

			ret



.strings
Game_name:	"GetAngleTester"
Str_name:	"Str"
Outfile:	"output.txt"
Format:		"(%d, %d)-(%d, %d): %d\n"
