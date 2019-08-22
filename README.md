#Non blocking io demos
##Version 1.0
The repo contains several utilites to help to understand linux non-blocking io:
- server\_stuff (server\_stuff.cc): sample server program (IN PROGRESS).
- nonblocking: client which demonstrates non-blocking write (use make var DEFINES=-DUSE\_SOCKET\_IO to compile network version)
Both utilites are using port 9997 to operate for now.
Todo:
- Enable server program to operate in non-blocking multi connection manner.
- Provide way to specify arbitrary port.

