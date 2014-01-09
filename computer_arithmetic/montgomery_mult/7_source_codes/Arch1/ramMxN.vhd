----------------------------------------------------------------------------------
-- ECE 645 
-- 
--
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;
use work.montyMult_pkg.all;

entity ramMxN is
	generic (M : integer := 16;  N : integer := 64);
	port(
		CLK, WE : in std_logic;
		A_IN : in std_logic_vector(N-1 downto 0);
		ADDR_A : in std_logic_vector(M-1 downto 0);
		A_OUT : out std_logic_vector(N-1 downto 0)
	);
end ramMxN;

architecture ramMxN_behavioral of ramMxN is

	type mem_type is array (2**M-1 downto 0) of std_logic_vector(N-1 downto 0) ;
	signal memory : mem_type := (others => (others => '0'));

begin
	process (CLK)
	begin		
		if rising_edge(CLK) then
			if WE = '1' then
				memory(conv_integer(unsigned(ADDR_A))) <= A_IN;
			end if;
		end if;	
		A_OUT <= memory(conv_integer(unsigned(ADDR_A)));
	end process;
end ramMxN_behavioral;

