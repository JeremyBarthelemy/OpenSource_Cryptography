/*
 *Note: This is just a test code for a distributed crypto project I am
 *working on as a hobby.  Must fully test the functionality, then will optimize!
 */
 
import java.net.Socket;
import java.util.Scanner;
//import javax.swing.JOptionPane;
import java.io.*;
import java.net.*;


class MyClient extends Thread
{
	public static void main(String srgs[])throws IOException
	{
		new MyClient().start();
	}
	
	public void run()
	{
		//add user input here for: ip, port, to tell where to connect to, then connect
		System.out.println("Enter IP address");
		Scanner scanner = new Scanner(System.in);
		String ip_address = scanner.nextLine();
		//System.out.println(ip_address);
		
		System.out.println("Enter Port");
		String temp = scanner.nextLine();
		int port_number = Integer.parseInt(temp);
		//System.out.println(port_number);
		
		Socket s=null;
		BufferedReader get=null;
		PrintWriter put=null;
		
		//CONNECTING TO SERVER
		try
		{
			s=new Socket(ip_address,port_number);
			get=new BufferedReader(new InputStreamReader(s.getInputStream()));
			put=new PrintWriter(s.getOutputStream(),true);   
		}  
		catch(Exception e)
		{
			System.exit(0);
		}
		
		try
		{
		//add user input for upload/download selection, pass this information to the server
		//Tell the server we are going to do a download here!
		InputStreamReader get2=new InputStreamReader(s.getInputStream());
		String u,f, x;
		System.out.println("Please Enter 1 for file Download and 2 for file Upload.");
		DataInputStream dis0 = new DataInputStream(System.in);
		
		x=dis0.readLine();  //x variable used to tell whether upload or download
		put.println(x);
		
/***********************************************************************************************
 ****************************************DOWNLOADING********************************************
 ***********************************************************************************************/
		if(x.equals("1"))
		{
			System.out.println("Downloading from server...");
			
			System.out.println("Enter the file name to transfer from server:");
			DataInputStream dis=new DataInputStream(System.in);
			f=dis.readLine();
			put.println("../server_files/" + f);
			
			System.out.println("Enter the file name to store locally:");
			DataInputStream dis2 = new DataInputStream(System.in);
			String text = dis2.readLine();
			File f1=new File("../client_files/" + text);

			/****************************RESUME CASE**************************/
			//if the file already exists on the client i.e. it's already been partially downloaded
			if(f1.exists())
			{
				System.out.println("File already exists!");
				System.out.println("Resuming!");
				//number of bytes already downloaded?
				
				int amountRead = ((int) f1.length()); //amount of bytes already read
				System.out.println("THIS IS THE AMOUNT OF BYTES READ: " + amountRead);
				
				
				FileOutputStream  fs=new FileOutputStream(f1, true);
				BufferedInputStream d=new BufferedInputStream(s.getInputStream());
				BufferedOutputStream outStream = new BufferedOutputStream(new FileOutputStream(f1));
				
				put.println(amountRead);
				
				byte buffer[] = new byte[1024];
				int read;
				//int i = 0;
				while((read = d.read(buffer))!=-1)
				{
					//i = i + 1;
					//System.out.println(i);
					outStream.write(buffer, 0, read);
					outStream.flush();
				}
				fs.close();
				System.out.println("File received successfully!");
				s.close();


			}
			
			
			
			/****************************NORMAL CASE**************************/
		
			else
			{					
				FileOutputStream  fs=new FileOutputStream(f1);
				BufferedInputStream d=new BufferedInputStream(s.getInputStream());
				BufferedOutputStream outStream = new BufferedOutputStream(new FileOutputStream(f1));
	
				put.println(0);

				byte buffer[] = new byte[1024];
				int read;
				//int i = 0;
				while((read = d.read(buffer))!=-1)
				{
					//i = i + 1;
					//System.out.println(i);
					outStream.write(buffer, 0, read);
					outStream.flush();
				}
				fs.close();
				System.out.println("File received successfully!");
				s.close();
			}
				
		}
		
		
/***********************************************************************************************
 ****************************************UPLOADING**********************************************
 ***********************************************************************************************/		
		
		else
		{
			System.out.println("Uploading to server...");
			//add upload support for telling which file to upload
			
			System.out.println("Enter the file name to deliver to the server:");
			String fileToUpload = scanner.nextLine();
			System.out.println("filetoUpload:"+ fileToUpload);
			System.out.println("enter the file name for the server to store it as.");
			String serverFilename = scanner.nextLine();
			System.out.println("serverFilename:"+ serverFilename);
			
			File slimeclient = new File("../client_files/" + fileToUpload);
			if(slimeclient.exists())
			{
				//System.out.println("Found the slime client file");
				put.println("../client_files/" + fileToUpload);
				put.println("../server_files/" + serverFilename);
				//upload the slimeclient.jpg file to the server!
				
				FileInputStream fis = new FileInputStream(slimeclient);
				DataOutputStream outStream = new DataOutputStream(s.getOutputStream());
				
				BufferedReader distest =new BufferedReader(new InputStreamReader(s.getInputStream()));
				int amountServerRead = Integer.parseInt(distest.readLine());
				System.out.println("\n\n\nAMOUNT THE SERVER HAS READ!" + amountServerRead);
				if(amountServerRead > 0){System.out.println("\n\nRESUMING UPLOAD!!!");}
				byte buffer[] = new byte[(int) slimeclient.length()]; // new byte[1024];
				int read;
				//int i = amountServerRead;
				while((read = fis.read(buffer))!=-1)
				{
					//i = i + 1;
					//System.out.println(i);
					//System.out.println(buffer[i]);
					System.out.println("amountServerRead: " + amountServerRead);
					System.out.println("buffer.length: " + buffer.length);
					outStream.write(buffer, amountServerRead, buffer.length - amountServerRead - 1);
					outStream.flush();
				}
				fis.close();
				outStream.close();
				System.out.println("File uploaded successfully!");
				s.close();
			}
		
		}
		}
		catch(Exception e)
		{
			System.out.println("An error has occurred");
		}
	}

    
}