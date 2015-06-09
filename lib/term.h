/*
 *    Terminal Module.
 *
 *    based on NxEdit2.04     :: term.c
 *             FD clone 1.03g :: term.c
 *             ne 3.00pre17   :: term.c
 *
 * Copyright (c) 1999, 2000 SASAKI Shunsuke.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions, and the following disclaimer,
 *    without modification, immediately at the beginning of the file.
 * 2. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * Where this Software is combined with software released under the terms of 
 * the GNU Public License ("GPL") and the terms of the GPL would require the 
 * combined work to also be released under the terms of the GPL, the terms
 * and conditions of this License will apply in addition to those of the
 * GPL with the exception of any terms or conditions of this License that
 * conflict with, or are expressly prohibited by, the GPL.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */


#ifndef	__TERM_H_
#define	__TERM_H_


extern	void	term_init();
extern	void	term_start();
extern	void	term_stop();

extern	void	term_report();

extern	char	*term_getent(const char *s_id,const char *s_def);


extern	void	term_kflush();		/* 入力フラッシュ */

extern	int 	term_kbhit(u_long usec);
extern	int 	term_getch();

extern	int 	term_sizex();
extern	int 	term_sizey();


extern	void	term_scroll(int n);
extern	void	term_bell();
extern	void	term_csrn();
extern	void	term_csrh();
extern	void	term_csr_flush();

extern	void	term_cls();
extern	void	term_clrtoe();


extern	void	term_locate(int y,int x);
extern	void	term_movex(int x);

extern	void	term_puts(const char *s);
extern	void	term_putch(int c);
extern	void	term_printf(const char *fmt,...);


typedef	u_char	color_t;

#define	AC_normal	0

#define	AC_black	0+8
#define	AC_red		1+8
#define	AC_green	2+8
#define	AC_yellow	3+8
#define	AC_blue		4+8
#define	AC_magenta	5+8
#define	AC_cyan		6+8
#define	AC_white	7+8

#define	AC_color(cl)	((cl)&15)

#define	AC_reverse	16
#define	AC_under	32
#define	AC_bold		64

#define	AC_ignore	255


extern	void	term_color(color_t c);
extern	void	term_color_normal();

extern	void	term_color_reverse();
extern	void	term_color_underline();
extern	void	term_color_bold();

extern	color_t	term_cftocol(const char *s);


extern	void	term_color_enable();
extern	void	term_color_disable();


/* term_inkey */
extern	void	term_escset(int n, const char *e, const char *d);
extern	int 	term_inkey();
extern	int 	keysdef_getcode(const char *s, int k[], int num);
extern	void	keys_set(const char *k, const char *e, const char *d);
extern	void	term_escdefault();




#endif	/* __TERM_H_ */
