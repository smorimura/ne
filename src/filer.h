/*--------------------------------------------------------------------
  filer module.

    Copyright (c) 1998,1999,2000 SASAKI Shunsuke.
    All rights reserved. 
--------------------------------------------------------------------*/
#include	"ed.h"

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<errno.h>


typedef	struct	fitem
{
	char			*fn;
	char			*e;
	struct	stat	stat;

	struct	fitem	*next;
}	fitem_t;

typedef	struct
{
	fitem_t	*fitem;
	int 	n;
}	flist_t;

typedef	struct
{
	char	path[LN_path +1];

	menu_t	menu;
	bool	df;

	flist_t	flist;
	fitem_t	**findex;
	int 	*mark;
	int 	markmax;
	int 	marknum;

	char	match[LN_path+1];
}	fw_t;
fw_t	fw[2];
#define	fw_c	fw[eff.wa]

typedef	struct
{
	int 	wn;			/* windowの数 */
	int 	wa;			/* Window Active 位置 */
	bool	df;			/* dirty flag */

	int 	sort[2];	/* sort方式 */
}	eff_t;
eff_t	eff;

enum	{SA_none,SA_fname,SA_ext};

typedef struct	fop
{
	int 	(*file_func)(const char*,struct stat *,const char*, struct fop*);
	int 	(*dir_func)(const char*,struct stat *,const char*
														,bool, struct fop*);
	char    *title;

	int 	om;		/* overwrite mode. */
	int 	pfm;	/* permition force mode. */

	int 	en;		/* error number */
}	fop_t;

#define	FR_err		-1
#define	FR_ok		0
#define	FR_end		1
#define	FR_nonrec	2

#define	FO_owrite	1
#define	FO_update	2
#define	FO_none		3
#define	FO_rename	4

#define	FP_ignore	1
#define	FP_force	2


extern	bool	need_filer( const char* pszFileName );
