/*--------------------------------------------------------------------
	nxeditor
			FILE NAME:line.c
			Programed by : I.Neva
			R & D  ADVANCED SYSTEMS. IMAGING PRODUCTS.
			1992.06.01

    Copyright (c) 1998,1999,2000 SASAKI Shunsuke.
    All rights reserved. 
--------------------------------------------------------------------*/
#include "ed.h"
#include "sh.h"
#include <ctype.h>


void	se_insert(const char *s,bool f)
{
	int 	lx;
	u_char	c;

	lx=csrle.lx;
	while(*s!='\0')
		{
		 c=*s++;
		 if (c<0xa0 || *(u_char *)s<0xa0)
		 	LeditInput(c, NONE); else
		 	{
		 	 LeditInput(c<<8|*(u_char *)s, NONE); 
		 	 ++s;
		 	}
		}
	if (f)
		csr_setlx(lx);
}

void	se_delete(int n, bool f)
{
	for (;n>0;--n)
		{
		 if (f&& IsKanjiPosition())
		 	--n;
		 Ledit(DELETE);
		}
}

static	void	se_nazo()
{
// FREE�⡼�ɤλ��˰�ö����������ΰ��֤ޤǥ��ڡ��������롣

//	LeditInput(CNTRL(' '), NONE);
	Ledit(NONE);
	Ledit(BACKSPACE);
}

static	void	line_catnext()
{
	if (GetLineOffset() <GetLastNumber())
		{
		 se_nazo();
		 csr_leupdate();

		 CatLine();
		 DeleteAndDraw();

		 undo_add(FALSE,"\n");
		}
}

static	void	line_catprev()
{
	if (GetLineOffset()>1)
		{
		 op_cursor_left();
		 CatLine();
//		 DeleteAndDraw();

		 undo_add(TRUE,"\n");
		}
}


SHELL	void	op_del_tknext()
{
	int 	lx;
	char	buf[MAXEDITLINE+1];

	if (GetBufferOffset()>=strlen(csrle.buf))
		{
		 line_catnext();
		 return;
		}

	lx=kanji_tknext(csrle.buf,csrle.lx,FALSE) -csrle.lx;

	strncpy(buf, csrle.buf+ csrle.lx, lx);
	buf[lx]='\0';

	se_delete(lx,TRUE);
	undo_add(FALSE,buf);
}

SHELL	void	op_del_tkprev()
{
	int 	lx,lx_a;
	
	char	buf[MAXEDITLINE+1];

	if (GetBufferOffset()==0)
		{
		 line_catprev();
		 return;
		}

	lx=kanji_tkprev(csrle.buf,csrle.lx,FALSE);
	lx_a=csrle.lx-lx;

	strncpy(buf, csrle.buf+lx, lx_a);
	buf[lx_a]='\0';

	csr_setlx(lx);
	se_delete(lx_a,TRUE);
	undo_add(TRUE,buf);
}

SHELL	void	op_del_sright()
{
	char	buf[MAXEDITLINE+1];

	if (GetBufferOffset()>=strlen(csrle.buf))
		return;
	strcpy(buf,csrle.buf+csrle.lx);
	se_delete(strlen(csrle.buf+csrle.lx),TRUE);
	undo_add(FALSE,buf);
}

SHELL	void	op_del_sleft()
{
	char	buf[MAXEDITLINE+1];
	int 	lx;

	if (GetBufferOffset()<=0)
		return;

	lx=csrle.lx;

	strncpy(buf,csrle.buf,lx);
	buf[lx]='\0';

	op_cursor_sleft();
	se_delete(lx,TRUE);
	undo_add(TRUE,buf);
}



SHELL	void	op_del_char()
{
	char	buf[2+1],*p;

	se_nazo();

	if (GetBufferOffset()>=strlen(csrle.buf)) /* �����ˤ���� */
		{
		 line_catnext();
		 return;
		}


	p=buf;
	*p++=csrle.buf[GetBufferOffset()];
	if (IsKanjiPosition()) 
		*p++=csrle.buf[GetBufferOffset()+1];
	*p='\0';

	se_delete(1,FALSE);
	undo_add(FALSE,buf);
}


