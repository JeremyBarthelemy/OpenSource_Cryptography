
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use ieee.std_logic_unsigned.all;

entity E is
	 GENERIC(w : INTEGER := 32);
    Port ( 
				--Inputs
				S_in : IN STD_LOGIC_VECTOR(w-1 DOWNTO 1);
				M_in, Y_in : IN STD_LOGIC_VECTOR(w-1 DOWNTO 0);
				xi_in, qi_in : IN STD_LOGIC;
				C_in : IN STD_LOGIC_VECTOR(1 DOWNTO 0);
				--Outputs
				CO_p, CE_p : OUT STD_LOGIC_VECTOR(1 DOWNTO 0);
				S_out : OUT STD_LOGIC_VECTOR(w-2 DOWNTO 0);
				SO_p, SE_p : OUT STD_LOGIC
				
			);
end E;

architecture e_arch of E is
SIGNAL R : STD_LOGIC_VECTOR(w-1 DOWNTO 0); --qi*M(0)
SIGNAL Z : STD_LOGIC_VECTOR(w-1 DOWNTO 0); --xi*Y(0)

SIGNAL TempA : STD_LOGIC_VECTOR(w+1 DOWNTO 0); --Needed to make sure addition works properly
SIGNAL TempB : STD_LOGIC_VECTOR(w+1 DOWNTO 0);
SIGNAL CO_SO_S_temp : STD_LOGIC_VECTOR(w+1 DOWNTO 0);
SIGNAL CE_SE_S_temp : STD_LOGIC_VECTOR(w+1 DOWNTO 0);
begin


WITH xi_in SElECT
Z <= Y_in WHEN '1',
	  (OTHERS => '0') WHEN OTHERS;

WITH qi_in SELECT
R <=  M_in WHEN '1',
	  (OTHERS => '0') WHEN OTHERS;

TempA <= "001" & S_in(w-1 DOWNTO 1);
TempB <= "000" & S_in(w-1 DOWNTO 1);
CO_SO_S_temp <= TempA + Z + R + C_in;	  
CE_SE_S_temp <= TempB + Z + R + C_in;

CO_p <= CO_SO_S_temp(w+1 DOWNTO w);
SO_p <= CO_SO_S_temp(w-1);
CE_p <= CE_SE_S_temp(w+1 DOWNTO w);
SE_p <= CE_SE_S_temp(w-1);
S_out <= CO_SO_S_temp(w-2 DOWNTO 0); --Or CE_SE_S_temp(w-2 DOWNTO 0)?  should not matter...

end e_arch;

