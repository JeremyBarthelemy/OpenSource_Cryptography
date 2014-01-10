

import java.io.FileWriter;
import java.io.File;
import java.io.IOException;
import java.io.BufferedWriter;
 
public class hash
{
    public static void main(String[] args)
    {
        
        BufferedWriter buffWrite = null;
        final int iv0 = 0x89, iv1 = 0xAB, iv2 = 0xCD, iv3 = 0xEF;
        final int k[] = {0x01, 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89, 0x9A, 0xAB, 0xBC, 0xCD, 0xDE, 0xEF, 0xF0};
        int ki, ri, a, b, c, d, e, f, h0_temp, h1_temp, h2_temp, h3_temp, counter, h0, h1, h2, h3, donelast_block_stored, last_block, last_block_stored;
        int temp1, temp2, temp3,test1 = 0;
        int EIGHT_BIT = 256;
        int[] m = {0x54, 0x53, 0x49, 0x4E};
        int[] next = {0x00, 0x00, 0x00, 0x00};
        int[] r = new int[4];
        int[] y = new int[4];
        
        h0 = iv0;
        h1 = iv1;
        h2 = iv2;
        h3 = iv3;
        h0_temp = h0;
        h1_temp = h1;
        h2_temp = h2;
        h3_temp = h3;
        last_block_stored = 0;
        last_block = 0;
        counter = 0;
        
        while(last_block_stored != 1)
        {
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
                else
                {
                	
                }
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
                f = (17*ri) % EIGHT_BIT;
                e = (2*d+1)*f % EIGHT_BIT;
                d = (4*c+2)*c % EIGHT_BIT;

                temp1 = b << 3;
                temp2 = b >> 5;
                c = (temp1 | temp2) % EIGHT_BIT;

                if(i % 8 != 0)
                {
                    temp1 = a >> i%8;
                    temp2 = a << 8-(i%8);

                    b = (temp1 | temp2) % EIGHT_BIT;
                }
                else if(i % 8 == 0)
                {
                    b = a;
                }
                else
                {
                    System.out.println("ERROR");
                }
                a = e ^ ki;
                                
                h0_temp = (h0 + a) % EIGHT_BIT;
            	h1_temp = (h1 + b) % EIGHT_BIT;
            	h2_temp = (h2 + c) % EIGHT_BIT;
            	h3_temp = (h3 + d) % EIGHT_BIT;
                String stra = Integer.toHexString(a);
                String strb = Integer.toHexString(b);
                String strc = Integer.toHexString(c);
                String strd = Integer.toHexString(d);
                String stri = Integer.toHexString(i);
                String str_r0 = Integer.toHexString(r[0]);
                String str_r1 = Integer.toHexString(r[1]);
                String str_r2 = Integer.toHexString(r[2]);
                String str_r3 = Integer.toHexString(r[3]);
                String str_h0 = Integer.toHexString(h0_temp);
                String str_h1 = Integer.toHexString(h1_temp);
                String str_h2 = Integer.toHexString(h2_temp);
                String str_h3 = Integer.toHexString(h3_temp);
                String str_lb = "0" + last_block;
                String str_lbs = "0" + last_block_stored;
                
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
                if(h0_temp <16)
                {
                	str_h0 = "0" + Integer.toHexString(h0_temp);
                }
                if(h1_temp <16)
                {
                	str_h1 = "0" + Integer.toHexString(h1_temp);
                }
                if(h2_temp <16)
                {
                	str_h2 = "0" + Integer.toHexString(h2_temp);
                }
                if(h3_temp <16)
                {
                	str_h3 = "0" + Integer.toHexString(h3_temp);
                }
                String str_tempy = str_h0 + str_h1 + str_h2 + str_h3;

                String tv_input = (stri + " " + stra + " " + strb + " " + strc + " " + strd + " " + str_r3
                					+ " " +  str_r2 + " " + str_r1 + " " + str_r0 + " " + str_h0 + " " + str_h1 + " " + str_h2
                    				+ " " + str_h3 + " " + str_tempy + " " + str_lb + " " + str_lbs);

                String tv_display = (tv_input);
                
                try
                { 
                    File file =new File("C:/Users/Jeremy/Desktop/vectortext.txt");
 
                    if(!file.exists())
                    {
                        file.createNewFile();
                    }
 
 	                buffWrite = new BufferedWriter(new FileWriter(file, true));
                    buffWrite.append(tv_display);
                    buffWrite.append("\n");
                    buffWrite.close();
 
                }
                catch(IOException ex)
                {
                    ex.printStackTrace();
                }
                
            }
        }
    }
} 