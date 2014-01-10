library IEEE;
use IEEE.STD_LOGIC_1164.ALL;


entity PE0 is
	 GENERIC(w : INTEGER := 32);
	 Port ( 
				--Inputs
				xi : IN STD_LOGIC;
				Y_in, M_in : IN STD_LOGIC_VECTOR(w-1 DOWNTO 0);
				REG_EN : IN STD_LOGIC;
				RESET : IN STD_LOGIC;
				CLOCK : IN STD_LOGIC;
				S_Next_In : IN STD_LOGIC; --lsb of next PE's word of S output
				--Outputs
				qi : OUT STD_LOGIC;
				C_out : OUT STD_LOGIC_VECTOR(1 DOWNTO 0);
				S_out : OUT STD_LOGIC_VECTOR(w-1 DOWNTO 0)
	  );

end PE0;

architecture Behavioral of PE0 is
SIGNAL REG_IN : STD_LOGIC_VECTOR(w + 4 downto 0);
SIGNAL REG_OUT : STD_LOGIC_VECTOR(w + 4 downto 0);
SIGNAL S_signal : STD_LOGIC_VECTOR(w-2 DOWNTO 0);
SIGNAL S_signal_top_bit : STD_LOGIC;
begin

S_out <= (S_signal_top_bit & S_signal);

WITH S_Next_In SELECT
S_signal_top_bit <= REG_OUT(w) WHEN '0',
						  REG_OUT(w-1) WHEN OTHERS;	
						  
S_signal <= REG_OUT(w-2 DOWNTO 0);						  
							
WITH S_Next_In SELECT
C_out <= REG_OUT(w+4 DOWNTO w+3) WHEN '1',
			REG_OUT(w+2 DOWNTO w+1) WHEN OTHERS;

REG : ENTITY work.REG(reg_arch)
		GENERIC MAP(size=>(w+5))
		PORT MAP(
			D=>REG_IN,
			Q=>REG_OUT,
			ENABLE=>REG_EN,
			RESET=>RESET,
			CLOCK=>CLOCK
		);


D_ENTITY : ENTITY work.D(d_arch)
			  GENERIC MAP(w => 32)
			  PORT MAP(
	 			--Inputs
				S_in=>S_signal, 
				M_in=>M_in,
				Y_in=>Y_in,
				xi_in=>xi,
				--Outputs
				CO_p=>REG_IN(w+4 DOWNTO w+3),
				CE_p=>REG_IN(w+2 DOWNTO w+1),
				SO_p=>REG_IN(w),
				SE_p=>REG_IN(w-1),
				S_out=>REG_IN(w-2 DOWNTO 0),
				qi=>qi
				
				);

end Behavioral;

