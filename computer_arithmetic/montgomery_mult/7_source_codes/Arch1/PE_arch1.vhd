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

entity PE_arch1 is
	generic (W : integer := 16);
	port(
		CLK, RSTN, EN, QEN : in std_logic;
		Y, M : in std_logic_vector(W-1 downto 0);
		S_IN : in std_logic_vector(W-1 downto 0);
		X : in std_logic;		
		S_OUT : out std_logic_vector(W-1 downto 0)
	);
end PE_arch1;

architecture PE_arch1_behavioral of PE_arch1 is
	
	signal qi, q_int : std_logic;	
	signal y_int, m_int : std_logic_vector(W-1 downto 0);
	signal s_lower, c_out : std_logic_vector(W downto 0);		
	signal even_upper, odd_upper, even_out, odd_out : std_logic_vector(2 downto 0);	

begin

	-- Combinational logic.
	-- Create and store Qi.				
	qi <= (X AND Y(0)) XOR S_IN(0);
	Q_Reg : DFlipFlop port map(CLK, QEN, qi, q_int);
	
	-- Multiply Y by Xi and M by Qi
	y_int <= Y when X = '1' else (others => '0');
	m_int <= M when q_int = '1' else (others => '0');

	-- Perform additions.
	s_lower <= ("00" & y_int(W-2 downto 0)) + ("00" & m_int(W-2 downto 0)) + ("00" & S_IN(W-1 downto 1)) + c_out;
	even_upper <= ('0' & s_lower(W downto W-1)) + ("00" & y_int(W-1)) + ("00" & m_int(W-1)) + "000";
	odd_upper <= ('0' & s_lower(W downto W-1)) + ("00" & y_int(W-1)) + ("00" & m_int(W-1)) + "001";
	
	-- Store while waiting for select bit.
	Lower_Reg : nBitRegSynch generic map(W-1) port map(s_lower(w-2 downto 0), RSTN, CLK, EN, S_OUT(W-2 downto 0));	
	Even_Reg : nBitRegSynch generic map(3) port map(even_upper, RSTN, CLK, EN, even_out);	
	Odd_Reg : nBitRegSynch generic map(3) port map(odd_upper, RSTN, CLK, EN, odd_out);	
	
	-- Output selection.
	c_out(1 downto 0) <= odd_out(2 downto 1) when S_IN(0) = '1' else even_out(2 downto 1);
	c_out(W downto 2) <= (others => '0');	
	S_OUT(W-1) <= odd_out(0) when S_IN(0) = '1' else even_out(0);

end PE_arch1_behavioral;

