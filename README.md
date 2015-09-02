# unletterbox_PGM_c
to detect and remove black letterboxes in a PGM image file using c

/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** <unletterbox_pgm>.cpp
** This is supposed to read a PGM file (P5 format), detect and crop the left 
** and right black bars in the image. 
**
** USAGE: (ideally all in the same folder)
** g++ -o unletterbox_pgm_final.o unletterbox_pgm_final.cpp
** ./unletterbox_pgm_final.o 17.pgm
**
** Author: <Akshay Pulipaka 08/04/2015>
** -------------------------------------------------------------------------*/

/* 
** ASSUMPTIONS:
** 
** pillar bars are always black 
** 8-bit depth supported only. Checks included.
** Support only P5 version of PGM. Checks included.
** currently doesn't automatically handle comments in between header starting with #. 
** basic check included for comments above, but needs to be manually turned on.
** -------------------------------------------------------------------------*/
