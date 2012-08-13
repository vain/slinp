ppres -- A PDF presentation toolchest.

[ppres project page](http://www.uninformativ.de/projects/?q=ppres) (german)

Released as PIZZA-WARE. See LICENSE.


History and the death of pdfpres
================================

Back in 2009, I needed a program to display presentations in a
comfortable way. This is why and when pdfpres was born. See also:

* [pdfpres on GitHub](https://github.com/vain/pdfpres)
* [pdfpres project page](http://www.uninformativ.de/projects/?q=pdfpres) (german)

pdfpres grew a lot over time. Featuritis. I didn't realize this until it
was too late. Sorry. I can no longer maintain the old code because I
simply don't have the resources to do so. The branches "master" (Gtk2
version) and "gtk3" will freeze. Feel free to fork the old program if
you want to keep it alive.

The new ppres is a minimalistic KISS toolchest that follows the Unix
philosophy. The main design goal is to keep it as simple and as small as
possible. This means it won't be a "typical" GUI application that just
"does everything". I'll only implement features that I *really* need.


New Architecture
================

ppres is a toolchest that comprises several individual programs.

One of them is showpdf. This tool's job is to display one page of one
PDF file in one window. It reads simple commands on STDIN to navigate in
the PDF file. That's it -- nothing else.

A control programm called prescontrol will act as the user interface. It
starts several instances of showpdf. prescontrol accepts your commands:
Show next slide, show previous slide and so on.

prescontrol and showpdf implement the core functionality of ppres.

Additional programs may be plugged into prescontrol. For now, there's
two of them: slidenotes retrieves notes for a specific slide. stopclock
shows a clock and a stopwatch. Just like showpdf, stopwatch reads
commands from STDIN (start, pause, reset) and is controlled by
prescontrol.

To sum it up, we get something like this:


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
	                             |  ^             +-----------------+
	                             |  |
	                             |  |
	                             |  |
	                             v  |
	                           ========
	                             USER
	                           ========


Why is this better than the old pdfpres?

* Each component is a simple component that is relatively easy to
  maintain. No more 2500 lines of C code.
* Each component is one process. This means, we automatically benefit
  from multiprocessor systems (they are ubiquitous today) without having
  to worry about multithreading!
* All components can be implemented in different programming languages.
  There's no need to implement everything in C. This saves me a hell lot
  of time.
* It's a lot easier to customize and extend. Components are exchangeable
  -- in theory, you could ditch showpdf and use Xpdf instead. Users can
  even re-implement whole components if they don't like mine.
* Some components (such as showpdf) may turn out to be universal
  components that can be used outside of ppres.
* Intelligent window managers can take care of arranging all the
  windows. This is a lot more flexible than the old layout (which was
  fixed) and saves a lot of code.


Downsides?

* ppres no longer arranges the windows for you. Well, it could do so,
  but I don't intend to implement that as a part of ppres. It's your
  window manager's job now. As part of awesome-vain, I provide a special
  layout for the Awesome Window Manager.
* It may get harder for other users to work with ppres. Sorry but I
  simply can not maintain a full blown GUI application (as pointed out
  above). A comparison: The old pdfpres was Firefox, the new ppres is
  uzbl. Something like that.


To sum it up once more: Occasional users will hate ppres, powerusers
will like it (at least more than pdfpres).
