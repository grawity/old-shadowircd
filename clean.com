$! $Id: clean.com,v 1.1 2004/07/29 15:26:30 nenolod Exp $
$! yes, its a hack. this needs to be merged into make.com.
$! (well, really, into a top-level descrip.mms.. one day).
$
$ WRITE SYS$OUTPUT "Cleaning tree from all compiled objects..."
$ DELETE [...]*.EXE;*
$ DELETE [...]*.OLB;*
$ DELETE [...]*.OBJ;*
$ WRITE SYS$OUTPUT "All done."

