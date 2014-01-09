----------------------------------------------------------------------------------
-- ECE 645 
-- 
--
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity nBitRegSynch is
	generic (N : integer := 128);
	port(
		D : in std_logic_vector(N-1 downto 0);
		RSTN, CLK, EN : in std_logic;
		Q : out std_logic_vector(N-1 downto 0)
	);		
end nBitRegSynch;

architecture nBitRegSynch_behavioral of nBitRegSynch is

begin
	process (RSTN, CLK)
	begin
		if rising_edge(CLK) then
			if(RSTN = '1') then
				Q <= (others => '0');
			elsif EN = '1' then
				Q <= D;
			end if;			
		end if;
	end process;
end nBitRegSynch_behavioral;

