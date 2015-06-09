/*--------------------------------------------------------------------
  -keyf- key input module.

    Copyright (c) 1998,1999,2000 SASAKI Shunsuke.
    All rights reserved. 
--------------------------------------------------------------------*/
#include "ed.h"
#include "sh.h"
#include	<ctype.h>

	/* KeyBoard Macro */

#define	MAX_km	4096

typedef	enum	{KM_none,KM_set,KM_do}	kmmode_t;

typedef	struct	keylist
{
	int 			*buf;
	size_t 			n;		/* size of buffer */
	struct	keylist	*next;
}	kmlist_t;

typedef	struct
{
	kmlist_t	*list;
	kmmode_t	mode;
	int 		buf[MAX_km];
	int 		n,x;

}	keymacro_t;

keymacro_t	km={NULL, KM_none};


void	km_init()
{
	km.list=NULL;
	km.mode=KM_none;
}

void	km_fin()
{
	kmlist_t	*list,*next;

	list=km.list;
	while(list!=NULL)
		{
		 next=list->next;
		 free(list);
		 list=next;
		}
	km.list=NULL;
}

SHELL	void	op_misc_kmacro()
{
	if (km.mode==KM_set)
		{
		 km.mode = KM_none;
		 return;
		}

	km.mode = KM_set;
	km.x=0;
}

static	kmlist_t	*km_seek(int n)
{
	kmlist_t	*p;

	p=km.list;
	while(n>0&& p!=NULL)
		{
		 p=p->next;
		 --n;
		}
	return p;
}

static	void	km_addkey(int c)
{
	if (km.mode!=KM_set)
		return;

	if (km.x<MAX_km)
		km.buf[km.x++]=c;
}

static	int 	km_getkey()
{
	int 	c;

	if (km.mode!=KM_do)
		return -1;

	c=km.buf[km.x++];
	if (km.x>=km.n)
		{
		 km.mode=KM_none;
		 if (km.n==0)
		 	return -1;
		}

	return c;
}


static	void	km_set(int region,keydef_t *def,int k1,int k2)
{
	kmlist_t	*list,*next;
	int 		n;

	km.mode=KM_none;

	if (def!=(void *)-1)
		{
		 if (def==NULL|| def->kdm!=KDM_macro)
		 	{
		 	 inkey_wait("キーボードマクロエラー km_set");
		 	 return;
		 	}

		 list=km_seek(def->funcNo);
		 if (list==NULL)
		 	{
		 	 inkey_wait("キーボードマクロエラー km_set NULL");
		 	 return;
		 	}
		 free(list->buf);
		 list->buf=mem_alloc(km.x*sizeof(int));
		 list->n=km.x;
		 memcpy(list->buf,km.buf,km.x*sizeof(int));
		 system_msg("KeyMacro をセットしました。");
		 return;
		}

	list=mem_alloc(sizeof(kmlist_t));
	list->buf=mem_alloc(km.x*sizeof(int));
	list->n=km.x;
	memcpy(list->buf,km.buf,km.x*sizeof(int));
	list->next=NULL;

	system_msg("KeyMacroをセットしました。");

	n=0;
	if (km.list==NULL)
		km.list=list; else
		{
		 next=km.list;
		 while(next->next!=NULL)
		 	{
		 	 ++n;
		 	 next=next->next;
		 	}
		 next->next=list;
		}
	keydef_set(region,KDM_macro,n,k1,k2);
}

static	void	km_macro(keydef_t *def)
{
	kmlist_t	*list;

	if (km.mode==KM_do)
		{
		 inkey_wait("キーボードマクロエラー KM_do");
		 km.mode=KM_none;
		 return;
		}

	list=km_seek(def->funcNo);
	if (list==NULL)
		{
		 inkey_wait("キーボードマクロエラー km_seek");
		 km.mode=KM_none;
		 return;
		}

	km.mode=KM_do;

	km.x=0;
	km.n=list->n;
	memcpy(km.buf,list->buf,list->n*sizeof(int));
}

/* keydef */