SHELL	void	op_del_bs()
{
	long LineOffset;
	char	buf[2+1],*p;

	LineOffset = GetLineOffset();
	if (GetBufferOffset() == 0)
		{
		 line_catprev();
		 return;
		}

	op_cursor_left();

	p=buf;
	*p++=csrle.buf[GetBufferOffset()];
	if (IsKanjiPosition()) 
		*p++=csrle.buf[GetBufferOffset()+1];
	*p='\0';

	se_delete(1,FALSE);
	undo_add(TRUE,buf);
}

static	void	split(bool f)
{
	EditLine	*ed;
	char	buf_nl[MAXEDITLINE+1];
	int 	a,n;

	se_nazo();

	n=GetBufferOffset();

	a=0;
	if (sysinfo.autoindentf)
		{
		 for (;a<strlen(csrle.buf);++a)
		 	{
		 	 if (csrle.buf[a]!='\t'&& csrle.buf[a]!=' ')
		 	 	break;
		 	 buf_nl[a] = csrle.buf[a];
		 	}
		 if (n<a)
		 	a=0;
		}

	strcpy(buf_nl+a,csrle.buf+n);

/* split�����饤�� */
	csrle.buf[n]='\0';
	csr_leupdate();

/* �������ɲä����饤�� */
	ed=MakeLine(buf_nl);
	InsertLine(GetList(GetLineOffset()), ed);

	SetFileChangeFlag();

//	CrtDrawLine(GetRow(),GetLineOffset(), FALSE);

	if (f)
		{
		 op_cursor_down();
		 csr_setlx(a);
		}
//		 InsertAndDraw();
//		} else
//		{
//		}
}

SHELL	void	op_line_cr()
{
	split(TRUE);
}





SHELL	void	op_line_new()
{
	EditLine *ed;

	csr_leupdate();

	ed=MakeLine("");
	InsertLine(GetList(GetLineOffset()-1), ed);
	csrse.ed=ed;

	csr_lenew();
	op_cursor_sleft();
	SetFileChangeFlag();
	InsertAndDraw();
}

int 	CatLine()
{
	char tmpbuff1[MAXEDITLINE+1];
	char tmpbuff2[MAXEDITLINE+1];
	EditLine *ed,*edb;

	if (GetLineOffset() >= GetLastNumber()) 
		return FALSE;

	strcpy(tmpbuff1, csrle.buf);

	edb= GetList(GetLineOffset());
	ed = edb->next;
	strcpy(tmpbuff2, ed->buffer);	//buffer
	if (strlen(tmpbuff1)+strlen(tmpbuff2) > MAXEDITLINE)
	{
		inkey_wait(LINE_BUFFER_OVER_MSG);
		return FALSE;
	}
	DeleteList(ed);
	SetFileChangeFlag();
	strcat(tmpbuff1, tmpbuff2);

	Realloc(edb, tmpbuff1);
	csr_lenew();

	return TRUE;
}



SHELL	void	op_char_input()
{
	int 	c;

	putDoubleKey(CNTRL('P'));
	system_guide();	// !!
	system_msg(CNTRL_INPUT_MSG);

	c = term_inkey();

	delDoubleKey();
//	system_guide();

	if (c=='\0')
		return;

	if (c<0x80)
		c&=0x1f;
	InputAndCrt(c);
}

void	tagJmp()
{
	int count;
	int j;
	int length;
	char buf[MAXEDITLINE+1];
	char tagFileName[LN_path+1];
	char tagJmpNoBuff[MAXEDITLINE+1];

	strcpy(buf, csrle.buf);
	length = strlen(buf);

	count=0;
	/*get filename*/
	for (j=0; j<LN_path&& count<length; count++,j++)
		{
		 if (strchr("*?\"\'():",buf[count])!=NULL)
			break;
		 tagFileName[j] = buf[count];
		}
	tagFileName[j] = '\0';

	/*to lineno_s*/
	while(count<length&& !isdigit(buf[count]))
		++count;

	/*get lineno_s*/
	for (j=0; count < length; count++, j++)
		{
		 if (!isdigit(buf[count])) 
		 	break;
		 tagJmpNoBuff[j] = buf[count];
		}
	tagJmpNoBuff[j] = '\0';
	system_msg(WAITING_MSG);
	op_cursor_down();

/*	if (FileOpenOp(tagFileName))
		csr_setly(atol(tagJmpNoBuff));*/

	FileOpenOp(tagFileName);
	csr_setly(atol(tagJmpNoBuff));
}

