% socklog-check(8)

# NAME

socklog-check - checks for the availability of a socklog(8) service.

# SYNOPSIS

**socklog-check** \[-v\] \[unix \[*address*\]\]

# DESCRIPTION

**socklog-check** attempts to connect to a **socklog**(8) unix service
and reports whether the connection attempt succeeded or not.

It connects to the datagram oriented unix domain socket *address*, and
reports success through the return code. If *address* is not specified,
**socklog-check** attempts to connect to */dev/log*.

When running services that use the syslog facility under runit\'s
service supervision, **socklog-check** should be added to the top of the
service\'s run script to ensure that no syslog messages are generated
until a syslog service is up and running to process the messages, e.g.:

    #!/bin/sh
    socklog-check || exit 1
    exec service_using_syslog

# OPTIONS

**-v**
:   verbose. Print verbose message to standard error.

# EXIT CODES

On success **socklog-check** returns 0.

On error, **socklog-check** prints a message to standard error and exits
111.

# SEE ALSO

socklog(8), socklog-conf(8), svlogd(8), tryto(1), uncat(1), sv(8),
runsv(8), runsvdir(8)

https://smarden.org/socklog/\
https://smarden.org/runit/

# AUTHOR

Gerrit Pape \<pape@smarden.org\>
