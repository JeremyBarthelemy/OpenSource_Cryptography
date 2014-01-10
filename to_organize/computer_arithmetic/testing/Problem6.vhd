library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
USE ieee.numeric_std;

entity Problem6 is
GENERIC(k : INTEGER := 4);
PORT( Clk : IN STD_LOGIC;
		a : IN STD_LOGIC_VECTOR(k-1 DOWNTO 0);
      b : IN STD_LOGIC;
		AND_OUT : OUT STD_LOGIC_VECTOR(k-1 DOWNTO 0);
		XOR_OUT : OUT STD_LOGIC_VECTOR(k-1 DOWNTO 0));
end Problem6;

architecture p6_arch of Problem6 is

begin

		AND_OUT <= a AND (k-1 DOWNTO 0 =>b);
		XOR_OUT <= a XOR (k-1 DOWNTO 0 =>b);

end p6_arch;