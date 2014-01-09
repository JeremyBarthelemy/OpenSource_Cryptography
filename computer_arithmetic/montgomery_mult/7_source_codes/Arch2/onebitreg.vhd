
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;


entity onebitreg is
    Port ( 
				D : in  STD_LOGIC;
            Q : out  STD_LOGIC;
            ENABLE : in  STD_LOGIC;
            CLOCK : in  STD_LOGIC;
            RESET : in  STD_LOGIC
			);
end onebitreg;

architecture ob_arch of onebitreg is

begin
	PROCESS(CLOCK, ENABLE) 
	BEGIN 
	IF (rising_edge(CLOCK)) AND (ENABLE = '1') THEN 
		IF RESET = '1' THEN 
			Q <= '0'; 
		ELSE
			Q <= D; 
		END IF; 
	END IF;
	END PROCESS;

end ob_arch;

