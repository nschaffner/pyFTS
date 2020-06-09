#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <dirent.h>
#include <fcntl.h>
#include <arpa/inet.h>

/* Global variables */
#define BUFFER_SIZE 100

void error(const char *msg){                                                    /* Error function used for reporting issues */
    perror(msg); 
    exit(1); 
} 

/**************************************************************** 
* Name: getAddressInfo()
* Description: This function receives the server port number as an argument in order to get the address information. It sets 
*               up the hints address info stucture, which is passed to the getaddrinfo() function. The getaddrinfo() function 
*               allocates and initalizes a linked list of addrinfo structures, one for each network address, each of which contains 
*               an address that can be specified in a call to bind or connect. If the call to the getaddrinfo() function is successful, 
*               result will contain a pointer to the start of the list addrinfo structures. This function will return a pointer to the
*               addrinfo structure back to main.
* Resources used: https://docs.microsoft.com/en-us/windows/win32/api/ws2def/ns-ws2def-addrinfoa
*                   https://beej.us/guide/bgnet/html/multi/syscalls.html#getaddrinfo
*                   https://beej.us/guide/bgnet/html/multi/clientserver.html
*                   http://man7.org/linux/man-pages/man3/getaddrinfo.3.html
****************************************************************/
struct addrinfo *getAddressInfo(char *portNum){
    struct addrinfo hints;
    struct addrinfo *result;                                                    /* Will point to the results */
    int status = 0;

    memset(&hints, 0, sizeof hints);                                            /* Ensure that the struct is empty */
    hints.ai_family = AF_INET;                                                  /* Designated IP v4 as the address family that this socket can communicate with */
    hints.ai_socktype = SOCK_STREAM;                                            /* TCP stream socket */
    hints.ai_flags = AI_PASSIVE;                                                /* Fill in the IP for us */

    status = getaddrinfo(NULL, portNum, &hints, &result);                       /* I utilized Beej's Guide: https://beej.us/guide/bgnet/html/multi/syscalls.html#getaddrinfo */
    
    if(status != 0){                                                            /* Print out an error message and exit if the call to the getaddrinfo() function was unsuccessful */
        error("Erroneous port number.\n");
    }

    return result;                                                              /* Return the response information */
}

/**************************************************************** 
* Name: createSocket()
* Description: This function receives a pointer to the addrinfo structure as an argument. This function will call the socket() function to 
*               create a socket, which will return a socket file descriptor. This function will return the socket file descriptor 
*               back to main.
*Resources used: https://beej.us/guide/bgnet/html/multi/syscalls.html#getaddrinfo
*                   https://beej.us/guide/bgnet/html/multi/clientserver.html
****************************************************************/
int createSocket(struct addrinfo *getInfo){
    int status = 0;

    status = socket(getInfo->ai_family, getInfo->ai_socktype, getInfo->ai_protocol);    /* I utilized Beej's Guide: https://beej.us/guide/bgnet/html/multi/syscalls.html#getaddrinfo */
    
    if(status == -1){                                                           /* Print out an error message and exit if the call to the socket() function was unsuccessful */
        error("Error in creating socket.\n");
    }

    return status;                                                              /* Return the socket file descriptor */
}

/**************************************************************** 
* Name: getNewAddressInfo()
* Description: This function receives the client address and client port number as arguments in order to get the address information. It sets 
*               up the hints address info stucture, which is passed to the getaddrinfo() function. The getaddrinfo() function 
*               allocates and initalizes a linked list of addrinfo structures, one for each network address, each of which contains 
*               an address that can be specified in a call to bind or connect. If the call to the getaddrinfo() function is successful, 
*               result will contain a pointer to the start of the list addrinfo structures. This function will return a pointer to the
*               addrinfo structure back to main.
* Resources used: https://docs.microsoft.com/en-us/windows/win32/api/ws2def/ns-ws2def-addrinfoa
*                   https://beej.us/guide/bgnet/html/multi/syscalls.html#getaddrinfo
*                   https://beej.us/guide/bgnet/html/multi/clientserver.html
*                   http://man7.org/linux/man-pages/man3/getaddrinfo.3.html
****************************************************************/
struct addrinfo *getNewAddressInfo(char *inAdd, char *portNum){
    struct addrinfo hints;
    struct addrinfo *result;                                                    /* Will point to the results */
    int status = 0;

