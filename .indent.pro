/* $Id: .indent.pro,v 1.1 2003/12/02 20:46:19 nenolod Exp $ */
/* copy this file to the source dir then run indent file.c */

--gnu-style
/* Disable tabs... */
-nut
/* This is the indent before the brace not inside the block. */
--brace-indent0
/* Indent case: by 2 and braces inside case by 0(then by 0)... */
--case-brace-indentation0
--case-indentation2
/* Put while() on the brace from do... */
--cuddle-do-while
/* Disable an annoying format... */
--no-space-after-function-call-names
/* Disable an annoying format... */
--no-space-after-casts
