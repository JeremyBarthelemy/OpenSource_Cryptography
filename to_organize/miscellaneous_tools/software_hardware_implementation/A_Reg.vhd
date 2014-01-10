LIBRARY ieee;
USE ieee.std_logic_1164.all;

ENTITY a_reg IS
	GENERIC ( w : INTEGER := 8 ) ;
	PORT(
			D : IN STD_LOGIC_VECTOR(w-1 DOWNTO 0);
			Q : OUT STD_LOGIC_VECTOR(w-1 DOWNTO 0);
			ENABLE : IN STD_LOGIC;
			RESET : IN STD_LOGIC;
			CLOCK : IN STD_LOGIC
		);
END a_reg;


ARCHITECTuRE a_reg_arch OF a_reg IS
BEGIN						   

	PROCESS(CLOCK, RESET)
	BEGIN
		IF(RESET = '1') THEN
			Q <= (OTHERS => '0'); --not sure what N is, so use OTHERS => '0' down to zero
		ELSIF(CLOCK = '1') THEN
			Q <= D;
		END IF;
	
	END PROCESS;
	
	
END a_reg_arch;
