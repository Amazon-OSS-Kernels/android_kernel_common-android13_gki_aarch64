// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/* #define	DEBUG	*/

#include <common.h>
#include <watchdog.h>
#include <autoboot.h>
#include <cli.h>
#include <console.h>
#include <version.h>
#include <flow.h>
#ifdef CONFIG_MULTICORES_PLATFORM
#include <bootflow_thread.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

/*
 * Board-specific Platform code can reimplement show_boot_progress () if needed
 */
__weak void show_boot_progress(int val) {}

//MTK START
static char * delete_char (char *buffer, char *p, int *colp, int *np, int plen);
static void cread_add_char(char ichar, int insert, unsigned long *num,
	       unsigned long *eol_num, char *buf, unsigned long len);
static const char erase_seq[] = "\b \b";		/* erase sequence	*/
static const char   tab_seq[] = "        ";		/* used to expand TABs	*/
unsigned int u32UbootStart = 0;
unsigned int u32UbootEnd = 0;
static int hist_max = 0;
static int hist_add_idx = 0;
static int hist_cur = -1;
unsigned hist_num = 0;

#define HIST_MAX		20
#define HIST_SIZE		CONFIG_SYS_CBSIZE
char* hist_list[HIST_MAX];
char hist_lines[HIST_MAX][HIST_SIZE + 1];	 /* Save room for NULL */

#define putnstr(str,n)	do {			\
		printf ("%.*s", (int)n, str);	\
	} while (0)
#define CTL_CH(c)		((c) - 'a' + 1)
#define CTL_BACKSPACE		('\b')
#define DEL			((char)255)
#define DEL7			((char)127)
#define CREAD_HIST_CHAR		('!')

#define getcmd_putch(ch)	putc(ch)
#define getcmd_getch()		getc()
#define getcmd_cbeep()		getcmd_putch('\a')
//MTK END


static void run_preboot_environment_command(void)
{
#ifdef CONFIG_PREBOOT
	char *p;

	p = env_get("preboot");
	if (p != NULL) {
		int prev = 0;

		if (IS_ENABLED(CONFIG_AUTOBOOT_KEYED))
			prev = disable_ctrlc(1); /* disable Ctrl-C checking */

		run_command_list(p, -1, 0);

		if (IS_ENABLED(CONFIG_AUTOBOOT_KEYED))
			disable_ctrlc(prev);	/* restore Ctrl-C checking */
	}
#endif /* CONFIG_PREBOOT */
}

/* We come here after U-Boot is initialised and ready to process commands */
void main_loop(void)
{
	const char *s;

	bootstage_mark_name(BOOTSTAGE_ID_MAIN_LOOP, "main_loop");

	if (IS_ENABLED(CONFIG_VERSION_VARIABLE))
		env_set("ver", version_string);  /* set version variable */

	cli_init();

	run_preboot_environment_command();

	if (IS_ENABLED(CONFIG_UPDATE_TFTP))
		update_tftp(0UL, NULL, NULL);

	s = bootdelay_process();
	if (cli_process_fdt(&s))
		cli_secure_boot_cmd(s);

#ifndef CONFIG_MULTICORES_PLATFORM
	do_before_console_input();
#endif
	autoboot_command(s);

	cli_loop();
	panic("No CLI available");
}

/****************************************************************************/
static void hist_init(void)
{
    int i;

    hist_max = 0;
    hist_add_idx = 0;
    hist_cur = -1;
    hist_num = 0;

    for (i = 0; i < HIST_MAX; i++) {
        hist_list[i] = hist_lines[i];
        hist_list[i][0] = '\0';
    }
}

static void cread_add_to_hist(char *line)
{
    strcpy(hist_list[hist_add_idx], line);

    if (++hist_add_idx >= HIST_MAX)
        hist_add_idx = 0;

    if (hist_add_idx > hist_max)
        hist_max = hist_add_idx;

    hist_num++;
}

static void cread_add_char(char ichar, int insert, unsigned long *num,
        unsigned long *eol_num, char *buf, unsigned long len)
{
    unsigned long wlen;

    /* room ??? */
    if (insert || *num == *eol_num) {
        if (*eol_num > len - 1) {
            getcmd_cbeep();
            return;
        }
        (*eol_num)++;
    }

    if (insert) {
        wlen = *eol_num - *num;
        if (wlen > 1) {
            memmove(&buf[*num+1], &buf[*num], wlen-1);
        }

        buf[*num] = ichar;
        putnstr(buf + *num, wlen);
        (*num)++;
        while (--wlen) {
            getcmd_putch(CTL_BACKSPACE);
        }
    } else {
        /* echo the character */
        wlen = 1;
        buf[*num] = ichar;
        putnstr(buf + *num, wlen);
        (*num)++;
    }
}

