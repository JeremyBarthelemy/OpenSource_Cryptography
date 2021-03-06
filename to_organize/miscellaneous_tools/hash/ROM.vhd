--simple ROM example
LIBRARY ieee;
USE ieee.std_logic_1164.all;

ENTITY ROM IS
	port(
		ADDRESS_IN : IN STD_LOGIC_VECTOR(3 DOWNTO 0);
		DOUT : OUT STD_LOGIC_VECTOR(7 DOWNTO 0)
	
		);
END ROM;

ARCHITECTURE arch_ROM of ROM IS
BEGIN
	
--design the internal values of ROM according to hw:  	 
--values in table are: {0x01, 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89, 0x9A, 0xAB, 0xBC, 0xCD, 0xDE, 0xEF, 0xF0}

WITH ADDRESS_IN SELECT
DOUT <= "00000001" WHEN "0000",
		"00010010" WHEN "0001",
		"00100011" WHEN "0010",
		"00110100" WHEN "0011",
		"01000101" WHEN "0100",
		"01010110" WHEN "0101",
		"01100111" WHEN "0110",
		"01111000" WHEN "0111",
		"10001001" WHEN "1000",
		"10011010" WHEN "1001",
		"10101011" WHEN "1010",
		"10111100" WHEN "1011",
		"11001101" WHEN "1100",
		"11011110" WHEN "1101",
		"11101111" WHEN "1110",
		"11110000" WHEN OTHERS;

END arch_ROM;
