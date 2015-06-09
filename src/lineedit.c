/*--------------------------------------------------------------------
  lineedit module.

    Copyright (c) 1998,1999,2000 SASAKI Shunsuke.
    All rights reserved. 
--------------------------------------------------------------------*/
#include "ed.h"
#include "sh.h"
#include <ctype.h>


/* 新型lineedit処理系 */
int 	le_getcsx(le_t *lep)
{
	int 	csx,i;

	csx=0;
	for (i=0; i< lep->lx;)
		{
		 if (lep->buf[i]=='\0')
		 	break;
		 csx+= kanji_countdsp(lep->buf[i], csx);
		 i=kanji_posnext(i, lep->buf);
		}
	csx+= lep->lx-i;
	return csx;
}

#define	N_scr	8

void	le_setlx(le_t *lep,int lx)
{
	int 	csx;

	lep->lx=kanji_poscanon(lx, lep->buf);

	csx=le_getcsx(lep);
	if (csx==lep->cx+ lep->sx)
		return;

	if (csx<=lep->sx)		// スクロール位置より左に
		lep->sx=(csx-1)/N_scr*N_scr; else
		{
		 if (csx>=lep->sx+lep->dsize-1)	// スクロール位置より右に
		 	lep->sx=((csx- lep->dsize)/N_scr+1)*N_scr;
		}
	lep->cx=csx- lep->sx;
//term_locate(2,2);term_printf("%d %d:%d",csx,lep->sx,lep->cx);term_clrtoe();
}

void	le_csrleftside(le_t *lep)
{
	le_setlx(lep,0);
}

void	le_csrrightside(le_t *lep)
{
	le_setlx(lep,strlen(lep->buf));
}

void	le_csrleft(le_t *lep)
{
	if (lep->lx>0)
		le_setlx(lep,lep->lx-1);
}

void	le_csrright(le_t *lep)
{
	if (lep->lx<strlen(lep->buf))
		le_setlx(lep, lep->lx+1+ (IsThisKanjiPosition(lep->lx,lep->buf)?1:0));
}



void	le_edit(le_t *lep,int ch,int cm)
{
	int 	i;
	int 	strlength;
	void	*p;

	strlength=strlen(lep->buf);
	if (!sysinfo.freecursorf)
		{
		 if (lep->lx >strlength)
		 	lep->lx=strlength;
		} else
		{
		 if (strlength< lep->lx)
		 	{
		 	 for (i=strlength;i< lep->lx;++i)
		 	 	lep->buf[i] = ' ';
		 	 lep->buf[i] = '\0';
		 	 strlength = i;
		 	}
		}

	switch(cm)
		{
	 case BACKSPACE:
	 	 if (lep->lx <=0)
	 	 	break;
	 	 le_csrleft(lep);
	 case DELETE:
	 	 if (lep->lx < strlength)
	 	 	{
			 p=lep->buf+lep->lx;
	 	 	 if (IsThisKanjiPosition(lep->lx,lep->buf))
	 	 	 	memmove(p,p+2, strlength- lep->lx-1); else
	 	 	 	memmove(p,p+1, strlength- lep->lx);
	 	 	}
	 	 break;
	 default:
		 if (strlength< lep->size)
		 	{
			 p=lep->buf+lep->lx;
	 	 	 if (ch>=0x8e00)
	 	 	 	{
	 	 	 	 memmove(p+1,p, strlength- lep->lx+1);
	 	 	 	 *(char *)p =ch>>8;
	 	 	 	 ++p;
	 	 	 	}
	 	 	 memmove(p+1,p, strlength- lep->lx+1);
	 	 	 *(char *)p =ch&0xff;

	 	 	 le_csrright(lep);
		 	}
		}
}

size_t	le_regbuf(const char *s,char *t)
{
	int 	n,a;

	for (n=0;n<MAXEDITLINE;++s)
		{
		 if (*s=='\0')
		 	break;
		 if (*s=='\t')
		 	{
		 	 char	c;

		 	 c=sysinfo.tabcode;
		 	 t[n]=sysinfo.tabmarkf&& c!=-1?c:' ';
		 	 a= (n/sysinfo.tabstop+1)*sysinfo.tabstop;
		 	 for (;n<a;++n)
		 	 	t[n+1]=' ';
		 	 continue;
		 	}
		 if (!isprint(*s&0xff)&&!iskanji(*s))
		 	{
		 	 if (*s<0x20)
		 	 	{
		 	 	 t[n++]= '^';
		 	 	 t[n++]= *s+'@';
		 	 	} else
		 	 	{
		 	 	 sprintf(t+n, "%02x", *s);
		 	 	 n+=2;
		 	 	}
		 	 continue;
		 	}
		 t[n++]= *s;
		}
	t[n]='\0';
	return n;
}


	/* legets 処理系 */

static	void	legets_hist(le_t *lep,int hn,int hy)
{
	int 	tmpListNo;
	EditLine	*ed;

	tmpListNo = CurrentFileNo;
	CurrentFileNo = hn;

	ed=GetList(hy+1);
	lep->lx=0;
	lep->sx=0;

	if (ed==NULL||ed->buffer==NULL)			//buffer
		*lep->buf='\0'; else
		{
		 strcpy(lep->buf, ed->buffer);		//buffer
		 le_csrrightside(lep);
		}

	CurrentFileNo=tmpListNo;

}

