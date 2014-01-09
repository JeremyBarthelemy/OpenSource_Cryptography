----------------------------------------------------------------------------------
-- Matt Drummond
-- ECE 645
-- Homework 2, 3/1/13
--
-- D FlipFlop
----------------------------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity DFlipFlop is
	port(
		CLK, EN, D : in std_logic;
		Q : out std_logic
	);
end DFlipFlop;

architecture DFlipFlop_behavioral of DFlipFlop is
begin
	process(CLK)
	begin
		if rising_edge(CLK) then
			if EN = '1' then
				Q <= D;
			end if;
		end if;
	end process;
end DFlipFlop_behavioral;