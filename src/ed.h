/*--------------------------------------------------------------------
	nxeditor
			FILE NAME:ed.h
			Programed by : I.Neva
			R & D  ADVANCED SYSTEMS. IMAGING PRODUCTS.
			1992.06.01

    Copyright (c) 1998,1999,2000
        SASAKI Shunsuke <ele@pop17.odn.ne.jp> All rights reserved. 
--------------------------------------------------------------------*/
#ifndef	__ED_H_
#define	__ED_H_


#include	"config.h"

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>

#include	<sys/param.h>

#ifdef	HAVE_UNISTD_H
#	include	<unistd.h>
#endif

#include	"../lib/generic.h"
#include	"../lib/hash.h"
#include	"kanji.h"


#define	SHELL

#ifdef	VAL_impl
#	define  VAL
#else
#	define  VAL	extern
#endif


#define	LN_path		2048
#define	LN_dspbuf	2048
#define	LN_buf		2048

#define	MAX_val		64


#include	"../lib/term.h"
#include	"disp.h"




#define	UNDO_SYSTEM		(0+MAX_edfiles)
#define	SEARCHS_SYSTEM	(1+MAX_edfiles)
#define	SYSTEM_SYSTEM	(2+MAX_edfiles)
#define	SHELLS_SYSTEM	(3+MAX_edfiles)
#define	FOPEN_SYSTEM	(4+MAX_edfiles)

#define	MAX_edfiles		8
#define	MAX_edbuf		(MAX_edfiles+5)


#define	ESCAPE	(-1)

#define	NONE		0
#define	DELETE		1
#define	BACKSPACE	2
#define	STAY		3

#define	MAXKEYDEF	200
#define	MAXSYSKEYDEF	200
#define	NumWidth	(sysinfo.numberf ? 6: 0)

//#define	MAXLINESTR	255
#define	MAXLINESTR	2048
//#define	MAXEDITLINE	1024
#define	MAXEDITLINE	2048
#define	MAXFILEMENU	512

#define	PROFILE_VERSION	"neprof02"


typedef	struct _ed
{
	struct _ed	*prev;
	struct _ed	*next;
	char		*buffer;

	size_t		size;		/* 確保してあるバッファのサイズ */
	size_t		bytes;		/* バッファ上の実際の文字サイズ*/
}	EditLine;

typedef	struct
{

	int 	x,y;			/* 表示の起点 */
	int 	sizex,sizey;	/* ディスプレイサイズ */


	int 	cy;			/* 画面上でのカーソル位置 */
	long 	ly;			/* 行番号 */
	EditLine	*ed;	/* その行のバッファ */

	int 	f_cx,f_sx;	/* fix されるまで同じ位置を維持しようとする。 */

	int 	l_cy;		/* latest. */
	long 	l_sy;		/* これらが変化すれば再描画が必要 */

	size_t	bytes;		/* 全体の編集サイズ */

	bool	gf;			/* 行末マークを消したフラグ */
}	se_t;

typedef	struct
{
	char	buf[MAXEDITLINE+1];	/* 編集バッファ */
	int 	size;		/* 編集バッファのサイズ */

	int 	dsize;		/* 表示可能サイズ */
	int 	dx;			/* 表示位置 */

	int 	cx,sx;		/* 正しい、カーソル/スクロール位置 */
	int 	lx;			/* 正しい、バッファ上の位置 */

	int 	l_sx;		/* latest. */
}	le_t;

#define	csrse	edbuf[CurrentFileNo].se
#define	csrle	edbuf[CurrentFileNo].le

//システム情報

typedef	struct
{
	char	nxpath[LN_path];
	hash_t	*vp_def;
	const char	*shell;

	int 		tabstop;
	char		tabcode;

	color_t		c_crmark;
	color_t		c_block;
	color_t		c_linenum;
	color_t		c_ctrl;
	color_t		c_sysmsg;
	color_t		c_search;
	color_t		c_menuc;
	color_t		c_menun;
	color_t		c_eff_dirc;
	color_t		c_eff_dirn;
	color_t		c_eff_normc;
	color_t		c_eff_normn;

	bool		crmarkf;	/* crmark処理を行うかどうか */
	bool		tabmarkf;
	bool		autoindentf;
	bool		numberf;
	bool		freecursorf;
	bool		overwritef;
	bool		japanesef;

	bool		backupf;
	bool		nocasef;

	bool		f_report;

	char		systemline[MAXEDITLINE+1];
	dspreg_t	*sl_drp;
	char		doublekey[4+1];
}	sysinfo_t;

VAL	sysinfo_t	sysinfo;
	VAL int 	CurrentFileNo;
	VAL int 	BackFileNo;

//csrse
	VAL int 	OnMessage_Flag;


typedef	enum	{REPLM_all,REPLM_before,REPLM_after,REPLM_block}	replm_t;


typedef	enum	{BLKM_none, BLKM_x, BLKM_y}	blkm_t;

