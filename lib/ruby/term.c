#include	"ruby.h"

#include	"config.h"

#include	<stdio.h>
#include	<unistd.h>

#include	"generic.h"
#include	"term.h"

static	VALUE	term_rb_puts(VALUE obj, VALUE value)
{
	term_puts(STR2CSTR(value));
}

static	VALUE	term_rb_scroll(VALUE obj, VALUE value)
{
	term_scroll(FIX2INT(value));
}

static	VALUE	term_rb_locate(VALUE obj, VALUE v1, VALUE v2)
{
	term_locate(FIX2INT(v1), FIX2INT(v2));
}




void	Init_term()
{
	VALUE	cTerm;

	term_init();

	cTerm=rb_define_class("Term", rb_cObject);
	rb_define_method(cTerm, "start", term_start, 0);
	rb_define_method(cTerm, "stop", term_stop, 0);
	rb_define_method(cTerm, "report", term_report, 0);

	rb_define_method(cTerm, "cls", term_cls, 0);
	rb_define_method(cTerm, "clrtoe", term_clrtoe, 0);
	rb_define_method(cTerm, "x", term_sizex, 0);
	rb_define_method(cTerm, "y", term_sizey, 0);
	rb_define_method(cTerm, "bell", term_bell, 0);
	rb_define_method(cTerm, "csrn", term_csrn, 0);
	rb_define_method(cTerm, "csrh", term_csrh, 0);
	rb_define_method(cTerm, "csr_flush", term_csr_flush, 0);
	rb_define_method(cTerm, "kflush", term_kflush, 0);
	rb_define_method(cTerm, "puts", term_rb_puts, 1);
	rb_define_method(cTerm, "scroll", term_rb_scroll, 1);
	rb_define_method(cTerm, "locate", term_rb_locate, 2);
	rb_define_method(cTerm, "inkey", term_inkey, 0);

//	rb_define_method(cTerm, "", term_, 0);

	report_set(stderr);
}

