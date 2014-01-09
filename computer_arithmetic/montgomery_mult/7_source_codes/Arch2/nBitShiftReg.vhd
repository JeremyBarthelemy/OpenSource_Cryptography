library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity nBitShiftReg is
	generic(N : integer := 32; M : integer := 256);
	port(
		CLK : in std_logic;
		DIN : in std_logic_vector(N-1 downto 0);
		DOUT : out std_logic_vector(N-1 downto 0)
	);
end nBitShiftReg;

architecture nBitShiftReg_behavioral of nBitShiftReg is
	signal internal : std_logic_vector(M-1 downto 0) := (others => '0');
begin
	process(CLK)
	begin
		if rising_edge(CLK) then
			DOUT <= internal(N-1 downto 0);
			internal(M-N-1 downto 0) <= internal(M-1 downto N);
			internal(M-1 downto M-N) <= DIN;
		end if;
	end process;

end nBitShiftReg_behavioral;

