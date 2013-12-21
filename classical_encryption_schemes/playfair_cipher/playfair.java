/*Author: Jeremy Barthelemy
 *Details: This is an implementation of the classical playfair encryption cipher.
 *This is open-source for educational purposes!  This cipher can be broken
 *extremely easily with modern computers so don't actually use it for encrypting
 *anything other than for fun and learning! :)
 *Possible addition: include passing this to a file or reading from a file
 */
import java.io.*;
import java.util.Scanner;

public class playfair
{
	public static void main(String[] args)
	{
		char[] alphabetUpper = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
		 + 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
		int counter = 0; //used for counting parsing the keyword string
		String optimizedKeyword = " "; //used for storing the value of the improved keyword text
		

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
			for(int scanAlpha = 0; scanAlpha < alphabetUpper.length; scanAlpha++)
			{
				//System.out.println(keywordFromUser.charAt(counter) + " vs. " + alphabetUpper[scanAlpha]);

				if(optimizedKeyword.indexOf(keywordFromUser.charAt(counter)) != -1)
				{
					//don't re-append the character to the optimized keyword string
				}
				else if(keywordFromUser.charAt(counter) == alphabetUpper[scanAlpha]) //uppercase scan
				{
					optimizedKeyword += alphabetUpper[scanAlpha]; //append the good character to our new keyword!
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


/*****************************USER ENTERS MESSAGE TEXT*****************************************************************/
		//Request the message
		System.out.println("What is your message to encrypt/decipher?");
		String messageText = scan.next();		

/*******************************ENCRYPTION OR DECRYPTION SELECT*******************************************************/		
		//Encryption or Decryption?
		System.out.println("Encryption = 1, Decryption = 2");
		int test = scan.nextInt();
		System.out.println(test);

/*******************************APPLY THE KEY TO THE MESSAGE FOR ENCRYPTION / DECRYPTION******************************/
		//Apply key to the message and encrypt / decrypt

	}
}
