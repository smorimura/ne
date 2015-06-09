/*--------------------------------------------------------------------
	nxeditor
			FILE NAME:iskanji.c
			Programed by : I.Neva
			R & D  ADVANCED SYSTEMS. IMAGING PRODUCTS.
			1992.06.01

    Copyright (c) 1998,1999,2000 SASAKI Shunsuke.
    All rights reserved. 
--------------------------------------------------------------------*/
#include "ed.h"
#include	<ctype.h>


bool	iskanji(int c)
{
//	return (c&0x00ff) >=0xa1 && (c&0x00ff) <= 0xfe;
	return ((c&0x00ff) >=0xa1 && (c&0x00ff) <= 0xfe)|| (c&0x00ff)==0x8e;
}



int 	IsThisKanjiPosition(int offset,const char *buffer)
{
	int i;
	int Iskanji_flg;

	Iskanji_flg=FALSE;
	for (i = 0; i <= offset; i++) {
		if (iskanji(buffer[i]) && iskanji(buffer[i+1]))
		{
			i++;
			Iskanji_flg = TRUE;
		} else {
			Iskanji_flg = FALSE;
		}
	}
	if (i == offset+1)
		Iskanji_flg=FALSE;
	return Iskanji_flg;
}

int 	IsKanjiPosition()
{
	int 	i;

	i=GetBufferOffset();

	return i+1<=strlen(csrle.buf)
		&& iskanji(csrle.buf[i]) && iskanji(csrle.buf[i+1]);
}










#define	CT_ank		(CT_space| CT_cntrl| CT_other| CT_alnum| CT_kana)
#define	CT_kana 	(CT_hira| CT_kata)

#define	CT_skip		1
#define	CT_other	2
#define	CT_alnum	8

#define	CT_hira 	16
#define	CT_kata 	32
#define	CT_kkigou	64
#define	CT_kalnum	128
#define	CT_kanji	256

static	int 	char_getctype(int c)
{
	if (c==0)
		return 0;

//	if (isspace(c&0xff)|| (c<0x80 && iscntrl(c)))
	if (isspace(c&0xff)|| (!isprint(c&0xff)&&!iskanji(c)))
		return CT_skip;
	if (isalnum(c&0xff)||c=='_')
		return CT_alnum;
//	if (c<=0xff)
	return CT_other;
}

static	int 	kanji_getctype(int c1, int c2)
{
	int 	c;

	c = ((c1&0xff) << 8) | (c2&0xff);

	if (c == 0xa1ab || c == 0xa1ac || c == 0xa1bc)
		return CT_kana;
	if (c >= 0xa1a1 && c <= 0xa3af)
		return CT_kkigou;
	if (c >= 0xa3b0 && c <= 0xa3fa)
		return CT_kalnum;
	if (c >= 0xa4a1 && c <= 0xa4f3)
		return CT_hira;
	if (c >= 0xa5a1 && c <= 0xa5f6)
		return CT_kata;
	return CT_kanji;
}

int 	kanji_tknext(const char *s,int a,bool f)
{
	int 	pa,pb;

	if (s[a]=='\0')
		return a;
	if (a>strlen(s))
		return strlen(s);

	if (iskanji(s[a]))
		{
		 pa=kanji_getctype(s[a],s[a+1]);
		 do	{
		 	 a+=2;
		 	} while(iskanji(s[a])&& (pa&kanji_getctype(s[a],s[a+1]))!=0);
		 pb=char_getctype(s[a]);
		}else
		{
		 pa=char_getctype(s[a]);
		 do	{
		 	 ++a;
		 	 pb=char_getctype(s[a]);
		 	} while(pa==pb&&!iskanji(s[a]));
		}

	if (a==0|| iskanji(s[a])|| pb!=CT_skip|| !f)
		return a;

	++a;
	while(pb==char_getctype(s[a])&&!iskanji(s[a]))
		++a;
	return a;
}

