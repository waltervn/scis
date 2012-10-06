#include "symbols.h"

.exports
			&Game_obj

.locals
			0				; 0: Output file handle
			0				; 1: Input file handle
			0				; 2: Dynmem buffer

			0				; 3: FindFirstNumber/FindNextNumber string pointer
			0				; 4: FindFirstNumber/FindNextNumber string offset
			0				; 5: Additional return value

			0				; 6: Array to be passed to MergePoly
			0				; 7: Polygon list

			0				; 8-12: Plot colors 0-4
			0
			0
			0
			0

			0				; 13: Event

.dict
Game_methDict:
			1				; Number of functions
			$27				; play()
			&Game_play

.code
OpenFiles:
; Deletes and reopens output file and opens input file
; No arguments
			pushi 2
			pushi 4			; Unlink
			lofss &Outfile
			callk k_FileIO 4

			pushi 3
			push0			; Open
			lofss &Outfile
			push0
			callk k_FileIO 6
			sal 0

			pushi 3
			push0			; Open
			lofss &Infile
			push1
			callk k_FileIO 6
			sal 1
			ret

CloseFiles:
; Closes input and output files
; No arguments
			push2
			push1			; Close
			lsl 0
			callk k_FileIO 4

			pushi 2
			push1			; Close
			lsl 1
			callk k_FileIO 4
			ret

AllocDynmem:
; Allocates a dynmem buffer
; Arguments:
; 1:	Size in bytes
; Returns:
;		Pointer to newly allocated dynmem
			push2
			push1			; Critical
			lsp 1
			callk k_Memory 4
			ret

FreeDynmem:
; Frees a dynmem buffer
; Arguments:
; 1:	Dynmem handle
			push2
			pushi 3			; Free
			lsp 1
			callk k_Memory 4
			ret

WriteString:
; Writes formatted string to output file
; Arguments:
; 1:	Format string
; 2...	Arguments
			pushi 1
			lsl 2
			&rest 1
			callk k_Format 2

			pushi 3
			pushi 6			; WriteString
			lsl 0
			lsl 2
			callk k_FileIO 6
			ret

IsNumberChar:
; Tests if a character is part of a number
; Arguments:
; 1:	The character
; Returns:
;		1 if the character is part of a number, 0 otherwise
			lsp 1
			dup
			ldi 45			; '-'
			eq?
			bt &IsNumberChar_Yes
			dup
			ldi 48			; '0'
			lt?
			bt &IsNumberChar_No
			dup
			ldi 57			; '9'
			le?
			bt &IsNumberChar_Yes
IsNumberChar_No:
			ldi 0
			ret
IsNumberChar_Yes:
			ldi 1
			ret

NumberStart:
; Finds the start of a number in a string
; Arguments:
; 1:	The string
; 2:	Offset to start searching from
; Returns:
;		The offset of the start of the number, or -1 if not found
			pushi 2
			lsp 1
			lsp 2
			callk k_StrAt 4
			bnt &NumberStart_NotFound
			push1
			push
			call &IsNumberChar 2
			bt &NumberStart_Found
			lap 2
			push1
			add
			sap 2
			jmp &NumberStart
NumberStart_Found:
			lap 2
			ret
NumberStart_NotFound:		; End of string
			ldi $ffff
			ret

NumberEnd:
; Finds the first non-number character in a string
; Arguments:
; 1:	The string
; 2:	Offset to start searching from
; Returns:
;		The offset of the first non-number character (might point to nul)
			pushi 2
			lsp 1
			lsp 2
			callk k_StrAt 4
			push1
			push
			call &IsNumberChar 2
			bnt &NumberEnd_Found
			lap 2
			push1
			add
			sap 2
			jmp &NumberEnd
NumberEnd_Found:
			lap 2
			ret

FindNumber:
; Finds the next number in a string
; Arguments:
; 1:	The string pointer
; 2:	Offset to start searching from
; Returns:
;		The offset of the first character after the found number, or $ffff
;		Parsed number in local 3
			link 1
			push2
			lsp 1
			lsp 2
			call &NumberStart 4
			sat 0
			pushi $ffff
			eq?
			bnt &FindNumber_Found
			ldi $ffff
			ret
FindNumber_Found:
			push2
			lsp 1
			lst 0
			call &ReadStrNumber 4
			sal 3		
			push2
			lsp 1
			lst 0
			call &NumberEnd 4
			ret

ReadStrNumber:
; Reads a number from a string
; Arguments:
; 1:	String pointer
; 2:	Offset
; Returns:
;		Number
			push1
			lsp 1
			lap 2
			add
			push
			callk k_ReadNumber 2
			ret

