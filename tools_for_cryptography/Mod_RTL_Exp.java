/*Author: Jeremy Barthelemy
 *Contact: J.L.Barthel@gmail.com
 *Note: This code was not debugged!  There could be possible issues.
 *I ran a few simple test cases that worked, so use at your own risk!
 *If you find any mistakes, please let me know.  Thanks!
 */
import java.util.*;

public class Mod_RTL_Exp
{
	
	public static void main(String[] args)
	{
		//Read in the inputs from the user
		Scanner input = new Scanner (System.in);
        System.out.println ("Input base value: ");
        int base = input.nextInt();
        System.out.println("Input exponential value: ");
        int exponential = input.nextInt();
        System.out.println("Input modulus value: ");
        int modulus = input.nextInt();
        input.close ();
	
	
		//Convert the exponential to a binary string
        String expStr;
        expStr = Integer.toBinaryString(exponential);
        
        //Convert expStr to a character array
        char[] charStr = expStr.toCharArray();
        System.out.println(charStr);
        char[] charStr2 = expStr.toCharArray();
        
        //Reverse the contents of charStr into charStr2
        int i = charStr.length;
        int j = 0;
                
        while(i > 0)
        {
        	charStr2[j] = charStr[i-1];
            i--;
            j++;
        }
        System.out.println(charStr2);
        
        //Now we have the reversed binary string, so we can multiply out
        
        int[] array = new int[charStr.length];
        array[0] = base;
    	            
    	i = charStr.length;
    	j = 0;
    	System.out.print(array[j] + " ");
    	        
    	int[] result = new int[charStr.length];
    	if(charStr2[j] == '1')
    	{
    		result[0] = array[0];
    	}
    	else
    	{
    		result[0] = 1;
    	}

    	j = 1;
    	int finalresult = 1;
    	while(j <= charStr.length-1)
    	{
    		array[j] = ((array[j-1])*(array[j-1])) % modulus;       
    		System.out.print(array[j] + " ");

    		if(charStr2[j] == '1')
    		{
	               		result[j] = array[j];
    		}
    		else
    		{
                		result[j] = 1;
    		}
    		System.out.println("Result[j]: = " + result[j-1]);
    		i--;
    		j++;
    		finalresult *= result[j-1];                	
    	}        

    	finalresult *= result[0];
    	System.out.println("Final Result before mod again: " + finalresult);
        System.out.println("Final Result: " + finalresult % modulus);

	}
	
}