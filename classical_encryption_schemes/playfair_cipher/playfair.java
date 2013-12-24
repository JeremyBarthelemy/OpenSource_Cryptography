/*Author: Jeremy Barthelemy
 *Details: This is a very simple text implementation of the classical playfair encryption cipher.
 *This is open-source for educational purposes!  This cipher can be broken
 *extremely easily with modern computers so don't actually use it for encrypting
 *anything other than for fun and learning! :)
 *Possible addition: include passing this to a file or reading from a file
 *Version 0.002: Must read in and optimize message (repeat of letters), encipher/decipher
 */
import java.io.*;
import java.util.Scanner;

public class playfair
{
	public static void main(String[] args)
	{
		char[] alphabet = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
		 + 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
		String alphaToRead = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"; //string used for knowing the letters not in the keyword for placing into the playfair square
		int counter = 0; //used for counting parsing the keyword string
		String optimizedKeyword = ""; //used for storing the value of the improved keyword text
		

		System.out.println("Playfair Cipher in Java");
		System.out.println("Would you like to encrypt a message or decrypt a message?");
		System.out.println("Enter 1 for Encryption and 2 for Decryption: ");
		int x = 0;
		Scanner scan = new Scanner(System.in);
		//Request the keyword
		System.out.println("What is your keyword?");
		String keywordFromUser = scan.nextLine();
		keywordFromUser = keywordFromUser.toUpperCase(); //convert to upper case
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
					if(alphabet[scanAlpha] == 'J')
					{
						alphabet[scanAlpha] = 'I'; // set all the J values to I 
					}
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
		
/*****************************USER ENTERS MESSAGE TEXT*****************************************************************/
		//Request the message
		System.out.println("What is your message to encrypt/decipher?");
		String messageText = scan.next();

/*******************************ENCRYPTION OR DECRYPTION SELECT*******************************************************/		
		//Encryption or Decryption?
		System.out.println("Encryption = 1, Decryption = 2");
		int test = scan.nextInt();
		System.out.println(test);

/*****************************************ENCRYPTION******************************************************************/
		//need to optimize the message by:
		//replacing repeated digraphs with appropriate new message text before encrypting
		
/*****************************************DECRYPTION******************************************************************/

		//optimize the message text for display such that repeated letters show properly

/*******************************APPLY THE KEY TO THE MESSAGE FOR ENCRYPTION / DECRYPTION******************************/
		//Apply key to the message and encrypt / decrypt

	}
}
