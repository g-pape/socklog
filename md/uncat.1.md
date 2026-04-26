% uncat(1)

# NAME

uncat - continuously reads stdin, splits up the data either by timeout
or number of bytes read to process it

# SYNOPSIS

**uncat** \[-vo\] \[-t *seconds*\] \[-s *size*\] *prog*

# DESCRIPTION

**uncat** continuously reads stdin and repeatedly processes the data
either after *seconds* timeout or maximal number of bytes *size* read.
To process the data, **uncat** starts *prog* and feeds the data into
*prog*\'s standard input. *prog*\'s standard output is redirected to
standard error. **uncat** never writes to standard output.

*prog* consist of one or more arguments.

If *prog* exits non-zero, **uncat** prints an error message, discards
the data and continues to read stdin.

Note that **uncat** is not crash proof.

**uncat** is used to run the *socklog-notify* service.

# OPTIONS

**-v**
:   verbose. Print verbose messages to stderr.

**-o**
:   once. Exit on end of input. Normally **uncat** continues waiting for
    data on standard input, even on end of file.

**-t** *seconds*
:   timeout. Set the timeout to *seconds* seconds. Default is 300.

**-s** *size*
:   Set the maximal number of bytes to *size*. Default is 1024.

# SIGNALS

If **uncat** receives a TERM signal, it starts to process the data in
memory and exits as soon as possible.

**uncat** also does this on end of input if the **-o** option was given.

# EXIT CODES

**uncat** returns 0 after receiving a TERM signal. **uncat** returns 111
on error.

# SEE ALSO

socklog(8), socklog-conf(8), sv(8), runsv(8), svlogd(8), tryto(1)

https://smarden.org/socklog/\
https://smarden.org/runit/

# AUTHOR

Gerrit Pape \<pape@smarden.org\>
