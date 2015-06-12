/*--------------------------------------------------------------------
  string search and replace module.

    Copyright (c) 1998,1999,2000 SASAKI Shunsuke.
    All rights reserved. 
--------------------------------------------------------------------*/
#include	"ed.h"
#include	"../lib/regexp.h"
#include	<ctype.h>



static	void	cursor_move(int lx,long ly)	// !!
{
	RefreshMessage();

	csr_setly(ly);
	csr_setlx(lx);
}


// multi search/replace用バッファ
	char	s_search[MAXEDITLINE+1];	/* search strings */
	char	s_repl[MAXEDITLINE+1];		/* replace strings */

//
	char	s_get[MAXEDITLINE+1];	/* 文字列取得バッファ */

void	search_init()
{
	*s_search='\0';
	*s_repl='\0';

	*s_get='\0';
}

SHELL	void	op_search_getword()
{
	int 	lx;

	if (*s_get=='\0')
		lx=csrle.lx; else
		lx=csrle.lx+strlen(s_get);

	lx=kanji_tknext(csrle.buf,lx,FALSE)-csrle.lx;
	if (lx==0)
		return;

	strncpy(s_get, csrle.buf+csrle.lx, lx);
	s_get[lx]='\0';

	system_msg(s_get);

	edbuf[CurrentFileNo].pm=TRUE;
	strcpy(s_search, s_get);
}

void	search_clrword()
{
	*s_get='\0';
}


static	long	search_next(const char *s,int x,long y,regm_t *rmp)
{
	EditLine	*ed;
	long	n;

	n=GetLastNumber();
	ed=GetList(y);
	if (ed!=NULL && ed->buffer!=NULL && x>=strlen(ed->buffer))
		{
		 x=0;
		 ++y;
		 ed=ed->next;
		}

	for (;y<=n;++y)
		{
		 if (ed==NULL || ed->buffer==NULL)
		 	break;
		 if (regexp_seeknext(ed->buffer, s, x ,rmp, sysinfo.nocasef))
		 	return y;
		 ed=ed->next;
		 x=0;
		}
	return -1;
}

static	long	search_prev(const char *s,int x,long y,regm_t *rmp)
{
	EditLine	*ed;

	if (x<0)
		--y;

	ed=GetList(y);
	if (ed==NULL || ed->buffer==NULL)
		return -1;
	if (x<0)
		x=strlen(ed->buffer);

	for (;y>0;--y)
		{
		 if (regexp_seekprev(ed->buffer, s, x, rmp, sysinfo.nocasef))
		 	return y;
		 ed=ed->prev;
		 if (ed==NULL||ed->buffer==NULL)			//buffer
		 	break;
		 x=strlen(ed->buffer);						//buffer
		}
	return -1;
}

SHELL	void	op_search_in()
{
	char	buf[MAXEDITLINE+1];

//	crt_ledraw();
	*buf='\0';
	if (HisGets(buf,GETS_FIND_MSG,SEARCHS_SYSTEM)==NULL 
			|| *buf=='\0')
		{
		 edbuf[CurrentFileNo].pm=FALSE;
		 *s_search='\0';
		}else
		{
		 lm_mark(GetLineOffset(), 0);
		 edbuf[CurrentFileNo].pm=TRUE;
		 strcpy(s_search,buf);
		}

//	system_guide();
}

SHELL	void	op_search_prev()
{
	regm_t	rm;
	long	y;

	if (*s_search=='\0')
		{
		 op_search_in();
		 if (*s_search=='\0')
		 	return;
		}

	csr_leupdate();
	system_msg(SEARCHING_MSG);

	y=search_prev(s_search, 
					GetBufferOffset()-1, GetLineOffset(), &rm);

//	crt_ledraw();
	if (y==-1)
		system_msg(NOT_FOUND_MSG); else
		cursor_move(rm.rm_so,y);
}