typedef	struct
{
	long	y_st, y_ed;
	int 	x_st, x_ed;

	blkm_t	blkm;
}	block_t;



typedef	struct
{
	char	path[LN_path+1];
	int 	ct;					/* Create Time */
	bool	cf;					/* file Change Flag */
	int 	kc;					/* EUC/JIS/SJIS */
	int 	rm;					/* LF/CRLF/CR */


	bool	pm;					/* Paging Mode */
	replm_t	replm;				/* replace Mode */

	se_t	se;
	le_t	le;

	block_t	block;
}	edbuf_t;


VAL	edbuf_t	edbuf[MAX_edbuf];

#define	CNTRL(c)	((c)-'@')


extern	void	block_set(block_t *bp);
extern	bool	block_range(long n, block_t *bp, int *x_st, int *x_ed);
extern	void	BlockInit();
extern	void	block_cmove();

extern	int 	GetMinRow();
extern	int 	GetMaxRow();
extern	int 	GetRowWidth();
extern	int 	GetMinCol();
extern	int 	GetMaxCol();
extern	int 	GetColWidth();
extern	void	widthputs(const char *s,size_t len);
extern	void	crt_crmark();
extern	void	CrtDrawAll();
extern	void	DeleteAndDraw();
extern	void	InsertAndDraw();
extern	void	RefreshMessage();
extern	void	putDoubleKey(int key);
extern	void	delDoubleKey();
extern	void	system_guide_init();
extern	void	system_guide();
extern	void	system_msg(const char *buffer);

extern	int 	GetCol();
extern	int 	GetScroll();
extern	int 	GetBufferOffset();
extern	int 	GetRow();
extern	long	GetLineOffset();
extern	void	csr_fix();
extern	void	csr_lenew();
extern	void	csr_leupdate();
extern	void	csr_movehook();
extern	void	OffsetSetByColumn();
extern	void	csr_setlx(int lx);
extern	long	csr_getsy();
extern	void	csr_setly(long n);
extern	void	csr_setdy(int dy);
extern	void	CursorMove();

extern	void	dsp_allview();
extern	void	dsp_allinit();
extern	void	dsp_regrm(dspreg_t *drp);
extern	void	dsp_regadd(dspreg_t *drp);
extern	int 	dsp_regexist(dspreg_t *drp);
extern	dspreg_t	*dsp_reginit();
extern	void	dsp_fmtsfin(dspfmt_t *dfp);
extern	dspfmt_t	*dsp_fmtinit(const char *s, dspfmt_t *dfp);
extern	void	dsp_regresize(dspreg_t *drp,int sizex,int sizey);
extern	void	dsp_regview(dspreg_t *drp);
extern	void	dsp_regfin(dspreg_t *drp);

extern	void	FileStartInit(bool f);
extern	bool	FileOpenOp(const char *path);

extern	bool	need_filer( const char* pszFilename );
extern	bool	eff_filer(char *fn);

extern	void	LeditInput(int c,int contrl_flag);
extern	void	Ledit(int contrl_flag);
extern	void	InputAndCrt(int key);
extern	char	*HisGets(char *dest,const char *message,int listID);
extern	int 	GetS(const char *message,char *buffer);
extern	void	InputAndCrt(int key);

extern	void	lists_init();
extern	void	lists_clear();
extern	void	lists_debug();
extern	long	GetTopNumber();
extern	long	GetLastNumber();
extern	EditLine	*GetTop();
extern	EditLine	*GetLast();
extern	EditLine	*GetList(long o_number);
extern	EditLine	*MakeLine(const char *buffer);
extern	void	Realloc(EditLine *li,const char *s);
extern	void	AppendLast(EditLine *li);
extern	void	InsertLine(EditLine *bli,EditLine *li);
extern	void	DeleteList(EditLine *li);
extern	void	lists_proc(void func(),void *gp,long n_st,long n_en);
extern	size_t	lists_size(long n_st,long n_en);
extern	void	lists_add(void *func(),void *gp);
extern	void	lists_delete(int n_st,int n_ed);

extern	void	edbuf_init();
extern	bool	edbuf_mv(int n, const char *fn);
extern	void	edbuf_rm(int n);
extern	bool	edbuf_add(const char *fn);
extern	void	SetFileChangeFlag();
extern	bool	FileOpenOp(const char *path);

extern	void	eff_init(const char *s1,const char *s2);

extern	void	SetSignal();

extern	void	se_insert(const char *s,bool f);
extern	void	se_delete(int n, bool f);
extern	int 	CatLine();
extern	void	lm_mark(int ln, int n);
extern	int 	lm_line(int n);
extern	void	undo_paste();
extern	void	undo_add(bool df,const char *s);
extern	void	udbuf_set(bool df,const char *s);
extern	void	udbuf_get(char *s);
extern	void	udbuf_init();