    memset(&hints, 0, sizeof hints);                                            /* Ensure that the struct is empty */
    hints.ai_family = AF_INET;                                                  /* Designated IP v4 as the address family that this socket can communicate with */
    hints.ai_socktype = SOCK_STREAM;                                            /* TCP stream socket */

    status = getaddrinfo(inAdd, portNum, &hints, &result);                      /* I utilized Beej's Guide: https://beej.us/guide/bgnet/html/multi/syscalls.html#getaddrinfo */
    
    if(status != 0){                                                            /* Print out an error message and exit if the call to the getaddrinfo() function was unsuccessful */
        error("Error in getting address.\n");
    }

    return result;                                                              /* Return the response information */
}

/**************************************************************** 
* Name: connectToSocket()
* Description: This function receives a socket file descriptor and a pointer to the addrinfo structure as arguments. This function 
*               will call the connect() function to create a connection. This function will not return any values back to main.
* Resources used: https://beej.us/guide/bgnet/html/multi/syscalls.html#getaddrinfo 
****************************************************************/
void connectToSocket(int socketFD, struct addrinfo *getInfo){
    int status = 0;

    status = connect(socketFD, getInfo->ai_addr, getInfo->ai_addrlen);          /* I utilized Beej's Guide: https://beej.us/guide/bgnet/html/multi/syscalls.html#getaddrinfo */
    
    if(status == -1){
        error("Error connecting.\n");
    }
}

