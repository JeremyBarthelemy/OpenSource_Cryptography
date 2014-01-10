LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE ieee.std_logic_textio.all;
LIBRARY std;
USE std.textio.all;
use IEEE.std_logic_unsigned.all;
use ieee.STD_LOGIC_ARITH.all;


ENTITY ROUND_TB IS
END ROUND_TB;

ARCHITECTURE ROUND_TB_ARCH OF ROUND_TB IS

COMPONENT ROUND
    PORT(
        a : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
        b : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
        c :    IN STD_LOGIC_VECTOR(7 DOWNTO 0);
        d :    IN STD_LOGIC_VECTOR(7 DOWNTO 0);
        i : IN STD_LOGIC_VECTOR(5 DOWNTO 0);
        r : IN STD_LOGIC_VECTOR(31 DOWNTO 0);                                                                                        
        ap : OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
        bp : OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
        cp : OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
        dp : OUT STD_LOGIC_VECTOR(7 DOWNTO 0)    
    );
END COMPONENT;    

FILE vectorFile: TEXT OPEN READ_MODE is "vectortext.txt";

SIGNAL test_a : STD_LOGIC_VECTOR(7 DOWNTO 0); --:= "10001001";
SIGNAL test_b : STD_LOGIC_VECTOR(7 DOWNTO 0); --:= "10101011";
SIGNAL test_c : STD_LOGIC_VECTOR(7 DOWNTO 0); --:= "11001101";
SIGNAL test_d : STD_LOGIC_VECTOR(7 DOWNTO 0); --:= "11101111";
SIGNAL test_i : STD_LOGIC_VECTOR(5 DOWNTO 0); --:= "000000";
SIGNAL test_r : STD_LOGIC_VECTOR(31 DOWNTO 0); --:= "01001110010010010101001101010100"; --NIST
SIGNAL test_ap : STD_LOGIC_VECTOR(7 DOWNTO 0);
SIGNAL test_bp : STD_LOGIC_VECTOR(7 DOWNTO 0);
SIGNAL test_cp : STD_LOGIC_VECTOR(7 DOWNTO 0);
SIGNAL test_dp : STD_LOGIC_VECTOR(7 DOWNTO 0);
SIGNAL expected_ap : STD_LOGIC_VECTOR(7 DOWNTO 0);
SIGNAL expected_bp : STD_LOGIC_VECTOR(7 DOWNTO 0);
SIGNAL expected_cp : STD_LOGIC_VECTOR(7 DOWNTO 0);
SIGNAL expected_dp : STD_LOGIC_VECTOR(7 DOWNTO 0);       
SIGNAL TestClk     : STD_LOGIC := '0';
CONSTANT ClkPeriod : TIME := 20 ns;


BEGIN
    
	TestClk <= NOT TestClk AFTER (ClkPeriod/2);
	
    UUT : ROUND
    PORT MAP(
            a => test_a,
            b => test_b,
            c => test_c,
            d => test_d,
            i => test_i,
            r => test_r,
            ap => test_ap,
            bp => test_bp,
            cp => test_cp,
            dp => test_dp
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
    VARIABLE    v_ap:    STD_LOGIC_VECTOR(7 DOWNTO 0);
    VARIABLE    v_bp:    STD_LOGIC_VECTOR(7 DOWNTO 0);
    VARIABLE    v_cp:    STD_LOGIC_VECTOR(7 DOWNTO 0);
    VARIABLE    v_dp:    STD_LOGIC_VECTOR(7 DOWNTO 0);           
    VARIABLE   space:         CHARACTER;      
    
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
      hread(VectorLine, v_ap);  
      read(VectorLine, space);
      hread(VectorLine, v_bp);  
      read(VectorLine, space);
      hread(VectorLine, v_cp);  
      read(VectorLine, space);
      hread(VectorLine, v_dp);    
 
      WAIT FOR ClkPeriod/4;
      test_i <= v_i(5 DOWNTO 0);
      test_a <= v_a;
      test_b <= v_b;
      test_c <= v_c;
      test_d <= v_d;
      test_r <= v_r3(7 DOWNTO 0) & v_r2(7 DOWNTO 0) & v_r1(7 DOWNTO 0) & v_r0(7 DOWNTO 0);
      expected_ap <= v_ap;
      expected_bp <= v_bp;
      expected_cp <= v_cp;
      expected_dp <= v_dp;          
      WAIT FOR (ClkPeriod/4) * 3;

    END LOOP;
 
     ASSERT FALSE
     REPORT "Simulation complete"
     SEVERITY WARNING;

   WAIT;
END PROCESS;

   -- Process to verify outputs
verify: PROCESS (TestClk)
variable ErrorMsg: LINE;
BEGIN
	IF (TestClk'event AND TestClk = '0') THEN
		IF((test_ap/=expected_ap) OR(test_bp/=expected_bp) OR (test_cp/=expected_cp) OR (test_dp/=expected_dp)) THEN			
			write(ErrorMsg, STRING'("Vector failed "));
			write(ErrorMsg, now);
			writeline(output, ErrorMsg);
		END IF;
	END IF;
   
END PROCESS;
    
END ROUND_TB_ARCH;