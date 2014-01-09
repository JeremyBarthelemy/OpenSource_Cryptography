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

entity TaskD is
	generic(W : integer := 32);
	port(
		CLK, RSTN, EN : in std_logic;
		Xi, S1 : in std_logic;
		Y, M : in std_logic_vector(W-1 downto 0);
		S0_in : in std_logic_vector(W-1 downto 1);
		Qi : out std_logic;
		C : out std_logic_vector(1 downto 0);
		S0_out : out std_logic_vector(W-1 downto 0)
	);
end TaskD;

architecture TaskD_behavioral of TaskD is

	signal qi_int : std_logic;
	signal odd_in, even_in, odd_out, even_out, xi_full, qi_full : std_logic_vector(W+1 downto 0);

begin

	qi_int <= (Xi AND Y(0)) XOR S0_in(1);
	xi_full <= (others => Xi);
	qi_full <= (others => qi_int);
	
	odd_in <= ("01" & S0_in) + (xi_full AND ("00" & Y)) + (qi_full AND ("00" & M));
	even_in <= ("00" & S0_in) + (xi_full AND ("00" & Y)) + (qi_full AND ("00" & M));
	
	Odd_Reg : nBitRegSynch generic map(W+2) port map(odd_in, RSTN, CLK, EN, odd_out);
	Even_Reg : nBitRegSynch generic map(W+2) port map(even_in, RSTN, CLK, EN, even_out);	
	
	Qi <= qi_int;
	C <= odd_out(W+1 downto W) when S1 = '1' else even_out(W+1 downto W);
	S0_out <= odd_out(W-1 downto 0) when S1 = '1' else even_out(W-1 downto 0);

end TaskD_behavioral;