int 		keydef_n[MAX_region];
keydef_t	keydef[MAX_region][MAXKEYDEF];

void	keydef_init()
{
	int 	i;

	for (i=0;i<MAX_region;++i)
		keydef_n[i]=0;
}

int 	keydef_num(int r)
{
	if (r>=MAX_region)
		return -1;
	return keydef_n[r];
}

keydef_t	*keydef_set(int r,kdm_t kdm,int n,int k1,int k2)
{
	int 	num;
	keydef_t	*kdp;

	num=keydef_num(r);
	if (num==-1|| num>=MAXKEYDEF)
		return NULL;

	kdp= &keydef[r][num];

	kdp->key1   = k1;
	if (isalpha(k2&0xff))
		k2 &= 0x1f;
	kdp->key2   = k2;
	kdp->funcNo = n;
	kdp->kdm    = kdm;
	++keydef_n[r];

//fprintf(stderr,"keydef_set: %02d  %04x %08x\n",n,k1,k2);
	return kdp;
}

void	keydef_report()
{
	int 	i, j;

	report_puts("キー定義:\n");
	for (i=0;i<MAX_region;++i)
		{
		 report_printf("#%d\n", i);
		 for (j=0;j<keydef_n[i];++j)
		 	report_printf("%c%3d (%02d): %08x %08x\n"
		 	  , keydef[i][j].kdm==KDM_func? ' ': '*', j
		 	  , keydef[i][j].funcNo, keydef[i][j].key1, keydef[i][j].key2);
		}
}

keydef_t	*keydef_get(int r,int k1,int k2)
{
	int 		i;
	keydef_t	*p;

	p=NULL;
	if (isalpha(k2&0xff))
		k2 &= 0x1f;

	for (i=0; i<keydef_num(r); i++) 
		{
/* fprintf(stderr,"k m: %02d  %04x %08x\n", i,
	keydef[r][i].key1, keydef[r][i].key2);
*/
		 if (k1==keydef[r][i].key1)
		 	{
		 	 p=(void *)-1;
		 	 
		 	 if (k2== keydef[r][i].key2)
		 	 	return &keydef[r][i];
		 	}
		}
	return p;
}

/* keyf */

int 	get_keyf(int region)
{
	int 	key1,key2;
	keydef_t	*def;

	if (region>=MAX_region)
		return -1;

	if (km.mode==KM_set)
		system_msg("KeyMacro設定中");
	if (km.mode==KM_do)
		system_msg("KeyMacro起動中");

	key1=km_getkey();
	if (key1!=-1)
		{
		 term_csr_flush();
		 return key1;
		}

	if (region==0)
		CursorMove();
	key1=term_inkey();
	key2=-1;

	def=keydef_get(region, key1, key2);

//fprintf(stderr,"%08p: %04x %08x\n",def, key1, key2);

	if (def!=(void *)-1)
		{
		 if (def==NULL)
		 	{
		 	 if (key1>0x100 || (iscntrl(key1) && key1!='\t' && key1<0x80))
		 	 	def=(void *)-1; else
		 	 	key1|=KF_normalcode;
		 	}
		} else
		{
		 putDoubleKey(key1);
		 if (region==0)
		 	CursorMove();
		 key2=term_inkey();
		 delDoubleKey();
		 def=keydef_get(region, key1, key2);
		}

/* 通常の有効なキー */
	if (def!=NULL&& def!=(void *)-1&& def->kdm==KDM_func)
		{
		 key1=def->funcNo;
		 keydef_args=def->args;

		 def=NULL;
		}
	if (def==NULL)
		{
		 km_addkey(key1);
		 return key1;
		}

/* マクロ登録中 */
	if (km.mode==KM_set)
		{
		 km_set(region,def,key1,key2);
		 return -1;
		}

/* 無効なキー */
	if (def==(void *)-1)
		return -1;

/* マクロ起動 */
	km_macro(def);
	return -1;
}

/*
	def					0			|			-1
	def->mode	func	N/A			|macro		N/A
				func	normalkey	|def		何も無し
	o 登録中	o		o			|-			-
	o 通常時	o		o			|macro起動	o
*/




