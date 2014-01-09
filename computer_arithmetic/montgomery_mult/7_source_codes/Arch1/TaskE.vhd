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

entity TaskE is
	generic(W : integer := 32);
	port(
		CLK, RSTN, EN : in std_logic;
		Qi, Xi, S0_in : in std_logic;
		C_in : in std_logic_vector(1 downto 0);
		Y, M : in std_logic_vector(W-1 downto 0);
		Sj_in : in std_logic_vector(W-1 downto 1);
		C_out : out std_logic_vector(1 downto 0);
		Sj_out : out std_logic_vector(W-1 downto 0)
	);
end TaskE;

architecture TaskE_behavioral of TaskE is

	signal odd_in, even_in, odd_out, even_out, c_full, qi_full, xi_full : std_logic_vector(W+1 downto 0);

begin

	c_full <= (0 => C_in(0), 1 => C_in(1), others => '0');
	qi_full <= (others => Qi);
	xi_full <= (others => Xi);

	odd_in <= ("01" & Sj_in) + c_full + (xi_full AND ("00" & Y)) + (qi_full AND ("00" & M));
	even_in <= ("00" & Sj_in) + c_full + (xi_full AND ("00" & Y)) + (qi_full AND ("00" & M));
	
	Odd_Reg : nBitRegSynch generic map(W+2) port map(odd_in, RSTN, CLK, EN, odd_out);
	Even_Reg : nBitRegSynch generic map(W+2) port map(even_in, RSTN, CLK, EN, even_out);	

	C_out <= odd_out(W+1 downto W) when S0_in = '1' else even_out(W+1 downto W);
	Sj_out <= odd_out(W-1 downto 0) when S0_in = '1' else even_out(W-1 downto 0);

end TaskE_behavioral;