int 	kanji_tkprev(const char *s,int a,bool f)
{
	int 	pa;

	if (a<=0)
		return 0;
	if (a>strlen(s))
		return strlen(s);

	--a;

	pa=char_getctype(s[a]);
	if (f&& !IsThisKanjiPosition(a-1,s)&& pa==CT_skip)
		{
		 --a;
		 while(a>0&& !IsThisKanjiPosition(a,s)
		 		&& pa==char_getctype(s[a]))
		 	--a;
		 pa=char_getctype(s[a]);
		}

	if (a<=0)
		return 0;

	if (IsThisKanjiPosition(a-1,s))
		{
		 --a;
		 pa=kanji_getctype(s[a],s[a+1]);
		 do	{
		 	 a-=2;
		 	} while(a>=0&& IsThisKanjiPosition(a,s)&&
		 			(pa&kanji_getctype(s[a],s[a+1]))!=0);
		 ++a;
		} else
		{
		 do	{
		 	 --a;
		 	} while(a>=0&& (a==0|| !IsThisKanjiPosition(a-1,s))
		 			&& pa== char_getctype(s[a]));
		}
	++a;
	return a;
}



/* kanji */
/* table、アルゴリズムなどに ekc のコードを利用しています。*/


//#define	issjis2(c)	((((u_char)c)>=0x40&&((u_char)c)<=0x7e)||(((u_char)c)>=0x80&&((u_char)c)<=0xfc))

//#define	issjis1(c)	(((u_char)(c)>=0x81 &&(u_char)(c)<=0x9f) || ((u_char)(c)>=0xe0 &&(u_char)(c)<=0xfc))
//#define	issjis2(c)	( (u_char)(c)>=0x40 &&(u_char)(c)<=0xfc)
//#define	iskana(c)	( (u_char)(c)>=0xa0 &&(u_char)(c)<=0xdf)
//#define	iseuc(c)	( (u_char)(c)>=0xa1 &&(u_char)(c)<=0xfe)
//#define	isjis(c)	( (u_char)(c)>=0x21 &&(u_char)(c)<=0x7e)

#define	ESC	0x1b
#define	CR	0x0d
#define	LF	0x0a
#define	SI	0x0f
#define	SO	0x0e

u_char	SJIStoEUCtable1[]=
{
	0x80,0xa1,0xa3,0xa5,0xa7,0xa9,0xab,0xad,0xaf,0xb1,0xb3,0xb5,0xb7,0xb9,0xbb,0xbd,
	0xbf,0xc1,0xc3,0xc5,0xc7,0xc9,0xcb,0xcd,0xcf,0xd1,0xd3,0xd5,0xd7,0xd9,0xdb,0xdd,
	0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,
	0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf,
	0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf,
	0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf,
	0xdf,0xe1,0xe3,0xe5,0xe7,0xe9,0xeb,0xed,0xef,0xf1,0xf3,0xf5,0xf7,0xf9,0xfb,0xfd,
	0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff
};

u_char	SJIStoEUCtable2[]=
{
	0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,0xb0,
	0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf,0xc0,
	0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf,0xd0,
	0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf,0x7f,
	0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef,
	0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xa1,
	0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,0xb0,0xb1,
	0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf,0xc0,0xc1,
	0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf,0xd0,0xd1,
	0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf,0xe0,0xe1,
	0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef,0xf0,0xf1,
	0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xfd,0xfe,0xff
};

u_char	EUCtoSJIStable1[]=
{
	     0x81,0x81,0x82,0x82,0x83,0x83,0x84,0x84,0x85,0x85,0x86,0x86,0x87,0x87,0x88,
	0x88,0x89,0x89,0x8a,0x8a,0x8b,0x8b,0x8c,0x8c,0x8d,0x8d,0x8e,0x8e,0x8f,0x8f,0x90,
	0x90,0x91,0x91,0x92,0x92,0x93,0x93,0x94,0x94,0x95,0x95,0x96,0x96,0x97,0x97,0x98,
	0x98,0x99,0x99,0x9a,0x9a,0x9b,0x9b,0x9c,0x9c,0x9d,0x9d,0x9e,0x9e,0x9f,0x9f,0xe0,
	0xe0,0xe1,0xe1,0xe2,0xe2,0xe3,0xe3,0xe4,0xe4,0xe5,0xe5,0xe6,0xe6,0xe7,0xe7,0xe8,
	0xe8,0xe9,0xe9,0xea,0xea,0xeb,0xeb,0xec,0xec,0xed,0xed,0xee,0xee,0xef,0xef
};


