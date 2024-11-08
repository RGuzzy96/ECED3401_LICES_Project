#pragma once
/*
 - VT100 demo header file

 - ECED 3401
	- 3 Oct 24 - Original (Dr. Larry Hughes)
*/
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>

/* Windows specific header files */
#include <windows.h>	/* VT-100 */
#include <conio.h>		/* kbhit() */

#define NUL	'\0'
#define ESCAPE	0x1b

/*
 - Formatting for ESC sequences
 - "\x1b" is treated by printf() as 0x1b (Escape)
 - Concatenated to following string
*/
#define ESC "\x1b"	/* Escape */
#define CSI "\x1b["	/* Control Sequence Introducer */

#define TRUE	1
#define FALSE	0

/* Some commonly used VT-100 commands */
#define CUP(c,r)	printf(CSI "%d;%dH", (r), (c)); /* Move cursor position to col, row */
#define EL(r)		printf(CSI "%d;1H" CSI "K", (r)); /* Erase in Line */
#define CLRSCR		printf(CSI "2J");

/* Cursor keys */
#define KPNM	printf(ESC ">");	/* Enable Keypad Numeric Mode '[' */
#define ECKM	printf(CSI "?1h");	/* Enable Cursor Keys Application Mode 'O' */
#define DCKM	printf(CSI "?1l");	/* Disable Cursor Keys Application Mode */
#define EDLDM	printf(ESC "(0");	/* Enable DEC Line Drawing Mode */
#define EAM		printf(ESC "(B");	/* Enable ASCII Mode */

/* DEC colour: ESC [ <n> m */
enum VT100_COLOURS {
FGBLACK = 30,
FGRED, FGGREEN, FGYELLOW, FGBLUE, FGMAGENTA, FGCYAN, FGWHITE,
BGBLACK = 40,
BGRED, BGGREEN, BGYELLOW, BGBLUE, BGMAGENTA, BGCYAN, BGWHITE
};

/* DEC line drawing mode characters*/
enum symbol { LR, UR, UL, LL, XX, HR, TR, TL, TU, TD, VR, EM };

/* External data */
extern HANDLE scrout, kbin;
