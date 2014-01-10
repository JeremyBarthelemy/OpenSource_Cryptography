LIBRARY ieee;
USE ieee.std_logic_1164.all;
--Sorry to have so many reg files and not generics, running
--low on time and i don't have much time to optimize the code


ENTITY Reg3 IS
	PORT(
			D : IN STD_LOGIC;
			Q : OUT STD_LOGIC;
			ENABLE : IN STD_LOGIC;
			RESET : IN STD_LOGIC;
			CLOCK : IN STD_LOGIC
		);
END Reg3;

ARCHITECTURE reg3_arch OF Reg3 IS
BEGIN	   
	
	PROCESS(CLOCK, ENABLE) 
	BEGIN 
	IF (rising_edge(CLOCK)) AND (ENABLE = '1') THEN 
		IF RESET = '1' THEN 
			Q <= '0'; 
		ELSE
			Q <= D ; 
		END IF ; 
	END IF;
	END PROCESS ;

END reg3_arch;
