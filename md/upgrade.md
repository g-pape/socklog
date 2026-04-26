% socklog - upgrading from previous versions

[G. Pape](https://smarden.org/pape/)\
[socklog](index.html)

---

# socklog - upgrading from previous versions

---

### 2.1.0 to 2.1.1 or 2.1.2

No further action from you is required.

### 2.0.x to 2.1.0

For new installations, the [socklog-conf](socklog-conf.8.html) program
now creates the socklog service directories in `/etc/sv/` and names them
`socklog-unix`, `socklog-klog`, \...; you don\'t necessarily need to
adapt existing installations.

Additionally [socklog-conf](socklog-conf.8.html) now by default creates
a `./check` script in the `socklog-unix` service directory, which is
utilized by *runit*\'s [sv](https://smarden.org/runit/sv.8.html) program
when starting or checking the service. To update existing installations,
you can create the script manually:

     # cd /var/service/socklog-unix
     # cat >check <<EOT
     #!/bin/sh
     exec socklog-check unix /dev/log
     EOT
     # chmod 755 check

### 1.5.0 to 2.0.x

The [tryto](tryto.1.html) program supports a new option -P to run the
program given at the command line in a new session, and signal the
process group on timeouts.

### 1.4.1 or 1.4.2 to 1.5.0

This version introduces the [socklog-check](socklog-check.8.html)
program which can be used to check for the availability of a syslog
service listening on /dev/log or other sockets. The
[socklog](socklog.8.html) program now understands the -U option in unix
mode to create unix domain sockets with permissions other than 777.

### 1.3.0 or 1.3.1 to 1.4.1 or 1.4.2

No further action from you is required.

### 1.2.0 to 1.3.0 or 1.3.1

With this version socklog supports old-style syslog network logging
through UDP in cooperation with the [runit](https://smarden.org/runit/)
package. The web page about [network logging](network.html) has been
updated accordingly.

### 1.1.0 or 1.1.1 to 1.2.0

The [socklog-conf](socklog-conf.8.html) program now creates slightly
different `run` scripts that utilize the new
[chpst](https://smarden.org/runit/chpst.8.html) program from the *runit*
package. Please refer to the [examples](examples.html) if you want to
adapt your current `run` scripts.

### 1.0.0 to 1.1.0 or 1.1.1

The socklog package now cooperates with the runit package instead of the
daemontools package to provide syslog services. If you don\'t want to
change your current configuration to utilize the programs from the runit
package, you don\'t need to upgrade. If you want to upgrade nonetheless,
but still use the programs from the daemontools package, no further
action from you but installing the new socklog version is required.

If you want to switch your configuration to utilize the runit package,
you need to re-create your service directories using the
[socklog-conf](socklog-conf.8.html) program.

---

[Gerrit Pape \<pape@smarden.org\>](mailto:pape@smarden.org)
