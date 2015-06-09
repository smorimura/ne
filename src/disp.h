/*--------------------------------------------------------------------
  dispray module.

    Copyright (c) 1998,1999,2000 SASAKI Shunsuke.
    All rights reserved. 
--------------------------------------------------------------------*/

typedef	struct dspfmt
{
	color_t 		col;
	char			*str;
	struct dspfmt	*next;
}	dspfmt_t;

typedef	struct
{
	int 		x,y;
	int 		sizex,sizey;
	void		*vp;
	dspfmt_t	*(*func)(void *vp,int a,int sizex,int sizey);
}	dspreg_t;


#define	MAX_dspreg	16

typedef	struct
{
	int 		sizex, sizey;

	int 		drp_num;
	dspreg_t	*drp[MAX_dspreg];
	bool		ff;
}	dspall_t;

VAL	dspall_t	dspall;

extern	dspfmt_t	*dsp_fmtinit(const char *s, dspfmt_t *dfp);
extern	void		dsp_fmtsfin(dspfmt_t *dfp);
extern	dspreg_t	*dsp_reginit();
extern	void		dsp_regadd(dspreg_t *drp);
extern	void		dsp_allinit();
extern	void		dsp_allview();
extern	int 	dsp_regexist(dspreg_t *drp);
extern	void	dsp_regrm(dspreg_t *drp);
extern	void	dsp_regresize(dspreg_t *drp,int sizex,int sizey);

