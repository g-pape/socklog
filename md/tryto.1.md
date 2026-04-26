% tryto(1)

# NAME

tryto - tries to run a command limited by a timeout or number of tries,
can be used to run as svlogd(8) processor.

# SYNOPSIS

**tryto** \[-pPv\] \[-t *sec*\] \[-k *ksec*\] \[-n *tries*\] *prog*

# DESCRIPTION

*prog* consist of one or more arguments.

**tryto** runs and watches *prog*, feeding its standard input to
*prog*\'s standard input. If *prog* exits with a return code other then
0, **tryto** runs *prog* again after sleeping one second.

If the number of retries reaches the maximal number of *tries*,
**tryto** prints an error message and gives up.

If the timeout *sec* seconds is reached and *prog* is still running,
**tryto** sends a TERM signal to *prog*, waits *ksec* seconds for *prog*
to terminate, then sends a KILL signal if *prog* still is there, and
exits as soon as possible.

# OPTIONS

**-t** *sec*
:   timeout. Set the timeout to send TERM to *prog* to *sec* seconds.
    Default is 180.

**-k** *ksec*
:   kill timeout. Set the timeout to send KILL to *prog* to *ksec*
    seconds. Default is 5.

**-n** *tries*
:   Set the maximal number of tries to *tries*. If *prog* exited with a
    return code other that 0, **tryto** tries to rewind standard input
    to the beginning using **lseek**(2) before starting *prog* again.
    Default is 5.

**-p**
:   processor. Use this option if you run **tryto** as a **svlogd**(8)
    processor (see below).

**-P**
:   process group. Run *prog* in a new session and process group, and
    send signals on timeout to *prog*\'s process group instead of its
    pid.

**-v**
:   verbose. Print verbose messages to standard error.

# PROCESSOR

If **tryto** sees the **-p** option, **tryto** runs as a **svlogd**(8)
or **multilog**(8) processor, making use of filedescriptors 4 and 5:

Before starting *prog*, **tryto** moves the filedescriptor 5 to 2, so
all error messages from **tryto** and *prog* will be saved in
**svlogd**(8)\'s *state* to be processed on the next run of **tryto**
**-p**.

After starting *prog*, **tryto** first feeds all data it reads from
filedescriptor 4 into *prog*\'s standard input, then all data from
filedescriptor 0.

If *prog* fails by timeout *sec* seconds or maximal number of *tries*,
**tryto** prints all data from standard input to standard output, an
error message to standard error, and exits with 0.

# EXIT CODES

If **tryto** itself fails, it returns 111.

If **tryto** runs as a **svlogd**(8) processor, **tryto** returns 0 in
all other cases.

If *prog* was run successfully, **tryto** returns 0.

If *prog* failed by timeout, **tryto** returns 100.

If *prog* failed by maximal number of *tries*, **tryto** returns the
last return code from *prog*.

# SEE ALSO

socklog(8), uncat(1), svlogd(8), multilog(8), lseek(2)

https://smarden.org/socklog/\
https://smarden.org/runit/

# AUTHOR

Gerrit Pape \<pape@smarden.org\>
