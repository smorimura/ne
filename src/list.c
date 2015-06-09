/*--------------------------------------------------------------------
	nxeditor
			FILE NAME:list.c
			Programed by : I.Neva
			R & D  ADVANCED SYSTEMS. IMAGING PRODUCTS.
			1992.06.01

    Copyright (c) 1998,1999,2000 SASAKI Shunsuke.
    All rights reserved. 
--------------------------------------------------------------------*/
#include "ed.h"


/*Base Pointer*/
static EditLine Baseline[MAX_edbuf];
static EditLine *LastLine[MAX_edbuf];
static long LastOffset[MAX_edbuf];


long	GetTopNumber()	{return 1;}
long	GetLastNumber()	{return LastOffset[CurrentFileNo];}
EditLine	*GetTop()	{return &Baseline[CurrentFileNo];}
EditLine	*GetLast()	{return LastLine[CurrentFileNo];}


void	lists_debug()
{
	EditLine	*ed;

	ed=GetTop();

	fprintf(stderr,"byte/size buffer\n");
	while(ed!=NULL)
		{
		 fprintf(stderr,"%4d/%4d %p[%s]\n"
		 	,ed->bytes,ed->size,ed->buffer,ed->buffer);
		 ed=ed->next;
		}
	fprintf(stderr,"***\n");
}


void	lists_init()
{
	int 	i;

	for (i=0;i<MAX_edbuf;++i)
		{
		 Baseline[i].prev=NULL;
		 Baseline[i].next=NULL;
		 LastLine[i]=&Baseline[i];
		 LastOffset[i] = 0;
		}
}

void	lists_clear()
{
	EditLine	*p, *q;

	p=Baseline[CurrentFileNo].next;
	while(p!=NULL)
		{
		 q=p;
		 p=p->next;

 		 q->next = NULL;
	 	 q->prev = NULL;
	 	 free(q->buffer);
	 	 free(q);
	 	}

	Baseline[CurrentFileNo].prev = NULL;
	Baseline[CurrentFileNo].next = NULL;
	LastLine[CurrentFileNo] = &Baseline[CurrentFileNo];
	LastOffset[CurrentFileNo] = 0;

	csrse.bytes=0;
}

EditLine	*MakeLine(const char *buffer)
{
	EditLine	*pli;
	int	 		n;

	pli=(EditLine *)mem_alloc(sizeof(EditLine));

	n=strlen(buffer);
//	if (n<80)
//		n=80;

	pli->buffer=(char *)mem_alloc(sizeof(char)*(n+1));
	strcpy(pli->buffer, buffer);
	pli->size=n;
	pli->bytes=n;
	return pli;
}


void	Realloc(EditLine *li,const char *s)
{
	size_t	n;

	n=strlen(s);
	if (n>li->size)
		{
		 li->buffer =(char *)mem_realloc(li->buffer, n+1);
		 li->size=n;
		}

	strcpy(li->buffer, s);
	csrse.bytes=csrse.bytes+ n- li->bytes;
	li->bytes=strlen(s);
}

void	AppendLast(EditLine *li)
{
	EditLine *last;

	last = GetLast();
	last->next = li;
	li->prev = last;
	li->next = NULL;
	++LastOffset[CurrentFileNo];
	LastLine[CurrentFileNo] = li;

	csrse.bytes+=li->bytes;
}

void	InsertLine(EditLine *bli,EditLine *li)
{
	if (bli->next == NULL)
	{
		AppendLast(li);
		return;
	}

	bli->next->prev=li;
	li->prev=bli;
	li->next=bli->next;
	bli->next=li;

	LastOffset[CurrentFileNo]++;
	csrse.bytes+=li->bytes;

//	csrse.ed=NULL;
}

void	DeleteList(EditLine *li)
{
	if (li->next != NULL)
		li->next->prev = li->prev; else
		LastLine[CurrentFileNo] = li->prev;

	csrse.bytes-=li->bytes;
	li->prev->next =li-> next;
	free(li->buffer);
	free(li);
	LastOffset[CurrentFileNo]--;
}


EditLine	*GetList(long o_number)
{
	long	num,off;
	EditLine *p;

// lists_debug();
	off= o_number - GetLineOffset();

	if (o_number <= abs(off))
		{
failed:
		 num = 0;
		 p = GetTop();

		 while (p->next!=NULL&& num++ <o_number)
		 	p = p->next;
		 return p;
		}

	if (GetLastNumber() - o_number <= abs(off))
		{
		 num=GetLastNumber();
		 p= GetLast();

		 if (p==NULL)
		 	goto failed; // あってはならない。

		 while(p->prev!=NULL&& num-- >o_number)
		 	p=p->prev;
		 return p;
		}

	num= o_number - off;
	p = csrse.ed;

	if (p==NULL)
		goto failed; // おこらないはず。

	if (off < 0)
		{
		 while (p->prev!=NULL&& off++ <0)
		 	p = p->prev;
		} else
		{
		 while (p->next!=NULL&& off-- >0)
		 	p = p->next;
		}

	return p;
}

size_t	lists_size(long n_st,long n_en)
{
	long	i;
	EditLine *ed;
	long	a; /* size */

	a=1;
	ed=GetList(n_st);
	for (i=n_st;i<=n_en;++i)
		{
		 if (ed==NULL|| ed->buffer==NULL)
		 	break;
	 	 a+= strlen(ed->buffer);
	 	 if (ed->next!=NULL)
	 	 	++a;
	 	 ed = ed->next;
		}

	return a;
}

void	lists_proc(void func(),void *gp,long n_st,long n_en)
{
	long		i;
	EditLine	*ed;
	char		buf[MAXEDITLINE+1];

	ed=GetList(n_st);
	for (i=n_st;i<=n_en;++i)
		{
		 if (ed==NULL|| ed->buffer==NULL|| i>GetLastNumber())
		 	{
		 	 func(NULL,gp);
		 	 continue;
		 	}
		 if (ed->next==NULL)
		 	strcpy(buf,ed->buffer); else
		 	sprintf(buf,"%s\n",ed->buffer);
		 func(buf,gp);
	 	 ed = ed->next;
		}
}

void	lists_add(void *func(),void *gp)
{
	int 	n;
	char	buf[MAXEDITLINE+1];
	EditLine	*ed, *edb, *ed_new;

	n=0;
	ed_new= edb= GetList(GetLineOffset()-1);
	while(gp!=NULL)
		{
//fprintf(stderr,"%d\n",n);
		 gp=func(buf,gp);
//fprintf(stderr," [%s]\n",buf);

		 ed = MakeLine(buf);
		 InsertLine(edb, ed);
		 edb=ed;
		 ++n;
		}

	csrse.ed = ed_new -> next;
}



void	lists_delete(int n_st,int n_ed)
{
	EditLine *ed, *ed_next, *ed_new;
	int 	n;

	ed_new= csrse.ed;
	n = n_ed-n_st+1;
	if (n_st <= GetLineOffset())
		{
		 int	m=n;
		 while (m-- >0 && ed_new != NULL)
		 	ed_new=ed_new->next;
		}

	ed = GetList(n_st);
	while (n-- >0)
		{
		 if (n_st > GetLastNumber()) // GetLastNumber は移動する。
		 	{
		 	 Realloc(ed, "");
		 	 return;
		 	}
		 ed_next=ed->next;
		 DeleteList(ed);
		 ed=ed_next;
		}

	csrse.ed=ed_new;
}

