############
Introduction
############

FastSTI is a framework for agent-based models of large simulations of sexually
transmitted infection epidemics. It is designed to handle up to tens of millions of
agents on standard consumer hardware, like your laptop or, or a high performance
computer.

It runs simulations in parallel. The output of the simulations is written to CSV
files, which can be further processed in scripting languages like Python
or R.

You can do sophisticated simulations without making any modifications to the
code. But if you do wish to extend the framework, it is written in C. The
framework has been designed with extensibility in mind.

FastSTI is developed and tested on GNU/Linux using the gcc compiler. It uses two
portable 3rd-party libraries: the GNU Scientific Library and GLib. The code
adheres strictly to the C11 standard and should compile on any
standards-compliant modern C compiler.