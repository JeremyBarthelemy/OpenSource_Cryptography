library IEEE;
use IEEE.STD_LOGIC_1164.ALL;


--Useful illustration: http://startingelectronics.com/software/VHDL-CPLD-course/tut11-shift-register/shift-register-elements.png
ENTITY shift_reg IS
	 GENERIC(size : INTEGER); 
    PORT(
				D : IN STD_LOGIC;
				Q_OF_REGS : OUT STD_LOGIC_VECTOR(size-1 DOWNTO 0);
            --Q : OUT  STD_LOGIC;
            ENABLE : IN STD_LOGIC;
            RESET : IN STD_LOGIC;
            CLOCK : IN  STD_LOGIC
			);
END shift_reg;

architecture shift_arch of shift_reg is
SIGNAL SIG_Q_OF_REGS : STD_LOGIC_VECTOR(size-1 DOWNTO 0);
begin

--Take in input, shift to the right by a single bit for each clock cycle
--Make a generate statement of e/e-1 regs

Q_OF_REGS <= SIG_Q_OF_REGS;

REG0 : ENTITY work.onebitreg(ob_arch)
		 PORT MAP(
			D=>D,
			Q=>SIG_Q_OF_REGS(0),
			ENABLE=>ENABLE,
			RESET=>RESET,
			CLOCK=>CLOCK
		 );


REG_GEN : FOR i IN 1 TO size-1 GENERATE 
REGS : ENTITY work.onebitreg(ob_arch)
		 PORT MAP(
			D=>SIG_Q_OF_REGS(i-1),
			Q=>SIG_Q_OF_REGS(i),
			ENABLE=>ENABLE,
			RESET=>RESET,
			CLOCK=>CLOCK
		 );


END GENERATE;

end shift_arch;