static	bool	legets_histprev(le_t *lep,int hn,int *hy)
{
	if (hn==-1 || *hy<=0)
		return FALSE;

	--*hy;
	legets_hist(lep,hn,*hy);
	return TRUE;
}

static	bool	legets_histnext(le_t *lep,int hn,int *hy)
{
	int 	tmpListNo;
	int 	hym;

	tmpListNo = CurrentFileNo;
	CurrentFileNo = hn;
	hym=GetLastNumber();
	CurrentFileNo=tmpListNo;

	if (hn==-1)
		return FALSE;

	++*hy;
	if (*hy>=hym)
		{
		 *hy=hym;
		 *lep->buf='\0';
		 return TRUE;
		}

	legets_hist(lep,hn,*hy);
	return TRUE;
}

le_t	*legets_lep;	// !!暫定的

static	dspfmt_t	*dspreg_legets(void *vp,int a,int sizex,int sizey)
{
	dspfmt_t	*dfp,*dfpb;
	char	buf[MAXEDITLINE+1];
	int 	n;


	dfpb=dfp=dsp_fmtinit((char *)vp, NULL);
	dfp->col=sysinfo.c_sysmsg;

	n=le_regbuf(legets_lep->buf, buf);
//	strcpy(buf+legets_lep->sx+ min(n,legets_lep->dsize-1), " <");
//	term_printf("%-.*s", min(n,lep->dsize), buf+lep->sx);
	dfp=dsp_fmtinit(buf+legets_lep->sx, dfp);

	return dfpb;
}

int 	legets_gets(const char *msg,char *s,int dsize,int size,int hn)
{
	dspreg_t	*drp;
	le_t	le;
	uint 	ch;
	int 	hy;
	int 	ret;


legets_lep=&le;

	OnMessage_Flag=TRUE;

	drp=dsp_reginit();
	drp->y    =dspall.sizey-1;
	drp->func =dspreg_legets;
	drp->sizex=dsize;
	drp->sizey=1;
	drp->vp   =(void *)msg;

	dsp_regadd(drp);


	if (hn!=-1)
		{
		 int 	tmpListNo;
		 
		 tmpListNo = CurrentFileNo;
		 CurrentFileNo = hn;
		 hy=GetLastNumber();
		 CurrentFileNo=tmpListNo;
		}

	le.dx=strlen(msg);

	le.dsize=dsize-strlen(msg)-2;
	le.size=size;

	strcpy(le.buf,s);
	le.sx=0;
	le.cx=0;
	le.lx=0;
	le.l_sx=0;
	le_setlx(&le,strlen(s));

	for (;;)
		{
		 dsp_allview();
		 term_locate(drp->y, le.dx+le.cx);

		 term_csrn();
		 ch=get_keyf(1);
		 switch(ch)
		 	{
		  case -1:
		  	 continue;
		  case KF_SysCursorleft:
		  	 le_csrleft(&le);
		  	 continue;
		  case KF_SysCursorright:
		  	 le_csrright(&le);
		  	 continue;
		  case KF_SysCursorleftside:
		  	 le_csrleftside(&le);
		  	 continue;
		  case KF_SysCursorrightside:
		  	 le_csrrightside(&le);
		  	 continue;
		  case KF_SysCursorup:
		  	 legets_histprev(&le,hn,&hy);
		  	 continue;
		  case KF_SysCursordown:
		  	 legets_histnext(&le,hn,&hy);
		  	 continue;

		  case KF_SysReturn:
		  	 strcpy(s,le.buf);
		  	 if (hn!=-1&& *s!='\0')
		  	 	{
		  	 	 int 	tmpListNo;
		  	 	 EditLine	*ed;

		  	 	 tmpListNo = CurrentFileNo;
		  	 	 CurrentFileNo = hn;
		  	 	 ed=MakeLine(s);
		  	 	 AppendLast(ed);
		  	 	 CurrentFileNo=tmpListNo;
		  	 	}
		  	 ret= TRUE;
		  	 break;
		  case KF_SysEscape:
		  	 ret= ESCAPE;
		  	 break;

		  case KF_SysBackspace:
		  	 le_edit(&le, ' ', BACKSPACE);
		  	 continue;
		  case KF_SysDeletechar:
		  	 le_edit(&le, ' ', DELETE);
		  	 continue;
		  case KF_SysCntrlInput:
		  	 putDoubleKey(CNTRL('P'));
		  	 system_guide(); // !!
		  	 term_locate(drp->y, le.dx+le.cx);
		  	 ch=term_inkey();
		  	 delDoubleKey();

		  	 le_edit(&le, ch, NONE);
		  	 continue;
		  default:
		  	 if (ch&KF_normalcode);
		  	 	{
		  	 	 ch&= ~KF_normalcode;
		  	 	 if (iseuc(ch) || ch==0x8e)
		  	 	 	le_edit(&le, ch<<8|term_inkey(), NONE); else
		  	 	 	le_edit(&le, ch, NONE);
		  	 	}
		  	 continue;
		  	}
		 break;
		}

	dsp_regfin(drp);
	RefreshMessage();
	return ret;
}

