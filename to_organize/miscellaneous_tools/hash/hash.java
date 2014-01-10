/*Author: Jeremy Barthélemy
 *Description: We are modifying our Task 2 code so as to print the values of our inputs
 *and outputs (with regard to the VHDL "ROUND" entity) to a separate file for
 *the purpose of verification of testbench results, for all 64 iterations of the for loop!
 *Designed for proof of concept, not for efficiency!
 */
 
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
 
public class hash
{
    public static void main(String[] args)
    {
        
        BufferedWriter bw = null;
        
        
        //Define constant values of initialization vectors, for k array
        final int iv0 = 0x89, iv1 = 0xAB, iv2 = 0xCD, iv3 = 0xEF;
        final int k[] = {0x01, 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89, 0x9A, 0xAB, 0xBC, 0xCD, 0xDE, 0xEF, 0xF0};
        int last_block_stored;
        int last_block;
        int h0, h1, h2, h3;
        int done;
        int counter;
        int temp1, temp2, temp3,test1 = 0;
        int EIGHT_BIT = 256;

        //two blocks of input and output, both will be 16 bits
        //Use the two cases presented in the homework ("NIST" and "VOLGENAU" as inputs for m)
        //Inputs were entered with the MSB being in the lowest array location previously
        //Now switched to LSB in the lowest array location to allow for the same hash as the class
        
        /*********Test Vector 1********/
        int[] m = {0x54, 0x53, 0x49, 0x4E}; //NIST
        int[] next = {0x00, 0x00, 0x00, 0x00};
        /*********Test Vector 2********/
        //int[] m = {0x47, 0x4C, 0x4F, 0x56}; //VOLG
        //int[] next = {0x55, 0x41, 0x4E, 0x45}; //ENAU
        /*********Test Vector 3********/
        //int[m] = {0xDD, 0xCC, 0xBB, 0xAA}; //Correct way
        //int[] next = {0x00, 0x00, 0x00, 0x00};
        /*********Test Vector 4********/
        //int[] m = {0x01, 0x23, 0x45, 0x67}; //NC
        //int[] next = {0x89, 0xAB, 0xCD, 0xEF};
        /*********Test Vector 5********/
        //int[] m = {0x22, 0x55, 0x88, 0xAA}; //NC
        
        //int[] next = {0x8B, 0x83, 0xF1, 0xF3};
        
        int[] r = new int[4]; //8 bit
        int[] y = new int[4];
        
        int ki, ri;
        int a, b, c, d, e, f;

        h0 = iv0;
        h1 = iv1;
        h2 = iv2;
        h3 = iv3;
        last_block_stored = 0;
        last_block = 0;
        counter = 0;
        
        while(last_block_stored != 1)
        {
            //wait until src_ready
            int x = 0;
            while(x < 4)
            {
                r[x] = m[x];
                m[x] = next[x];
                next[x] = 0;
                if(m[x] == 0)
                {
                    counter++;
                }
                else{}
                if(counter == 4)
                {
                    last_block = 1;
                }
                else{}
                x++;
            }
            
            last_block_stored = last_block;
            
            a = h0;
            b = h1;
            c = h2;
            d = h3;
            
            for(int i = 0; i < 64; i++)
            {
                ki = k[i % 16];
                ri = r[i % 4];
                
                String stra = Integer.toHexString(a);
                String strb = Integer.toHexString(b);
                String strc = Integer.toHexString(c);
                String strd = Integer.toHexString(d);
                //String strki = Integer.toHexString(ki);
                String stri = Integer.toHexString(i);
                String str_r0 = Integer.toHexString(r[0]);
                String str_r1 = Integer.toHexString(r[1]);
                String str_r2 = Integer.toHexString(r[2]);
                String str_r3 = Integer.toHexString(r[3]);
                
                if(i < 16)
                {
                    stri = "0" + Integer.toHexString(i);
                }
                if(r[0] < 16)
                {
                    str_r0 = "0" + Integer.toHexString(r[0]);
                }
                if(r[1] < 16)
                {
                    str_r1 = "0" + Integer.toHexString(r[1]);
                }
                if(r[2] < 16)
                {
                    str_r2 = "0" + Integer.toHexString(r[2]);
                }
                if(r[3] < 16)
                {
                    str_r3 = "0" + Integer.toHexString(r[3]);
                }
                if(a < 16)
                {
                    stra = "0" + Integer.toHexString(a);
                }
                if(b < 16)
                {
                    strb = "0" + Integer.toHexString(b);
                }
                if(c < 16)
                {
                    strc = "0" + Integer.toHexString(c);
                }
                if(d < 16)
                {
                    strd = "0" + Integer.toHexString(d);
                }
                
                
                String tv_input = (stri + " " + stra + " " + strb + " " + strc + " " + strd + " " + str_r3
                    + " " +  str_r2 + " " + str_r1 + " " + str_r0 + " ");//for text vector inputs
                String inputs = ("i: 0x" + stri + " a: 0x" + stra + " b: 0x" + strb + " c: 0x" + strc + " d: 0x" + strd
                    + " r: 0x" + str_r3 + str_r2 + str_r1 + str_r0); //for display of inputs
                
                System.out.println(inputs);

                f = (17*ri) % EIGHT_BIT;
                e = (2*d+1)*f % EIGHT_BIT;
                d = (4*c+2)*c % EIGHT_BIT;
                //To rotate the bits 3 times to the left (calculating c = b <<< 3)
                temp1 = b << 3;
                temp2 = b >> 5;
                c = (temp1 | temp2) % EIGHT_BIT;
                //To rotate the bits i times to the right (calculating b = a >>> i)
                if(i % 8 != 0)
                {
                    //Perform shifts
                    temp1 = a >> i%8;
                    temp2 = a << 8-(i%8);
                    //OR the two rotations, cut off the upper bits to obtain rotation
                    b = (temp1 | temp2) % EIGHT_BIT;
                }
                else if(i % 8 == 0)
                {
                    //Don't rotate a by i, simply set b to a
                    b = a;
                }
                else
                {
                    System.out.println("ERROR");
                }
                a = e ^ ki;
                
                String strap = Integer.toHexString(a);
                String strbp = Integer.toHexString(b);
                String strcp = Integer.toHexString(c);
                String strdp = Integer.toHexString(d);
                
                if(a < 16)
                {
                    strap = "0" + Integer.toHexString(a);
                }
                if(b < 16)
                {
                    strbp = "0" + Integer.toHexString(b);
                }
                if(c < 16)
                {
                    strcp = "0" + Integer.toHexString(c);
                }
                if(d < 16)
                {
                    strdp = "0" + Integer.toHexString(d);
                }
                String outputs = ("ap: 0x" + strap + " bp: 0x" + strbp + " cp: 0x" + strcp + " dp: 0x" + strdp + "\n"); //For display
                String tv_output = (strap + " " + strbp + " " + strcp + " " + strdp); //for test vector file
                String tv_display = (tv_input + tv_output);
                System.out.println(outputs);
                
                try
                {
 
                        File file =new File("C:/Users/Jeremy/Desktop/vectortext.txt");
 
                    //if file doesnt exists, then create it
                    if(!file.exists())
                    {
                        file.createNewFile();
                    }
 
                    //Construct the BufferedWriter object
                    bw = new BufferedWriter(new FileWriter(file, true));
 
                    //Start writing to the output stream
                    bw.append(tv_display);
                    bw.append("\n");
 
                    bw.close();
 
                System.out.println("Done");
 
                }
                catch(IOException ex)
                {
                    ex.printStackTrace();
                }
                
            }
            
            //y := h0 || h1 || h2 || h3
            h0 = (h0 + a) % EIGHT_BIT;
            h1 = (h1 + b) % EIGHT_BIT;
            h2 = (h2 + c) % EIGHT_BIT;
            h3 = (h3 + d) % EIGHT_BIT;

            
        }
                    //Adding a '0' before so that the output produced is correct for values 0-15 hex

        String str0 = Integer.toHexString(h0);
        String str1 = Integer.toHexString(h1);
        String str2 = Integer.toHexString(h2);
        String str3 = Integer.toHexString(h3);
        
        if(h0 < 16)
        {
            str0 = "0" + Integer.toHexString(h0);
        }
        if(h1 < 16)
        {
            str1 = "0" + Integer.toHexString(h1);
        }
        if(h2 < 16)
        {
            str2 = "0" + Integer.toHexString(h2);
        }
        if(h3 < 16)
        {
            str3 = "0" + Integer.toHexString(h3);
        }
        
        String result = "0x " + str0 + " " + str1 + " " + str2 + " " + str3;
        System.out.println(result);
    }
} 