ReadNumbers:
; Reads numbers from a string into an array
; Arguments:
; 1:	String pointer
; 2:	Offset to start reading from
; 3:	1 = Add terminator
; Returns:
;		Dynmem block containing the numbers
;		Stores block size (in bytes) in local 5
			link 2			; Temp 0: Total numbers read
							; Temp 1: Dynmem block
			ldi 0
			sat 0

ReadNumbers_Loop:
			push2
			lsp 1
			lsp 2
			call &FindNumber 4
			sap 2
			pushi $ffff
			eq?
			bt &ReadNumbers_Done
			lal 3
			push
			+at 0
			+at 0
			jmp &ReadNumbers_Loop
ReadNumbers_Done:
			lap 3
			bnt &ReadNumbers_Output
			pushi $7777
			+at 0
			+at 0
ReadNumbers_Output:
			pushi 1
			lst 0
			call &AllocDynmem 2
			sat 1
			pushi 4
			pushi 4			; MemCpy
			push
			lea 4 2
			push
			lst 0
			callk k_Memory 8
			lat 0
			sal 5
			lat 1
			ret

WritePoints:
; Writes points from an array to output file in human-readable form
; If $7777 is found in the input, reading is stopped
; Arguments:
; 1:	Array pointer
; 2:	Maximum points to write
			pushi 3
			lofss &PolyPoint
				push1
				lsp 1
				+ap 1
				+ap 1
				call &ReadRawNumber 2
				pushi $7777
				eq?
				bt &WritePoints_End
			pprev
				push1
				lsp 1
				+ap 1
				+ap 1
				call &ReadRawNumber 2
			push
			call &WriteString 6
			-ap 2
			bt &WritePoints
WritePoints_End:
			push1
			lofss &PolyEnd
			call &WriteString 2
			ret

FindColor:
; Finds a close color in the palette
; Arguments:
; 1:		r
; 2:		g
; 3:		b
			push1
			pushi 5
			&rest 1
			callk k_Palette 2
			ret

DrawLine:
; Draws a line
; Arguments:
; 1:		x1
; 2:		y1
; 3:		x2
; 4:		y2
; 5:		color
; 6:		priority (optional)
; 7:		control (optional)
			pushi 6
			pushi 4
			lsp 2
			lsp 1
			lsp 4
			lsp 3
			lap 5
			lsli 8
			callk k_Graph 12
			ret

DrawPoints:
; Draws points from an array to the screen
; If $7777 is found in the input, reading is stopped
; Arguments:
; 1:	Array pointer
; 2:	Number of points in array, or $ffff for terminated list
; 3:	Color
			link 6			; Temp 0+1: (prevx, prevy)
							; Temp 2+3: (x, y)
							; Temp 4+5: (firstx, firsty)

			; Check for 0 points in array
			lap 2
			bnt &DrawPoints_End

			; Load x-coordinate of first point
			push1
			lsp 1
			+ap 1
			+ap 1
			call &ReadRawNumber 2
			sat 0
			sat 4
			pushi $7777
			eq?
			bt &DrawPoints_End

			; Load y-coordinate of first point
			push1
			lsp 1
			+ap 1
			+ap 1
			call &ReadRawNumber 2
			sat 1
			sat 5

			; Check for 1 point in array
			-ap 2
			bnt &DrawPoints_Finish

DrawPoints_Loop:
			; Load next x-coordinate
			push1
			lsp 1
			+ap 1
			+ap 1
			call &ReadRawNumber 2
			sat 2
			pushi $7777
			eq?
			bt &DrawPoints_Finish

			; Load next y-coordinate
			push1
			lsp 1
			+ap 1
			+ap 1
			call &ReadRawNumber 2
			sat 3

			pushi 5
			lst 0
			lst 1
			lst 2
			lst 3
			lsp 3
			call &DrawLine 10

			lat 2
			sat 0
			lat 3
			sat 1
			
			-ap 2
			bt &DrawPoints_Loop

DrawPoints_Finish:
			pushi 5
			lst 0
			lst 1
			lst 4
			lst 5
			lsp 3
			call &DrawLine 10

DrawPoints_End:
			ret

ReadPolygon:
; Reads polygon spec from next line in input file
; Returns:
;		New polygon object or NULL on error
			link 4			; Temp 0: Polygon object
							; Temp 1: String offset
							; Temp 2: Points array
							; Temp 3: Current size of points array in bytes
			pushi 4
			pushi 5			; ReadString
			lsl 2
			pushi 1024
			lsl 1
			callk k_FileIO 8

			push2
			lsl 2
			push0
			call &FindNumber 4
			sat 1
			pushi $ffff
			eq?
			bnt &ReadPolygon_0
			ldi 0
			ret

