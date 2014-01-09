----------------------------------------------------------------------------------
-- ECE 645
-- 
-- 
-- 
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity bitShiftReg is
	generic(N : integer := 32);
	port(
		CLK, EN, RSTN : in std_logic;
		DIN : in std_logic;
		LOAD : in std_logic_vector(N-1 downto 0);
		DOUT : out std_logic_vector(N-1 downto 0)
	);
end bitShiftReg;

architecture bitShiftReg_behavioral of bitShiftReg is
	signal internal : std_logic_vector(N-1 downto 0) := (others => '0');
begin
	process(CLK)
	begin
		if rising_edge(CLK) then
			if RSTN = '1' then
				internal <= LOAD;			
			elsif EN = '1' then
				DOUT <= internal(N-1 downto 0);
				internal(N-2 downto 0) <= internal(N-1 downto 1);
				internal(N-1) <= DIN;
			end if;
		end if;
	end process;
end bitShiftReg_behavioral;

