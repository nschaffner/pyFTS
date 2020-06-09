#!/bin/python

from socket import *
import sys

def validateParamaters():
    if (len(sys.argv) < 5):
        print "Too few arguments."
        exit(1)
    if (len(sys.argv) > 6):
        print "Too many arguments."
        exit(1)
    if (sys.argv[1] != "flip1"):
        if (sys.argv[1] != "flip2"):
            if (sys.argv[1] != "flip3"):
                print "Please use flip1, flip2 or flip3 as the server host name."
                exit(1)
    if (int(sys.argv[2]) > 65535 or int(sys.argv[2]) < 1024):
        print "Please use a port number between 1024-65535."
        exit(1)
    if (sys.argv[3] == "-l" and len(sys.argv) != 5):
        print "Too few arguments for -l command."
        exit(1)
    if (sys.argv[3] == "-l"):
        if (int(sys.argv[4]) > 65535 or int(sys.argv[4]) < 1024):
            print "Please use a port number between 1024-65535."
            exit(1)
    if (sys.argv[3] == "-g" and len(sys.argv) != 6):
        print "Too many arguments for -g command."
        exit(1)
    if (sys.argv[3] == "-g"):
        if (int(sys.argv[5]) > 65535 or int(sys.argv[5]) < 1024):
            print "Please use a port number between 1024-65535."
            exit(1)

def initiateContact():
    # Concatentate the server host name with the path for the host name. I utilized: https://realpython.com/python-string-split-concatenate-join/
    hostName = sys.argv[1] + ".engr.oregonstate.edu"
    # Take the second argument from the command prompt as the port number. I utlized: https://www.geeksforgeeks.org/simple-chat-room-using-python/
    hostPortNum = int(sys.argv[2])
    # Create a socket file descriptor with the address of the socket and the type of socket, which is TCP. I utilized: https://stackoverflow.com/questions/8033552/python-socket-bind-to-any-ip 
    createdClientSocket = socket(AF_INET, SOCK_STREAM)
    # Establish a connection with the server host name and port number provided by the user. I utilized: https://www.geeksforgeeks.org/socket-programming-python/
    createdClientSocket.connect((hostName, hostPortNum))

    return createdClientSocket

def receiveData(newSConnection):
    # If the command input by the user is "-l", the client should expect to receive a list of file names of the directory where the server program is located
    if sys.argv[3] == "-l":
        pLHost = sys.argv[1]
        pLPort = int(sys.argv[4])
        print "Receiving directory structure from {}: {}".format(pLHost, pLPort)
        # Receive the chunk of data sent by the server and set it to the directoryList variable to hold the current file to be displayed. I utilized: https://www.geeksforgeeks.org/simple-chat-room-using-python/
        directoryList = newSConnection.recv(100)

        # While loop will continue until the server program send the "EOD" message, indicating that there are no more files in the current directory to display
        while directoryList != "EOD":
            print directoryList
            directoryList = newSConnection.recv(100)
    # If the command input by the user is "-g", the client should expect to receive the contents of the file named by the user, which matches a file in the directory where the server program is located
    else:
        pGFile = sys.argv[4]
        pGHost = sys.argv[1]
        pGPort = int(sys.argv[5])
        print "Receiving {} from {}: {}".format(pGFile, pGHost, pGPort)
        # Receive the chunk of data sent by the server and set it to the directoryList variable to hold the current file to be displayed. I utilized: https://www.geeksforgeeks.org/simple-chat-room-using-python/
        fileBuffer = newSConnection.recv(100)
        # Open a file with the name indicated by the user to write the data that will be sent by the server program and point the fileName to that file. I utilized: https://www.w3schools.com/python/python_file_write.asp
        fileName = open(sys.argv[4], 'w')
        # This while loop will continue until it finds the sequence "EOF" in the fileBuffer. I utilized: https://www.geeksforgeeks.org/python-membership-identity-operators-not-not/
        while "EOF" not in fileBuffer:
            # Append the content held by fileBuffer to the file pointed to be fileName. I utilized: https://www.w3schools.com/python/python_file_write.asp
            fileName.write(fileBuffer)
            fileBuffer = newSConnection.recv(100)
        print "File transfer complete."

