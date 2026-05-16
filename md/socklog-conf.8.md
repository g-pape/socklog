% socklog-conf(8)

# NAME

socklog-conf - sets up a socklog(8) service.

# SYNOPSIS

**socklog-conf** unix\|inet\|klog\|ucspi-tcp *acct* *logacct*\
**socklog-conf** notify *acct* *grp*

# DESCRIPTION

**socklog-conf** creates */etc/sv* if necessary and a subdirectory that
runs the **unix**, **inet**, **ucspi-tcp** or **notify** service (see
below for each service). Except for the notify service, **socklog-conf**
also creates a log directory with subdirectories.

*acct*, *logacct* and *grp* must not contain any special characters.

# UNIX SERVICE

**socklog-conf** unix *acct* *logacct*

Running **socklog-conf** with the 1st argument *unix*, **socklog-conf**
creates the service directory */etc/sv/socklog-unix* and the log
directory */var/log/socklog*.

**socklog-conf** arranges for **socklog**(8) to run under the uid and
gid of *acct* and to listen for syslog messages on */dev/log*.

It also creates automatically rotated log directories in
*/var/log/socklog*. The logs are owned by *logacct*. The corresponding
**svlogd**(8) process runs under the uid and gid of *logacct*.

You can run the service under **runsvdir**(8) by creating a symbolic
link in the services directory:

ln -s /etc/sv/socklog-unix /service/

# INET SERVICE

**socklog-conf** inet *acct* *logacct*

Running **socklog-conf** with the 1st argument *inet*, **socklog-conf**
creates the service directory */etc/sv/socklog-inet* and the log
directory */var/log/socklog-inet*.

**socklog-conf** arranges for **socklog**(8) to run under the uid and
gid of *acct* and to listen for syslog messages on the UDP socket
*0.0.0.0:514*.

It also creates automatically rotated log directories in
*/var/log/socklog-inet*. The logs are owned by *logacct*. The
corresponding **svlogd**(8) process runs under the uid and gid of
*logacct*.

You can run the service under **runsvdir**(8) by creating a symbolic
link in the services directory:

ln -s /etc/sv/socklog-inet /service/

# KLOG SERVICE

**socklog-conf** klog *acct* *logacct*

Running **socklog-conf** with the 1st argument *klog*, **socklog-conf**
creates the service directory */etc/sv/socklog-klog* and the log
directory */var/log/socklog-klog*.

**socklog-conf** arranges for **socklog**(8) to run under the uid and
gid of *acct* and to read kernel messages from */proc/kmsg* on Linux, or
*/dev/socklog-klog* on BSD.

It also creates automatically rotated log directories in
*/var/log/socklog-klog*. The logs are owned by *logacct*. The
corresponding **svlogd**(8) process runs under the uid and gid of
*logacct*.

You can run the service under **runsvdir**(8) by creating a symbolic
link in the services directory:

ln -s /etc/sv/socklog-klog /service/

# UCSPI-TCP SERVICE

**socklog-conf** ucspi-tcp *acct* *logacct*

Running **socklog-conf** with the 1st argument *ucspi-tcp*,
**socklog-conf** creates the service directory
*/etc/sv/socklog-ucspi-tcp* and the log directory
*/var/log/socklog-ucspi-tcp*.

**socklog-conf** arranges for **tcpsvd**(1) to run **socklog**(8) under
the uid and gid of *acct* and to listen on the TCP socket
*0.0.0.0:10116*.

It also creates automatically rotated log directories in
*/var/log/socklog-ucspi-tcp*. The logs are owned by *logacct*. The
corresponding **svlogd**(8) process runs under the uid and gid of
*logacct*.

You can run the service under **runsvdir**(8) by creating a symbolic
link in the services directory:

ln -s /etc/sv/socklog-ucspi-tcp /service/

# NOTIFY SERVICE

**socklog-conf** notify *acct* *grp*

Running **socklog-conf** with the 1st argument *notify*,
**socklog-conf** creates the service directory */etc/sv/socklog-notify*.

**socklog-conf** arranges for **uncat**(1) to run under the uid and gid
of *acct* and to listen on the named pipe */var/log/socklog/.notify*.
The named pipe will have mode 0620, the uid of *acct* and the gid of
*grp*.

All uids running a log service that is configured to push log events to
the socklog-notify service must be member of the group *grp*.

You can run the service under **runsvdir**(8) by creating a symbolic
link in the services directory:

ln -s /etc/sv/socklog-notify /service/

# SEE ALSO

socklog(8), svlogd(8), nc(1), tryto(1), uncat(1), socklog-check(8),
tcpsvd(8), sv(8), runsv(8), runsvdir(8)

https://smarden.org/socklog/\
https://smarden.org/runit/

# AUTHOR

Gerrit Pape \<pape@smarden.org\>
