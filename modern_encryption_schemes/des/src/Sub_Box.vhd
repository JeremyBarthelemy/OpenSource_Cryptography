LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;


ENTITY Sub_Box IS
    Port ( 
				Clk : IN  STD_LOGIC;
            Input : IN  STD_LOGIC_VECTOR (47 downto 0);
            Output : OUT STD_LOGIC_VECTOR (31 downto 0)
			);
END Sub_Box;

ARCHITECTURE sub_arch OF Sub_Box IS

BEGIN

S1: ENTITY work.S1_box(s1_arch)
	PORT MAP(Clk => Clk, ADDR => Input(47 DOWNTO 42), DOUT => Output(31 DOWNTO 28));

S2: ENTITY work.S2_box(s2_arch)
	PORT MAP(Clk => Clk, ADDR => Input(41 DOWNTO 36), DOUT => Output(27 DOWNTO 24));

S3: ENTITY work.S3_box(s3_arch)
	PORT MAP(Clk => Clk, ADDR => Input(35 DOWNTO 30), DOUT => Output(23 DOWNTO 20));

S4: ENTITY work.S4_box(s4_arch)
	PORT MAP(Clk => Clk, ADDR => Input(29 DOWNTO 24), DOUT => Output(19 DOWNTO 16));

S5: ENTITY work.S5_box(s5_arch)
	PORT MAP(Clk => Clk, ADDR => Input(23 DOWNTO 18), DOUT => Output(15 DOWNTO 12));

S6: ENTITY work.S6_box(s6_arch)
	PORT MAP(Clk => Clk, ADDR => Input(17 DOWNTO 12), DOUT => Output(11 DOWNTO 8));

S7: ENTITY work.S7_box(s7_arch)
	PORT MAP(Clk => Clk, ADDR => Input(11 DOWNTO 6), DOUT => Output(7 DOWNTO 4));

S8: ENTITY work.S8_box(s8_arch)
	PORT MAP(Clk => Clk, ADDR => Input(5 DOWNTO 0), DOUT => Output(3 DOWNTO 0));


end sub_arch;

