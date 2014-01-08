Hi Thabet
In order to run my project, I made the assumption of using 3 RMI registries
(localhost, thor, and zeus - and then running multiple instances of FileServers from there)
I have an example setup already in this submission folder
with a simulated LOCAL, THOR, and ZEUS folder to run the respective tests in, 
although you can do however you like - so long as the RMI registry is running in the respective
"src" folders and there is a server_files folder in the same directory as the "src" 
directory there should be no problem.  
The easiest way is to just download a copy of LOCAL on your localhost and placing THOR and ZEUS wherever else.

Here are instructons on how to run it:

1. Set up the rmi registry in the different directories for multiple "virtual" servers
on localhost, thor, and zeus to whichever ports you choose.
2. Run my FileServer.java files - they are already compiled into .class so you should not need to compile
them (note: hello.java must also be included with fileserver.java 
as it is interface for the RMI).
3.  Here you will need to give each server the port to listen on, the port of the RMI registry,
and then also the IP address and RMI port for the other two RMI registries in the system.
After these 6 values are entered, the server will start listening for connection.
4. You can then start up the MyClient.java file.
5.  Here you enter the IP address and port # of the server which you wish to connect to.
You will be presented with the option of a download or an upload (entering a 1
for download or a 2 for upload).
6. In order to test out the functionality, you should try looking for a file which is
not on the server - for example look for slimeserver.jpg on your localhost when it's not
there but it is on Thor as well as Zeus.
7. After you request this file which is not at the server, the server will reply
with the IP address and the Port # of other servers which have the requested file
(along with a lot of debugging messages - sorry).
8.  User can then enter the new ip address and port and download should work then progress.
9. Please note that the client will hang or crash depending if you enter the
wrong IP/port (i.e. you enter another server that doesn't have the file) - also an issue
if you try requesting a file which does not exist on any of the servers - low on time
and couldn't fix these minor issues but the overall functionality should work.
Please do not hesitate to contact me if you have
any issues while running this.
jbarthel@gmu.edu and 571-437-2896
Thanks very much and happy holidays :)
Jeremy 