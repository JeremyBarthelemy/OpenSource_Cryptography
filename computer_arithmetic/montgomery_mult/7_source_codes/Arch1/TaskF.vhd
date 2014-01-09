----------------------------------------------------------------------------------
-- ECE 645 
-- 
--
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use ieee.std_logic_unsigned.all;
use work.montyMult_pkg.all;

entity TaskF is
	generic(W : integer := 32);
	port(
		CLK, RSTN, EN : in std_logic;
		Qi, Xi, Ce_in : in std_logic;
		C_in : std_logic_vector(1 downto 0);
		Y, M : in std_logic_vector(W-1 downto 0);
		Se_in : in std_logic_vector(W-1 downto 1);
		C_out : out std_logic_vector(1 downto 0);
		Se_out : out std_logic_vector(W-1 downto 0)
	);
end TaskF;

architecture TaskF_behavioral of TaskF is
	
	signal qi_int : std_logic;
	signal reg_in, reg_out, c_full, xi_full, qi_full : std_logic_vector(W+1 downto 0);

begin
	
	c_full <= (0 => C_in(0), 1=> C_in(1), others => '0');
	qi_full <= (others => Qi);
	xi_full <= (others => Xi);
	
	reg_in <= (Ce_in & Se_in) + c_full + (xi_full AND ("00" & Y)) + (qi_full AND ("00" & M));
	
	Reg : nBitRegSynch generic map(W+2) port map(reg_in, RSTN, CLK, EN, reg_out);

	C_out <= reg_out(W+1 downto W);
	Se_out <= reg_out(W-1 downto 0);

end TaskF_behavioral;

