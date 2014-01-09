LIBRARY ieee;
USE ieee.std_logic_1164.all;

ENTITY Variable_Rotator IS
	port(
		--DIR is labeled separately from amount for a clearer view of the logic, but in effect it's just another bit of amount
		var : IN STD_LOGIC_VECTOR(7 DOWNTO 0); --variable to rotate
		DIR : IN STD_LOGIC; --DIR = 0 is left rotate, 1 is right rotate
		amount : IN STD_LOGIC_VECTOR(2 DOWNTO 0); --amount of rotations
		output : OUT STD_LOGIC_VECTOR(7 DOWNTO 0)
	);
END Variable_Rotator;

ARCHITECTURE var_arch OF Variable_Rotator IS
BEGIN

output <= (var(7 DOWNTO 0)) WHEN (amount = "000") ELSE
	(var(6 DOWNTO 0) & var(7)) WHEN (amount = "001" AND DIR = '0') ELSE
	(var(5 DOWNTO 0) & var(7 DOWNTO 6)) WHEN (amount = "010" AND DIR = '0') ELSE
	(var(4 DOWNTO 0) & var(7 DOWNTO 5)) WHEN (amount = "011" AND DIR = '0')	ELSE
	(var(3 DOWNTO 0) & var(7 DOWNTO 4)) WHEN (amount = "100" AND DIR = '0') ELSE
	(var(2 DOWNTO 0) & var(7 DOWNTO 3)) WHEN (amount = "101" AND DIR = '0') ELSE
	(var(1 DOWNTO 0) & var(7 DOWNTO 2)) WHEN (amount = "110" AND DIR = '0') ELSE
	(var(0) & var(7 DOWNTO 1)) WHEN (amount = "111" AND DIR = '0') ELSE
	
	(var(0) & var(7 DOWNTO 1)) WHEN (amount = "001" AND DIR = '1') ELSE
	(var(1 DOWNTO 0) & var(7 DOWNTO 2)) WHEN (amount = "010" AND DIR = '1') ELSE
	(var(2 DOWNTO 0) & var(7 DOWNTO 3)) WHEN (amount = "011" AND DIR = '1') ELSE
	(var(3 DOWNTO 0) & var(7 DOWNTO 4)) WHEN (amount = "100" AND DIR = '1') ELSE
	(var(4 DOWNTO 0) & var(7 DOWNTO 5)) WHEN (amount = "101" AND DIR = '1') ELSE
	(var(5 DOWNTO 0) & var(7 DOWNTO 6)) WHEN (amount = "110" AND DIR = '1') ELSE
	(var(6 DOWNTO 0) & var(7)) WHEN (amount = "111" AND DIR = '1');

END var_arch;
