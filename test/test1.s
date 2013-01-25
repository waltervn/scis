.exports
	&MAINOBJ
.code
MAINFN:	SelfID
	ret
	
.class
	$1234
	$0000			; Locals
	18			; Functarea
	4			; # of varselcs
MAINOBJ:
	;; Varselectors
	0
	0
	$8000
	&NAME
	
	$0			; species
	$1			; superclass
	$2			; -info-
	$17			; Name

	;; Funcselectors
	
	1			; One overridden
	$2a			; play
	0			; dummy
	&MAINFN

.strings
NAME:	"my-\"name\""