/**************************************************************** 
* Name: handleRequest()
* Description: This function receives a socket file descriptor and will handle the request that is sent by the client program.
*               Once the TCP control connection has been established, the server program will wait for the client program
*               to send a command, which will be accompanied by the port that will server as the TCP data connection. This function
*               will assess the data that is sent by the client program to either send a list of the files in the current directory,
*               send the contents of a file that matches the file name sent by the client program or send an appropriate error message. 
*               The function will also close the TCP data connection once the applicable information has been sent to client. This 
*               function will not return any values back to main.
* Resources used: https://beej.us/guide/bgnet/html/multi/syscalls.html#getaddrinfo 
*                   https://www.gnu.org/software/libc/manual/html_node/Directory-Entries.html 
*                   https://www.geeksforgeeks.org/c-program-list-files-sub-directories-directory/
*                   http://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html
*                   https://www.geeksforgeeks.org/input-output-system-calls-c-create-open-close-read-write/ 
*                   https://stackoverflow.com/questions/2014033/send-and-receive-a-file-in-socket-programming-in-linux-with-c-c-gcc-g
****************************************************************/
void handleRequest(int new_socketFD){
    char portNum[BUFFER_SIZE];
    char command[BUFFER_SIZE];
    char hostName[BUFFER_SIZE];
    char fileName[BUFFER_SIZE];   
    char *commandError = "CE";
    char *fileError = "FE";
    char *noCommandError = "NCE";
    char *noFileError = "NFE";
    char *noError = "NE";
    char *endOfDirectory = "EOD";
    char *endOfFile = "EOF";
    int charsWritten, charsRead;
    int directoryCount = 0;
    int fileCount = 0;
    int matchingFile = 0;
    int newestSocketFD;
    int writtenStatus = 0;
    int readStatus = 0;
    int i = 0;

    /* Receive message from client to set up handshake for TCP connection */
    memset(portNum, 0, sizeof(portNum));                                        /* Ensure that the portNum array is empty */
    charsRead = recv(new_socketFD, portNum, sizeof(portNum) - 1, 0);            /* Read data from the socket */

    if (charsRead < 0){
        error("ERROR reading from socket");
    }

    /* Send message to client to set up handshake for TCP connection */
    charsWritten = send(new_socketFD, noError, strlen(noError), 0);             /* Write to the client */

    if (charsWritten < 0){
        error("ERROR writing to socket");
    }

    /* Receive the command from client in the TCP connection */
    memset(command, 0, sizeof(command));
    charsRead = recv(new_socketFD, command, sizeof(command) - 1, 0);            /* Read data from the socket */

    if (charsRead < 0){
        error("ERROR reading from socket");
    }

    send(new_socketFD, noError, strlen(noError), 0);                            /* Write to client to inform it that no errors have occurred thus far */

    /* Receive the hostName from client in the TCP connection */
    memset(hostName, 0, sizeof(hostName));
    charsRead = recv(new_socketFD, hostName, sizeof(hostName) - 1, 0);          /* Read data from the socket */

    if (charsRead < 0){
        error("ERROR reading from socket");
    }

    printf("Connection from flip2 at %s.\n", hostName);

    if(strcmp(command, "l") == 0){                                              /* If statement to assess whether the command sent by client was "-l" */
        send(new_socketFD, noCommandError, strlen(noCommandError), 0);          /* Write to client to inform it that the command that was sent was successfully received */
        printf("List directory requested on port %s.\n", portNum);
        printf("Sending directory contents to flip2 at %s: %s\n", hostName, portNum);

        char **directoryArray = malloc(BUFFER_SIZE*sizeof(char *));             /* Create and allocate memory for a pointer to a pointer to directoryArray char in order to hold each of the directory file names */
        
        for(i = 0; i < BUFFER_SIZE; i++){                                       /* For loop to allocate memory and ensure that each char array pointed to by directoryArray is empty */
            directoryArray[i] = malloc(BUFFER_SIZE*sizeof(char *));
            memset(directoryArray[i], 0, sizeof(directoryArray[i]));
        }

        /* Please note, for this section, I referred to the following resources:
        *   1) https://www.gnu.org/software/libc/manual/html_node/Directory-Entries.html 
        *   2) https://www.geeksforgeeks.org/c-program-list-files-sub-directories-directory/
        *   3) http://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html */
        struct dirent *de;
        DIR *dr = opendir(".");                                                 /* opendir returns a pointed of DIR type, which dr will now point to. I referenced #2 above */
        if(dr != NULL){
            while((de = readdir(dr)) != NULL){                                  /* readdir() returns a pointer to a structre representing the directory entry at the current position in the director stream. I referenced #2 above */
                if(de->d_type == DT_REG){                                       /* Verify if the current position pointed to is a regular file in the directory. I referenced #1 above */
                    strcpy(directoryArray[directoryCount], de->d_name);         /* Copy the name of the file pointed to in the directory into the current element in the directoryArray variable */
                    directoryCount++;                                           /* Increment the variable holding the number of files in the current directory */
                }
            }
        }
        closedir(dr);                                                           /* Close the current directory */

        struct addrinfo *newAddInfo1 = getNewAddressInfo(hostName, portNum);    /* Call the getNewAddressInfo() function and return a pointer to newAddrinfo structure */

        newestSocketFD = createSocket(newAddInfo1);                             /* Call the createSocket() function to create a socket and return a socket file descriptor */

        connectToSocket(newestSocketFD, newAddInfo1);                           /* Call the connectToSocket() function to set up a connection */

        for(i = 0; i < directoryCount; i++){                                    /* For loop to send each of the file names at each position in the directoryArray[] to client */
            writtenStatus = send(newestSocketFD, directoryArray[i], BUFFER_SIZE, 0);  /* Write to client */

            if(writtenStatus == -1){
                error("Error occurred. File name not sent.\n");
            }
        }

        send(newestSocketFD, endOfDirectory, strlen(endOfDirectory), 0);        /* Inform client that there are no more file names to send */

        close(newestSocketFD);                                                  /* Close the connection on the socket file descriptor. I utilized Beej's Guide: https://beej.us/guide/bgnet/html/multi/syscalls.html#getaddrinfo */

        freeaddrinfo(newAddInfo1);                                              /* Free the linked list allocated by getaddrinfo(). I utilized Beej's Guide: https://beej.us/guide/bgnet/html/multi/syscalls.html#getaddrinfo */

        for(i = 0; i < BUFFER_SIZE; i++){                                       /* Free the memory that was previously allocated to each char array pointed to by the directoryArray */
            free(directoryArray[i]);
        }
        free(directoryArray);                                                   /* Free the memory that was previously allocated for the pointer to a pointer to directoryArray char */

    }
    else if(strcmp(command, "g") == 0){                                         /* If statement to assess whether the command sent by client was "-g" */
        send(new_socketFD, noCommandError, strlen(noCommandError), 0);          /* Write to client to inform it that the command that was sent was successfully received */

        memset(fileName, 0, sizeof(fileName));                                  /* Ensure that fileName is empty */

        readStatus = recv(new_socketFD, fileName, sizeof(fileName) - 1, 0);     /* Read data from the socket and assign it to fileName */

        if(readStatus == -1){
            error("Error occurred. File name not received.\n");
        }

        printf("File %s requested on port %s.\n", fileName, portNum);

        char **fileArray = malloc(BUFFER_SIZE*sizeof(char *));                  /* Create and allocate memory for a pointer to a pointer to fileArray char in order to hold each of the file names */
        
        for(i = 0; i < BUFFER_SIZE; i++){                                       /* For loop to allocate memory and ensure that each char array pointed to by fileArray is empty */
            fileArray[i] = malloc(BUFFER_SIZE*sizeof(char *));
            memset(fileArray[i], 0, sizeof(fileArray[i]));
        }

        /* Please note, for this section, I referred to the following resources:
        *   1) https://www.gnu.org/software/libc/manual/html_node/Directory-Entries.html 
        *   2) https://www.geeksforgeeks.org/c-program-list-files-sub-directories-directory/
        *   3) http://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html */
        struct dirent *de;
        DIR *dr = opendir(".");                                                 /* opendir returns a pointed of DIR type, which dr will now point to. I referenced #2 above */
        if(dr){
            while((de = readdir(dr)) != NULL){                                  /* readdir() returns a pointer to a structre representing the directory entry at the current position in the director stream. I referenced #2 above */
                if(de->d_type == DT_REG){                                       /* Verify if the current position pointed to is a regular file in the directory. I referenced #1 above */
                    strcpy(fileArray[fileCount], de->d_name);                   /* Copy the name of the file pointed to in the directory into the current element in the fileArray variable */
                    fileCount++;                                                /* Increment the variable holding the number of files in the current directory */
                }
            }
        }
        closedir(dr);                                                           /* Close the current directory */

        for(i = 0; i < fileCount; i++){                                         /* For loop to compare the file name that was passed to server with the each of the file names that are in the current directory */
            if(strcmp(fileArray[i], fileName) == 0){
                matchingFile = 1;                                               /* If one of the file names in the current directory matches the file name that was passed to server, set matchingFile = 1 */
            }
        }

        if(matchingFile){                                                       /* If statement to assess whether one of the files in the current directory matches the file named passed to server */
            printf("Sending %s to flip2 at %s: %s\n", fileName, hostName, portNum);

            send(new_socketFD, noFileError, strlen(noFileError), 0);            /* Inform client that server found the file */

            struct addrinfo *newAddInfo2 = getNewAddressInfo(hostName, portNum);     /* Call the getNewAddressInfo() function and return a pointer to newAddrinfo structure */

            newestSocketFD = createSocket(newAddInfo2);                         /* Call the createSocket() function to create a socket and return a socket file descriptor */

            connectToSocket(newestSocketFD, newAddInfo2);                       /* Call the connectToSocket() function to set up a connection */

            char buffer[BUFFER_SIZE];
            memset(buffer, 0, sizeof(buffer));                                  /* Ensure that buffer is empty */

            int openedFile = open(fileName, O_RDONLY);                          /* Open the file name held by fileName in the current directory and set the file descriptor equal to the returned value */                
                                                                                /* I utilized: https://www.geeksforgeeks.org/input-output-system-calls-c-create-open-close-read-write/ */
            /* Please note, for the following while loop section I utilized: https://stackoverflow.com/questions/2014033/send-and-receive-a-file-in-socket-programming-in-linux-with-c-c-gcc-g */
            while(1){
                int readBytes = read(openedFile, buffer, sizeof(buffer) - 1);   /* Read data into the buffer and store how many bytes were read in readBytes */

                if(readBytes == 0){                                             /* if readBytes == 0, we are done reading from the file */
                    break;
                }

                if(readBytes < 0){
                    error("Error occurred with reading the file contents.\n");
                }

                void *filePtr = buffer;                                         /* filePtr will keep track of where in the buffer we currently are */

                while(readBytes > 0){                                           /* While loop is used because not all of the data may be sent in a single call */
                    int writtenBytes = send(newestSocketFD, filePtr, sizeof(buffer), 0);    /* Send the current chunk of data to client and store how many bytes were sent in writtenBytes */
                    if(writtenBytes < 0){
                        error("Error occurred with sending the file contents.\n");
                    }
                    readBytes -= writtenBytes;                                  /* Decrement the number of readBytes by the number of writtenBytes*/
                    filePtr += writtenBytes;                                    /* Move filePtr to point to the next chunk of data in the buffer */
                }

                memset(buffer, 0, sizeof(buffer));                              /* Ensure that buffer is empty */
            }

            send(newestSocketFD, endOfFile, strlen(endOfFile), 0);              /* Inform client that all the file contents have been sent */

            close(newestSocketFD);                                              /* Close the connection on the socket file descriptor. I utilized Beej's Guide: https://beej.us/guide/bgnet/html/multi/syscalls.html#getaddrinfo */

            freeaddrinfo(newAddInfo2);                                          /* Free the linked list allocated by getaddrinfo(). I utilized Beej's Guide: https://beej.us/guide/bgnet/html/multi/syscalls.html#getaddrinfo */
        }    
        else{                                                                   /* Otherwise, there is not a file in the current directory that matches the file name sent by client */
            printf("File not found. Sending error message to flip2 at %s: %s\n", hostName, portNum);

            send(new_socketFD, fileError, strlen(fileError), 0);                /* Inform client that the file could not be found */
        }

        for(i = 0; i < BUFFER_SIZE; i++){                                       /* Free the memory that was previously allocated to each char array pointed to by the fileArray */
            free(fileArray[i]);
        }
        free(fileArray);                                                        /* Free the memory that was previously allocated for the pointer to a pointer to fileArray char */
    }
    else{
        printf("Received invalid command.\n");
        send(new_socketFD, commandError, strlen(commandError), 0);              /* Inform client that an invalid command was sent */
    }

    printf("Connection closed. Wait for new connection.\n");
}

