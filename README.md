# socket-ns-assignment

NOTE: WORKS ONLY IN LINUX

##how to compile:
```code
gcc -o server server.c
gcc -o client client.c
```

##how to execute:
```code
./server bind_ip port work_directory
(eg, if client & server are same machine) ./server 127.0.0.1 5001 /tmp
(eg, if client & server are different machine) ./server 192.168.1.7 5001 /tmp
```

```code
./client server_bind_ip port work_directory
(eg, if client & server are same machine) ./client 127.0.0.1 5001 /tmp
(eg, if client & server are different machine) ./client 192.168.1.7 5001 /tmp
```

note: do not add / at the end of work_directory, correct: "/tmp", wrong: "/tmp/"

##mandatory files should be available in server work_directory:
 - password.db
 - details.txt (hardcoded in code), this will be copied to client

##Issue/Bugs/Improvements:
 - Memory not handled properly
 - Client writes into file by adding "DATA <filesize>" in front of file content
