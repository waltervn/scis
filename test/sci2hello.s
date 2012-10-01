; Prints "Hello world" to output.txt (for SQ6)

.exports
				&Hello_obj
			
.locals
				0

.class
Hello_methDict:
				1
				$33
				&Hello_play

.object
Hello_obj:
				$1234			; -objID -
				17				; -size-
				&Hello_methDict	; -propDict-
				&Hello_methDict	; -methDict-
				0				; -classScript-
				$ffff			; -script-
				$50				; -super- (Game)
				0				; -info-
				&Hello_name		; name
				0				; scratch
				0				; script
				1				; printLang
				3				; _detailLevel
				0				; panelObj
				0				; panelSelector
				0				; handsOffCode
				0				; handsOnCode

				0				; Object list terminator (FIXME)

.code
Hello_play:
				; Open output.txt
				pushi 3
				push0			; Open
				lofss &Filename
				push2			; Truncate
				callk $5d 6		; FileIO

				; If file open failed, quit
				pushi $ffff
				eq?
				bt &Quit

				pushi 3
				pushi 6			; WriteString
				pprev			; Handle
				lofss &Greeting
				callk $5d 6		; FileIO

				push2
				push1			; Close
				pprev			; Handle
				callk $5d 4		; FileIO

Quit:
				ret

.strings
Hello_name:		"Hello"
Filename:		"output.txt"
Greeting:		"Hello World!\n"
