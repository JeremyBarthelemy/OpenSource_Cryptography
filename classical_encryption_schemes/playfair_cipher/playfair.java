/*Author: Jeremy Barthélemy
 *Details: This is a very simple text implementation of the classical playfair encryption cipher.
 *This is open-source for educational purposes!  This cipher can be broken
 *extremely easily with modern computers so don't actually use it for encrypting
 *anything other than for fun and learning! :)
 *Possible addition: include passing this to a file or reading from a file
 *Version: 0.004
 *Fixes:
 *1. Clean the messages such that invalid characters are removed (DONE)
 *2. Optimize the message such that repeated characters are replaced accordingly (In progress, some issues)
 *3. Add the encryption
 *4. Add the decryption
 *Known Issues:
 *1. There will be added information generated at the end of a message for some cases (i.e. multiple Xes)
 *  --> will add a fix for this later
 */
import java.io.*;
import java.util.Scanner;

public class playfair
{
	public static void main(String[] args)
	{
		char[] alphabet = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'K', 'L',
		 + 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
		String alphaToRead = "ABCDEFGHIKLMNOPQRSTUVWXYZ"; //string used for knowing the letters not in the keyword for placing into the playfair square
		int counter = 0; //used for counting parsing the keyword string
		String optimizedKeyword = ""; //used for storing the value of the improved keyword text

		System.out.println("Playfair Cipher in Java by Jeremy Barthélemy 2013");
		System.out.println("Would you like to Encrypt a message or Decrypt a message?");
		System.out.println("Enter 1 for Encryption and 2 for Decryption: ");
		int x = 0;
		Scanner scan = new Scanner(System.in);
		//Request the keyword
		System.out.println("What is your keyword?");
		String keywordFromUser = scan.nextLine();
		keywordFromUser = keywordFromUser.toUpperCase(); //convert to upper case
		keywordFromUser = keywordFromUser.replace(String.valueOf("J"), "I");
/*************************OPTIMIZING THE USER INPUT FOR KEY GENERATION********************************************/

		System.out.println("Your keyword before 'optimizing' it: " + keywordFromUser);
		//remove all letters and characters which are not in the English langauge
		//can perhaps scan through english alphabet array?
		
		while(counter < keywordFromUser.length()) //reading through keyword and comparing to alphabet
		{
			for(int scanAlpha = 0; scanAlpha < alphabet.length; scanAlpha++)
			{
				//System.out.println(keywordFromUser.charAt(counter) + " vs. " + alphabet[scanAlpha]);

				if(optimizedKeyword.indexOf(keywordFromUser.charAt(counter)) != -1)
				{
					//don't re-append the character to the optimized keyword string
				}
				else if(keywordFromUser.charAt(counter) == alphabet[scanAlpha]) //uppercase scan
				{
					alphaToRead = alphaToRead.replace(String.valueOf(alphabet[scanAlpha]), "");
					//remove all occurrences of these letters (twice bc of i and j...I know not coded well but gets job done for this example)
					optimizedKeyword += alphabet[scanAlpha]; //append the good character to our new keyword!
					//in this case we add the letter to the new keyword string
				}
				else{}//don't append the character as it is not a valid character
			}
			counter++;
			//use the string.charAt(#) function to read through the string and if the char is not in the
			//accepted list of letters (i.e. the alphabet for the English language, do not append this
			//to our new string! :)
		}
		System.out.println("This is your optimized keyword: " + optimizedKeyword);


/*****************************BUILDING THE PLAYFAIR SQUARE (THE KEY) FROM THIS KEYWORD*********************************/

		char[][] key = new char [5][5];

		//make toRead string
		//first read in all of the optimizedKeyword characters into toRead
		//then read the alphabet letters which are not in optimizedKeyword
		System.out.println("Alphabet to read: " + alphaToRead);
		for(int y1 = 0; y1 < 5; y1++)
		{
			for(int x1 = 0; x1 <5; x1++)
			{
				if(optimizedKeyword.length() > 0)
				{
					key[x1][y1] = optimizedKeyword.charAt(0);
					//if we have letters in optimizedKeyword, add that letter
					optimizedKeyword = optimizedKeyword.replace(String.valueOf(optimizedKeyword.charAt(0)), "");
					//remove that letter from optimizedKeyword
				}
				else
				{
					key[x1][y1] = alphaToRead.charAt(0);
					//else we will read alphaToRead until all read in
					alphaToRead = alphaToRead.replace(String.valueOf(alphaToRead.charAt(0)),"");
					//remove the letter from alphaToRead
				}
				if(x1 < 4)
				{
					System.out.print(key[x1][y1]);
				}
				else
				{
					System.out.println(key[x1][y1]);
				}
			}
		}
		
/*****************************USER ENTERS MESSAGE TEXT, OPTIMIZED MESSAGE*********************************************/
		//Request the message
		System.out.println("What is your message to encrypt/decipher?");
		String messageText = scan.nextLine();
		messageText = messageText.toUpperCase();
		String optimizedMessage = "";
		String messageWithoutJ = messageText.replace(String.valueOf("J"), "I");;
		
		counter = 0;
		while(counter < messageWithoutJ.length()) //reading through message and comparing to alphabet
		{
			for(int scanAlpha = 0; scanAlpha < alphabet.length; scanAlpha++)
			{
				if(messageWithoutJ.charAt(counter) == alphabet[scanAlpha]) //uppercase scan
				{
					//System.out.println("Compare!" + optimizedMessage.charAt(counter) + ":::" + alphabet[scanAlpha]);
					optimizedMessage += alphabet[scanAlpha]; //append the good character to our new message!
					//in this case we add the letter to the new message string
				}
				else{}//don't append the character as it is not a valid character
			}
			counter++;
		}

		System.out.println("This is your optimized message: " + optimizedMessage);
		


/*******************************ENCRYPTION OR DECRYPTION SELECT*******************************************************/		
		//Encryption or Decryption?
		System.out.println("Please enter 1 for Encryption or 2 for Decryption: ");
		int decision = scan.nextInt();

/*****************************************ENCRYPTION******************************************************************/
		if(decision == 1)
		{
			String finalMessage = ""; //our message without repeated characters in a digraph
			String nextDigraph = ""; //keeps track of the next digraph to encrypt
			System.out.println("Your message to encrypt: " + optimizedMessage);

			//FIRST must optimize message by adding an X at the end if there are an odd number of characters
			
			if((optimizedMessage.length() % 2) == 1) //if there are an odd number of characters,
			{
				//append the 'X' character to the end so we have an even number of individual characters
				optimizedMessage += 'X';
				//System.out.println("new optimized message: " + optimizedMessage);
			}
			//SECOND we must replace repeated letters in a digraph as such: LL becomes two new digraphs - LX followed by L and the next char
			//step through optimized message for two characters...if they are not the same, append them to finalMessage
			//if they are the same, fix and then append

			while((optimizedMessage.length() > 0))
			{
				//System.out.println("From the top: " + optimizedMessage);
				if(optimizedMessage.charAt(0) == optimizedMessage.charAt(1))//repeated characters in a digraph
				{
					//System.out.println("Two repeated characters: " + optimizedMessage.charAt(0) + ":" + optimizedMessage.charAt(1));
					finalMessage += optimizedMessage.charAt(0);
					finalMessage += 'X';
					finalMessage += optimizedMessage.charAt(0);
				}
				else //two different characters in a digraph
				{
					finalMessage += optimizedMessage.charAt(0);
					finalMessage += optimizedMessage.charAt(1);
					//System.out.println("FF: " + finalMessage);
				}
				optimizedMessage = optimizedMessage.substring(2);
			}
			//Note: This is poorly coded and will result in extra characters in some cases -- ISSUE to fix later
			if((finalMessage.length() % 2) == 1) //if there are an odd number of characters,
			{
				//append the 'X' character to the end so we have an even number of individual characters
				finalMessage += 'X';
				//System.out.println("new optimized message: " + optimizedMessage);
			}
			System.out.println("The final message to encrypt is: " + finalMessage);
			/*2. Encrypt using the following rules:
			*2a. if letters in same row, shift right by one position (with wraparound)
			*2b. if letters in same column, shift down by one position (with wraparound)
			*2c. if letters are in neither the same row nor the same column, read in "box fashion"
			*/
			//we will need to scan for the location of characters through the key in order
			//to determine which case to proceed with
			System.out.println("Key[0][0] data: " + key[0][0]);
		}
		
/*****************************************DECRYPTION******************************************************************/
		else if(decision ==2)
		{
		//optimize the message text for display such that repeated letters show properly
		}

/*******************************APPLY THE KEY TO THE MESSAGE FOR ENCRYPTION / DECRYPTION******************************/
		//Apply key to the message and encrypt / decrypt

	}
}
