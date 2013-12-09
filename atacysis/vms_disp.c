/****************************************************************
*Copyright (c) 1993 Bell Communications Research, Inc. (Bellcore)
*
*Permission to use, copy, modify, and distribute this material
*for any purpose and without fee is hereby granted, provided
*that the above copyright notice and this permission notice
*appear in all copies, and that the name of Bellcore not be
*used in advertising or publicity pertaining to this
*material without the specific, prior written permission
*of an authorized representative of Bellcore.  BELLCORE
*MAKES NO REPRESENTATIONS ABOUT THE ACCURACY OR SUITABILITY
*OF THIS MATERIAL FOR ANY PURPOSE.  IT IS PROVIDED "AS IS",
*WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES.
****************************************************************/
#ifdef MVS
#include <mvapts.h>
MODULEID(%M%,%J%/%D%/%T%)
#endif /* MVS */

static char vms_disp_c[] = "$Id: vms_disp.c,v 3.6 2013/12/09 00:34:18 tom Exp $";
static char bellcoreCopyRight[] =
"Copyright (c) 1993 Bell Communications Research, Inc. (Bellcore)";

/*
*@Log: vms_disp.c,v @
*Revision 3.4  1994/04/04 13:51:28  saul
*FROM_KEYS
*
* Revision 3.4  94/04/04  13:51:28  saul
* Fix binary copyright.
* 
* Revision 3.3  94/04/04  10:26:48  jrh
* Add Release Copyright
* 
* Revision 3.2  93/08/04  15:59:36  ewk
* Added MVS and solaris support.  Squelched some ANSI warnings.
* 
* Revision 3.1  93/07/09  12:02:16  saul
* Add disp_elipsis and disp_windowsize.
* 
* Revision 3.0  92/11/06  07:48:07  saul
* propagate to version 3.0
* 
* Revision 2.3  92/03/17  15:27:19  saul
* copyright
* 
* Revision 2.2  91/12/13  09:28:35  saul
* same length file name bug
* 
* Revision 2.1  91/06/19  13:10:13  saul
* Propagate to version 2.0
* 
* Revision 1.1  91/06/12  16:58:42  saul
* Aug 1990 baseline
* 
*-----------------------------------------------end of log
*/
#include stdio
#include descrip
#include stsdef
#include smgdef
#include "disp.h"

/*
* VMS system status:
*/
#define NORMAL(code)	$VMS_STATUS_SUCCESS(code)
#define ABNORMAL(code)	(! $VMS_STATUS_SUCCESS(code))

#define MAX_LINE	200

static display_init = 0;
static unsigned display_id;
static unsigned new_pid;
static unsigned kb_id;
static unsigned kbd_id;
static outcol;
static outline;
static disp_lines;

static void
init_display(void)
{
    int status;
    int kb_cols;
    unsigned short inchar;
    char v_str[] = "Input was <n>.";
    $DESCRIPTOR(v, v_str);

    display_init = 1;

    status = SMG$CREATE_PASTEBOARD(&new_pid, 0, &disp_lines, &kb_cols,
				   0, 0);
    if (ABNORMAL(status))
	LIB$STOP(status);
    --disp_lines;

    status = SMG$CREATE_VIRTUAL_DISPLAY(&disp_lines, &MAX_LINE, &display_id,
					0, 0, 0);
    if (ABNORMAL(status))
	LIB$STOP(status);

    status = SMG$CREATE_VIRTUAL_DISPLAY(&1, &kb_cols, &kbd_id, 0, 0, 0);
    if (ABNORMAL(status))
	LIB$STOP(status);

    status = SMG$CREATE_VIRTUAL_KEYBOARD(&kb_id, 0, 0, 0, 0);
    if (ABNORMAL(status))
	LIB$STOP(status);

    status = SMG$PASTE_VIRTUAL_DISPLAY(&display_id, &new_pid,
				       &1, &1);
    if (ABNORMAL(status))
	LIB$STOP(status);

    ++disp_lines;
    status = SMG$PASTE_VIRTUAL_DISPLAY(&kbd_id, &new_pid,
				       &disp_lines, &1);
    if (ABNORMAL(status))
	LIB$STOP(status);
    --disp_lines;

    outcol = 1;
    outline = 1;
}

