/*--------------------------------------------------------------------
  menu module.

    Copyright (c) 1998,1999,2000 SASAKI Shunsuke.
    All rights reserved. 
--------------------------------------------------------------------*/
#include	"ed.h"
#include	"sh.h"
#include	<ctype.h>
#include	<stdarg.h>


dspfmt_t	*dspreg_menu(void *vp,int y,int sizex,int sizey);

void	menu_iteminit(menu_t *mnp)
{
	mnp->nums =0;

	mnp->sy   =0;
	mnp->cy   =0;

	mnp->title="";
	mnp->drp  =dsp_reginit();
	mnp->drp->func =dspreg_menu;
	mnp->drp->vp   =(void *)mnp;

	mnp->drp->y    +=1;
	mnp->drp->sizey-=1;

	mnp->df=FALSE;
}

void	menu_itemfin(menu_t *mnp)
{
	if (mnp->nums>0)
		{
		 int 	i;

		 term_color(AC_normal);
		 for (i=mnp->drp->y; i<mnp->drp->y+mnp->drp->sizey; ++i)
		 	{
		 	 term_locate(i, mnp->drp->x);
		 	 widthputs("", mnp->drp->sizex);
		 	}

		 free(mnp->mitem);
		 mnp->nums=0;
		}
}

void	menu_itemmake(menu_t *mnp,void func(int,mitem_t *,void *),size_t nums,void *vp)
{
	int 	i;
	int 	ln;

	menu_itemfin(mnp);

	mnp->mitem=mem_alloc(sizeof(mitem_t) * nums);

	ln=0;
	for (i=0;i<nums;++i)
		{
		 mnp->mitem[i].cc= sysinfo.c_menuc;
		 mnp->mitem[i].nc= sysinfo.c_menun;
		 mnp->mitem[i].mf=FALSE;

		 func(i, mnp->mitem+i, vp);
		 ln=max(ln,strlen(mnp->mitem[i].str));
		 ++mnp->nums;
		}

	dsp_regresize(mnp->drp, ln+4, min(mnp->nums+2, dspall.sizey-2));
	dsp_regadd(mnp->drp);
}


typedef	struct
{
	size_t	width;
	size_t	num;
	char	*s;
}	itemstr_t;

void	makelists_proc(int a,mitem_t *mip,void *vp)
{
	itemstr_t	*isp;

	isp=vp;
	strcpy(mip->str, isp->s +isp->width*a);
}

void	menu_itemmakelists(menu_t *mnp,size_t width,size_t num,char *s)
{
	itemstr_t	is;

	is.num=num;
	is.width=width;
	is.s=s;

	menu_itemmake(mnp, makelists_proc, num, &is);
}

void	makev_proc(int a, mitem_t *mip, void *vp)
{
	va_list 	args;
	char	*p;

	va_copy(args, *(va_list *)vp);

	p=va_arg(args, char *);
	if (p==NULL)
		strcpy(mip->str,"null"); else
		strcpy(mip->str,p);

	va_copy(*(va_list *)vp, args);
}


int 	menu_vselect(int x, int y, size_t num, ...)
{
	menu_t	menu;
	va_list 	args;

	menu_iteminit(&menu);
	if (x!=-1)
		menu.drp->x=x;
	if (y!=-1)
		menu.drp->y=y;

	va_start(args,num);
	menu_itemmake(&menu, makev_proc, num, &args);
	va_end(args);

	return menu_select(&menu);
}

void	menu_dview(menu_t *mnp)
{
	int 	i;

	for (i=0;i<mnp->nums;++i)
		fprintf(stderr,"%2d:%s\n",i,mnp->mitem[i].str);
}

void	menu_csrmove(menu_t *mnp,int ly)
{
	int 	ly_b;

	ly_b=mnp->cy+mnp->sy;

	ly=max(ly,0);
	ly=min(ly,mnp->nums-1);

	if (ly_b==ly)
		return;

	if (ly_b-ly>mnp->cy)
		{
		 ly_b-=mnp->cy;
		 mnp->cy=0;
		 mnp->sy-=ly_b-ly;

		 return;
		}

	if (ly-ly_b>mnp->drp->sizey- 2 -mnp->cy -1)
		{
		 ly-=mnp->drp->sizey- 2 -mnp->cy-1;
		 mnp->cy=mnp->drp->sizey-2 -1;
		 mnp->sy+= ly-ly_b;

		 return;
		}

	mnp->cy+=ly-ly_b;
}

