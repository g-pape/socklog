% socklog - use dietlibc

[G. Pape](https://smarden.org/pape/)\
[socklog](index.html)

---

# socklog - use dietlibc

---

To recompile the *socklog* programs with the [diet
libc](http://www.fefe.de/dietlibc/), check that you have the recent
version of [dietlibc](http://www.fefe.de/dietlibc/) installed.

Change to the package directory of *socklog*:

      # cd /package/admin/socklog/

Change the `conf-cc` and `conf-ld` to use `diet`:

      # echo 'diet -Os gcc -O2 -Wall' >src/conf-cc
      # echo 'diet -Os gcc -s -Os -pipe' >src/conf-ld

Rebuild and install the *socklog* programs:

      # package/install

---

[Gerrit Pape \<pape@smarden.org\>](mailto:pape@smarden.org)