extern	void	le_setlx(le_t *lep,int lx);
extern	int 	legets_gets(const char *, char *, int, int, int);
extern	int 	le_getcsx(le_t *lep);
extern	size_t	le_regbuf(const char *s,char *t);
extern	void	le_setlx(le_t *lep,int lx);
extern	void	le_csrleftside(le_t *lep);
extern	void	le_csrrightside(le_t *lep);
extern	void	le_csrleft(le_t *lep);
extern	void	le_csrright(le_t *lep);
extern	void	le_edit(le_t *lep,int ch,int cm);
extern	size_t	le_regbuf(const char *s,char *t);

extern	void	ne_fin();
extern	void	sysinfo_path(char *s,const char *t);
extern	void	CommandCom(char *sys_buff);

extern	int 	profile_read();
extern	void	profile_write();

extern	void	search_init();
extern	void	search_clrword();

extern	void	key_set();
extern	void	sysinfo_optset();
extern	void	opt_set(const char *s,const char *t);
extern	void	opt_default();
extern	void	config_read(char *path);


extern	void	inkey_wait(const char *buffer);
extern	int 	keysel(const char *s,const char *t);
extern	bool	keysel_ynq(const char *s);
extern	int 	keysel_yneq(const char *s);









typedef	struct
{
	char	str[MAXLINESTR+1];
	color_t	nc;	/* normal時 */
	color_t	cc;	/* cursorがある時 */

	bool	mf;	/* mark flag */
}	mitem_t;

typedef	struct
{
	mitem_t		*mitem;
	size_t		nums;

	char		*title;
	bool		df;				/* filerで利用するdisable flag */

	int 		sy,cy;			/* 現在の座標 */

	dspreg_t	*drp;
}	menu_t;

extern	void	menu_itemmake(menu_t *mnp,void func(int,mitem_t *,void *),size_t nums,void *vp);
extern	void	menu_itemmakelists(menu_t *mnp,size_t width,size_t num,char *s);
extern	void	menu_iteminit(menu_t *mnp);
extern	void	menu_itemfin(menu_t *mnp);
extern	void	menu_dview(menu_t *mnp);
extern	void	menu_itemview(menu_t *mnp,int a,bool f);
extern	void	menu_view(menu_t *mnp);
extern	void	menu_csrmove(menu_t *mnp,int ly);
extern	void	menu_csrnext(menu_t *mnp,char c);
extern	int 	menu_select(menu_t *mnp);
extern	int 	menu_vselect(int x, int y, size_t num, ...);

extern	dspreg_t	*menu_regset(menu_t *mnp);


typedef	struct
{
	FILE	*fp;
	int 	kc;
	int 	jm;	/* jis処理中 */

	int 	n_cr,n_lf;
}	kinfo_t;

#define	issjis1(c)	(((u_char)(c)>=0x81 &&(u_char)(c)<=0x9f) || ((u_char)(c)>=0xe0 &&(u_char)(c)<=0xfc))
#define	issjis2(c)	( (u_char)(c)>=0x40 &&(u_char)(c)<=0xfc)
#define	iskana(c)	( (u_char)(c)>=0xa0 &&(u_char)(c)<=0xdf)
#define	iseuc(c)	( (u_char)(c)>=0xa1 &&(u_char)(c)<=0xfe)
#define	isjis(c)	( (u_char)(c)>=0x21 &&(u_char)(c)<=0x7e)

extern	const	char	*kanji_fromeuc(char *s, size_t bytes, const char *t, int kc);

extern	int 	file_knjchk(FILE *fp);
extern	int 	file_gets(char *s, size_t bytes, FILE *fp, int *n_cr, int *n_lf);
extern	void	kanji_toeuc(char *s,size_t bytes, const char *t
									, bool f_sjis, int *jm);

#define	JM_ank	0
#define	JM_kana	1
#define	JM_kanji	2
#define	JM_so	3

#define	KC_euc	0
#define	KC_jis	1
#define	KC_sjis	2

#define	RM_lf	0
#define	RM_crlf	1
#define	RM_cr	2

extern	int 	IsThisKanjiPosition(int offset,const char *buffer);
extern	int 	IsKanjiPosition();
extern	int 	kanji_poscanon(int offset, const char *buf);
extern	int 	kanji_poscandsp(int offset, const char *buf);
extern	int 	kanji_posnext(int offset, const char *buf);
extern	int 	kanji_posprev(int offset, const char *buf);
extern	int 	kanji_posbuf(int offset, const char *buf);
extern	int 	kanji_posdsp(int offset, const char *buf);
extern	int 	kanji_countdsp(char c, int n);
extern	int 	kanji_countbuf(char c);
extern	int 	kanji_tknext(const char *s,int a,bool f);
extern	int 	kanji_tkprev(const char *s,int a,bool f);
extern	bool	iskanji(int c);

extern	void	strjfcpy(char *s,const char *t,size_t bytes,size_t len);

extern	const char	*keyf_getarg(int n);
extern	int 	keyf_numarg();
extern	void	keyf_set(int region,const char *k,const char *s);
extern	int 	keyf_getname(const char *s,int n);
extern	int 	get_keyf(int region);
extern	void	keydef_report();


#endif	/* __ED_H_ */