void	menu_csrnext(menu_t *mnp,char c)
{
	int 	n,m;

	m=n=mnp->cy+mnp->sy;
	for (;;)
		{
		 ++n;
		 if (n>=mnp->nums)
		 	n=0;
		 if (n==m)
		 	return;
		 if (toupper(*mnp->mitem[n].str)==toupper(c))
		 	{
		 	 menu_csrmove(mnp,n);
		 	 return;
		 	}
		}
}


dspfmt_t	*dspreg_menu(void *vp,int y,int sizex,int sizey)
{
	dspfmt_t	*dfp,*dfpb;
	char	buf[LN_dspbuf+1];
	menu_t	*mnp;

	mnp=(menu_t *)vp;

	if (y==0)
		{
		 int 	a;

	 	 if (*mnp->title=='\0')
	 	 	y=sizey-1; else
	 	 	{
		 	 a= min(strlen(mnp->title), sizex-3 -3);

		 	 strcpy(buf, "+- ");
		 	 strjfcpy(buf+3, mnp->title, LN_dspbuf-3, a);
		 	 buf[a+3]=' ';
		 	 if (sizex-a-5>0)
			 	 memset(buf+4+a, '-', sizex-a-5);
			}
		}

	if (y==sizey-1)
		{
		 *buf='+';
		 memset(buf+1, '-', sizex-2);
		}

	if (y==0|| y==sizey-1)
		{
		 buf[sizex-1]='+';
		 buf[sizex]='\0';

		 dfp=dsp_fmtinit(buf, NULL);
		 return dfp;
		}

	--y;


	buf[0]='|';
	buf[1]=mnp->mitem[mnp->sy+y].mf? '*': ' ';
	buf[2]='\0';
	dfpb=dfp=dsp_fmtinit(buf, NULL);

	strjfcpy(buf, mnp->mitem[mnp->sy+y].str, LN_dspbuf, sizex-4);
	dfp=dsp_fmtinit(buf, dfp);
	if (mnp->cy==y &&!mnp->df)
		dfp->col= mnp->mitem[mnp->sy+y].cc; else
		dfp->col= mnp->mitem[mnp->sy+y].nc;

	dfp=dsp_fmtinit(" |", dfp);

	return dfpb;
}

int 	menu_select(menu_t *mnp)
{
	int 		c;

	for (;;)
		{
		 dsp_allview();
		 term_csrh();
		 c=get_keyf(1);
		 switch(c)
		 	{
		  case -1:
		  	 continue;

		  default:
		  	 if (c&0xff00)
		  	 	menu_csrnext(mnp,c&0x00ff);
		  	 continue;
		  case KF_SysCursorup:
		  	 menu_csrmove(mnp,mnp->cy +mnp->sy -1);
		  	 continue;
		  case KF_SysCursordown:
		  	 menu_csrmove(mnp,mnp->cy +mnp->sy +1);
		  	 continue;
		  case KF_SysScrolldown:
		  	 menu_csrmove(mnp,mnp->cy +mnp->sy -mnp->drp->sizey -2);
		  	 continue;
		  case KF_SysScrollup:
		  	 menu_csrmove(mnp,mnp->cy +mnp->sy +mnp->drp->sizey -2);
		  	 continue;
		  case KF_SysCursortopside:
		  	 menu_csrmove(mnp,0);
		  	 continue;
		  case KF_SysCursorendside:
		  	 menu_csrmove(mnp,mnp->nums-1);
		  	 continue;

		  case KF_SysReturn:
		  	 c=mnp->cy+mnp->sy;
		  	 break;
		  case KF_SysEscape:
		  	 c=-1;
		  	 break;
		  	}
		 break;
		}


	menu_itemfin(mnp);
	dsp_regfin(mnp->drp);
	return c;
}

