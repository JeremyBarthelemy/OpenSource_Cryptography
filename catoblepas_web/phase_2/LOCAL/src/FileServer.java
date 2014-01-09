/*
 *Note: This is just a test code for a distributed crypto project I am
 *working on as a hobby.  Must fully test the functionality, then will optimize!
 */


import java.net.Socket;
import java.util.Scanner;
//import javax.swing.JOptionPane;
import java.io.*;
import java.rmi.RemoteException;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.rmi.server.UnicastRemoteObject;
import java.util.Scanner;
import java.net.ServerSocket;
import java.net.InetAddress;

public class FileServer extends Thread implements Hello
{   
    public FileServer() {} //add code to possibly modify this and allow for some fix to give ip and port ?

    public String sayHello(String filename)
    {
    	//check if we have the file
    	
    	//System.out.println("The requested file is : "+filename);
		File f=new File(filename);
    	//File f=new File("../../what.jpg");
    	
    	//System.out.println(filename);
    	if(f.exists())
    	{
    		return "YES";
    	}
    	else
    	{
    		return "NO";
    	}
        
    }
    
    public static void main(String args[]) throws IOException
    {
    	new FileServer().start();
    }
	public void run()
	{
		
		int remote_RMI_port_integer1 = 0;
		int remote_RMI_port_integer2 = 0;
		int RMI_port_integer = 0;
		String IP_Address_Remote_RMI1_string = "";
		String IP_Address_Remote_RMI2_string = "";
		String s = "";
		
		Scanner scanServ = new Scanner(System.in);

	    System.out.println("What is the server port?");
		String port_string = scanServ.nextLine();
	
		int port_value = Integer.parseInt(port_string);
	
		//Setting up connection!
		ServerSocket ss=null;
	    try
	    {  
	        ss=new ServerSocket(port_value);
	    }
	    catch(IOException e)
	    {
	        System.out.println("Couldn't Listen!");
	        System.exit(0);
	    }
	    
	    //Add code here to bind to the RMI Registry
        try 
        {

        	System.out.println("Local RMI Port: ");
			String RMI_port_string = scanServ.nextLine();
			System.out.println("IP Address of Remote Server 1: ");
			IP_Address_Remote_RMI1_string = scanServ.nextLine();
			System.out.println("Remote RMI Registry Port 1: ");
			String remote_RMI_port_string1 = scanServ.nextLine();
			System.out.println("IP Address of Remote Server 2: ");
			IP_Address_Remote_RMI2_string = scanServ.nextLine();
			System.out.println("Remote RMI Registry Port 2: ");
			String remote_RMI_port_string2 = scanServ.nextLine();

	    	InetAddress myIPAddress = InetAddress.getLocalHost();
	    	//System.out.println(myIPAddress);
	    	String hostaddress = myIPAddress.getHostAddress();
	    	String registry_string = "IP: " + hostaddress + " Port: " + port_string;
	    	System.out.println(registry_string);

            FileServer obj = new FileServer();
            Hello stub = (Hello) UnicastRemoteObject.exportObject(obj, 0);
            // Bind the remote object's stub in the registry
            //add code to convert FROM STRING TO INTEGER!!!
            RMI_port_integer = Integer.parseInt(RMI_port_string);
			
			remote_RMI_port_integer1 = Integer.parseInt(remote_RMI_port_string1);
			remote_RMI_port_integer2 = Integer.parseInt(remote_RMI_port_string2);
            
            Registry registry = LocateRegistry.getRegistry(RMI_port_integer);
            registry.bind(registry_string, stub);
            Registry registry_remote1 = LocateRegistry.getRegistry(IP_Address_Remote_RMI1_string, remote_RMI_port_integer1);
            Registry registry_remote2 = LocateRegistry.getRegistry(IP_Address_Remote_RMI2_string, remote_RMI_port_integer2);
            //add code here also for the case of the ip address of the remote host! :)

            System.err.println("Server ready");
        }
        catch (Exception e)
        {
            System.err.println("Server exception: " + e.toString());
            e.printStackTrace();
        }
        
        
	    
    	while(true)
    	{

	        System.out.println("Listening...");
	        
	        Socket cs=null;
	        try
	        { 
	            cs=ss.accept();
	            System.out.println("Connection established"+cs);
	        }
	        catch(Exception e)
	        { 
	            System.out.println("Accept failed");
	            System.exit(1);
	        }
	        try
	        {
		        PrintWriter put=new PrintWriter(cs.getOutputStream(),true);
				BufferedReader st=new BufferedReader(new InputStreamReader(cs.getInputStream()));	
				String x = st.readLine();
				

				
	/***********************************************************************************************
	 ****************************************Client Download!***************************************
	 ***********************************************************************************************/
	    	if(x.equals("1"))
	    	{
	    		System.out.println("\nAttempting to Send File to Client!");
	
				s=st.readLine();
				String testfilename = s;
				System.out.println("The requested file is : "+s);
				File f=new File(s);
	            
				//***********************SERVER HAS FILE**********************/
				if(f.exists()) //server has the file, so we service the request
				{
					put.println("SERVER_HAS_FILE");

					BufferedInputStream d=new BufferedInputStream(new FileInputStream(s));
					BufferedOutputStream outStream = new BufferedOutputStream(cs.getOutputStream());
					
					BufferedReader disb =new BufferedReader(new InputStreamReader(cs.getInputStream()));
					int amountClientRead = Integer.parseInt(disb.readLine());
					System.out.println("\n\n\nAMOUNT THE CLIENT HAS READ!" + amountClientRead);
					if(amountClientRead > 0){System.out.println("\n\nRESUMING DOWNLOAD!!!");}
					
					
					byte buffer[] = new byte[1024];
					int read;
					while((read = d.read(buffer))!=-1)
					{
						outStream.write(buffer, 0, read);
						outStream.flush();
					}
					//put.println("SERVER_HAS_FILE");
					d.close();
					outStream.close();
					System.out.println("File Transferred to Client Succesfully!");
					cs.close();
				}
				//***********************SERVER DOES NOT HAVE REQUESTED FILE**********************/
	            else
	            {
	            	System.out.println("Looks like we don't have the file on this server!");
	            	//This server does not have the requested file!
	            	
	            	 //String host = (args.length < 1) ? null : args[0];
	            	 try
	            	 {
	            	 	System.out.println("\nSending out a broadcast to the other servers to see if they have the file!");
	            	 	//wait until receiving the responses...
	            	 	Registry registry = LocateRegistry.getRegistry(RMI_port_integer);
	            	 	String[] registry_list = registry.list();
	            	 	String[] hasFileList = new String[registry_list.length]; //an array of strings for holding the ip&port of servers that have the file
           				
           				System.out.println("DEBUGGING THIS");
           				
           				Registry registry_remote1 = LocateRegistry.getRegistry(IP_Address_Remote_RMI1_string, remote_RMI_port_integer1);
            			String[] registry_list_remote1 = registry_remote1.list();
	            	 	String[] hasFileList_remote1 = new String[registry_list_remote1.length]; //an array of strings for holding the ip&port of servers that have the file
           				Registry registry_remote2 = LocateRegistry.getRegistry(IP_Address_Remote_RMI2_string, remote_RMI_port_integer2);
            			String[] registry_list_remote2 = registry_remote2.list();
	            	 	String[] hasFileList_remote2 = new String[registry_list_remote2.length]; //an array of strings for holding the ip&port of servers that have the file
           				
           				System.out.println("I got here!!!");
	            	 	//going through all of the entries in the registry, assuming we have fewer than 10000 servers
	            	 	///*****************************************LOCAL check**********************/
	            	 	try
	            	 	{
	            	 		int numHasFile = 0; //the number of mirrors with the requested file
	            	 		int testcount = 0;
	            	 		while(testcount < 10000)
	            	 		{
	            	 			System.out.println(registry_list[testcount]);
	            	 			//add code fix here where you get rid of the hardcoded lookup!  i.e. we will need to broadcast!
	            	 			Hello stub = (Hello) registry.lookup(registry_list[testcount]);
	            	 			//Hello stub = (Hello) registry.lookup("1235");
	            	 			String response = stub.sayHello(testfilename);
	            	 			System.out.println("response: " + response);

	            	 			//response == yes, has file list places the value 
	            	 			if(response.equals("YES"))
	            	 			{   //adds the ip and port values for servers which responded with yes to an array of strings!
	            	 				hasFileList[numHasFile] = registry_list[testcount];
	            	 				//System.out.println(hasFileList[numHasFile]);
	            	 				numHasFile++;
	            	 			}
	            	 			testcount++;
	            	 			//add code for NOW passing the list to the client!
	            	 			//make a list based upon the responses to transmit the (IP Address, Port #)
	            	 			//passing the responses from the broadcast back to the client here
	            	 		}
	            	 	}
	            	 	catch(Exception fail)
	            	 	{
	            	 		System.out.println("registry end");
	            	 	}
	            	 	///*****************************************1st**********************/
	            	 	try
	            	 	{
	            	 		int numHasFile = 0; //the number of mirrors with the requested file
	            	 		int testcount = 0;
	            	 		while(testcount < 10000)
	            	 		{
	            	 			System.out.println(registry_list_remote1[testcount]);
	            	 			//add code fix here where you get rid of the hardcoded lookup!  i.e. we will need to broadcast!
	            	 			Hello stub1 = (Hello) registry_remote1.lookup(registry_list_remote1[testcount]);
	            	 			String response_remote1 = stub1.sayHello(testfilename);
	            	 			System.out.println("response: " + response_remote1);
	            	 			//response == yes, has file list places the value 
	            	 			if(response_remote1.equals("YES"))
	            	 			{   //adds the ip and port values for servers which responded with yes to an array of strings!
	            	 				hasFileList_remote1[numHasFile] = registry_list_remote1[testcount];
	            	 				//System.out.println(hasFileList[numHasFile]);
	            	 				numHasFile++;
	            	 			}
	            	 			testcount++;
	            	 			//add code for NOW passing the list to the client!
	            	 			//make a list based upon the responses to transmit the (IP Address, Port #)
	            	 			//passing the responses from the broadcast back to the client here
	            	 		}
	            	 	}
	            	 	catch(Exception fail)
	            	 	{
	            	 		System.out.println("registry end 2");
	            	 	}
	            	 	////****************************************2nd**********************/
	            	 	try
	            	 	{
	            	 		int numHasFile = 0; //the number of mirrors with the requested file
	            	 		int testcount = 0;
	            	 		while(testcount < 10000)
	            	 		{
	            	 			System.out.println(registry_list_remote2[testcount]);
	            	 			//add code fix here where you get rid of the hardcoded lookup!  i.e. we will need to broadcast!
	            	 			Hello stub2 = (Hello) registry_remote2.lookup(registry_list_remote2[testcount]);
	            	 			String response_remote2 = stub2.sayHello(testfilename);
	            	 			System.out.println("response: " + response_remote2);
	            	 			//response == yes, has file list places the value 
	            	 			if(response_remote2.equals("YES"))
	            	 			{   //adds the ip and port values for servers which responded with yes to an array of strings!
	            	 				hasFileList_remote2[numHasFile] = registry_list_remote2[testcount];
	            	 				//System.out.println(hasFileList[numHasFile]);
	            	 				numHasFile++;
	            	 			}
	            	 			testcount++;
	            	 			//add code for NOW passing the list to the client!
	            	 			//make a list based upon the responses to transmit the (IP Address, Port #)
	            	 			//passing the responses from the broadcast back to the client here
	            	 		}
	            	 	}
	            	 	catch(Exception fail)
	            	 	{
	            	 		System.out.println("registry end 3");
	            	 	}
/*************************FORWARDING DETAILS OF MIRROR SERVERS TO CLIENT********************************************/
						//print what you want to send to the client here from hasFileList arrays of strings for now
						System.out.println("Will send these mirrors to the client: ");	            	 	
	            	 	try //ADD CODE HERE
	            	 	{
	            	 		//display the contents of the three arrays of strings concatenated!
	            	 		//handle the case of no other servers being available
	            	 		
	            	 		String[] combinedHasFileArray = new String[hasFileList.length + hasFileList_remote1.length + hasFileList_remote2.length];
	            	 		System.arraycopy(hasFileList, 0, combinedHasFileArray, 0,  hasFileList.length);
	            	 		System.arraycopy(hasFileList_remote1, 0, combinedHasFileArray, hasFileList.length, hasFileList_remote1.length);
	            	 		System.arraycopy(hasFileList_remote2, 0, combinedHasFileArray, hasFileList.length+hasFileList_remote1.length, hasFileList_remote2.length);
	            	 		
	            	 		put.println("SERVER_NOT_HAVE_FILE");
	            	 		System.out.println("THE CODE GOT HERE!");
	            	 			int i = 0;
								while(i < combinedHasFileArray.length)
								{
									put.println(combinedHasFileArray[i]);
									System.out.println(combinedHasFileArray[i]);  //add code to send this to client!!!
									i++;
								}
								put.println("END!!!");
	            	 	}
	            	 	catch(Exception Noooooooooooo)
	            	 	{
	            	 		System.out.println("Noooooooo");
	            	 	}

	            	 }
	            	 catch (Exception e)
	            	 {
	            	 	System.err.println("Server without file exception: " + e.toString());
	            	 	e.printStackTrace();
	            	 }
	            	 cs.close(); //temporarily have this to close the client socket!
	            	
	            	
	            }
	    	}
	    	
	    	
	    	
	    	
	    	
	    	
	    	
	    	
	    	
	    	
	    	
	    	
	/***********************************************************************************************
	 ****************************************Client Upload!***************************************
	 ***********************************************************************************************/
	    	else if (x.equals("2"))
	    	{
	    		System.out.println("Attempting to Retrieve File from Client!");
				
				s=st.readLine();
				System.out.println("\nThe file to be uploaded is : "+s);
				s = st.readLine();
				System.out.println("\nThe uploading file will be stored here : "+s);
				
				//Take the file from the client!!!
				File test = new File(s);
				/**************************RESUME CASE***********************************/
				if(test.exists())
				{
					System.out.println("File already exists!");
					System.out.println("Resuming!");
					
					//get the amount of bytes already uploaded to the server by the client!
					int amountRead = ((int) test.length()); //amount of bytes already read
					System.out.println("THIS IS THE AMOUNT OF BYTES READ: " + amountRead);
	
					FileOutputStream  fs=new FileOutputStream(s, true);
					BufferedInputStream d=new BufferedInputStream(cs.getInputStream());
	
					//send file size to server
					put.println(amountRead);
	
					byte buffer[] = new byte[1024];
					int read;
					while((read = d.read(buffer))!=-1)
					{
						fs.write(buffer, 0, read);
						fs.flush();
					}
					d.close();
					System.out.println("File taken from client successfully!");
					cs.close();
					fs.close();
				}
				
				/**************************NORMAL CASE***********************************/
				else
				{
					FileOutputStream  fs=new FileOutputStream(s);
					BufferedInputStream d=new BufferedInputStream(cs.getInputStream());
					
					put.println(0); //send the file size as zero to the client so we know to send the full file
					byte buffer[] = new byte[1024];
					int read;
					while((read = d.read(buffer))!=-1)
					{
						fs.write(buffer, 0, read);
						fs.flush();
					}
					d.close();
					System.out.println("File taken from client successfully!");
					cs.close();
					fs.close();
	
				}
	
	    	}
	/***********************************************************************************************
	 ****************************************Mirror Download!***************************************
	 ***********************************************************************************************/	    	
	    	else if (x.equals("3"))
	    	{
	    		System.out.println("Service Discovery!");

	    		//add code here to service the request
	    		//this is where the client has selected this server for download after the initial server
	    		//did not have the file!
	    		//so basically just do a normal download ?
	    		//this server is listening and a request comes in from another server
				//to search the files located at this server
	    	}
	    	
			}
	        catch(Exception e)
	        {
		        System.out.println("error");
	        }
    	}
	}

   
}