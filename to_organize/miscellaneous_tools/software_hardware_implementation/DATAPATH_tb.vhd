LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE ieee.std_logic_textio.all;
LIBRARY std;
USE std.textio.all;
use IEEE.std_logic_unsigned.all;
use ieee.STD_LOGIC_ARITH.all;


ENTITY DATAPATH_TB IS
END DATAPATH_TB;

ARCHITECTURE DATAPATH_TB_ARCH OF DATAPATH_TB IS

COMPONENT DATAPATH
    PORT(
			m 				  : IN STD_LOGIC_VECTOR(31 DOWNTO 0);
			last_block 	  	  : IN STD_LOGIC_VECTOR(0 DOWNTO 0);			 
			clk 			  : IN STD_LOGIC;
			rst 			  : IN STD_LOGIC;
			sh 				  : IN STD_LOGIC := '1';
			eh 				  : IN STD_LOGIC;
			sn 				  : IN STD_LOGIC;
			en 				  : IN STD_LOGIC;
			er 				  : IN STD_LOGIC;
			ei 				  : IN STD_LOGIC;
			el 				  : IN STD_LOGIC; 
			last_block_stored : OUT STD_LOGIC_VECTOR(0 DOWNTO 0) := "0";
			zi 				  : OUT STD_LOGIC;			
			y 				  : OUT STD_LOGIC_VECTOR(31 DOWNTO 0);
			a_debug			  : OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
			b_debug			  : OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
			c_debug			  : OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
			d_debug			  : OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
			r_debug			  : OUT STD_LOGIC_VECTOR(31 DOWNTO 0)   
    );
END COMPONENT;    

FILE vectorFile: TEXT OPEN READ_MODE is "vectortext.txt";
																
SIGNAL test_r : STD_LOGIC_VECTOR(31 DOWNTO 0);	        
SIGNAL test_m 				  : STD_LOGIC_VECTOR(31 DOWNTO 0);
SIGNAL test_last_block 	  	  : STD_LOGIC_VECTOR(0 DOWNTO 0) := "1";
SIGNAL test_rst 			  : STD_LOGIC := '0';
SIGNAL test_sh 				  : STD_LOGIC := '1';
SIGNAL test_sn 				  : STD_LOGIC := '1';
SIGNAL test_eh 				  : STD_LOGIC := '1';
SIGNAL test_en 				  : STD_LOGIC := '1';
SIGNAL test_er 				  : STD_LOGIC := '1';
SIGNAL test_ei 				  : STD_LOGIC := '1';
SIGNAL test_el 				  : STD_LOGIC := '1'; 
SIGNAL test_last_block_stored : STD_LOGIC_VECTOR(0 DOWNTO 0);
SIGNAL test_zi 				  : STD_LOGIC := '0';			
SIGNAL test_y 				  : STD_LOGIC_VECTOR(31 DOWNTO 0) := "00000000000000000000000000000000";
SIGNAL expected_y			  : STD_LOGIC_VECTOR(31 DOWNTO 0);
SIGNAL test_a_debug			  : STD_LOGIC_VECTOR(7 DOWNTO 0) := "00000000";
SIGNAL test_b_debug			  : STD_LOGIC_VECTOR(7 DOWNTO 0) := "00000000";
SIGNAL test_c_debug			  : STD_LOGIC_VECTOR(7 DOWNTO 0) := "00000000";
SIGNAL test_d_debug			  : STD_LOGIC_VECTOR(7 DOWNTO 0) := "00000000";
SIGNAL expected_a_debug : STD_LOGIC_VECTOR(7 DOWNTO 0);
SIGNAL expected_b_debug : STD_LOGIC_VECTOR(7 DOWNTO 0);
SIGNAL expected_c_debug : STD_LOGIC_VECTOR(7 DOWNTO 0);
SIGNAL expected_d_debug : STD_LOGIC_VECTOR(7 DOWNTO 0);	 			
SIGNAL test_r_debug			  : STD_LOGIC_VECTOR(31 DOWNTO 0);
SIGNAL expected_r_debug : STD_LOGIC_VECTOR(31 DOWNTO 0);		 
SIGNAL TestClk     : STD_LOGIC := '0';
CONSTANT ClkPeriod : TIME := 20 ns;


BEGIN
    
	TestClk <= NOT TestClk AFTER (ClkPeriod/2);
	
    UUT : DATAPATH
    PORT MAP(
				m => test_m, 
				last_block => test_last_block, 			 
				clk => TestClk, 			  
				rst => test_rst, 			  
				sh => test_sh, 				  
				eh => test_eh, 				  
				sn => test_sn, 				  
				en => test_en, 				  
				er => test_er, 				  
				ei => test_ei, 				  
				el => test_el, 				  
				last_block_stored => test_last_block_stored, 
				zi => test_zi, 				  			
				y => test_y, 				  
				a_debug => test_a_debug,			  
				b_debug => test_b_debug,			  
				c_debug => test_c_debug,			  
				d_debug => test_d_debug,			  
				r_debug => test_r_debug		
            );
            