#define BEGINNING_OF_LINE() {			\
    while (num) {				\
        getcmd_putch(CTL_BACKSPACE);	\
        num--;				\
    }					\
}

#define ERASE_TO_EOL() {				\
    if (num < eol_num) {				\
        printf("%*s", (int)(eol_num - num), ""); \
        do {					\
            getcmd_putch(CTL_BACKSPACE);	\
        } while (--eol_num > num);		\
    }						\
}

#define REFRESH_TO_EOL() {			\
    if (num < eol_num) {			\
        wlen = eol_num - num;		\
        putnstr(buf + num, wlen);	\
        num = eol_num;			\
    }					\
}

static char* hist_prev(void)
{
    char *ret;
    int old_cur;

    if (hist_cur < 0)
        return NULL;

    old_cur = hist_cur;
    if (--hist_cur < 0)
        hist_cur = hist_max;

    if (hist_cur == hist_add_idx) {
        hist_cur = old_cur;
        ret = NULL;
    } else
        ret = hist_list[hist_cur];

    return (ret);
}

static char* hist_next(void)
{
    char *ret;

    if (hist_cur < 0)
        return NULL;

    if (hist_cur == hist_add_idx)
        return NULL;

    if (++hist_cur > hist_max)
        hist_cur = 0;

    if (hist_cur == hist_add_idx) {
        ret = "";
    } else
        ret = hist_list[hist_cur];

    return (ret);
}

static void cread_add_str(char *str, int strsize, int insert, unsigned long *num,
        unsigned long *eol_num, char *buf, unsigned long len)
{
    while (strsize--) {
        cread_add_char(*str, insert, num, eol_num, buf, len);
        str++;
    }
}

