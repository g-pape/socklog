% socklog - log events notification

[G. Pape](https://smarden.org/pape/)\
[socklog](index.html)

---

# socklog - log events notification

---

The service *socklog-notify* provides instant notification of log
events. There is an interval in seconds for the creation of
notifications to prevent a flood.

Set up the *socklog-notify* service as described in
[Configuration](configuration.html#notify) to ensure that the named pipe
`/var/log/socklog/.notify` exists. Do not delete this file after
creation.

Read [disable notifications](#disable) before stopping the
*socklog-notify* service once it is started.

*socklog-notify* is handled by the [uncat](uncat.1.html) program.

---

## How to configure log events

Choose a log controlled by
[svlogd](https://smarden.org/runit/svlogd.8.html) to be watched for log
events, say `/var/log/socklog/main/`.

Make sure the account running this log service is member of the
*socklog* group `adm`. If it isn\'t, change it now. Make sure that the
*socklog-notify* service is running.

Redirect the standard error output of
[svlogd](https://smarden.org/runit/svlogd.8.html) to the named pipe
*socklog-notify* is reading from. To do so, insert
`exec 2> /var/log/socklog/.notify` as second line into
`/service/socklog-unix/log/run`:

    #!/bin/sh
    exec 2> /var/log/socklog/.notify
    exec chpst -ulog svlogd \
      main/main main/auth main/cron main/daemon main/debug main/ftp \
      main/kern main/local main/mail main/news main/syslog main/user

[svlogd](https://smarden.org/runit/svlogd.8.html)\'s error messages are
always log events.

To configure additional log events in `/var/log/socklog/main/`, use the
`e` instruction in the `config` file of this log, see the [svlogd man
page](https://smarden.org/runit/svlogd.8.html). For example, if you want
to be notified about all log messages of the priority *alert*, add a
corresponding `e` instruction to the `config` file
`/var/log/socklog/main/config` like this:

    s999999
    n10
    e*.alert: *

Then tell the log service to re-read the `config` files:

    sv hup socklog-unix/log

---

## How to configure the notification

Per default, *socklog-notify* injects a mail to a specified address
containing the log events, but the behavior is configurable, so you can
change it to use instant or short messaging service for example.

To configure the kind of notification, edit `/etc/sv/socklog-notify/run`
and change the `prog` argument of [uncat](uncat.1.html) to your needs.
This example uses *sms_client* to notify:

    #!/bin/sh -e
    PIPE=/var/log/socklog/.notify
    if [ ! -p "$PIPE" ]; then mkfifo -m0620 "$PIPE"; chown log:adm "$PIPE"; fi
    exec <> "$PIPE"
    exec chpst -ulog uncat -s49999 -t90 \
      sh -c 'head -c140 | sms_client pager'

Then restart the service:

    sv restart socklog-notify

Another example using *wall*:

    exec chpst -ulog uncat -vs49999 -t180 sh -c 'head | wall'

---

[]{#disable}

## Disable log event notifications

You need to disable all log event notifications before stopping the
*socklog-notify* service. To check which services are configured for
sending log events, run:

    grep -F /var/log/socklog/.notify /service/*/log/run

For each of these services, edit the corresponding `log/run` script to
remove the `exec 2>/var/log/socklog/.notify` line and the configured log
event(s) and restart its log service:

    sv restart <service>/log

Now it is safe to stop the *socklog-notify* service.

---

[Gerrit Pape \<pape@smarden.org\>](mailto:pape@smarden.org)