readVec: PROCESS
    VARIABLE     VectorLine:     LINE;
    VARIABLE     VectorValid:     BOOLEAN;
    VARIABLE    v_i:    STD_LOGIC_VECTOR(7 DOWNTO 0);
    VARIABLE    v_a:    STD_LOGIC_VECTOR(7 DOWNTO 0);    
    VARIABLE    v_b:    STD_LOGIC_VECTOR(7 DOWNTO 0);
    VARIABLE    v_c:    STD_LOGIC_VECTOR(7 DOWNTO 0);
    VARIABLE    v_d:    STD_LOGIC_VECTOR(7 DOWNTO 0);      
    VARIABLE    v_r0:   STD_LOGIC_VECTOR(7 DOWNTO 0);
    VARIABLE    v_r1:   STD_LOGIC_VECTOR(7 DOWNTO 0);
    VARIABLE    v_r2:   STD_LOGIC_VECTOR(7 DOWNTO 0);
    VARIABLE    v_r3:   STD_LOGIC_VECTOR(7 DOWNTO 0);
	VARIABLE    v_h0:	STD_LOGIC_VECTOR(7 DOWNTO 0);
	VARIABLE    v_h1:	STD_LOGIC_VECTOR(7 DOWNTO 0);
	VARIABLE    v_h2:	STD_LOGIC_VECTOR(7 DOWNTO 0);
	VARIABLE    v_h3:	STD_LOGIC_VECTOR(7 DOWNTO 0);  
	VARIABLE    v_y:	STD_LOGIC_VECTOR(31 DOWNTO 0);			   
    VARIABLE   space:   CHARACTER;

    
 BEGIN
    
     WHILE NOT ENDFILE (vectorFile) LOOP
      readline(vectorFile, VectorLine); -- put file data into line

     hread(VectorLine, v_i, good => VectorValid);
      NEXT WHEN NOT VectorValid;   
      read(VectorLine, space);               
      hread(VectorLine, v_a);
      read(VectorLine, space);
      hread(VectorLine, v_b);
      read(VectorLine, space);
      hread(VectorLine, v_c);
      read(VectorLine, space);
      hread(VectorLine, v_d);
      read(VectorLine, space);
      hread(VectorLine, v_r3);  
      read(VectorLine, space);
      hread(VectorLine, v_r2);  
      read(VectorLine, space);
      hread(VectorLine, v_r1);  
      read(VectorLine, space);
      hread(VectorLine, v_r0);
	  read(VectorLine, space);
      hread(VectorLine, v_h0);
	  read(VectorLine, space);
      hread(VectorLine, v_h1);
	  read(VectorLine, space);
      hread(VectorLine, v_h2);
	  read(VectorLine, space);
      hread(VectorLine, v_h3);
	  read(VectorLine, space);
      hread(VectorLine, v_y);
	  --read(VectorLine, space);
      --hread(VectorLine, v_lb);
	  --read(VectorLine, space);
      --hread(VectorLine, v_lbs);		
 
 
      WAIT FOR ClkPeriod/4;
      test_m <= v_r3(7 DOWNTO 0) & v_r2(7 DOWNTO 0) & v_r1(7 DOWNTO 0) & v_r0(7 DOWNTO 0);
	  test_r <= v_r3(7 DOWNTO 0) & v_r2(7 DOWNTO 0) & v_r1(7 DOWNTO 0) & v_r0(7 DOWNTO 0);
	  expected_a_debug <= v_a;
	  expected_b_debug <= v_b;
	  expected_c_debug <= v_c;
	  expected_d_debug <= v_d;
	  expected_y <= v_y;
	  
	  
      --expected_ap <= v_ap;      
      WAIT FOR (ClkPeriod/4) * 3;

    END LOOP;
 
     ASSERT FALSE
     REPORT "Simulation complete"
     SEVERITY WARNING;

   WAIT;
END PROCESS;


verify: PROCESS(TestClk)
variable ErrorMsg: LINE;	
BEGIN	 
	
	
							   
		IF (TestClk'event AND TestClk = '0') THEN
			test_sh <= '0';
			test_sh <= TestClk;
			IF(test_y /= expected_y) THEN			
				write(ErrorMsg, STRING'("Vector failed "));
				write(ErrorMsg, now);
				writeline(output, ErrorMsg);
			END IF;
		END IF;

		
END PROCESS;
    
END DATAPATH_TB_ARCH;