const	char	*kanji_fromeuc(char *s,size_t bytes,const char *t,int kc)
{
	u_char	c;
	const	char	*p;
	enum	{KM_ank, KM_kanji, KM_kana}	km;

	p=s;
	switch(kc)
		{
	 case KC_euc:
		 return t;

	 case KC_sjis:
		 for(;*t!='\0'&& bytes>0;)
		 	{
		 	 c=*t++;
		 	 if (c==0x8e&& iskana(*t))
		 	 	c=*t++; else
		 	 	{
		 	 	 if (iseuc(c)&& iseuc(*t))
		 	 		{
		 	 	 	 *s++= EUCtoSJIStable1[c - 0xa1];
		 	 	 	 --bytes;

		 	 	 	 c= *(u_char *)t - ((c&1)==0 ? 2 : *(u_char *)t>=0xe0? 0x60: 0x61);
		 	 	 	 ++t;
		 	 	 	}
		 	 	}
		 	 *s++=c;
		 	 --bytes;
		 	}
		 *s='\0';
		 break;

	 case KC_jis:
		 km=KM_ank;
		 for(;*t!='\0'&& bytes>0;)
		 	{
		 	 c=*t++;
		 	 if (iseuc(c)&& km!=KM_kanji)
		 	 	{
		 	 	 if (bytes<3)
		 	 	 	break;
		 	 	 *s++='\x1b';
		 	 	 *s++='$';
		 	 	 *s++='B';
		 	 	 bytes-=3;
		 	 	 km=KM_kanji;
		 	 	}
		 	 if (c==0x8e && iskana(*t))
		 	 	{
		 	 	 if (km!=KM_kana)
		 	 	 	{
		 	 	 	 if (bytes<3)
		 	 	 	 	break;
		 	 	 	 *s++='\x1b';
		 	 	 	 *s++='(';
		 	 	 	 *s++='I';
		 	 	 	 bytes-=3;
		 	 	 	 km=KM_kana;
		 	 	 	}

		 	 	 c=*t++;
		 	 	} else
		 	 	{
		 	 	 if (!iseuc(c) && km!=KM_ank)
		 	 	 	{
		 	 	 	 if (bytes<3)
		 	 	 	 	break;
		 	 	 	 *s++='\x1b';
		 	 	 	 *s++='(';
		 	 	 	 *s++='B';
		 	 	 	 bytes-=3;
		 	 	 	 km=KM_ank;
		 	 	 	}
		 	 	}

			 *s++= (c&0x7f);
			 --bytes;
		 	}
		 if (km!=KM_ank&& bytes>=3)
		 	strcpy(s,"\x1b(B"); else
		 	*s='\0';
		}
	return p;
}

int 	file_knjchk(FILE *fp)
{
	int 	c;
	int 	f_sjis,f_euc;
	int 	n_sjis,n_sjis_i,n_euc,n_euc_i;

	n_sjis=0;
	n_sjis_i=0;
	n_euc=0;
	n_euc_i=0;

	f_sjis=FALSE;
	f_euc=FALSE;

	for(;;)
		{
		 c=fgetc(fp);
		 if (c==EOF|| n_euc>32||n_sjis>32||n_euc_i>8||n_sjis_i>8)
			return n_euc- n_euc_i*4>= n_sjis- n_sjis_i*4? KC_euc:KC_sjis;
		 if (c==ESC)
		 	{
		 	 c=fgetc(fp);
		 	 if (c=='K')
		 	 	return KC_jis;
		 	 if (c=='$')
		 	 	{
		 	 	 c=fgetc(fp);
		 	 	 if (c=='B'||c=='@')
		 	 	 	return KC_jis;
		 	 	}
		 	}

		 if (f_euc)
		 	{
		 	 if (iseuc(c))
		 	 	++n_euc; else
		 	 	++n_euc_i;
		 	 f_euc=FALSE;
		 	} else
		 	{
		 	 if (iseuc(c))
		 	 	f_euc=TRUE;
		 	}
		 
		 if (f_sjis)
		 	{
		 	 if (issjis2(c))
		 	 	++n_sjis; else
		 	 	++n_sjis_i;
		 	 f_sjis=FALSE;
		 	} else
		 	{
		 	 if (issjis1(c))
		 	 	f_sjis=TRUE;
		 	}
		}
}

