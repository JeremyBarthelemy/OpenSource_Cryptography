LIBRARY ieee;
USE ieee.std_logic_1164.all;

--Could have used generic but had to do this to save time

ENTITY Reg2 IS
	PORT(
			D : IN STD_LOGIC_VECTOR(31 DOWNTO 0);
			Q : OUT STD_LOGIC_VECTOR(31 DOWNTO 0);
			ENABLE : IN STD_LOGIC;
			RESET : IN STD_LOGIC;
			CLOCK : IN STD_LOGIC
		);
END Reg2;

ARCHITECTURE reg2_arch OF Reg2 IS
BEGIN	   
	
	PROCESS(CLOCK, ENABLE) 
	BEGIN 
	IF (rising_edge(CLOCK)) AND (ENABLE = '1') THEN 
		IF RESET = '1' THEN 
			Q <= (OTHERS => '0'); 
		ELSE
			Q <= D ; 
		END IF ; 
	END IF;
	END PROCESS ;

END reg2_arch;
