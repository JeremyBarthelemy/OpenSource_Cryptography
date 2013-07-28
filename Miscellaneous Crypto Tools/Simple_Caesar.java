/*Author: Jeremy Barthelemy
 *Description: A simple Caesar Cipher, which can be modified to function for Android applications
 *Version: 2.0
 *Upgrades to make:
 *Allow for lowercase letters, spaces, symbol inputs, and numbers
 *Improve Commenting
 */

//Basically you take the alphabet array element at the key value, and from there, populate the shifted alphabet array
//from 0 onward to 25.  When you reach 25 reading the normal alphabet, you should go back to to 0 and all the way to key - 1


import java.util.Scanner;

public class CipherCaesar
{
      public static void main (String [] args)
      {

        Scanner keyboard = new Scanner(System.in); //Initialize the scanner, which is used to read user input
        
        System.out.println("Enter 1 to Encode ");
        System.out.println("Enter 2 to Decode ");
        int decision = Integer.parseInt(keyboard.nextLine()); //Reads just the next int and consumes the newline return so that
        //there is no problem for the String text nextLine() return
        
        if(decision == 1)
        {
            System.out.println("What would you like to encode?");
            String plaintext = keyboard.nextLine();
            System.out.println(plaintext);            
            System.out.println("What is your key?");
            int key = Integer.parseInt(keyboard.nextLine());
            System.out.println(key);
            
            Encode(plaintext, key);  //Calls the Encode(String, int) method
        }
        else if(decision == 2)
        {
            System.out.println("What would you like to decode?");
            String ciphertext = keyboard.nextLine(); //Not nextString, but next, Need to allow for multiple words
            System.out.println(ciphertext);
            
            System.out.println("What is your key?");
            int key = Integer.parseInt(keyboard.nextLine());
            System.out.println(key);
            
            Decode(ciphertext, key);  //Calls the Decode(String, int) method
        }
        else
        {
            System.out.println("Invalid Input");
        }
      }
      
      public static void Encode(String plaintext, int keycode)
      {
          //Used to store the normal alphabet values
          char[] alphabet = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
          //the shifted alphabet, which we will shift later depending upon the key
          char[] shifted = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
          
//**********This code is used to reposition the letters to find the shifted alphabet
          int count = keycode;
          int currentIndex = 0;
          
          while(count < 26)
          {
              shifted[currentIndex] = alphabet[count];
              //System.out.println(shifted[currentIndex]);
              count++;
              currentIndex++;
          }
          count = 0;
          while(count < keycode)
          {
              shifted[currentIndex] = alphabet[count];
              //System.out.println(shifted[currentIndex]);
              count++;
              currentIndex++;
          }
//**********
          
          //Here we will need to use the alphabet and the shifted alphabet to determine the ciphertext from the plaintext
          int j = 0;
          int k = 0;
          int currentLetter = 0; //used to find the current letter of the alphabet we are using
          String currentCipherText = ""; //used to keep track of the current cipher text
          char c; //temporary char
          String s1; //temporary string
          while(j < plaintext.length())
          {
              //read the character, get the location of that letter in the alphabet, then take that location's letter to encrypt
              while(k < 26) //go through the alphabet to find the location of the matching letter
              {
                  if(plaintext.charAt(j) == alphabet[k])
                  {
                      currentLetter = k;
                  }
                  k++;
              }
              //now that we have found the current letter's location, we simply add the letter to the ciphertext string
              c = shifted[currentLetter];
              s1 = Character.toString(c);
              currentCipherText += s1;
              j++;
              k = 0;
          }
          System.out.println("Ciphertext: " + currentCipherText); //Output the enciphered text
      }
      
      public static void Decode(String ciphertext, int keycode)
      {
          //Used to store the normal alphabet values
          char[] alphabet = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
          //Used to store the current shifted alphabet, depending upon the key (number of letters to shift)
          //We will shift this later on depending upon the key
          char[] shifted = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
          int count = keycode;
          int currentIndex = 0;

//**********This code is used to reposition the letters to find the shifted alphabet          
          while(count < 26)
          {
              shifted[currentIndex] = alphabet[count];
              System.out.println(shifted[currentIndex]);
              count++;
              currentIndex++;
          }
          count = 0;
          while(count < keycode)
          {
              shifted[currentIndex] = alphabet[count];
              System.out.println(shifted[currentIndex]);
              count++;
              currentIndex++;
          }
//**********
//Here we will need to use the alphabet and the shifted alphabet to determine the plaintext from the ciphertext

          int j = 0;
          int k = 0;
          int currentLetter = 0; //used to find the current letter of the alphabet we are using
          String currentPlainText = ""; //used to keep track of the current cipher text
          char c; //temporary char
          String s1; //temporary string
          while(j < ciphertext.length())
          {
              //read the character, get the location of that letter in the alphabet, then take that location's letter to encrypt
              while(k < 26) //go through the alphabet to find the location of the matching letter
              {
                  if(ciphertext.charAt(j) == shifted[k])
                  {
                      currentLetter = k;
                  }
                  k++;
              }
              //now that we have found the current letter's location, we simply add the letter to the ciphertext string
              //Need to convert the individual characters to strings to add to the currentPlainText string, seen below
              c = alphabet[currentLetter];
              s1 = Character.toString(c);
              currentPlainText += s1;
              j++;
              k = 0;
          }
          System.out.println("Plaintext: " + currentPlainText); //Output the decoded result
      }