int 	file_gets(char *s, size_t bytes, FILE *fp, int *n_cr, int *n_lf)
{
	int 	c;
	bool	f_cr;

	f_cr=FALSE;
	for (;bytes>0;)
		{
		 c=fgetc(fp);

		 if (c==LF|| f_cr)
		 	{
		 	 if (c==LF)
		 	 	 ++*n_lf; else
		 	 	 ungetc(c, fp);
		 	 c=0;
		 	 break;
		 	}
		 if (c==EOF)
		 	{
		 	 c=-1;
		 	 break;
		 	}
		 if (c==CR)
		 	{
		 	 f_cr=TRUE;
		 	 ++*n_cr;
		 	 continue;
		 	}

		 *s++=c;
		 --bytes;
		}
	*s='\0';
	return c;
}


void	kanji_toeuc(char *s,size_t bytes,const char *t,bool f_sjis,int *jm)
{
	u_char 	c,sjc;
	bool	sf;		// (SI/SO)shift flag

	sjc=0;
	sf=FALSE;
	for (;*t!='\0'&& bytes>0;)
		{
		 c=(u_char)*t++;

		 switch(c)
		 	{
		  case SI:
		  	 sf=FALSE;
		  	 continue;
		  case SO:
		  	 sf=TRUE;
		  	 continue;

		  case ESC:
		 	 switch(*t++)
		 	 	{
		 	  case 'K':		// NEC kanji
		 	  	 *jm=JM_kanji;
		 	  	 sf=FALSE;
		 	 	 continue;

		 	  case 'H':		// NEC ank
		 	  	 *jm=JM_ank;
		 	  	 sf=FALSE;
		 	 	 continue;

		 	  case '&':
		 	  	 if (*t=='@') // JIS X0208-1990 切り替え準備
		 	  	 	{
		 	  	 	 ++t;
		 	  	 	 continue;
		 	  	 	}
		 	  	 break;

		 	  case '$':
		 	 	 if (*t=='B'||*t=='@')	// JIS X0208
		 	 	 	{
		 	 	 	 ++t;
		 	  	 	 sf=FALSE;
		 	  	 	 *jm=JM_kanji;
		 	 	 	 continue;
		 	 	 	}
		 	 	 break;

		 	  case '(':
		 	 	 if (*t=='J'||*t=='B'||*t=='H') // JIS X0201(roman)/ank
		 	 	 	{
		 	 	 	 ++t;
		 	  	 	 sf=FALSE;
		 	  	 	 *jm=JM_ank;
		 	 	 	 continue;
		 	 	 	}
		 	 	 if (*t=='I')	// JIS X0201(kana)
		 	 	 	{
		 	 	 	 ++t;
		 	 	 	 *jm=JM_kana;
		 	 	 	 continue;
		 	 	 	}
		 	 	}
		 	 --t;
		 	}

		 if (f_sjis)
			{
			 if (sjc!=0)
			 	{
			 	 if (issjis2(c))
			 	 	{
			 	 	 if (bytes<2)
			 	 	 	break;

				 	 if (c>=0x9f)
				 	 	++sjc;
				 	 *s++=sjc;
				 	 *s++=SJIStoEUCtable2[c-0x40];
				 	 bytes-=2;
				 	 sjc=0;
				 	 continue;
			 	 	}
			 	 sjc=0;
			 	 --t;
			 	 c=*t;
			 	} else
			 	{
			 	 if (issjis1(c))
			 	 	{
			 	 	 sjc= SJIStoEUCtable1[c&0x7f];
			 	 	 continue;
			 	 	}
			 	 if (iskana(c))
			 	 	{
			 	 	 if (bytes<2)
			 	 	 	break;

			 	 	 *s++=0x8e;
			 	 	 --bytes;
			 	 	}
			 	}
			}

		 if (*jm==JM_kana|| sf)
		 	{
		 	 if (bytes<2)
		 	 	break;
		 	 *s++=0x8e;
		 	 --bytes;
		 	}
		 if (*jm!=JM_ank|| sf)
		 	c |= 0x80;
		 *s++=c;
		 --bytes;
		}

	*s='\0';
}


