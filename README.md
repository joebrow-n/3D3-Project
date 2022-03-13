# P2P
A Peer-to-Peer network application that allows multiple computers or peers to transfer files directly amongst themselves.

# Peer Program
The Peer starts with the declaration of some struct types that set up the necessary structure for the PDUs from the PDU table in Table 1. The Acknowledgement and Error type PDUs use a simpler PDU structure that includes the type ‘A’ or ‘E’ and char type to hold the error message.

Next, the program sets up the UDP and TCP sockets, connecting the UDP socket to the designated address sin. For the TCP socket, a function newAddress() is called, assigning a new, random address with a random port to the TCP socket, then assigning it to an ‘R’ type PDU. This PDU will be used later when registering content to the Index Server. Similarly, the standard Acknowledgement and Error PDUs are created to be used later. The user is asked for their username, which will be attached to the PDU and used to identify to the Index Server who the sender of the PDU was.

The program now starts an infinite while loop, initializing the current set of file descriptors that will be used to manage the program. The file descriptors are reset at the beginning of each loop to check for a new set of inputs. The program is managed using the select function, checking the file descriptors to see if it received an input from either stdin or a TCP connection. Once select has run, if() statements are used to check which bit of the file descriptor was triggered, and depending on the bit, it will enter that if() statement. Once the if statement is complete, it returns to the top of the while() loop.

The first if() statement triggers if the input was stdin. This stdin input will consist of a command: ?, R, T, L, D, O, or Q. ‘?’ Will provide a list of commands and their functions. R will create a PDU of type R, assign the username provided earlier, and have the assigned random address/port created earlier. This information is sent to the Index Server to be verified and stored, then the Peer awaits an Acknowledgement or Error PDU. The program then stores the registered content if an Acknowledgement is received, so that it can be later deregistered if the user decides to quit.

The second case, T, asks the user for the file to deregister. It assigns the name of the file, name of the peer, and the type, ‘T’, to a PDU to be sent to the Index server. The program then awaits an Acknowledgement or Error from the server.

The third case, L, lists out the local content in the peer’s local folder.

The fourth case, ‘D’, is used to request a download from the index server. The user is asked for the file name to be downloaded. An ‘S’ type PDU containing type ‘S’, the peer name, and file name is sent to the Index Server to request the information of where to find this file. The Peer then waits for an Error or type ‘S’ PDU. The ‘S’ type PDU received will contain the peer name, content name, and address of the desired content. A temporary TCP socket is used to connect to the address received. When the connection is successful, receiveFile() is called and a file download process is started. A PDU type ‘D’ is sent to the content server, then the file is downloaded from the content server 100 bytes at a time through PDU type ‘C’. The user can verify who the file was received from by comparing the port numbers that the peer is connected to, and the port that the content was registered to. and Once the file is downloaded, the peer automatically registers the file to become a content server for that file as well. The register process follows the same concept as the R case above, but with a new TCP address and port. The peer awaits an Acknowledgement or Error from the Index server following the Register.

The fifth case, ‘O’, sends a PDU of type ‘O’ to the Index Server, requesting a list of the available online content. The peer awaits an Error or ‘O’ type PDU which contains the list of file names registered on the Index Server and proceeds to print them out.

The last case, ‘Q’, allows the user to quit the program, automatically sending a ‘T’ type PDU to the Index Server, deregistering the registered files that were stored earlier. The peer will receive an Error or Acknowledgement of this process.

By default, if the input is not recognized by the switch statement’s cases, then the program will exit. This also concludes the first if() statement.

The second and third if() statements are checking for the select bits that correspond to incoming TCP connections. If a connection request is made, the peer now acts as a content server, and a new socket is declared and the connection is accepted. The program then calls the sendFile() function which communicates with the earlier receiveFile() function. A PDU type ‘D’ is received then the sendFile() function will send the contents of the file 100 bytes at a time through PDU type ‘C’.

# Server Program
The Server initializes the same PDU structures as the Peer, as to send and receive them. An array of PDUs using the struct used to register content is created to store the PDUs. The Index Server acts as a directory for the peers, storing and handling the information about the registered content.

There are four functions created, the first being addToList(), which is called when a PDU type ‘R’ is received and a peer wants to register some content. The PDU that is sent from the peer is passed into this function, including the PDU type, peer name, content name, and address. The PDU is compared for duplicates, and if there are none, then the PDU is added to an array.

The next function is removeFromList(), which is called when a PDU type ‘T’ is received and a peer wants to deregister some content. The function compares the received PDU for a matching peer name and content name, and when found, the function will remove it from the array.

The next function is searchList(), which is used when an ‘S’ type PDU is received from a peer. The ‘S’ type PDU indicates that the peer is looking for a certain file to download, and so the searchList() will look for that file, then return the details of the PDU with the associated name. If there are files with the same content name, then the more recently registered one will be returned.

The last function is List(), which returns a list of content names in the array that are not duplicates. This function is called when a peer sends a type ‘O’ PDU and is requesting the list of online content.

The rest of the code sets up the UDP socket and initializes the Acknowledgement and Error PDUs. The while() loop contains a similar switch statement as the Peer program, including cases R, S, T, and O. These cases are used depending on the PDU type received. These cases will correspondingly call the aforementioned functions, sending an Acknowledgement to the peer when successful, or Error when unsuccessful. By default, if an unrecognized PDU type is received, the switch statement will exit.
