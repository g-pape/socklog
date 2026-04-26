% socklog - installation

[G. Pape](https://smarden.org/pape/)\
[socklog](index.html)

---

# socklog - installation

---

*socklog* by default installs into the [/package
hierarchy](https://cr.yp.to/slashpackage.html). To install as non-root
user and without the [/package](https://cr.yp.to/slashpackage.html)
directory, make the programs and documentation available manually.

---

[Install into /package](#package)\
[Install programs and documentation manually](#manually)

---

[]{#package}

## Install socklog into /package

Check that you have the recent version of
[runit](https://smarden.org/runit/) installed.

Download [socklog-2.1.2.tar.gz](socklog-2.1.2.tar.gz) into `/package`
([sha256sum](https://smarden.org/socklog/sha256sum.asc)) and unpack the
archive

    cd /package
    gunzip socklog-2.1.2.tar
    tar -xpf socklog-2.1.2.tar
    rm socklog-2.1.2.tar
    cd admin/socklog-2.1.2

On solaris read [using socklog on solaris](readme.solaris.html) before
proceeding.

Compile and install the *socklog* programs

    package/install

If you want to make the man pages available in the `/usr/local/man/`
hierarchy, do:

    package/install-man

To report success:

    mail pape-socklog-2.1.2@xxiv.smarden.org <compile/sysdeps

If you use *socklog* regularly, please
[contribute](https://smarden.org/pape/#contribution) to the project.

Refer to [Configuration](configuration.html) for configuring and setting
up *socklog* services.

---

[]{#manually}

## Install socklog programs and documentation manually

Download [socklog-2.1.2.tar.gz](socklog-2.1.2.tar.gz) into the current
directory ([sha256sum](https://smarden.org/socklog/sha256sum.asc)) and
unpack the archive

    gunzip socklog-2.1.2.tar
    tar -xpf socklog-2.1.2.tar
    cd admin/socklog-2.1.2

Compile and check the *socklog* programs

    package/compile
    package/check

The *socklog* programs are available in the `command/` directory. You
probably want to install them into `/bin`. As non-root user you probably
want to install them into `$HOME/bin`.

The documentation is available in the `doc/` directory, and the man
pages in the `man/` directory.

To report success:

    mail pape-socklog-2.1.2@xxiv.smarden.org <compile/sysdeps

If you use *socklog* regularly, please
[contribute](https://smarden.org/pape/#contribution) to the project.

Refer to [Configuration](configuration.html) for configuring and setting
up *socklog* services.

---

[Gerrit Pape \<pape@smarden.org\>](mailto:pape@smarden.org)