/* Please note, I referenced Beej's Guide: https://beej.us/guide/bgnet/html/multi/syscalls.html#getaddrinfo */
int main(int argc, char *argv[]){
    int socketFD, new_socketFD;
    int status = 0;
    struct sockaddr_storage their_addr;                                         /* I utilized Beej's Guide: https://beej.us/guide/bgnet/html/multi/syscalls.html#getaddrinfo */
    socklen_t addr_size;

    if (argc != 2){                                                             /* Verify if the correct number of arguments were used. There should be 3 arguments, which are the command, the server address and the port # */
        error("Incorrect number of arguments.\n");                                                      
    }
    else{
        printf("Server open on %s\n", argv[1]);
    }

    struct addrinfo *addInfo = getAddressInfo(argv[1]);                         /* Call the getAddressInfo() function and return a pointer to addrinfo structure */

    socketFD = createSocket(addInfo);                                           /* Call the createSocket() function to create a socket and return a socket file descriptor */
  
    status = bind(socketFD, addInfo->ai_addr, addInfo->ai_addrlen);             /* Call bind() method to bind socket to host the server is running on. I utilized Beej's Guide: https://beej.us/guide/bgnet/html/multi/syscalls.html#getaddrinfo */

    if(status == -1){                                                           /* Close the socket, print out an error message and exit if the call to the bind() function was unsuccessful */
        close(socketFD);
        error("Error in binding socket to port.\n");
    }

    status = 0;

    status = listen(socketFD, 3);                                               /* Call listen() method to listen to incoming connections on the specified port. I specified 3 connections on the incoming queue */

    if(status == -1){                                                           /* Close the socket, print out an error message and exit if the call to the listen() function was unsuccessful */
        close(socketFD);                                                        
        error("Error in listening on bound socket.\n");
    }

    while(1){                                                                   /* Accept the incoming connection. I utilized Beej's Guid for the folloiwng section: https://beej.us/guide/bgnet/html/multi/syscalls.html#getaddrinfo */
        addr_size = sizeof(their_addr);
        new_socketFD = accept(socketFD, (struct sockaddr *)&their_addr, &addr_size);

        if(new_socketFD == -1){
            continue;
        }
        handleRequest(new_socketFD);                                            /* Call the handleRequest() function to handle the incoming request from the client */
        close(new_socketFD);                                                    /* Close the connection on the socket file descriptor. I utilized Beej's Guide: https://beej.us/guide/bgnet/html/multi/syscalls.html#getaddrinfo */
    }

    freeaddrinfo(addInfo);                                                      /* Free the linked list allocated by getaddrinfo(). I utilized Beej's Guide: https://beej.us/guide/bgnet/html/multi/syscalls.html#getaddrinfo */

    return 0;
}
