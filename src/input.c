/*--------------------------------------------------------------------
	nxeditor
			FILE NAME:input.c
			Programed by : I.Neva
			R & D  ADVANCED SYSTEMS. IMAGING PRODUCTS.
			1992.06.01

    Copyright (c) 1998,1999,2000 SASAKI Shunsuke.
    All rights reserved. 
--------------------------------------------------------------------*/
#include 	"ed.h"


void	LeditInput(int c,int contrl_flag)
{
	int 	lx;

	lx=csrle.lx;

//	if (c>=0xa0&&c<=0xff)
//		return;

	le_edit(&csrle,c,contrl_flag);

	csr_fix();
}

void	Ledit(int contrl_flag)
{
	LeditInput(' ', contrl_flag);
}

void	InputAndCrt(int key)
{
	if (sysinfo.overwritef)
		se_delete(IsKanjiPosition()?2:1,FALSE);

	if (iseuc(key)|| key==0x8e) {
		LeditInput(key<<8 |term_inkey(), NONE);
		term_locate(0,GetRow());
	} else {
		LeditInput(key, NONE);
	}

	csr_movehook();
}




char	*HisGets(char *dest,const char *message,int listID)
{
	return 
		legets_gets(message,dest,GetColWidth(),MAXLINESTR,listID)==ESCAPE?
		NULL:dest;
}

int 	GetS(const char *message,char *buffer)
{
	return legets_gets(message,buffer,GetColWidth(),MAXLINESTR,-1);
}