ReadPolygon_0:
			class c_Polygon
			pushi s_new
			push0
			send 4
			sat 0

			pushi s_type
			push1
			lsl 3
			pushi s_dynamic
			push1
			push1
			pushi s_points
			push1
				pushi 3
				lsl 2
				lst 1
				push0
				call &ReadNumbers 6
			push
			pushi s_size
			push1
				lsl 5
				ldi 2
				shr
			push
			lat 0
			send 24
			ret

WriteRawNumber:
; Writes a raw number
; Arguments:
; 1:	Pointer to memory block
; 2:	Offset to write at
; 3:	The number to write
			pushi 3
			pushi 6			; Poke
				lap 1
				lsp 2
				add
			push
			lsp 3
			callk k_Memory 6
			ret

ReadRawNumber:
; Reads a number from a raw pointer
; Arguments:
; 1:	Pointer
			pushi 2
			pushi 5			; Peek
			lsp 1
			callk k_Memory 4
			ret

DrawPolygon:
;1:		Polygon
			link 1			; 0: Polygon type
			pushi s_type
			push0
			lap 1
			send 4
			sat 0

			; Ignore polygons marked by MergePoly
			pushi $10
			and
			bt &DrawPolygon_End

			pushi 3
				pushi s_points
				push0
				lap 1
				send 4
			push
				pushi s_size
				push0
				lap 1
				send 4
			push
			lst 0
			call &DrawPoints 6

DrawPolygon_End:
			ret

WritePolygon:
; Writes an array of points to output file
; Arguments:
; 1:	Polygon
			link 3
			push2
			lofss &PolyType
				pushi s_type
				push0
				lap 1
				send 4
			push
			call &WriteString 4
			pushi s_points
			push0
			lap 1
			send 4
			sat 1
			pushi s_size
			push0
			lap 1
			send 4
			sat 2
			ldi 0
WritePolygon_1:
			sat 0
			lst 2
			eq?
			bnt &WritePolygon_2
			push1
			lofss &PolyEnd
			call &WriteString 2
			ret
WritePolygon_2:
			pushi 3
			lofss &PolyPoint
				push1
				lst 1
				call &ReadRawNumber 2
			push
				push1
				lat 1
				pushi 2
				add
				push
				call &ReadRawNumber 2
			push			
			call &WriteString 6
			lat 1
			pushi 4
			add
			sat 1
			lat 0
			push1
			add
			jmp &WritePolygon_1

ReadPolygons:
; Reads polygons from the input file and stores them in a list in local 7
			push0
			callk k_NewList 0
			sal 7

ReadPolygons_Loop:
			push0
			call &ReadPolygon 0
			bt &ReadPolygons_Add
			ret
ReadPolygons_Add:
			push2
			lsl 7
				push2
				push
				push
				callk k_NewNode 4
			push
			callk k_AddToEnd 4
			jmp &ReadPolygons_Loop		

WritePolygons:
; Writes a polygon list to the output file in human-readable form
; Arguments:
; 1:	Polygon list
			link 1				; Temp 0: Current node
			push1
			lsp 1
			callk k_FirstNode 2

WritePolygons_Loop:
			sat 0
			bt &WritePolygons_Node
			ret

WritePolygons_Node:
			push1
				push1
				push
				callk k_NodeValue 2
			push
			call &WritePolygon 2
			push1
			lst 0
			callk k_NextNode 2
			jmp &WritePolygons_Loop

DrawPolygons:
; Draws a polygon list to the screen
; Arguments:
; 1:	Polygon list
			link 1				; Temp 0: Current node
			push1
			lsp 1
			callk k_FirstNode 2

DrawPolygons_Loop:
			sat 0
			bt &DrawPolygons_Node
			ret

DrawPolygons_Node:
			push1
				push1
				push
				callk k_NodeValue 2
			push
			call &DrawPolygon 2
			push1
			lst 0
			callk k_NextNode 2
			jmp &DrawPolygons_Loop

FreePolygons:
; Frees a polygon list
; Arguments:
; 1:	Polygon list
			link 1				; Temp 0: Current node
			push1
			lsp 1
			callk k_FirstNode 2

FreePolygons_Loop:
			sat 0
			bt &FreePolygons_Node

			push1
			lsp 1
			callk k_DisposeList 2
			ret

