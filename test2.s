.exports
	&MAINOBJ
	&TESTFN
.code
MAINFN: 
	push2
	ldi &NAME
	push
	push0
	callk $29 4
	ret

TESTFN:	ldi	$42
	ret
		
.class
	$1234
	$0000			; Locals
	22			; Functarea
	5			; # of varselcs
MAINOBJ:
	;; Varselectors
	0
	0
	0
	&NAME
	13
	
	$0			; species
	$1			; superclass
	$2			; -info-
	$17			; Name
	$2b			; number

	;; Funcselectors
	
	1			; One overridden
	$2a			; play
	0			; dummy
	&MAINFN

.strings
NAME:	"testfile"