int 	keysel(const char *s,const char *t)
{
	int 	c;

	system_msg(s);

	do	{
		 c = term_inkey();
		} while (strchr(t,c)==NULL);

	return tolower(c);
}

bool	keysel_ynq(const char *s)
{
	char	tmpbuff[MAXLINESTR+1];
	char	c;

	sprintf(tmpbuff, "%s... ? (y/n) :", s);
	c=keysel(tmpbuff,"Yy\r\nNn \x1b");
	return c=='y'||c=='\r'||c=='\n';
}

int 	keysel_yneq(const char *s)
{
	char	tmpbuff[MAXLINESTR+1];
	int 	c;

	sprintf(tmpbuff, "%s... ? (y/n or ESC) :", s);
	c=keysel(tmpbuff,"Yy\r\nNn \x1b");

	switch(c)
		{
	 case 'y':
	 case '\r':
	 case '\n':
	 	 return TRUE;
	 case 'n':
	 case ' ':
	 	 return FALSE;
	 case '\x1b':
	 default:
	 	 return ESCAPE;
	 	}
}

void	inkey_wait(const char *buffer)
{
	term_bell();
	if (buffer!=NULL)
		system_msg(buffer);

	term_inkey();
}



char	**keydef_args;

const	char	*keyf_getarg(int n)
{
	if (n>MAX_args)
		return NULL;
	return keydef_args[n];
}

int 	keyf_numarg()
{
	int 	i;

	i=0;
	while(keydef_args[i]!=NULL)
		++i;
	return i;
}





int 	keyf_getname(const char *s,int n)
{
	int 	i;
	int 	a;

/*fprintf(stderr," get_keyf [%s],n\n",s,n);*/
	if (n>=MAX_region)
		return -1;

	if (strncasecmp(s,"op_",3)==0)
		a=0; else
		a=3;

	for (i=0;i<MAX_func;++i)
		{
		 if (*keyfcode[n][i]=='\0')
		 	break;
		 
		 if (strcasecmp(keyfcode[n][i]+a,s)==0)
		 	return i;
		}
	return -1;
}


void	keyf_setarg(keydef_t *kdp, const char *s)
{
	char	buf[2048+1],*p;
	int 	n;
	bool	f;


	while(*s!='\0'&&isspace(*s))
		++s;

	f=FALSE;
	n=0;
	p=buf;
	do	{
		 if (*s=='\"')
		 	{
		 	 f= !f;
		 	 ++s;
		 	 if (*s!='\0')
		 	 	continue;
		 	}

		 if (f|| (*s!='\0'&& !isspace(*s)))
		 	{
		 	 *p++ = *s++;
		 	 if (*s!='\0')
		 	 	continue;
		 	}

		 *p='\0';
//fprintf(stderr, "  \'%s\'\n" ,buf);
		 kdp->args[n]= (char *)mem_alloc(strlen(buf)+1);
		 strcpy(kdp->args[n], buf);
		 ++n;

		 while(*s!='\0'&&isspace(*s))
		 	++s;
		 p=buf;
		} while (*s!='\0');

	kdp->args[n]=NULL;
}

void	keyf_set(int region,const char *k,const char *s)
{
	keydef_t	*kdp;
	int 	key[2];
	int 	n;
	char	buf[LN_buf+1],*p;


	p=strchr(s, ' ');
	if (p==NULL)
		n=keyf_getname(s, region); else
		{
		 strjncpy(buf, s, p-s);
		 n=keyf_getname(buf, region);
		}
	if (n==-1)
		return;

	keysdef_getcode(k, key, 2);
	kdp=keydef_get(region,key[0],key[1]);
	if (kdp==NULL|| kdp==(keydef_t *)-1)
		kdp=keydef_set(region, KDM_func, n, key[0], key[1]);

// ?? 上のif分の中にいれておいてよいのではなかったか?
	if (p!=NULL&& kdp!=NULL&& kdp!= (keydef_t *)-1)
	 	keyf_setarg(kdp, p); else
	 	kdp->args[0]=NULL;
}

