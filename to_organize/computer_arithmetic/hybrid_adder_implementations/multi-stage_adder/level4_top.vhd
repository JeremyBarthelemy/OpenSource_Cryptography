
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity level4_top is
    Port (
					CLOCK : IN STD_LOGIC;
	 				X : IN STD_LOGIC_VECTOR(31 DOWNTO 0);
					Y : IN STD_LOGIC_VECTOR(31 DOWNTO 0);
					S : OUT STD_LOGIC_VECTOR(31 DOWNTO 0);
					Cin : IN STD_LOGIC;
					Cout : OUT STD_LOGIC;
					MUX_SEL : IN STD_LOGIC;
					g_out : OUT STD_LOGIC;
					p_out : OUT STD_LOGIC
			);
end level4_top;

architecture top_arch of level4_top is
SIGNAL g_input_signal : STD_LOGIC_VECTOR(255 DOWNTO 0);
SIGNAL p_input_signal : STD_LOGIC_VECTOR(255 DOWNTO 0);
SIGNAL TempCout : STD_LOGIC_VECTOR(255 DOWNTO 0);
SIGNAL g_signal : STD_LOGIC_VECTOR(3 DOWNTO 0);
SIGNAL p_signal : STD_LOGIC_VECTOR(3 DOWNTO 0);
SIGNAL carry_signal : STD_LOGIC_VECTOR(3 DOWNTO 1);
SIGNAL X_to_Circuit : STD_LOGIC_VECTOR(255 DOWNTO 0);
SIGNAL Y_to_Circuit : STD_LOGIC_VECTOR(255 DOWNTO 0);
SIGNAL Circuit_to_S : STD_LOGIC_VECTOR(255 DOWNTO 0);
SIGNAL Cin_to_Circuit : STD_LOGIC;
SIGNAL Circuit_to_CoutReg : STD_LOGIC;


BEGIN
--Determine g_in and p_in for the CLGs using X and Y as inputs
--4 level3 entities, as well as 1 CLG entity

IN_REG2: ENTITY work.REG2(reg2_arch)
		 PORT MAP(D=>Cin,Q=>Cin_to_Circuit,ENABLE=>'1',RESET=>'0',CLOCK=> CLOCK);
		 
OUT_REG2: ENTITY work.REG2(reg2_arch)
		 PORT MAP(D=>Circuit_to_CoutReg, Q=>Cout, ENABLE=>'1', RESET=>'0', CLOCK=>CLOCK);
		 

SIPOX : ENTITY work.SIPO(sipo_arch)
		 PORT MAP(Clk=>CLOCK,ENABLE=>'1',RESET=>'0',Input=>X,Output=>X_to_Circuit); 

SIPOY : ENTITY work.SIPO(sipo_arch)
		 PORT MAP(Clk=>CLOCK,ENABLE=>'1',RESET=>'0',Input=>Y,Output=>Y_to_Circuit);

		 
PISO : ENTITY work.PISO(piso_arch)
		 PORT MAP(Clk=>CLOCK,ENABLE=>'1',RESET=>'0',Input=>Circuit_to_S,Output=>S, MUX_SEL=>MUX_SEL);
		 

Circuit_to_S <= TempCout XOR p_input_signal;
--Define Cout
Circuit_to_CoutReg <= (TempCout(255) AND X_to_Circuit(255)) OR (TempCout(255) AND Y_to_Circuit(255)) OR (X_to_Circuit(255) AND Y_to_Circuit(255));

g_input_signal <= X_to_Circuit AND Y_to_Circuit;
p_input_signal <= X_to_Circuit XOR Y_to_Circuit;

LeftLevel3: ENTITY work.level3(level3_arch)
			PORT MAP(
						g_in=>g_input_signal(255 DOWNTO 192),
						p_in=>p_input_signal(255 DOWNTO 192),
						g_out=>g_signal(3),
						p_out=>p_signal(3),
						Cin=>carry_signal(3),
						Cout=>TempCout(255 DOWNTO 192)
			);
Mid1Level3: ENTITY work.level3(level3_arch)
			PORT MAP(
						g_in=>g_input_signal(191 DOWNTO 128),
						p_in=>p_input_signal(191 DOWNTO 128),
						g_out=>g_signal(2),
						p_out=>p_signal(2),
						Cin=>carry_signal(2),
						Cout=>TempCout(191 DOWNTO 128)
			);
Mid2Level3: ENTITY work.level3(level3_arch)
			PORT MAP(
						g_in=>g_input_signal(127 DOWNTO 64),
						p_in=>p_input_signal(127 DOWNTO 64),
						g_out=>g_signal(1),
						p_out=>p_signal(1),
						Cin=>carry_signal(1),
						Cout=>TempCout(127 DOWNTO 64)
			);
RightLevel3: ENTITY work.level3(level3_arch)
			PORT MAP(
						g_in=>g_input_signal(63 DOWNTO 0),
						p_in=>p_input_signal(63 DOWNTO 0),
						g_out=>g_signal(0),
						p_out=>p_signal(0),
						Cin=>Cin_to_Circuit,
						Cout=>TempCout(63 DOWNTO 0)
			);

CLG_Bottom : ENTITY work.CLG(clg_arch)				
				 PORT MAP(
								g_in=>g_signal,
								p_in=>p_signal,
								g_out=>g_out,
								p_out=>p_out,
								Cin =>Cin_to_Circuit,
								Cout=>carry_signal
							);

end top_arch;