static int cread_line(const char *const prompt, char *buf, unsigned int *len)
{
    unsigned long num = 0;
    unsigned long eol_num = 0;
    unsigned long wlen;
    char ichar;
    int insert = 1;
    int esc_len = 0;
    char esc_save[8];
    int init_len = strlen(buf);

    if (init_len)
        cread_add_str(buf, init_len, 1, &num, &eol_num, buf, *len);

    while (1) {
#ifdef CONFIG_BOOT_RETRY_TIME
        while (!tstc()) {	/* while no incoming data */
            if (retry_time >= 0 && get_ticks() > endtime)
                return (-2);	/* timed out */
            WATCHDOG_RESET();
        }
#endif

        ichar = getcmd_getch();

        if ((ichar == '\n') || (ichar == '\r')) {
            putc('\n');
            break;
        }

        /*
        * handle standard linux xterm esc sequences for arrow key, etc.
        */
        if (esc_len != 0) {
            if (esc_len == 1) {
                if (ichar == '[') {
                    esc_save[esc_len] = ichar;
                    esc_len = 2;
                } else {
                    cread_add_str(esc_save, esc_len, insert,
                            &num, &eol_num, buf, *len);
                    esc_len = 0;
                }
                continue;
            }

            switch (ichar) {

            case 'D':	/* <- key */
                ichar = CTL_CH('b');
                esc_len = 0;
                break;
            case 'C':	/* -> key */
                ichar = CTL_CH('f');
                esc_len = 0;
                break;	/* pass off to ^F handler */
            case 'H':	/* Home key */
                ichar = CTL_CH('a');
                esc_len = 0;
                break;	/* pass off to ^A handler */
            case 'A':	/* up arrow */
                ichar = CTL_CH('p');
                esc_len = 0;
                break;	/* pass off to ^P handler */
            case 'B':	/* down arrow */
                ichar = CTL_CH('n');
                esc_len = 0;
                break;	/* pass off to ^N handler */
            default:
                esc_save[esc_len++] = ichar;
                cread_add_str(esc_save, esc_len, insert,
                            &num, &eol_num, buf, *len);
                esc_len = 0;
                continue;
            }
        }

        switch (ichar) {
        case 0x1b:
            if (esc_len == 0) {
                esc_save[esc_len] = ichar;
                esc_len = 1;
            } else {
                puts("impossible condition #876\n");
                esc_len = 0;
            }
            break;

        case CTL_CH('a'):
            BEGINNING_OF_LINE();
            break;
        case CTL_CH('c'):	/* ^C - break */
            *buf = '\0';	/* discard input */
            return (-1);
        case CTL_CH('f'):
            if (num < eol_num) {
                getcmd_putch(buf[num]);
                num++;
            }
            break;
        case CTL_CH('b'):
            if (num) {
                getcmd_putch(CTL_BACKSPACE);
                num--;
            }
            break;
        case CTL_CH('d'):
            if (num < eol_num) {
                wlen = eol_num - num - 1;
                if (wlen) {
                    memmove(&buf[num], &buf[num+1], wlen);
                    putnstr(buf + num, wlen);
                }

                getcmd_putch(' ');
                do {
                    getcmd_putch(CTL_BACKSPACE);
                } while (wlen--);
                eol_num--;
            }
            break;
        case CTL_CH('k'):
            ERASE_TO_EOL();
            break;
        case CTL_CH('e'):
            REFRESH_TO_EOL();
            break;
        case CTL_CH('o'):
            insert = !insert;
            break;
        case CTL_CH('x'):
        case CTL_CH('u'):
            BEGINNING_OF_LINE();
            ERASE_TO_EOL();
            break;
        case DEL:
        case DEL7:
        case 8:
            if (num) {
                wlen = eol_num - num;
                num--;
                memmove(&buf[num], &buf[num+1], wlen);
                getcmd_putch(CTL_BACKSPACE);
                putnstr(buf + num, wlen);
                getcmd_putch(' ');
                do {
                    getcmd_putch(CTL_BACKSPACE);
                } while (wlen--);
                eol_num--;
            }
            break;
        case CTL_CH('p'):
        case CTL_CH('n'):
        {
            char * hline;

            esc_len = 0;

            if (ichar == CTL_CH('p'))
                hline = hist_prev();
            else
                hline = hist_next();

            if (!hline) {
                getcmd_cbeep();
                continue;
            }

            /* nuke the current line */
            /* first, go home */
            BEGINNING_OF_LINE();

            /* erase to end of line */
            ERASE_TO_EOL();

            /* copy new line into place and display */
            strcpy(buf, hline);
            eol_num = strlen(buf);
            REFRESH_TO_EOL();
            continue;
        }
#ifdef CONFIG_AUTO_COMPLETE
        case '\t': {
            int num2, col;

            /* do not autocomplete when in the middle */
            if (num < eol_num) {
                getcmd_cbeep();
                break;
            }

            buf[num] = '\0';
            col = strlen(prompt) + eol_num;
            num2 = num;
            if (cmd_auto_complete(prompt, buf, &num2, &col)) {
                col = num2 - num;
                num += col;
                eol_num += col;
            }
            break;
        }
#endif
        default:
            cread_add_char(ichar, insert, &num, &eol_num, buf, *len);
            break;
        }
    }
    *len = eol_num;
    buf[eol_num] = '\0';	/* lose the newline */

    if (buf[0] && buf[0] != CREAD_HIST_CHAR)
        cread_add_to_hist(buf);
    hist_cur = hist_add_idx;

    return 0;
}

