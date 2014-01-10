
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

--VERIFY THIS
entity PElast is
	 GENERIC(w : INTEGER := 32);
	 Port ( 
				--Inputs
				xi : IN STD_LOGIC;
				Y_in, M_in : IN STD_LOGIC_VECTOR(w-1 DOWNTO 0);
				C_In : IN STD_LOGIC_VECTOR(1 DOWNTO 0);
				qi : IN STD_LOGIC;
				--C_e_1 : IN STD_LOGIC_VECTOR(1 DOWNTO 0);
				REG_EN : IN STD_LOGIC;
				RESET : IN STD_LOGIC;
				CLOCK : IN STD_LOGIC;
				--Outputs
				--C_out : OUT STD_LOGIC_VECTOR(1 DOWNTO 0);
				S_PassBack : OUT STD_LOGIC;
				S_out : OUT STD_LOGIC_VECTOR(w-1 DOWNTO 0)
	  );
end PElast;

architecture Behavioral of PElast is
SIGNAL REG_IN : STD_LOGIC_VECTOR(w+1 DOWNTO 0);
SIGNAL REG_OUT : STD_LOGIC_VECTOR(w+1 DOWNTO 0);
BEGIN


S_out <= REG_OUT(w-1 DOWNTO 0);
S_PassBack <= REG_OUT(0);

F_REG : ENTITY work.REG(reg_arch)
		  GENERIC MAP(size => w+2)
		  PORT MAP(D=>REG_IN,Q=>REG_OUT,ENABLE=>REG_EN,RESET=>RESET,CLOCK=>CLOCK);
		  
--F Combinational Logic Entity
F_ENTITY : ENTITY work.F(f_arch)
			  GENERIC MAP(w=>w)
			  PORT MAP( 
				--Inputs
				S_in=>REG_OUT(w-1 DOWNTO 1),
				M_in=>M_in,
				Y_in=>Y_in,
				xi_in=>xi,
				qi_in=>qi,
				Ce0=>REG_OUT(w),
				C_In => C_In,
				--Outputs
				C_out=>REG_IN(w+1 DOWNTO w),
				S_out=>REG_IN(w-1 DOWNTO 0)
				
				);


end Behavioral;

