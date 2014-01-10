LIBRARY ieee;
USE ieee.std_logic_1164.all;
use IEEE.std_logic_unsigned.all;
use ieee.STD_LOGIC_ARITH.all;

ENTITY Homework4_Task6 IS
	GENERIC(w : INTEGER := 8);
	PORT(	
			rst : STD_LOGIC;
			clk : STD_LOGIC;
			DOUT : IN STD_LOGIC_VECTOR(w-1 DOWNTO 0);
			WR : IN STD_LOGIC;
			ADDR : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
			RD : IN STD_LOGIC;
			sel : IN STD_LOGIC_VECTOR(2 DOWNTO 0);
			DIN : OUT STD_LOGIC_VECTOR(w-1 DOWNTO 0)
		);
END Homework4_Task6;

ARCHITECTURE arch_Homework4_Task6 OF Homework4_Task6 IS
--SIGNAL sel : STD_LOGIC_VECTOR(2 DOWNTO 0); 
SIGNAL ROM_OUT : STD_LOGIC_VECTOR(w-1 DOWNTO 0);
SIGNAL R0_OUT : STD_LOGIC_VECTOR(w-1 DOWNTO 0);
SIGNAL R1_OUT : STD_LOGIC_VECTOR(w-1 DOWNTO 0);
SIGNAL R2_OUT : STD_LOGIC_VECTOR(w-1 DOWNTO 0);
SIGNAL R3_OUT : STD_LOGIC_VECTOR(w-1 DOWNTO 0);
SIGNAL MUX_OUT : STD_LOGIC_VECTOR(w-1 DOWNTO 0);
SIGNAL BUFFER_OUT : STD_LOGIC_VECTOR(w-1 DOWNTO 0);
SIGNAL DEC_SEL : STD_LOGIC_VECTOR(2 DOWNTO 0);
SIGNAL DEC_EN_OUT : STD_LOGIC_VECTOR(3 DOWNTO 0);
SIGNAL en0 : STD_LOGIC;
SIGNAL en1 : STD_LOGIC;
SIGNAL en2 : STD_LOGIC;
SIGNAL en3 : STD_LOGIC;
SIGNAL NOR_OUT : STD_LOGIC;
SIGNAL AND_OUT : STD_LOGIC;
SIGNAL DEC_EN : STD_LOGIC;


BEGIN

	
ROM : ENTITY work.ROM_HW5(arch_ROM)
	GENERIC MAP(w => 8)
	PORT MAP(
				ADDRESS_IN => ADDR(2 DOWNTO 0),
				DOUT=> ROM_OUT 
	
			);
			
R0 : ENTITY work.A_REG(a_reg_arch)
	 GENERIC MAP(w => 8)
	 PORT MAP(D => DOUT,Q => R0_OUT, RESET => rst, ENABLE => en0, CLOCK => clk);
	
R1 : ENTITY work.A_REG(a_reg_arch)
	 GENERIC MAP(w => 8)
	 PORT MAP(D => DOUT,Q => R1_OUT, RESET => rst, ENABLE => en1, CLOCK => clk);
	 
R2 : ENTITY work.A_REG(a_reg_arch)
	 GENERIC MAP(w => 8)
	 PORT MAP(D => DOUT,Q => R2_OUT, RESET => rst, ENABLE => en2, CLOCK => clk);
	 
R3 : ENTITY work.A_REG(a_reg_arch)
	 GENERIC MAP(w => 8)
	 PORT MAP(D => DOUT,Q => R3_OUT, RESET => rst, ENABLE => en3, CLOCK => clk);	 

BUFFER_OUT <= MUX_OUT WHEN (RD = '1')
			  ELSE "ZZZZZZZZ"; 
	
DIN <= BUFFER_OUT;	

WITH sel SELECT				  
MUX_OUT <= R0_OUT WHEN "000",
		   R1_OUT WHEN "001",
		   R2_OUT WHEN "010",
		   R3_OUT WHEN "011",
		   ROM_OUT WHEN OTHERS;		  

NOR_OUT <= NOT(ADDR(13) OR ADDR(12) OR ADDR(3) OR ADDR(2) OR ADDR(1) OR ADDR(0));						  
AND_OUT <= ADDR(15) AND ADDR(14) AND ADDR(11) AND ADDR(8) AND ADDR(7) AND ADDR(6) AND ADDR(5) AND ADDR(4);		
DEC_EN <= WR AND NOR_OUT AND AND_OUT;

DEC_SEL <= DEC_EN & ADDR(10) & ADDR(9);

WITH DEC_SEL SELECT
DEC_EN_OUT <= "0001" WHEN "100",
			  "0010" WHEN "101",
			  "0100" WHEN "110",
			  "1000" WHEN "111",
			  "0000" WHEN OTHERS; 
			  
en0 <= DEC_EN_OUT(0);
en1 <= DEC_EN_OUT(1);
en2 <= DEC_EN_OUT(2);
en3 <= DEC_EN_OUT(3);				  


END arch_Homework4_Task6;