
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

--VERIFY THIS
entity PEj is
	 GENERIC(w : INTEGER := 32);
	 Port ( 
				--Inputs
				xi : IN STD_LOGIC;
				Y_in, M_in : IN STD_LOGIC_VECTOR(w-1 DOWNTO 0);
				C_In : IN STD_LOGIC_VECTOR(1 DOWNTO 0);
				REG_EN : IN STD_LOGIC;
				qi : IN STD_LOGIC;
				RESET : IN STD_LOGIC;
				CLOCK : IN STD_LOGIC;
				S_Next_In : IN STD_LOGIC; --lsb of next PE's word of S output				
				--Outputs
				C_out : OUT STD_LOGIC_VECTOR(1 DOWNTO 0);
				S_PassBack : OUT STD_LOGIC;
				S_out : OUT STD_LOGIC_VECTOR(w-1 DOWNTO 0)
	  );
end PEj;

architecture Behavioral of PEj is
SIGNAL REG_IN : STD_LOGIC_VECTOR(w + 4 downto 0);
SIGNAL REG_OUT : STD_LOGIC_VECTOR(w + 4 downto 0);
SIGNAL S_signal : STD_LOGIC_VECTOR(w-1 DOWNTO 0);
SIGNAL S_signal_top_bit : STD_LOGIC;
BEGIN

S_signal <= S_signal_top_bit & REG_OUT(w-2 DOWNTO 0);
S_out <= S_signal;

S_PassBack <= REG_OUT(0);

WITH S_Next_In SELECT
S_signal_top_bit <= REG_OUT(w) WHEN '0',
						  REG_OUT(w-1) WHEN OTHERS;	
							
WITH S_Next_In SELECT
C_out <= REG_OUT(w+4 DOWNTO w+3) WHEN '1',
			REG_OUT(w+2 DOWNTO w+1) WHEN OTHERS;

REG_E : ENTITY work.REG(reg_arch)
		  GENERIC MAP(size =>w+5)
		  PORT MAP(
		   D=>REG_IN,
			Q=>REG_OUT,
			ENABLE=>REG_EN,
			RESET=>RESET,
			CLOCK=>CLOCK
			);
		  

E_ENTITY : ENTITY work.E(e_arch)
			  GENERIC MAP(w=>32)
			  PORT MAP(
			  	--Inputs
				S_in=>S_signal(w-1 DOWNTO 1),
				M_in=>M_in,
				Y_in=>Y_in,
				xi_in=>xi,
				qi_in=>qi,
				C_in=>C_In,
				--Outputs
				CO_p=>REG_IN(w+4 DOWNTO w+3),
				CE_p=>REG_IN(w+2 DOWNTO w+1),
				SO_p=>REG_IN(w),
				SE_p=>REG_IN(w-1),
				S_out=>REG_IN(w-2 DOWNTO 0)
			   );
				
end Behavioral;

