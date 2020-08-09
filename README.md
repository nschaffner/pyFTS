## pyFTS
This program is a simple file transfer system with a file transfer server and a file transfer client:
- The server.c program serves as the file transfer while the client.py program serves as the file transfer client
- The server.c program starts on Host A and validates the command-line parameters that accompany it. Once executed on the port chosen by the user, it will wait for a client request
- The client.py program starts on Host B and validates any pertinent command-line parameters that accompany it
- The server and client programs will establish a TCP control connection on the server port chosen by the user
- The server program will then wait on this connection for the client program to send a command
- The client program will send a command ("-l" or "-g") on this TCP control connection
- When the server receives the command on this connection, it will validate the command:
    - If the command is invalid, the server program sends an error message to the client program on the TCP control connection and the client displays the message on-screen
    - If the command is valid, the server program initiates a TCP data connection with the client program on a port specified by a parameter that accompanied the client program execution on the command-line
- If the client sends the "-l" command, the server program sends its directory to clients on the TCP data connection and the client program displays the directory on-screen
- If the client sends the "-g" command, the server program validates the file name that was sent to it by the client progam
    - If a file with the name sent exists in the directory where the server.c is located, it will send the contents of the named file over the TCP data connection to the client program. The client program will save the contents of the sent file in the current directory and will display a message indicating when the transfer of the file is complete
    - If a file with the name sent does not exits in the directory where the server.c program is located, it will send an error message to the client program on the TCP control connection. The client program will then display an error message on-screen
- The server program will close the TCP data connection and the client program will close the TCP control connection before terminating
- The server program will accept new connections until terminated by a user via SIGINT

### Deployment
Please follow the steps below to run the server.c and client.py programs.
1) Put server.c and client.py into 2 different directories

2) In the first terminal, log into flip1 and run the following 2 commands in the directory containing the server.c file:\
    gcc -o server server.c\
    ./server <port #>

3) In the second terminal, log into flip2 and run the following command in the directory containing the client.py file:\
    chmod +x client.py 

4) If you would like to list the current directory's contents where server.c is located, type the following command:\
    python client.py flip1 <server port #> -l <new port #>

5) If you would like to get the contents of a file where server.c is located, type the following command:\
    python client.py flip1 <server port #> -g <file name> <new port #>

### Notes
- If a connection is closed, the server.c program will continue to run and accept new connections. To stop this program, use SIGINT
- Please use a port # between 1024-65535
- The server.c program was tested on flip1 and the client.py program was tested on flip2 of my university's UNIX servers
