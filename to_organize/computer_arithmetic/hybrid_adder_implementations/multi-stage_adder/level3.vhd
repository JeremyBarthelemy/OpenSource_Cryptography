library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity level3 is
    Port (
	 			g_in : in  STD_LOGIC_VECTOR (63 downto 0);
				p_in : in  STD_LOGIC_VECTOR (63 downto 0);
				g_out : OUT STD_LOGIC;
				p_out : OUT STD_LOGIC;
				Cin : IN STD_LOGIC;
				Cout : out  STD_LOGIC_VECTOR (63 downto 0)
			);
end level3;

architecture level3_arch of level3 is
SIGNAL g_signal : STD_LOGIC_VECTOR(3 DOWNTO 0);
SIGNAL p_signal : STD_LOGIC_VECTOR(3 DOWNTO 0);
SIGNAL carry_signal : STD_LOGIC_VECTOR(3 DOWNTO 1);

begin
--4 level2 entities, as well as 1 CLG entity

LeftLevel2 : ENTITY work.level2(level2_arch)
    PORT MAP(
					g_in=>g_in(63 DOWNTO 48),
					p_in=>p_in(63 DOWNTO 48),
					g_out=>g_signal(3),
					p_out=>p_signal(3),
					Cin=>carry_signal(3),
					Cout=>Cout(63 DOWNTO 48)
				);

Mid1Level2 : ENTITY work.level2(level2_arch)
    PORT MAP(
					g_in=>g_in(47 DOWNTO 32),
					p_in=>p_in(47 DOWNTO 32),
					g_out=>g_signal(2),
					p_out=>p_signal(2),
					Cin=>carry_signal(2),
					Cout=>Cout(47 DOWNTO 32)
				);

Mid2Level2 : ENTITY work.level2(level2_arch)
    PORT MAP(
					g_in=>g_in(31 DOWNTO 16),
					p_in=>p_in(31 DOWNTO 16),
					g_out=>g_signal(1),
					p_out=>p_signal(1),
					Cin=>carry_signal(1),
					Cout=>Cout(31 DOWNTO 16)
				);

RightLevel2 : ENTITY work.level2(level2_arch)
				  PORT MAP(
							g_in=>g_in(15 DOWNTO 0),
							p_in=>p_in(15 DOWNTO 0),
							g_out=>g_signal(0),
							p_out=>p_signal(0),
							Cin=>Cin,
							Cout=>Cout(15 DOWNTO 0)
				);

CLG_Bottom : ENTITY work.CLG(clg_arch)				
				 PORT MAP(
								g_in=>g_signal,
								p_in=>p_signal,
								g_out=>g_out,
								p_out=>p_out,
								Cin =>Cin,
								Cout=>carry_signal
							);

end level3_arch;
