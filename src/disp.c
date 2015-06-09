/*--------------------------------------------------------------------
  display module.

    Copyright (c) 1998,1999,2000 SASAKI Shunsuke.
    All rights reserved. 
--------------------------------------------------------------------*/
#include	"ed.h"


dspfmt_t	*dsp_fmtinit(const char *s, dspfmt_t *dfp)
{
	dspfmt_t *dfpb;

	dfpb=mem_alloc(sizeof(dspfmt_t));

	if (dfp!=NULL)
		dfp->next=dfpb;

	dfpb->col=AC_normal;
	dfpb->str=mem_alloc(strlen(s)+1);
	strcpy(dfpb->str, s);
	dfpb->next=NULL;

	return dfpb;
}

void	dsp_fmtsfin(dspfmt_t *dfp)
{
	dspfmt_t	*next;

//fprintf(stderr,"dsp_fmtsfin %p\n",dfp);

	while(dfp!=NULL)
		{
		 next=dfp->next;
		 free(dfp->str);
		 free(dfp);
		 dfp=next;
		}
}

/*
void	df_debug(dspfmt_t *dfp)
{
	while(dfp!=NULL)
		{
		 fprintf(stderr,"[%s] %02x\n",dfp->str,dfp->col);
		 dfp=dfp->next;
		}
	fprintf(stderr,"\n");
}
*/

void	dsp_regresize(dspreg_t *drp,int sizex,int sizey)
{
	int 	a;

	if (sizex> dspall.sizex)
		sizex= dspall.sizex;
	if (sizey> dspall.sizey)
		sizey= dspall.sizey;

	if (sizex!=-1)
		drp->sizex=sizex;

	if (sizey!=-1|| drp->sizey!= sizey)
		{
		 drp->sizey= sizey;

		 a=drp->y+ sizey -dspall.sizey;
		 if (a>0)
		 	 drp->y-=a;
		}
}


void	dsp_regview(dspreg_t *drp)
{
	dspfmt_t	*dfp, *dfp_b;
	char		*p;
	int 		n,m,a,i;

	char		spc_buf[LN_dspbuf+1];

	for (i=0;i<drp->sizey;++i)
		{
//fprintf(stderr,"**\n");

		 dfp=drp->func(drp->vp, i, drp->sizex, drp->sizey);
		 if (dfp==NULL)
		 	continue;

		 dfp_b = dfp;
		 p=dfp->str;
		 n=drp->sizex;
		 m=0;

		 memset(spc_buf, ' ', n);
		 spc_buf[n]='\0';

		 for (;;)
		 	{
		 	 if (*p=='\0')
		 	 	{
//fprintf(stderr," *%p\n", dfp->next);
		 	 	 if (dfp->next==NULL)
		 	 	 	p=spc_buf; else
		 	 	 	{
		 	 	 	 dfp=dfp->next;
		 	 	 	 p=dfp->str;
		 	 	 	}
		 	 	}
//fprintf(stderr,"*>[%s]\n",p);

		 	 a= min(strlen(p),n-m);

		 	 term_color(dfp->col);
		 	 term_locate(drp->y+i, drp->x +m);
		 	 widthputs(p,a);

		 	 m+=a;
		 	 p+=a;

//fprintf(stderr,"---\n");

		 	 if (m>=drp->sizex)
		 	 	break;
		 	}

		 dsp_fmtsfin(dfp_b);
		}
//fprintf(stderr,"-\n");
}

dspreg_t	*dsp_reginit()
{
	dspreg_t	*drp;

	drp=(dspreg_t *)mem_alloc(sizeof(dspreg_t));
	drp->x=0;
	drp->y=0;
	drp->sizex=dspall.sizex;
	drp->sizey=dspall.sizey;
	drp->vp=NULL;

// fprintf(stderr,"reg_ini %p\n",drp);

	return drp;
}

int 	dsp_regexist(dspreg_t *drp)
{
	int 	i;

//	if (drp==NULL)
//		return -1;

	for (i=0; i<dspall.drp_num; ++i)
		{
		 if (drp== dspall.drp[i])
		 	return i;
		}

	return -1;
}

void	dsp_regadd(dspreg_t *drp)
{
// fprintf(stderr,"reg_add %p\n",drp);
	if (dspall.drp_num>=MAX_dspreg || dsp_regexist(drp)!=-1)
		return;

	dspall.drp[dspall.drp_num]=drp;
	++dspall.drp_num;
}


void	dsp_regrm(dspreg_t *drp)
{
	int 	i;

// fprintf(stderr,"reg_rm %p\n",drp);

	i=dsp_regexist(drp);
	if (i==-1)
		return;


	if (i+1<dspall.drp_num)
		memcpy(&dspall.drp[i], &dspall.drp[i+1]
							, sizeof(dspreg_t *) * dspall.drp_num-i-1);
	--dspall.drp_num;
}

void	dsp_regfin(dspreg_t *drp)
{
// fprintf(stderr,"reg_fin\n",drp);

	dsp_regrm(drp);
	free(drp);
}


void	dsp_allinit()
{
	dspall.drp_num=0;
	dspall.sizex= term_sizex()-1;
	dspall.sizey= GetMaxRow()+1;	//!!

}

void	dsp_allview()
{
	int 	i;


// fprintf(stderr,"!%d\n", dspall.drp_num);

	for (i=0;i<dspall.drp_num;++i)
		{
//fprintf(stderr,"* %p ",dspall.drp[i]);
		 dsp_regview(dspall.drp[i]);
		}
//fprintf(stderr,"*\n",dspall.drp[i]);
}

