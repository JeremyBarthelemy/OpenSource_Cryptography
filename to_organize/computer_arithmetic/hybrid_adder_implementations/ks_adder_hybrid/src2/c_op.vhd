library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.std_logic_unsigned.all;

entity C_OP is
    Port ( 
				g1_in : in  STD_LOGIC;
				g2_in : in  STD_LOGIC;
				p1_in : in  STD_LOGIC;
				p2_in : in  STD_LOGIC;
				g_out : out  STD_LOGIC;
				p_out : out  STD_LOGIC
			);
end c_op;

ARCHITECTURE cop_arch OF C_OP IS
BEGIN
--g  = g" + g' p"
--p = p' p"

g_out <= g2_in OR (g1_in AND p2_in);
p_out <= (p1_in AND p2_in);

END cop_arch;

