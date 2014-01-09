--This is a Modified Half Adder

LIBRARY ieee;
USE ieee.std_logic_1164.all;

ENTITY MHA IS
	PORT(							
			X : IN STD_LOGIC;
			Y : IN STD_LOGIC;
			C : OUT STD_LOGIC;
			S : OUT STD_LOGIC
		);
END MHA;

ARCHITECTURE mha_arch OF MHA IS
BEGIN			   
	
C <= X OR Y;
S <= (NOT (X XOR Y));
	
END mha_arch;
