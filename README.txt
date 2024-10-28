SMTP like TCP/FTP

making a mailing system in c languaga using Sockets

HOW TO RUN THE PROGRAM:
run server first check port, gcc server.c -o /server
excute the exe file as ./server
run client , gcc client.c -o /client1
./client1: and register
run client, gcc client.c -o /client2
./client2: and register

now you can send and recieve files between client 1 and 2 
to do more just register more clients and do so

SERVER CODE:
    server will handle 5 things:
        1.  Register_Clients: registers the clients using username, passowrd, and their address and save them in a list
        2.  Authenticate_Clients: authenticate the clients before sending or recieving the file using name and passowrd
        3.  Send_Client_List: show the list of registered clients that can send or recive files
        4.  file_Transfer: Transfer file to one pf the registered clients
        5.  recive_file:  Recieve File from the sender 

CLIENT CODE:
    client will do 4 things:
        1. Register
        2. Recieve
        3. Send
        4. Read client list
        