SHELL	void	op_jump_tag()
{
	BlockInit();
	tagJmp();
}

SHELL	void	op_jump_line()
{
	char	buf[MAXLINESTR+1];
	int 	n;

	if (keyf_numarg()>0)
		n=atoi(keyf_getarg(0)); else
		{
		 *buf='\0';
		 if (GetS("���ֹ�:",buf)==ESCAPE)
		 	return ;
		 n=atol(buf);
		}
	lm_mark(GetLineOffset(), 0);
	csr_setly(n);

	


}

static	int 	lm_lines[5]={0,0,0,0,0};

void	lm_mark(int ln, int n)
{
	if (n>4)
		n=0;

	lm_lines[n]=ln;
}

int 	lm_line(int n)
{
	if (n>4)
		n=0;
	return lm_lines[n];
}



SHELL	void	op_jump_mark()
{
	int 	n;

	char	buf[LN_dspbuf+1];

	n= (keyf_numarg()==0)? 0: atoi(keyf_getarg(0));
	if (n>4)
		n=0;

	lm_mark(GetLineOffset(), n);
	sprintf(buf, "mark���ޤ����� #%d",n);
	system_msg(buf);
}

SHELL	void	op_jump_before()
{
	int 	n,ln;
	char	buf[LN_dspbuf+1];

	n= (keyf_numarg()==0)? 0: atoi(keyf_getarg(0));
	if (n>4)
		n=0;

	ln=lm_line(n);
	lm_mark(GetLineOffset(), 0);
	csr_setly(ln);

	sprintf(buf, "#%d",n);
	system_msg(buf);
}

SHELL	void	op_char_undo()
{
	undo_paste();
}



#define	MAX_udbuf	1024

void	udbuf_init()
{
	int		tmpFileno;
	EditLine	*ed;

	tmpFileno = CurrentFileNo;
	CurrentFileNo = UNDO_SYSTEM;
	FileStartInit(FALSE);
	ed=MakeLine("");
	AppendLast(ed);
	CurrentFileNo = tmpFileno;

}

void	udbuf_get(char *s)
{
	int		tmpFileno;
	EditLine	*ed;


	tmpFileno = CurrentFileNo;
	CurrentFileNo = UNDO_SYSTEM;

	ed = GetLast();

	strcpy(s,ed->buffer);	// buffer
	if (*s!='\0')
		DeleteList(ed);

	CurrentFileNo = tmpFileno;
}

void	udbuf_set(bool df,const char *s)
{
	int			tmpFileno;
	EditLine	*ed;
	char		buf[MAXEDITLINE+1];

	tmpFileno = CurrentFileNo;
	CurrentFileNo = UNDO_SYSTEM;

	if (GetLastNumber()>=MAX_udbuf)
		{
		 DeleteList(GetTop()->next);
		 csrse.ed = NULL;
		}

	buf[0]=df?'1':'2';
	strcpy(buf+1,s);
	ed=MakeLine(buf);
	AppendLast(ed);

	CurrentFileNo = tmpFileno;
}

void	undo_add(bool df,const char *s)
{
	udbuf_set(df,s);
}

void	undo_paste()
{
	char	buf[MAXEDITLINE+1];

	udbuf_get(buf);

	if (buf[0]!='1'&&buf[0]!='2')
		return;

	if (buf[1]=='\n')
		{
		 bool	f;

		 f= sysinfo.autoindentf;
		 sysinfo.autoindentf= FALSE;
		 split(buf[0]=='1');
		 sysinfo.autoindentf= f;

		 return;
		}

	se_insert(buf+1,buf[0]!='1');
}

SHELL	void	op_line_undo()
{
	csr_lenew();
}
