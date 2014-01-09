
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.std_logic_unsigned.all;

entity F is
	 GENERIC(w : INTEGER := 32);
    Port ( 
				--Inputs
				S_in : IN STD_LOGIC_VECTOR(w-1 DOWNTO 1);
				M_in, Y_in : IN STD_LOGIC_VECTOR(w-1 DOWNTO 0);
				xi_in, qi_in : IN STD_LOGIC;
				Ce0 : IN STD_LOGIC;
				C_In : IN STD_LOGIC_VECTOR(1 DOWNTO 0);
				--C_e_1 : IN STD_LOGIC_VECTOR(1 DOWNTO 0);
				--Outputs
				C_out : OUT STD_LOGIC_VECTOR(1 DOWNTO 0);
				S_out : OUT STD_LOGIC_VECTOR(w-1 DOWNTO 0)
				
			);
end F;

architecture f_arch of F is
SIGNAL Z : STD_LOGIC_VECTOR(w-1 DOWNTO 0);
SIGNAL R : STD_LOGIC_VECTOR(w-1 DOWNTO 0);
SIGNAL TempA : STD_LOGIC_VECTOR(w+1 DOWNTO 0); --Needed to make sure addition works properly
SIGNAL C_S_OUT_temp : STD_LOGIC_VECTOR(w+1 DOWNTO 0);

begin

TempA <= "00" & Ce0 & S_in(w-1 DOWNTO 1);
C_S_OUT_temp <= TempA + C_In + Z + R;

WITH xi_in SElECT
Z <= Y_in WHEN '1',
	  (OTHERS => '0') WHEN OTHERS;

WITH qi_in SELECT
R <=  M_in WHEN '1',
	  (OTHERS => '0') WHEN OTHERS;

C_out <= C_S_OUT_temp(w+1 DOWNTO w);
S_out <= C_S_OUT_temp(w-1 DOWNTO 0);


end f_arch;

