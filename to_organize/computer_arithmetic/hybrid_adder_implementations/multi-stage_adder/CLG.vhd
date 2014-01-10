library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use ieee.std_logic_unsigned.all;

entity CLG is
	 --GENERIC(k : INTEGER);
	 PORT( 
				g_in : in  STD_LOGIC_VECTOR (3 downto 0);
				p_in : in  STD_LOGIC_VECTOR (3 downto 0);
				g_out : out  STD_LOGIC;--STD_LOGIC_VECTOR(3 DOWNTO 0);
				p_out : out  STD_LOGIC;--STD_LOGIC_VECTOR((4*k-1) DOWNTO 0);
				Cin : in  STD_LOGIC;
				Cout : out  STD_LOGIC_VECTOR(3 downto 1)
			);
end CLG;

ARCHITECTURE clg_arch OF CLG IS
BEGIN

--g_out <= g_in(k/2+k/4-1) OR (g_in(k/2-1) AND p_in(k/2+k/4-1)) OR (g_in(k/4-1) AND p_in(k/2-1) AND p_in(k/2+k/4-1)) OR (g_in(0) AND p_in(k/4-1) AND p_in(k/2-1) AND p_in(k/2+k/4-1));
--p_out <= p_in(0) AND p_in(k/4-1) AND p_in(k/2-1) AND p_in(k/2+k/4-1);
--Cout(3) <= g_in(k/2-1) OR (g_in(k/4-1) AND p_in(k/2-1)) OR (g_in(0) AND p_in(k/4-1) AND p_in(k/2-1)) OR (Cin AND p_in(0) AND p_in(k/4-1) AND p_in(k/2-1));
--Cout(2) <= g_in(k/2-1) OR (g_in(0) AND p_in(k/2-1)) OR ((Cin AND p_in(0)) AND p_in(k/2-1));
--Cout(1) <= g_in(0) OR (Cin AND p_in(0));

g_out <= g_in(3) OR (g_in(2) AND p_in(3)) OR (g_in(1) AND p_in(2) AND p_in(3)) OR (g_in(0) AND p_in(1) AND p_in(2) AND p_in(3));
p_out <= p_in(0) AND p_in(1) AND p_in(2) AND p_in(3);
Cout(3) <= g_in(2) OR (g_in(1) AND p_in(2)) OR (g_in(0) AND p_in(1) AND p_in(2)) OR (Cin AND p_in(0) AND p_in(1) AND p_in(2));
Cout(2) <= g_in(1) OR (g_in(0) AND p_in(1)) OR ((Cin AND p_in(0)) AND p_in(1));
Cout(1) <= g_in(0) OR (Cin AND p_in(0));


END clg_arch;