int 	kanji_poscanon(int offset, const char *buf)
{
	int 	n,m;

	offset=min(strlen(buf), offset);

	n=m=0;
	for (;;)
		{
		 if (n==offset)
		 	return offset;
		 if (n>offset)
		 	return m;
		 if (buf[n]=='\0')
		 	return n;
		 m=n;
		 n+=kanji_countbuf(buf[n]);
		}
}

int 	kanji_poscandsp(int offset, const char *buf)
{
	int 	n,m,a,ln;

	ln=strlen(buf);
	n=m=a=0;
	for (;;)
		{
		 if (n==offset)
		 	return offset;
		 if (n>offset)
		 	return m;
		 if (a>ln || buf[a]=='\0')
		 	return n;
		 m=n;
		 n+=kanji_countdsp(buf[a], n);
		 a+=kanji_countbuf(buf[a]);
		}
}


int 	kanji_posnext(int offset, const char *buf)
{
	int 	i,n;

	n=kanji_countbuf(buf[offset]);

	for (i=0;i<n;++i)
		{
		 if (buf[offset+i]=='\0')
			break;
		}

	return offset+i;
}

int 	kanji_posprev(int offset, const char *buf)
{
	int 	n,m;

	n=m=0;
	for (;;)
		{
		 if (n>=offset|| buf[n]=='\0')
		 	return m;
		 m=n;
		 n+=kanji_countbuf(buf[n]);
		}
}

int 	kanji_posdsp(int offset, const char *buf)
{
	int 	n, m;

	n=0;
	m=0;
	for (;;)
		{
		 if (n>=offset|| buf[n]=='\0')
		 	return m;
		 m+=kanji_countdsp(buf[n], m);
		 n+=kanji_countbuf(buf[n]);
		}
}

int 	kanji_posbuf(int offset, const char *buf)
{
	int 	n,m;

	n=0;
	m=0;
	for (;;)
		{
		 if (m>=offset|| buf[n]=='\0')
		 	return n;
		 m+=kanji_countdsp(buf[n], m);
		 n+=kanji_countbuf(buf[n]);
		}
}

void	strjfcpy(char *s,const char *t,size_t bytes,size_t len)
{
	int 	n,m;

	for (;*t!=0;)
		{
		 n=kanji_countbuf(*t);
		 m=kanji_countdsp(*t, -1);
		 if (bytes<n || len<m)
		 	break;
		 memcpy(s, t, n);
		 s+=n;
		 t+=n;
		 bytes-=n;
		 len-=m;
		}

	for (;len>0&& bytes>0; --len,--bytes)
		*s++=' ';

	*s='\0';
}

int 	kanji_countbuf(char c)
{
	if (c==0)
		return 0;

	if (iseuc(c))
		return 2;
	if ((u_char)c==0x8e)
		return 2;
	if ((u_char)c==0x8f)
		return 3;
	return 1;
}

int 	kanji_countdsp(char c, int n)
{
	if (c==0)
		return 0;

	if (c=='\t' && n!=-1)
		return (n/sysinfo.tabstop+1)*sysinfo.tabstop - n;

	if ((u_char)c==0x8e)	// 半角かな
		return 2;

	if (iseuc(c)|| (u_char)c==0x8f|| !isprint(c&0xff))
		return 2;
	return 1;
}

