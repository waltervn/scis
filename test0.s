.exports
	&MAINOBJ
	&TESTFN
.code
MAINFN: selfID
	push0
	callb 1 0
	push0
	call &TESTFN 0
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
NAME:	"my-\"name\""
