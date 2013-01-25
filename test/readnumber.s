#include "symbols.h"
#include "subfuncs.h"

.exports
			&Game_obj

.locals
			0

#if SCI_VERSION >= SCI_VERSION_2
#error SCI2+ not supported
#endif

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

.code
#include "lib/file.h"

Game_play:
#define IN_FILE 0
#define OUT_FILE 1
#define INBUF_PTR 2
#define OUTBUF_PTR 3
#define READ_VAL 4
#if SCI_VERSION < SCI_VERSION_2
#define INBUF 5				; 128-word string input buffer
#define OUTBUF 133			; 128-word string output buffer
			link 261
			lea 4 INBUF
			sat INBUF_PTR
			lea 4 OUTBUF
			sat OUTBUF_PTR
#else
#endif

			push2
			lofss &OutName
			push2			; Truncate
			call &FileOpen 4
			sat OUT_FILE
			push2
			lofss &InName
			push1			; Read
			call &FileOpen 4
			sat IN_FILE

loop:

			pushi 3
			lst INBUF_PTR
			pushi 256
			lst IN_FILE
			call &FileGets 6

			bnt	&quit		; Quit if nothing read

			push1
			lst INBUF_PTR
			callk k_ReadNumber 2
			sat READ_VAL

#if SCI_VERSION < SCI_VERSION_2
			pushi 4
			lst OUTBUF_PTR
			lofss &Format
			lst INBUF_PTR
			lst READ_VAL
			callk k_Format 8
#else
#endif

			push2
			lst OUT_FILE
			lst OUTBUF_PTR
			call &FilePuts 4

			jmp &loop

quit:

			push1
			lst IN_FILE
			call &FileClose 2
			push1
			lst OUT_FILE
			call &FileClose 2

			ret

.strings
Game_name:	"ReadNumberTester"
Str_name:	"OutputStr"
OutName:	"output.txt"
InName:		"input.txt"
Format:		"\"%s\": 0x%04x\n"
