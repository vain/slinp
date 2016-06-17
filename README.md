slinp -- A PDF presentation tool chest.

[slinp project page](https://www.uninformativ.de/projects/slinp/)

Released as PIZZA-WARE. See LICENSE.


What is slinp?
==============

Many people use the [LaTeX Beamer
class](https://bitbucket.org/rivanvx/beamer/wiki/Home) to create
presentations. What's missing is a tool to comfortably display those
presentations. slinp provides a framework to do this.

slinp is a minimalistic KISS tool chest that follows the Unix
philosophy. The main design goal is to keep it as simple and as small as
possible. This means it is not a "typical" GUI application that just
"does everything". slinp gives you some tools but it's your job to
connect them. An exemplary implementation (prescontrol) is shipped,
though.

slinp is the "little brother" of and successor to
[pdfpres](https://github.com/vain/pdfPres).


What does "slinp" mean?
=======================

In the tradition of recursive acronyms: *sl*inp *i*s *n*ot *p*dfpres.
Or, if you like: *sli*ck and *n*eat *p*resentations.

Pronounce it "slin-p" or "slinpy".


New Architecture
================

slinp is a tool chest that comprises several individual programs.

One of them is showpdf. This tool's job is to display one page of one
PDF file in one window. It reads simple commands on STDIN to navigate in
the PDF file. That's it -- nothing else.

A control programm called prescontrol will act as the user interface. It
starts several instances of showpdf. prescontrol accepts your commands:
Show next slide, show previous slide and so on.

prescontrol and showpdf implement the core functionality of slinp.

Additional programs are plugged into prescontrol. For now, there's three
of them: slidenotes retrieves notes for a specific slide. stopclock
shows a clock and a stopwatch. Just like showpdf, stopclock reads
commands from STDIN (like start, pause, reset) and is controlled by
prescontrol. pdfinfo from the poppler package is used to read some
metadata of the PDF file.

To sum it up, we get something like this (this sketch is not necessarily
updated to reflect recent changes -- refer to the man pages for
up-to-date information):

	+---------------+ +--------------+ +--------------+   +-------------+
	|    showpdf    | |   showpdf    | |   showpdf    |   |   showpdf   |
	| (prev. slide) | | (cur. slide) | | (next slide) |   | (projector) |
	+---------------+ +--------------+ +--------------+   +-------------+
	        ^                ^                ^                  ^
	        |                |                |                  |
	        |                +---+            |                  |
	        |                    |            |                  |
	        +------------------+ | +----------+                  |
	                           | | |                             |
	                           | | | +---------------------------+
	                           | | | |
	 +------------+        +-------------+        +-----------------+
	 | slidenotes | <----> | prescontrol | -----> |    stopclock    |
	 +------------+        +-------------+        | (timer + clock) |
	                          ^  |  ^             +-----------------+
	 +---------+              |  |  |
	 | pdfinfo | <------------+  |  |
	 +---------+                 |  |
	                             v  |
	                           ========
	                             USER
	                           ========

Again, this is just an example. It's how I want my presentation program
to behave. You are free to do things differently!

Why is this better than the old pdfpres?

* Each component is a simple component that is relatively easy to
  maintain. No more 2500 lines of C code.
* Each component is one process. This means, we automatically benefit
  from multiprocessor systems (they are ubiquitous today) without having
  to worry about multithreading!
* Each component can be implemented in a different programming language.
  There's no need to implement everything in C. This saves me a hell lot
  of time.
* It's a lot easier to customize and extend. Components are exchangeable
  -- in theory, you could ditch showpdf and use Xpdf instead. Users can
  also re-implement whole components if they don't like mine.
* Some components (such as showpdf) may turn out to be universal
  components that can be used outside of slinp.
* Intelligent window managers can take care of arranging all the
  windows. This is a lot more flexible than the old layout (which was
  fixed) and saves a lot of code.

Downsides?

* slinp no longer arranges the windows for you. Well, it could do so,
  but I don't intend to implement that as a part of slinp. It's your
  window manager's job now. See the directory `layout_example` for some
  examples (`wmctrl`, a layout for the Awesome WM and an exemplary patch
  for dwm -- those are just examples, though, and not maintained).
* It may get harder for other users to work with slinp. A comparison:
  The old pdfpres was Firefox, the new slinp is uzbl. Something like
  that.

To sum it up once more: Occasional users will hate slinp, powerusers
will like it (at least more than pdfpres).


Dependencies and Building
=========================

You should install the most recent version of all those tools and
libraries.

* prescontrol: Requires Python 3.2 and pdfinfo from the poppler package.
* showpdf: Requires gtk3 and poppler-glib.
* stopclock: Requires gtk3.
* slidenotes: Requires GNU Bash and GNU sed.

To build it, do:

	$ cd /path/to/sources
	$ make

To install it:

	$ make DESTDIR=/foo prefix=/bar install

Arch Linux is the only supported platform.