SHELL	void	op_search_next()
{
	regm_t	rm;
	long	y;

	if (*s_search=='\0')
		{
		 op_search_in();
		 if (*s_search=='\0')
		 	return;
		}

	csr_leupdate();
	system_msg(SEARCHING_MSG);
	y=search_next(s_search, 
					GetBufferOffset()+1, GetLineOffset(), &rm);
//	crt_ledraw();

	if (y==-1)
		system_msg(NOT_FOUND_MSG); else
		cursor_move(rm.rm_so,y);
}

static	void	replace()
{
	regm_t	rm;
	long	y,my;
	int 	x,mx;
	int 	chgNo;
	char	buf[MAXLINESTR+1];
	bool	rf;

	int 	cx,sx,lx,cy;
	long	ly;
	EditLine	*ed;

	csr_leupdate();

	cx=csrle.cx;
	sx=csrle.sx;
	lx=csrle.lx;
	cy=csrse.cy;
	ly=csrse.ly;
	ed=csrse.ed;

	chgNo=0;
	mx=0;
	my=0;
	switch(edbuf[CurrentFileNo].replm)
		{
	 case REPLM_before:
	 	 mx=lx;
	 	 my=ly;
	 case REPLM_all:
	 default:
	 	 x=0;
	 	 y=1;
	 	 break;
	 case REPLM_after:
	 	 x=lx;
	 	 y=ly;
	 	 break;
/*	 case REPLM_block:
		 x=1;
		 y=GetBlockStart();
		 my=GetBlockEnd();*/
		}

	rf=FALSE;
	for (;;)
		{
		 system_msg(SEARCHING_MSG);
		 y=search_next(s_search, x, y, &rm);

		 if (y==-1)
		 	break;

		 x=rm.rm_so;
		 if (my!=0&& (y>my|| (y==my&& mx!=0&& x>mx)))
		 	break;

		 cursor_move(x,y);

		 if (!rf)
		 	{
		 	 char	c;
		 	 char	buf[LN_dspbuf+1];

		 	 CrtDrawAll();
		 	 term_locate(GetRow(), GetCol()+NumWidth);

		 	 term_color(sysinfo.c_search);
		 	 memcpy(buf, GetList(y)->buffer+rm.rm_so, rm.rm_eo-rm.rm_so);
		 	 buf[rm.rm_eo-rm.rm_so]='\0';
		 	 term_puts(buf);

		 	 c=keysel("置換しますか? (Y/N/A) :","Yy\r\nNnAa \x1b");
		 	 if (c=='\x1b')
		 	 	break;
		 	 if (c=='n'||c==' ')
		 	 	{
		 	 	 ++x;
		 	 	 continue;
		 	 	}
		 	 if (c=='a')
		 	 	rf=TRUE;
		 	}

		 csr_lenew();

		 se_delete(rm.rm_eo-x, TRUE);
		 se_insert(s_repl,FALSE);
		 x+=strlen(s_repl);

		 csr_leupdate();

		 ++chgNo;
		}
// 変換を開始した時のカーソル位置に戻す。
	csrle.cx=cx;
	csrle.sx=sx;
	csrle.lx=lx;
	csrse.cy=cy;
	csrse.ly=ly;
	csrse.ed=ed;
	csr_fix();
	csr_lenew();

	sprintf(buf, "[ %.30s -> %-.30s ] %d %s..."
		, s_search, s_repl, chgNo, REPLACED_MSG);
	system_msg(buf);

}




SHELL	void	op_search_paging()
{
	edbuf[CurrentFileNo].pm= edbuf[CurrentFileNo].pm?FALSE:TRUE;
//	system_guide();
}

SHELL	void	op_search_repl_redo()
{
	if (*s_search=='\0'||*s_repl=='\0')
		return;
	edbuf[CurrentFileNo].replm=REPLM_all;
	replace();
}

