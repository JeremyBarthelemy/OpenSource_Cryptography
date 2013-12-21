--One round of DES encryption
--Currently using this for testing purposes.  May
--change this to simply the function in each DES
--round to simplify the hardware.

LIBRARY ieee;
USE ieee.std_logic_1164.all;

ENTITY ROUND IS
		 PORT(
					test_addr : IN STD_LOGIC_VECTOR(5 DOWNTO 0);
					test_dout : OUT STD_LOGIC_VECTOR(3 DOWNTO 0);
					Clk : IN STD_LOGIC
			  );
END ROUND;
	
	
ARCHITECTURE round_arch of ROUND IS
BEGIN

S6: ENTITY work.S6_box(s6_arch)
	PORT MAP(Clk => Clk, ADDR => test_addr, DOUT => test_dout);
	
	
END round_arch;