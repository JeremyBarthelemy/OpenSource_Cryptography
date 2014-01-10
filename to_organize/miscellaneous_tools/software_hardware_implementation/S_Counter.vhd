LIBRARY ieee;
USE ieee.std_logic_1164.all;
use IEEE.std_logic_unsigned.all;
use ieee.STD_LOGIC_ARITH.all;

ENTITY S_COUNTER IS
	GENERIC(N : INTEGER := 6);
	PORT(	
			Q      : OUT STD_LOGIC_VECTOR(N-1 DOWNTO 0);
			ENABLE : IN STD_LOGIC;
			RESET  : IN STD_LOGIC;
			CLOCK  : IN STD_LOGIC	
		);
END S_COUNTER;

ARCHITECTURE s_counter_arch OF S_COUNTER IS
SIGNAL Count : STD_LOGIC_VECTOR(N-1 DOWNTO 0);
BEGIN		   
	PROCESS(CLOCK)
	BEGIN
		IF(rising_Edge(CLOCK)) THEN
			IF(RESET = '1') THEN
				Count <= (OTHERS => '0');
			ELSIF (ENABLE = '1') THEN
				Count <= Count + 1;
			END IF;
		END IF;
	END PROCESS;
	Q <= Count;
	
END s_counter_arch;
