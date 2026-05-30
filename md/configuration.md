% socklog - configuration

[G. Pape](https://smarden.org/pape/)\
[socklog](index.html)

---

# socklog - configuration

---

The *socklog-unix* service listens on the unix domain socket `/dev/log`.
Usually this service replaces the *syslogd* system log daemon.

The *socklog-inet* service listens on the UDP port `0.0.0.0:514`.
Usually this service replaces *syslogd*\'s support for remote logging.

The *socklog-klog* service reads kernel messages from `/proc/kmsg` on
Linux or `/dev/klog` on BSD. Usually this service replaces *klogd* on
Linux or *syslogd* on BSD.

The *socklog-ucspi-tcp* service listens on the TCP port `0.0.0.0:10116`,
this is a server for [socklog network logging](network.html), a
different remote logging concept.

The *socklog-notify* handles [log event notification](notify.html),
scheduled notification of specified log entries.

---

## How to configure the socklog services

[The socklog-unix service](#unix)\
[The socklog-inet service](#inet)\
[The socklog-klog service](#klog)\
[The socklog-ucspi-tcp service](#ucspi-tcp)\
[The socklog-notify service](#notify)

[Removing socklog services](https://smarden.org/runit/faq.html#remove)

---

### [The socklog-unix service]{#unix}

Before starting the *socklog-unix* service, you will have to make two
decisions:

-   The *socklog-unix* account name, normally `nobody`. The `socklog`
    program runs under this account. If this account does not exist,
    create it now.
-   The *socklog* account name, normally `log`. Logging programs run
    under this account. If this account does not exist, create it now.

Create the service directory and log directories by running the
[`socklog-conf`](socklog-conf.8.html) program:

    socklog-conf unix nobody log

If you are replacing another service providing system logging through
the socket `/dev/log`, such as *syslogd*, stop this service now and
ensure that it will not be restarted on runlevel change or system
reboot.

Tell [runsvdir](https://smarden.org/runit/runsvdir.8.html) about the new
service:

    ln -s /etc/sv/socklog-unix /service/

Wait five seconds for the service to start and then check the log
directories, e.g.:

    less /var/log/socklog/main/current

Optionally configure the *socklog-unix* service by editing the
corresponding `run` scripts and `config` files and
[restarting](https://smarden.org/runit/faq.html#control) the service:

-   /service/socklog-unix/run
-   /service/socklog-unix/log/run
-   /var/log/socklog/\*/config

---

### [The socklog-inet service]{#inet}

Before starting the *socklog-inet* service, you will have to make two
decisions:

-   The *socklog-inet* account name, normally `nobody`. The `socklog`
    program runs under this account. If this account does not exist,
    create it now.
-   The *socklog* account name, normally `log`. Logging programs run
    under this account. If this account does not exist, create it now.

Create the service directory and log directories by running the
[`socklog-conf`](socklog-conf.8.html) program:

    socklog-conf inet nobody log

If you are replacing another service providing system logging through
the syslog UDP port 514, such as *syslogd*, stop this service now and
ensure that it will not be restarted on runlevel change or system
reboot.

Tell [runsvdir](https://smarden.org/runit/runsvdir.8.html) about the new
service:

    ln -s /etc/sv/socklog-inet /service/

Wait five seconds for the service to start and then check the log
directory:

    less /var/log/socklog-inet/main/current

Optionally configure the *socklog-inet* service by editing the
corresponding `run` scripts and `config` file and
[restarting](https://smarden.org/runit/faq.html#control) the service:

-   /service/socklog-inet/run
-   /service/socklog-inet/log/run
-   /var/log/socklog-inet/main/config

---

### [The socklog-klog service]{#klog}

Before starting the *socklog-klog* service, you will have to make two
decisions:

-   The *socklog-klog* account name, normally `nobody`. The `socklog`
    program runs under this account. If this account does not exist,
    create it now.
-   The *socklog* account name, normally `log`. Logging programs run
    under this account. If this account does not exist, create it now.

Create the service directory and log directories by running the
[`socklog-conf`](socklog-conf.8.html) program:

    socklog-conf klog nobody log

If you are replacing another service providing kernel logging, such as
*klogd* on Linux or *syslogd* on BSD, stop this service now and ensure
that it will not be restarted on runlevel change or system reboot.

Tell [runsvdir](https://smarden.org/runit/runsvdir.8.html) about the new
service:

    ln -s /etc/sv/socklog-klog /service/

Wait five seconds for the service to start and then check the log
directory:

    less /var/log/socklog-klog/main/current

Optionally configure the *socklog-klog* service by editing the
corresponding `run` scripts and `config` file and
[restarting](https://smarden.org/runit/faq.html#control) the service:

-   /service/socklog-klog/run
-   /service/socklog-klog/log/run
-   /var/log/socklog-klog/config

---

### [The socklog-ucspi-tcp service]{#ucspi-tcp}

Before starting the *socklog-ucspi-tcp* service, you will have to make
two decisions:

-   The *socklog-ucspi-tcp* account name, normally `nobody`. The
    `socklog` program runs under this account. If this account does not
    exist, create it now.
-   The *socklog* account name, normally `log`. Logging programs run
    under this account. If this account does not exist, create it now.

Make sure that you have the [ipsvd](https://smarden.org/ipsvd/) package
installed. Then create the service directory and log directories by
running the [`socklog-conf`](socklog-conf.8.html) program:

    socklog-conf ucspi-tcp nobody log

Tell [runsvdir](https://smarden.org/runit/runsvdir.8.html) about the new
service:

    ln -s /etc/sv/socklog-ucspi-tcp /service/

Wait five seconds for the service to start and then check the log
directory:

    less /var/log/socklog-ucspi-tcp/main/current

Optionally configure the *socklog-ucspi-tcp* service by editing the
corresponding `run` scripts and `config` file and
[restarting](https://smarden.org/runit/faq.html#control) the service:

-   /service/socklog-ucspi-tcp/run
-   /service/socklog-ucspi-tcp/log/run
-   /var/log/socklog-ucspi-tcp/main/config

Refer to [Network logging](network.html) for setting up clients.

---

### [The socklog-notify service]{#notify}

Before starting the *socklog-notify* service, you will have to make two
decisions:

-   The *socklog* account name, normally `log`. The `uncat` program runs
    under this account. If this account does not exist, create it now.
-   The *socklog* group name, normally `adm`. Logging programs that
    report [log events](notify.html) to the *socklog-notify* service
    must run under an account that is member of this group. If this
    group does not exist, create it now.

Create the service directory by running the
[`socklog-conf`](socklog-conf.8.html) program:

    socklog-conf notify log adm

Tell [runsvdir](https://smarden.org/runit/runsvdir.8.html) about the new
service:

    ln -s /etc/sv/socklog-notify /service/

Wait five seconds for the service to start, you can test the service
with:

    echo test | tai64n > /var/log/socklog/.notify

Optionally configure the *socklog-notify* service by editing the
corresponding `run` script and
[restarting](https://smarden.org/runit/faq.html#control) the service:

-   /service/socklog-notify/run

Read [Log events notification](notify.html) before disabling this
service and for configuring arbitrary log services to notify events.

---

[Gerrit Pape \<pape@smarden.org\>](mailto:pape@smarden.org)
