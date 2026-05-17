# socklog - system and kernel logging services

The release is available through <https://smarden.org/socklog/>.

To build an experimental version from git, do
```
git clone -b next https://github.com/g-pape/socklog
cd socklog
package/compile
package/check
```
To install the experimental programs, as root do
```
install -m0755 command/* /bin/
```