static void
do_prompt(void)
{
    int status;
    short inchar;
    $DESCRIPTOR(prompt, "<Return> or <q> to quit: ");

    status = SMG$READ_KEYSTROKE(&kb_id, &inchar, &prompt, 0,
				&kbd_id, &SMG$M_BOLD, 0);
    if (ABNORMAL(status))
	LIB$STOP(status);
    outline = 1;
    outcol = 1;
    if (inchar == 'q') {
	disp_end();
	exit(0);
    }
    status = SMG$ERASE_DISPLAY(&kbd_id, 0, 0, 0, 0);
    if (ABNORMAL(status))
	LIB$STOP(status);
}

void
disp_str(const char *str,
	 int attributes)
{
    struct dsc$descriptor_s string;
    int status;
    int len;
    int disp_attributes;
    $DESCRIPTOR(empty, "");

    if (display_init == 0)
	init_display();

    if ((attributes & DISP_CLEAR) && (outline != 1 || outcol != 1)) {
	do_prompt();
	status = SMG$ERASE_DISPLAY(&display_id, 0, 0, 0, 0);
	if (ABNORMAL(status))
	    LIB$STOP(status);
    } else if (outline > disp_lines)
	do_prompt();

    len = strlen(str);
    string.dsc$w_length = len;
    string.dsc$b_dtype = DSC$K_DTYPE_T;
    string.dsc$b_class = DSC$K_CLASS_S;
    string.dsc$a_pointer = str;

    if (attributes & DISP_HILI)
	disp_attributes = SMG$M_REVERSE;
    else
	disp_attributes = 0;
    if (attributes & DISP_NEWLINE) {
	status = SMG$PUT_LINE(&display_id, &string, 0, &disp_attributes,
			      0, 0, 0, 0);
	if (ABNORMAL(status))
	    LIB$STOP(status);
	++outline;
	outcol = 1;
    } else {
	if (outcol == 1) {
	    status = SMG$PUT_LINE(&display_id, &empty, &0, 0,
				  0, 0, 0, 0);
	    if (ABNORMAL(status))
		LIB$STOP(status);
	}
	status = SMG$PUT_CHARS(&display_id, &string, 0, 0,
			       0, &disp_attributes, 0, 0);
	if (ABNORMAL(status))
	    LIB$STOP(status);
	outcol += len;
    }
}