FreePolygons_Node:
				push1
				push
				callk k_NodeValue 2
			pushi s_dispose
			push0
			send 4

			push1
			lst 0
			callk k_NextNode 2

			jmp &FreePolygons_Loop

Redraw:
			pushi 5
			pushi 13		; RedrawBox
			push0
			push0
			pushi 190
			pushi 320
			callk k_Graph 10
			ret

ClearScreen:
			pushi 5
			pushi  10		; FillBoxBackground
			push0
			push0
			pushi 190
			pushi 320
			callk k_Graph 10
			ret

InitColors:
			pushi 3
			push0
			pushi $ff
			push0
			call &FindColor 6
			sal 8

			pushi 3
			push0
			push0
			pushi $ff
			call &FindColor 6
			sal 9

			pushi 3
			pushi $ff
			push0
			push0
			call &FindColor 6
			sal 10

			pushi 3
			pushi $ff
			pushi $ff
			push0
			call &FindColor 6
			sal 11

			pushi 3
			push0
			pushi $ff
			pushi $ff
			call &FindColor 6
			sal 12
			ret

WaitKeyDown:
			push2
			pushi 5			; Keyboard or mouse press
			lsl 13
			callk k_GetEvent 4
			bnt &WaitKeyDown
			ret

Game_play:
			link 3			; Temp 0: Input sets handled
							; Temp 1: MergePoly output
							; Temp 2: Interactive mode flag
			
			ldi 0
			sat 2

			class c_Event
			pushi s_new
			push0
			send 4
			sal 13

			push1
			push0			; Cursor off
			callk k_SetCursor 2

			push0
			call &InitColors 0

			ldi 0
			sat 0
			push0
			call &OpenFiles 0
			push1
			pushi 1024
			call &AllocDynmem 2
			sal 2

Loop:
			pushi 4
			pushi 5			; ReadString
			lsl 2
			pushi 1024
			lsl 1
			callk k_FileIO 8

			lat 0
			bt &SkipCommandCheck

			pushi 2
			lsl 2
			lofss &Interact
			callk k_StrCmp 4
			bt &SkipCommandCheck ; Command not found, try reading numbers

			+at 2
			jmp &Loop

SkipCommandCheck:
			pushi 3
			lsl 2
			push0
			push1
			call &ReadNumbers 6
			sal 6

			; If we failed to load any points, exit
			lal 5
			push2
			eq?
			bt &Exit

			push0
			call &ReadPolygons 0

			push2
			lsl 6
			lat 0
			bnt &SkipSep
				push1
				lofss &SepStr
				call &WriteString 2
SkipSep:
				push1
				lofss &InputStr
				call &WriteString 2
				push1
				lofss &PolyRaw
				call &WriteString 2
			pushi $ffff
			call &WritePoints 4

			push1
			lsl 7
			call &WritePolygons 2

			lat 2
			bnt &SkipDraw_1

			push0
			call &ClearScreen 0

			pushi 3
			lsl 6
			pushi $ffff
			pushi 4
			call &DrawPoints 6

			push1
			lsl 7
			call &DrawPolygons 2

			push0
			call &Redraw 0
			push0
			call &WaitKeyDown 0

SkipDraw_1:
			pushi 3
			lsl 6
			lsl 7
			pushi 2
			callk k_MergePoly 6
			sat 1

			push2
			push
				push1
				lofss &OutputStr
				call &WriteString 2
				push1
				lofss &PolyRaw
				call &WriteString 2
			pushi $ffff
			call &WritePoints 4

			push1
			lsl 7
			call &WritePolygons 2

			lat 2
			bnt &SkipDraw_2

			push0
			call &ClearScreen 0

			pushi 3
			lst 1
			pushi $ffff
			pushi 4
			call &DrawPoints 6

			push1
			lsl 7
			call &DrawPolygons 2

			push0
			call &Redraw 0
			push0
			call &WaitKeyDown 0

SkipDraw_2:
			push1
			lsl 7
			call &FreePolygons 2

			push1
			lsl 6
			call &FreeDynmem 2

			push1
			lst 1
			call &FreeDynmem 2

			+at 0
			jmp &Loop

Exit:
			push0
			call &CloseFiles 0
			ret

.object
Game_obj:
			$1234
			$10
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
			0
			3
			0
			0

.strings
Game_name:	"MergePoly"
Infile:		"input.txt"
Outfile:	"output.txt"
PolyType:	"%d:"
PolyPoint:	" (%d, %d)"
PolyEnd:	";\n"
PolyRaw:	"R:"
InputStr:	"Input:\n"
OutputStr:	"Output:\n"
SepStr:		"\n"
Interact:	"interactive"