def makeRequest(newSocketFD):
    if (sys.argv[3] == "-l" or len(sys.argv) == 5):
        portNumPos = 4
    else:
        portNumPos = 5

    # Set the portNum variable to the port number input by the user
    portNum = sys.argv[portNumPos]
    # Send a message to the server containing the port number that will be used. I utilized: https://www.geeksforgeeks.org/simple-chat-room-using-python/
    newSocketFD.send(portNum)
    # Receive the initial message sent by server. I utilized: https://www.geeksforgeeks.org/simple-chat-room-using-python/
    newSocketFD.recv(512)
    # Assess the command input by the user and send a message to server containing that command letter. I utilized: https://www.geeksforgeeks.org/simple-chat-room-using-python/
    if (sys.argv[3] == "-l"):
        newSocketFD.send("l")
    elif (sys.argv[3] == "-g"):
        newSocketFD.send("g")
    else:
        newSocketFD.send("a")

    # Receive the next message sent by server. I utilized: https://www.geeksforgeeks.org/simple-chat-room-using-python/
    newSocketFD.recv(512) 

    # Find the IP address for the client to send to server. I utilized: https://stackoverflow.com/questions/166506/finding-local-ip-addresses-using-pythons-stdlib
    s = socket(AF_INET, SOCK_DGRAM)
    s.connect(("8.8.8.8", 80))
    clientIP = s.getsockname()[0]

    # Send a message to the server containing the client's IP address. I utilized: https://www.geeksforgeeks.org/simple-chat-room-using-python/
    newSocketFD.send(clientIP)
    # Receive the next message sent by the server and set it to the status variable. I utilized: https://www.geeksforgeeks.org/simple-chat-room-using-python/
    status = newSocketFD.recv(512)

    if status == "CE":
        print "Invalid command. Please use -l or -g as a command."
        exit(1)
    
    if sys.argv[3] == "-g":
        # If the user command is "-g", send a message to the server containing the name of the file. I utilized: https://www.geeksforgeeks.org/simple-chat-room-using-python/
        newSocketFD.send(sys.argv[4])
        # Receive the next message sent by the server and set it to the verifyFile variable. I utilized: https://www.geeksforgeeks.org/simple-chat-room-using-python/
        verifyFile = newSocketFD.recv(512)
        if verifyFile == "FE":
            pEHost = sys.argv[1]
            pEPort = int(sys.argv[2])
            print "{}: {} says FILE NOT FOUND".format(pEHost, pEPort)
            return
    
    # Create a socket file descriptor with the address of the socket and the type of socket, which is TCP. I utilized: https://stackoverflow.com/questions/8033552/python-socket-bind-to-any-ip 
    newestSocketFD = socket(AF_INET, SOCK_STREAM)
    # Bind the server to an available IP address at specified port number. I utilized: https://stackoverflow.com/questions/8033552/python-socket-bind-to-any-ip and https://www.geeksforgeeks.org/simple-chat-room-using-python/
    newestSocketFD.bind(('', int(portNum)))
    # Listen for 1 connection at a time. I utilized: https://stackoverflow.com/questions/8033552/python-socket-bind-to-any-ip 
    newestSocketFD.listen(1)
    # Accepts a connection request and stores 2 parameters: the socket object for that user and the IP address of the server that has just connected
    # I utilized: https://stackoverflow.com/questions/8033552/python-socket-bind-to-any-ip 
    newSocketConnection, newAddress = newestSocketFD.accept()

    receiveData(newSocketConnection)
    # Close the connection on the socket file descriptor. I utilized: https://www.geeksforgeeks.org/simple-chat-room-using-python/
    newSocketConnection.close()

if __name__ == "__main__":
    validateParamaters()

    socketFD = initiateContact()

    makeRequest(socketFD)
