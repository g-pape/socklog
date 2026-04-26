% socklog(8)

# NAME

socklog - small and secure syslogd replacement for use with runit

# SYNOPSIS

**socklog** \[-rRTU\] \[unix\] \[*path\]*\
**socklog** \[-rRT\] inet \[*ip\]* \[*port*\]\
**socklog** \[-rRT\] ucspi \[*args*\]

# DESCRIPTION

**socklog** is run under runit\'s **runsv**(8), writing syslog messages
it receives from unix domain socket *path* or an inet udp socket
*ip*:*port* through a pipe provided by **runsv**(8) to a **svlogd**(8)
process.

**socklog** can be run as an ucspi application to listen to an unix
domain stream socket and for more flexible distributed logging.

If the environment variables $UID and/or $GID are present, **socklog**
drops permissions to those ids after creating and binding the socket
(not in *ucspi* mode).

**socklog** converts syslog facility and priority information to names
(*facility*.*priority*:) as found in */usr/include/syslog.h* at compile
time if present.

On solaris **socklog** also accepts *sun_stream* as first argument.
Please see the web page for details.

# UNIX SOCKET

**socklog** \[ unix \] \[ *path* \]

Starting **socklog** with the 1st argument *unix*, **socklog** will
listen to the unix domain socket *path*. If *path* is omitted, the
default */dev/log* is used.

The 1st argument may be omitted, default is *unix*.

# INET SOCKET

**socklog** inet \[ *ip* \] \[ *port* \]

Starting **socklog** with the 1st argument *inet*, **socklog** will
listen to the inet udp socket *ip*:*port*.

If *ip* starts with 0, **socklog** will bind to all local interfaces.

If *port* is less or equal 1024, **socklog** must be run by root.

*port* may be omitted, default is 514. *ip* may be omitted, default is
0.

**socklog** prepends *a.b.c.d:* to each syslog message it receives,
where *a.b.c.d* is the ip address of the connecting system.

# UCSPI MODE

**socklog** ucspi \[ *args* \]

Starting **socklog** with the 1st argument *ucspi*, **socklog** will run
as an ucspi application. Normally **socklog** will only be started in
*ucspi* mode by an ucspi server tool, such as **tcpsvd**(8),
**tcpserver**(1) or **unixserver**.

For each *arg*, **socklog** will prepend $*arg*: to each syslog
message, if the environment variable $*arg* is present (maximum is 8).

# OPTIONS

**-r**
:   raw. Write the raw syslog messages (no conversion of facility and
    priority) to the pipe, additionally to the log messages with syslog
    facility and priority converted to names.

**-R**
:   raw only. Same as -r above, but write the raw syslog messages only.

**-T**
:   Drop the timestamp (in classic RFC3164 format, \"Mmm dd hh:mm:ss\")
    in front of the log message; helpful if you add your own timestamps
    later.

**-U**
:   respect umask. Don\'t set umask to 0 before creating a unix domain
    socket, but respect the current setting of **umask**(2). This option
    only takes effect in unix mode.

# SEE ALSO

sv(8), runsvdir(8), runsv(8), svlogd(8), tryto(1), uncat(1),
socklog-check(8), tcpsvd(8), nc(1)

https://smarden.org/socklog/\
https://smarden.org/runit/

# AUTHOR

Gerrit Pape \<pape@smarden.org\>
