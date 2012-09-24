.exports
	&Game_obj

.locals
	9
	8
	7

.class
Game_methDict:
	1		; Number of functions
	$27		; play()
	&Game_play

.code
Game_play:
	ret

.object
Game_obj:
	$1234
	$10
	&Game_methDict
	&Game_methDict
	0
	$ffff
	$44		; Game
	0
	&Game_name
	0
	1
	1
	0
	3
	0
	0

	0		; Fixme
.strings
Game_name:
	"Sci11Test"