SHELL	void	op_search_repl()
{
	int 	res;
	int 	nums;

	csr_leupdate();
//	crt_ledraw();

//	nums=blck_ismode()?4:3;
	nums=3;
	res=menu_vselect(GetCol(), GetRow(), nums, REPLACE_ALL_MSG
		, REPLACE_FORWARD_MSG, REPLACE_BACK_MSG, REPLACE_INSIDEBLOCK_MSG);

	switch(res) {
	case -1:
		return;
	case 0:
		edbuf[CurrentFileNo].replm=REPLM_all;
		break;
	case 1:
		edbuf[CurrentFileNo].replm=REPLM_after;
		break;
	case 2:
		edbuf[CurrentFileNo].replm=REPLM_before;
		break;
	case 3:
		edbuf[CurrentFileNo].replm=REPLM_block;
		break;
	}

	CrtDrawAll();
	*s_search='\0';
	if (HisGets(s_search, GETS_FIND_MSG, SEARCHS_SYSTEM) == NULL) 
		{
//		 crt_ledraw();
		 return;
		}
	*s_repl='\0';
	if (HisGets(s_repl, GETS_REPLACE_MSG, SEARCHS_SYSTEM) == NULL) 
		{
//		 crt_ledraw();
		 return;
		}

	replace();
//	crt_ledraw();
}



/*=============================================================================
	search appropriate brace.

	assuming that brace is a pair of same-width character.
	i.e. narrow brace always matches to narrow opponent, not wide one.
*/
/*-----------------------------------------------------------------------------
	find appropriate pair of braces according to *pchBrace.
	returns each brace and length.
*/
static	size_t	find_brace_opponent(const char* pchCursor, const char** ppchBrace, const char** ppchOpponent)
{
	static	const char*	vpszBraces[ ] = {
		/* 0 byte  */	NULL,
		/* 1 byte  */	"()<>[]{}",
		/* 2 bytes */	"(**)/**/�ơǡȡɡʡˡ̡͡ΡϡСѡҡӡԡա֡סء١ڡ�",
	} ;

	size_t	cbBrace ;


	for (cbBrace = sizeof vpszBraces / sizeof *vpszBraces ; 0 < cbBrace ; --cbBrace) {
		int			ich ;
		const char*	pszBraces ;

		pszBraces = vpszBraces[ cbBrace ] ;
		for (ich = 0 ; pszBraces[ ich ] != '\0' ; ich = kanji_posnext(ich, pszBraces)) {
			if (strncmp(pszBraces + ich, pchCursor, cbBrace) == 0) {
				/*	braces are assumed to be always a pair.
					so that a pair occupies (cbBraces * 2) bytes.
					when we found the *pchBrace on (cbBraces * 2) boundary,
					pchBrace point to open brace, its opponent is a close brace.
					otherwise, the opponent is an open brace.
				*/
				*ppchBrace = &pszBraces[ ich ] ;
				if (ich % (cbBrace * 2) == 0)
					*ppchOpponent = *ppchBrace + cbBrace ;
				else
					*ppchOpponent = *ppchBrace - cbBrace ;

				return cbBrace ;
				/* NOTREACHED */
			}
		}
	}

	/* no appropriate opponent has been found. */
	return cbBrace ;
}