void
disp_file(const char *filename,
	  int f_line,
	  int f_col,
	  int t_line,
	  int t_col,
	  int attributes)
{
    static char cur_file[MAX_SRCFILE_NAME];
    static cur_line;
    static cur_col;
    static FILE *f = NULL;
    int c;
    int status;
    int disp_attributes;
    char *p;
    char buf[MAX_LINE];
    $DESCRIPTOR(outchar, buf);
    $DESCRIPTOR(spaces, "        ");
    $DESCRIPTOR(empty, "");

    if (display_init == 0)
	init_display();

    if (f == NULL || strcmp(filename, cur_file) || f_line < cur_line
	|| (f_line == cur_line && f_col < cur_col)) {
	strncpy(cur_file, filename, MAX_SRCFILE_NAME);
	cur_line = 1;
	cur_col = 1;
	if (f)
	    fclose(f);
	f = fopen(filename, "r");
	if (f == NULL) {
	    disp_end();
	    fprintf(stderr, "can't open %s\n", filename);
	    exit(1);
	}
    }
    while (cur_line < f_line ||
	   (cur_line == f_line && cur_col < f_col)) {
	c = getc(f);
	if (c == EOF) {
	    fclose(f);
	    f = NULL;
	    return;
	}
	if (c == '\n') {
	    ++cur_line;
	    cur_col = 1;
	} else
	    ++cur_col;
    }
    if (attributes & DISP_HILI)
	disp_attributes = SMG$M_REVERSE;
    else
	disp_attributes = 0;
    p = buf;
    while (cur_line < t_line || (cur_line == t_line && cur_col <= t_col)) {
	c = getc(f);
	if (c == EOF) {
	    fclose(f);
	    f = NULL;
	    return;
	}
	if (c == '\n') {
	    ++outline;
	    outcol = 1;
	    ++cur_line;
	    cur_col = 1;
	    outchar.dsc$w_length = p - buf;
	    if (outline > disp_lines)
		do_prompt();
	    status = SMG$PUT_LINE(&display_id, &outchar, 0,
				  &disp_attributes, 0, 0, 0, 0);
	    if (ABNORMAL(status))
		LIB$STOP(status);
	    p = buf;
	} else {
	    if ((attributes & DISP_INDENT) && outcol == 1) {
		if (outline > disp_lines)
		    do_prompt();
		status = SMG$PUT_LINE(&display_id, &empty, &0,
				      0, 0, 0, 0, 0);
		if (ABNORMAL(status))
		    LIB$STOP(status);
		status = SMG$PUT_CHARS(&display_id, &spaces,
				       0, 0, 0, 0, 0, 0);
		if (ABNORMAL(status))
		    LIB$STOP(status);
		outcol = 9;
	    }
	    if (p - buf < MAX_LINE)
		*p++ = c;
	    ++outcol;
	    ++cur_col;
	}
    }
    if (p != buf) {
	if (outline > disp_lines)
	    do_prompt();
	if (outcol == 1) {
	    status = SMG$PUT_LINE(&display_id, &empty, &0, 0,
				  0, 0, 0, 0);
	    if (ABNORMAL(status))
		LIB$STOP(status);
	}
	outchar.dsc$w_length = p - buf;
	status = SMG$PUT_CHARS(&display_id, &outchar, 0, 0,
			       0, &disp_attributes, 0, 0);
	if (ABNORMAL(status))
	    LIB$STOP(status);
    }
}

void
disp_end(void)
{
    int status;

    if (display_init == 0)
	return;

    if (outline != 1 || outcol != 1)
	do_prompt();
    display_init = 0;

    status = SMG$DELETE_PASTEBOARD(&new_pid, 0);
    if (ABNORMAL(status))
	LIB$STOP(status);
    status = SMG$DELETE_VIRTUAL_DISPLAY(&display_id);
    if (ABNORMAL(status))
	LIB$STOP(status);
    status = SMG$DELETE_VIRTUAL_DISPLAY(&kbd_id);
    if (ABNORMAL(status))
	LIB$STOP(status);
    status = SMG$DELETE_VIRTUAL_KEYBOARD(&kb_id);
    if (ABNORMAL(status))
	LIB$STOP(status);
}

void
disp_elipsis(int nSkipped)
{
    static int windowSize = 0;
    static char *buf;
    static char msg[] = " (%d lines skipped)";
    int i;

    if (windowSize == 0) {
	windowSize = disp_windowSize();
	if (windowSize < 2)
	    windowSize = 2;
	buf = (char *) malloc(windowSize + 1);
    }

    disp_str("", DISP_NEWLINE);

    for (i = 0; i < windowSize - 3; i += 2) {
	buf[i] = 'o';
	buf[i + 1] = ' ';
    }
    buf[windowSize - 2] = '\0';

    if ((windowSize - sizeof msg) >= 10) {
	sprintf(buf + (windowSize - sizeof msg) / 2 - 5, msg, nSkipped);
	buf[strlen(buf)] = ' ';
    }

    disp_str(buf, DISP_NEWLINE);

    disp_str("", DISP_NEWLINE);
}

int
disp_windowSize(void)
{
#ifdef TIOCGWINSZ
    struct winsize win;

    if (isatty(1) && ioctl(1, TIOCGWINSZ, &win) != -1 && win.ws_col != 0) {
	return win.ws_col;
    }
#endif /* TIOCGWINSZ */

    return 80;
}

