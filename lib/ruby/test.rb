#! /usr/local/bin/ruby

require	'term'

term= Term.new
term.start

	term.cls
	term.locate(1,2)
	term.puts("hello, ruby world.")
	term.scroll(1)

	term.inkey

term.stop
