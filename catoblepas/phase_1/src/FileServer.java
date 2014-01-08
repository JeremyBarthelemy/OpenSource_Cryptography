/*
 *Note: This is just a test code for a distributed crypto project I am
 *working on as a hobby.  Must fully test the functionality, then will optimize!
 */
 
import java.net.Socket;
import java.util.Scanner;
//import javax.swing.JOptionPane;
import java.io.*;
import java.util.Scanner;
import java.net.ServerSocket;


public class FileServer extends Thread
{   
    public static void main(String args[]) throws IOException
    {
    	new FileServer().start();

    }
	public void run()
	{
		
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
	
				String s=st.readLine();
				System.out.println("The requested file is : "+s);
				File f=new File(s);
	
				if(f.exists())
				{
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
					d.close();
					outStream.close();
					System.out.println("File Transferred to Client Succesfully!");
					cs.close();
				}
	    	}
	/***********************************************************************************************
	 ****************************************Client Upload!***************************************
	 ***********************************************************************************************/
	    	else
	    	{
	    		System.out.println("Attempting to Retrieve File from Client!");
				
				String s=st.readLine();
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
	    	
			        }
	        catch(Exception e)
	        {
		        System.out.println("error");
	        }
    	}
	}

   
}