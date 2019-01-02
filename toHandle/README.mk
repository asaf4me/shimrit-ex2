EX2 - implemention of http server request
Author details - name : asaf jospeh , id : 203819065
Files - client.c

compile with : gcc -Wall client.c -o client -g

valgrind with :  valgrind --track-origins=yes --tool=memcheck --leak-check=full -v

Description : in this exercise we have implemented an HTTP header request using sockets.

we parsed the command line command :

./client -p blabla -r 2 addr=jecrusalem tel=02-6655443 http://www.google.co.il

make the HTTP header and then send it to the server.

In case of command error the output will be : 

Usage: client -p  -r n <pr1=value1 pr2=value2 ... prN=valueN>  <URL>
