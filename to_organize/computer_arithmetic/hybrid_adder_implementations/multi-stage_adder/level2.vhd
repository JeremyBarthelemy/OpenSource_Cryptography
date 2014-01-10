library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

ENTITY level2 IS
    PORT(
				g_in : IN STD_LOGIC_VECTOR(15 downto 0);
				p_in : IN STD_LOGIC_VECTOR(15 downto 0);
				g_out : OUT STD_LOGIC;
				p_out : OUT STD_LOGIC;
				Cin : IN STD_LOGIC;
				Cout : OUT STD_LOGIC_VECTOR(15 downto 0)
			);
END level2;

ARCHITECTURE level2_arch OF level2 IS
SIGNAL g_signal : STD_LOGIC_VECTOR(3 DOWNTO 0);
SIGNAL p_signal : STD_LOGIC_VECTOR(3 DOWNTO 0);
SIGNAL carry_signal : STD_LOGIC_VECTOR(3 DOWNTO 1);

BEGIN

--5 CLG Entities
--Passing the Cin values up:(VERIFY THAT Cin = C0 for this case)
Cout(12) <= g_in(11) OR (g_in(10) AND p_in(11)) OR (g_in(9) AND p_in(10) AND p_in(11)) OR 
(g_in(8) AND p_in(9) AND p_in(10) AND p_in(11)) OR (Cin AND p_in(8) AND p_in(9) AND p_in(10) AND p_in(11));
Cout(8) <= g_in(11) OR (g_in(10) AND p_in(11)) OR (g_in(9) AND p_in(10) AND p_in(11)) OR 
(Cin AND p_in(9) AND p_in(10) AND p_in(11));
Cout(4) <= g_in(11) OR (g_in(10) AND p_in(11)) OR (Cin AND p_in(10) AND p_in(11));
Cout(0) <= Cin;


CLG1_Left : ENTITY work.CLG(clg_arch)
				--GENERIC MAP(k => 4)
				PORT MAP(
								g_in=> g_in(15 DOWNTO 12),
								p_in=>p_in(15 DOWNTO 12),
								g_out=>g_signal(3),
								p_out=> p_signal(3),
								Cin => carry_signal(3),
								Cout=> Cout(15 DOWNTO 13)
							);

CLG2 : ENTITY work.CLG(clg_arch)
		 --GENERIC MAP(k => 4)
		 PORT MAP(
							g_in=> g_in(11 DOWNTO 8),
							p_in=>p_in(11 DOWNTO 8),
							g_out=>g_signal(2),
							p_out=>p_signal(2),
							Cin => carry_signal(2),
							Cout=> Cout(11 DOWNTO 9)
					);

CLG3 : ENTITY work.CLG(clg_arch)
		 --GENERIC MAP(k => 4)
		 PORT MAP(
							g_in=> g_in(7 DOWNTO 4),
							p_in=>p_in(7 DOWNTO 4),							
							g_out=>g_signal(1),
							p_out=>p_signal(1),
							Cin => carry_signal(1),
							Cout=>Cout(7 DOWNTO 5)
					);

CLG4_Right : ENTITY work.CLG(clg_arch)
				 --GENERIC MAP(k => 4)
				 PORT MAP(
								g_in=> g_in(3 DOWNTO 0),
								p_in=>p_in(3 DOWNTO 0),
								g_out=>g_signal(0),
								p_out=>p_signal(0),							
								Cin => Cin,							
								Cout=>Cout(3 DOWNTO 1)
							);

CLG_Bottom : ENTITY work.CLG(clg_arch)
				 --GENERIC MAP(k => 16)
				 PORT MAP(														
								g_in=> g_signal,
								p_in=> p_signal,
								g_out=> g_out,
								p_out=> p_out,
								Cin=> Cin,
								Cout=> carry_signal
							);


END level2_arch;

