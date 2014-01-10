--Bit Serial Decrementer

LIBRARY ieee;
USE ieee.std_logic_1164.all;

ENTITY BSD IS
	PORT(					 
			X : IN STD_LOGIC;
			START : IN STD_LOGIC;
			CLK : IN STD_LOGIC;
			C : OUT STD_LOGIC;
			V : OUT STD_LOGIC;
			S : OUT STD_LOGIC
		);
END BSD;

ARCHITECTURE bsd_arch OF BSD IS		
SIGNAL C_Temp : STD_LOGIC;	
SIGNAL REG_OUT : STD_LOGIC;	   
SIGNAL MUX_OUT : STD_LOGIC; --Controlled by start to determine if we give C0 (initial carry), or every other carry

BEGIN						  
	
MHA : ENTITY work.MHA(mha_arch)
	  PORT MAP(X => X,Y => MUX_OUT, C => C_Temp,S=>S);	 


REG : ENTITY work.REG(reg_arch)
		PORT MAP(D=>C_Temp,Q=>REG_OUT, ENABLE => '1', RESET =>'0', CLOCK=>CLK);

WITH START SELECT
MUX_OUT <= '0' WHEN '1',
				REG_OUT WHEN OTHERS;	  
			
C <= REG_OUT;
V <= MUX_OUT XOR C_TEMP;
	
END bsd_arch;