/*-----------------------------------------------------------------------------
	search appropriate brace or letter for top of file.
*/
SHELL	void	op_search_brace_prev()
{
	size_t		cbBrace ;	/* 1 for narrow, 2 for wide, ...	*/
	const char*	pszBrace ;	/* a brace */
	const char*	pszPair  ;	/* and its pair */

	int			ich ;

	EditLine*	pLine ;	/* line buffer in iLine-th Line */
	long		iLine ;	/* line number */
	int			iTail ;	/* where search ends in pLine->buffer */
	int			cNest ;	/* nesting levels of braces */

	bool	b_found ;


	/*	find the appropriate pair of brace under the cursor.
		return if nothing seems to be appropriate.
	*/
	cbBrace = find_brace_opponent(csrle.buf + csrle.lx, &pszBrace, &pszPair) ;
	if (cbBrace == 0)
		return ;
		/* NOTREACHED */

	/*	now we try to find anything in the search buffer(i.e. pair of braces) */
	csr_leupdate();
	system_msg(SEARCHING_MSG);

	cNest = 0 ;
	b_found = FALSE ;
	iLine = GetLineOffset( ) ;
	pLine = GetList( iLine ) ;
	iTail = GetBufferOffset( ) + cbBrace ;

	do {
		/*	scoreboard.

			checking an characters one by one from tail to head is an
			expensive job in MBCS environment.
			i.e. an character would be composed of 2 or more bytes.
			here we 'scoreboard' each character from head to tail and
			see if we got match by utilizing the scoreboard.

			this scoreboard holds
				1  for character same as pszBrace
				-1 for character same as pszPair
				0  for otherwise
			if 1 < cbBrace case(i.e. we're searching for MBCS brace pair),
			-1 and 1 are held for its first byte only.

			Although we could use calloc here safely,
			original author tend to use mem_alloc instead of malloc and etc.
			we'll belong to the author.
			Or we can define the scoreboard as an array.
			(MAXLINESTR is defined as 2048,
			 which seems to be a little huge for me to be on a stack.)

			iTail + 1 avoids mem_alloc(0) when the line is empty.
		*/
		char*	pch_sb = mem_alloc( iTail + 1 ) ;
		memset(pch_sb, 0, iTail + 1) ;

		/*	see if braces we're searching for are in the pLine->buffer.
			if we see any of them, remember where and what it is.
		*/
		for (ich = 0 ; ich < iTail ; ich = kanji_posnext(ich, pLine->buffer))
			if (memcmp(pLine->buffer + ich, pszBrace, cbBrace) == 0)
				pch_sb[ ich ] = 1 ;
			else if (memcmp(pLine->buffer + ich, pszPair, cbBrace) == 0)
				pch_sb[ ich ] = -1 ;

		/*	now we have scoreboard in pchsb.
			see if we'll got to nest level 0 in backward.
		*/
		for (ich = iTail - 1 ; 0 <= ich ; --ich) {
			cNest += pch_sb[ ich ] ;
			if (pch_sb[ ich ] == -1 && cNest == 0)
				break ;
				/* NOTREACHED */
		}

		/* free scoreboard */
		free( pch_sb ) ;

		if (0 <= ich) {
			cursor_move(ich, iLine) ;
			return ;
			/* NOTERACHED */
		}

		--iLine ;
		pLine = pLine->prev ;
		if ( pLine->buffer )
			iTail = strlen( pLine->buffer ) ;
	} while (0 < iLine && pLine != NULL && pLine->buffer != NULL) ;

	system_msg(NOT_FOUND_MSG);
}



/*-----------------------------------------------------------------------------
	search appropriate brace or letter for end of file.
*/
SHELL	void	op_search_brace_next()
{
	size_t		cbBrace ;
	const char*	pszBrace ;
	const char*	pszPair ;

	int		ich ;

	EditLine*	pLine ;	/* line buffer in iLine-th Line */
	long		iLine ;	/* line number */
	int			iHead ;	/* where search begins in pLine->buffer */
	int			cNest ;	/* nesting levels of braces */


	/*	find the appropriate pair of brace under the cursor.
		return if nothing seems to be appropriate.
	*/
	cbBrace = find_brace_opponent(csrle.buf + csrle.lx, &pszBrace, &pszPair) ;
	if (cbBrace == 0)
		return ;
		/* NOTREACHED */

	csr_leupdate();
	system_msg(SEARCHING_MSG);

	/*	unlike to op_search_brace_prev case,
		we can see the pLine->buffer from head to tail,
		we don't need scoreboard this time.
	*/
	cNest = 0 ;
	iLine = GetLineOffset( ) ;
	pLine = GetList( iLine ) ;
	iHead = GetBufferOffset( ) ;

	do {
		for (ich = iHead ; pLine->buffer[ ich ] != '\0' ; ich = kanji_posnext(ich, pLine->buffer)) {
			if (memcmp(pLine->buffer + ich, pszBrace, cbBrace) == 0)
				++cNest ;
			else if (memcmp(pLine->buffer + ich, pszPair, cbBrace) == 0) {
				if (--cNest == 0) {
					cursor_move(ich, iLine) ;
					return ;
					/* NOTREACHED */
				}
			}
		}

		++iLine ;
		pLine = pLine->next ;
		iHead = 0 ;
	} while (pLine != NULL && pLine->buffer != NULL) ;

	system_msg(NOT_FOUND_MSG);
}