int readline_into_buffer (const char *const prompt, char * buffer)
{
    char *p = buffer;
#ifdef CONFIG_CMDLINE_EDITING
    unsigned int len = CONFIG_SYS_CBSIZE;
    int rc;
    static int initted = 0;

    /*
    * History uses a global array which is not
    * writable until after relocation to RAM.
    * Revert to non-history version if still
    * running from flash.
    */
    if (gd->flags & GD_FLG_RELOC) {
        if (!initted) {
            hist_init();
            initted = 1;
        }

        if (prompt)
            puts (prompt);

        rc = cread_line(prompt, p, &len);
        return rc < 0 ? rc : len;

    } else {
#endif	/* CONFIG_CMDLINE_EDITING */
    char * p_buf = p;
    int	n = 0;				/* buffer index		*/
    int	plen = 0;			/* prompt length	*/
    int	col;				/* output column cnt	*/
    char	c;

    /* print prompt */
    if (prompt) {
        plen = strlen (prompt);
        puts (prompt);
    }
    col = plen;
    for (;;) {
#ifdef CONFIG_BOOT_RETRY_TIME
        while (!tstc()) {	/* while no incoming data */
            if (retry_time >= 0 && get_ticks() > endtime)
                return (-2);	/* timed out */
            WATCHDOG_RESET();
        }
#endif
        WATCHDOG_RESET();		/* Trigger watchdog, if needed */

#ifdef CONFIG_SHOW_ACTIVITY
        while (!tstc()) {
            extern void show_activity(int arg);
            show_activity(0);
            WATCHDOG_RESET();
        }
#endif
        c = getc();

        /*
        * Special character handling
        */
        switch (c) {
        case '\r':				/* Enter		*/
        case '\n':
            *p = '\0';
            puts ("\r\n");
            return (p - p_buf);

        case '\0':				/* nul			*/
            continue;

        case 0x03:				/* ^C - break		*/
            p_buf[0] = '\0';	/* discard input */
            return (-1);
        case 0x15:				/* ^U - erase line	*/
            while (col > plen) {
                puts (erase_seq);
                --col;
            }
            p = p_buf;
            n = 0;
            continue;

        case 0x17:				/* ^W - erase word	*/
            p=delete_char(p_buf, p, &col, &n, plen);
            while ((n > 0) && (*p != ' ')) {
                p=delete_char(p_buf, p, &col, &n, plen);
            }
            continue;

        case 0x08:				/* ^H  - backspace	*/
        case 0x7F:				/* DEL - backspace	*/
            p=delete_char(p_buf, p, &col, &n, plen);
            continue;

        default:
            /*
            * Must be a normal character then
            */
            if (n < CONFIG_SYS_CBSIZE-2) {
                if (c == '\t') {	/* expand TABs		*/
#ifdef CONFIG_AUTO_COMPLETE
                    /* if auto completion triggered just continue */
                    *p = '\0';
                    if (cmd_auto_complete(prompt, console_buffer, &n, &col)) {
                        p = p_buf + n;	/* reset */
                        continue;
                    }
#endif
                    puts (tab_seq+(col&07));
                    col += 8 - (col&07);
                } else {
                    ++col;		/* echo input		*/
                    putc (c);
                }
				*p++ = c;
                ++n;
            } else {			/* Buffer full		*/
                putc ('\a');
            }
        }
    }
#ifdef CONFIG_CMDLINE_EDITING
    }
#endif
}

/*
 * Prompt for input and read a line.
 * If  CONFIG_BOOT_RETRY_TIME is defined and retry_time >= 0,
 * time out when time goes past endtime (timebase time in ticks).
 * Return:	number of read characters
 *		-1 if break
 *		-2 if timed out
 */
int readline (const char *const prompt)
{
    /*
    * If console_buffer isn't 0-length the user will be prompted to modify
    * it instead of entering it from scratch as desired.
    */
    console_buffer[0] = '\0';

    return readline_into_buffer(prompt, console_buffer);
}
/****************************************************************************/
static char * delete_char (char *buffer, char *p, int *colp, int *np, int plen)
{
    char *s;

    if (*np == 0) {
        return (p);
    }

    if (*(--p) == '\t') {			/* will retype the whole line	*/
        while (*colp > plen) {
            puts (erase_seq);
            (*colp)--;
        }
        for (s=buffer; s<p; ++s) {
            if (*s == '\t') {
                puts (tab_seq+((*colp) & 07));
                *colp += 8 - ((*colp) & 07);
            } else {
                ++(*colp);
                putc (*s);
            }
        }
    } else {
        puts (erase_seq);
        (*colp)--;
    }
    (*np)--;
    return (p);
}
/****************************************************************************/
int parse_line (char *line, char *argv[])
{
    int nargs = 0;

#ifdef DEBUG_PARSER
    printf ("parse_line: \"%s\"\n", line);
#endif
    while (nargs < CONFIG_SYS_MAXARGS) {

        /* skip any white space */
        while ((*line == ' ') || (*line == '\t')) {
            ++line;
        }

        if (*line == '\0') {	/* end of line, no more args	*/
            argv[nargs] = NULL;
#ifdef DEBUG_PARSER
        printf ("parse_line: nargs=%d\n", nargs);
#endif
            return (nargs);
        }

        argv[nargs++] = line;	/* begin of argument string	*/

        /* find end of string */
        while (*line && (*line != ' ') && (*line != '\t')) {
            ++line;
        }

        if (*line == '\0') {	/* end of line, no more args	*/
            argv[nargs] = NULL;
#ifdef DEBUG_PARSER
        printf ("parse_line: nargs=%d\n", nargs);
#endif
            return (nargs);
        }

        *line++ = '\0';		/* terminate current arg	 */
    }

    printf ("** Too many args (max. %d) **\n", CONFIG_SYS_MAXARGS);

#ifdef DEBUG_PARSER
    printf ("parse_line: nargs=%d\n", nargs);
#endif
    return (nargs);
}

/****************************************************************************/