#ifdef TEST
int
main(void)
{
    disp_str(" 1--------heading--------", DISP_CLEAR | DISP_NEWLINE);
    disp_str(" 2junk and stuff", DISP_NEWLINE);
    disp_str(" 3junk and stuff", DISP_NEWLINE);
    disp_str(" 4from==> ", DISP_HILI);
    disp_str("aaaaa", 0);
    disp_str("bbbbbbbbbb", DISP_HILI);
    disp_str("cccccc", DISP_NEWLINE);
    disp_str(" 5junk and stuff", DISP_NEWLINE);
    disp_str(" 6junk and stuff", DISP_NEWLINE);
    disp_str(" 7junk and stuff", DISP_NEWLINE);
    disp_str(" 8junk and stuff", DISP_NEWLINE);
    disp_str(" 9junk and stuff", DISP_NEWLINE);
    disp_str("10junk and stuff", DISP_NEWLINE);
    disp_str("11from==> ", DISP_HILI);
    disp_str("aaaaa", 0);
    disp_str("bbbbbbbbbb", DISP_HILI);
    disp_str("bbbbbbbbbb", DISP_HILI | DISP_NEWLINE);
    disp_str("12bbbbbbbbbb", DISP_HILI | DISP_NEWLINE);
    disp_str("13bbbbbbbbbb", DISP_HILI | DISP_NEWLINE);
    disp_str("14bbbbbbbbbb", DISP_HILI | DISP_NEWLINE);
    disp_str("15bbbbbbbbbb", DISP_HILI | DISP_NEWLINE);
    disp_str("16bbbbbbbbbb", DISP_HILI);
    disp_str("ccc", DISP_NEWLINE);
    disp_str("17junk and stuff", DISP_NEWLINE);
    disp_str("18junk and stuff", DISP_NEWLINE);
    disp_str("19junk and stuff", DISP_NEWLINE);
    disp_str("20junk and stuff", DISP_NEWLINE);
    disp_str("21junk and stuff", DISP_NEWLINE);
    disp_str("22junk and stuff", DISP_NEWLINE);
    disp_str("23from==> ", DISP_HILI);
    disp_str("aaaaa", 0);
    disp_str("bbbbbbbbbb", DISP_HILI);
    disp_str("cccccc", DISP_NEWLINE);
    disp_str("24junk and stuff", DISP_NEWLINE);
    disp_str("25junk and stuff", DISP_NEWLINE);
    disp_str("26junk and stuff", DISP_NEWLINE);
    disp_str("27junk and stuff", DISP_NEWLINE);
    disp_str("28junk and stuff", DISP_NEWLINE);
    disp_str("29junk and stuff", DISP_NEWLINE);
    disp_str("30from==> ", DISP_HILI);
    disp_str("aaaaa", 0);
    disp_str("bbbbbbbbbb", DISP_HILI);
    disp_str("bbbbbbbbbb", DISP_HILI | DISP_NEWLINE);
    disp_str("31bbbbbbbbbb", DISP_HILI | DISP_NEWLINE);
    disp_str("32bbbbbbbbbb", DISP_HILI | DISP_NEWLINE);
    disp_str("33bbbbbbbbbb", DISP_HILI | DISP_NEWLINE);
    disp_str("34bbbbbbbbbb", DISP_HILI | DISP_NEWLINE);
    disp_str("35bbbbbbbbbb", DISP_HILI);
    disp_str("ccc", DISP_NEWLINE);
    disp_str("36junk and stuff", DISP_NEWLINE);
    disp_str("37junk and stuff", DISP_NEWLINE);
    disp_str("38junk and stuff", DISP_NEWLINE);
    disp_str("39junk and stuff", DISP_NEWLINE);
    disp_str("40junk and stuff", DISP_NEWLINE);
    disp_str("41junk and stuff", DISP_NEWLINE);
    disp_str("42from==> ", DISP_HILI);
    disp_str("aaaaa", 0);
    disp_str("bbbbbbbbbb", DISP_HILI);
    disp_str("cccccc", DISP_NEWLINE);
    disp_str("43junk and stuff", DISP_NEWLINE);
    disp_str("44junk and stuff", DISP_NEWLINE);
    disp_str("45junk and stuff", DISP_NEWLINE);
    disp_str("46from==> ", DISP_HILI);
    disp_str("aaaaa", 0);
    disp_str("bbbbbbbbbb", DISP_HILI);
    disp_str("bbbbbbbbbb", DISP_HILI | DISP_NEWLINE);
    disp_str("47bbbbbbbbbb", DISP_HILI | DISP_NEWLINE);
    disp_str("48bbbbbbbbbb", DISP_HILI);
    disp_str("ccc", DISP_NEWLINE);
    disp_str("49junk and stuff", DISP_NEWLINE);
    disp_str("50junk and stuff", DISP_NEWLINE);
    disp_str("junk and stuff", DISP_NEWLINE);
    disp_str("junk and stuff", DISP_NEWLINE);
    disp_str("junk and stuff", DISP_NEWLINE);
    disp_str("from==> ", DISP_HILI);
    disp_str("aaaaa", 0);
    disp_str("bbbbbbbbbb", DISP_HILI);
    disp_str("cccccc", DISP_NEWLINE);
    disp_str("junk and stuff", DISP_NEWLINE);
    disp_str("junk and stuff", DISP_NEWLINE);
    disp_str("junk and stuff", DISP_NEWLINE);
    disp_str("from==> ", DISP_HILI);
    disp_str("aaaaa", 0);
    disp_str("bbbbbbbbbb", DISP_HILI);
    disp_str("bbbbbbbbbb", DISP_HILI | DISP_NEWLINE);
    disp_str("bbbbbbbbbb", DISP_HILI | DISP_NEWLINE);
    disp_str("bbbbbbbbbb", DISP_HILI);
    disp_str("ccc", DISP_NEWLINE);
    disp_str("junk and stuff", DISP_NEWLINE);
    disp_str("junk and stuff", DISP_NEWLINE);
    disp_str("junk and stuff", DISP_NEWLINE);
    disp_str("junk and stuff", DISP_NEWLINE);
    disp_str("junk and stuff", DISP_NEWLINE);
    disp_str("from==> ", DISP_HILI);
    disp_str("aaaaa", 0);
    disp_str("bbbbbbbbbb", DISP_HILI);
    disp_str("cccccc", DISP_NEWLINE);
    disp_str("junk and stuff", DISP_NEWLINE);
    disp_str("junk and stuff", DISP_NEWLINE);
    disp_str("junk and stuff", DISP_NEWLINE);
    disp_str("from==> ", DISP_HILI);
    disp_str("aaaaa", 0);
    disp_str("bbbbbbbbbb", DISP_HILI);
    disp_str("bbbbbbbbbb", DISP_HILI | DISP_NEWLINE);
    disp_str("bbbbbbbbbb", DISP_HILI | DISP_NEWLINE);
    disp_str("bbbbbbbbbb", DISP_HILI);
    disp_str("ccc", DISP_NEWLINE);
    disp_str("junk and stuff", DISP_NEWLINE);
    disp_str("junk and stuff", DISP_NEWLINE);
    disp_elipsis(27);
    disp_str("junk and stuff", DISP_NEWLINE);
    disp_str("heading", DISP_CLEAR | DISP_NEWLINE);
    disp_file("disp.c", 3, 0, 8, 11, 0);
    disp_file("disp.c", 8, 12, 8, 18, DISP_HILI);
    disp_file("disp.c", 8, 19, 15, 0, 0);
    disp_str("heading", DISP_CLEAR | DISP_HILI | DISP_NEWLINE);
    disp_file("disp.c", 3, 0, 8, 0, DISP_INDENT);
    disp_str("from==> ", DISP_HILI);
    disp_file("disp.c", 8, 1, 8, 11, DISP_INDENT);
    disp_file("disp.c", 8, 12, 8, 18, DISP_HILI | DISP_INDENT);
    disp_file("disp.c", 8, 19, 15, 0, DISP_INDENT);

    disp_end();
}
#endif
