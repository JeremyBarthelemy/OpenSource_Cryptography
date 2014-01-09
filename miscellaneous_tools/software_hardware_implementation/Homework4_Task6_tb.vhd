LIBRARY ieee;
USE ieee.std_logic_1164.all;

ENTITY Homework4_Task6_tb IS
END Homework4_Task6_tb;

ARCHITECTURE arch_Homework4_Task6_tb OF Homework4_Task6_tb IS

COMPONENT Homework4_Task6 IS
	GENERIC(w : INTEGER := 8);
	PORT(	
			rst : IN STD_LOGIC;
			clk : IN STD_LOGIC;
			DOUT : IN STD_LOGIC_VECTOR(w-1 DOWNTO 0);
			WR : IN STD_LOGIC;
			ADDR : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
			RD : IN STD_LOGIC;						
			sel : IN STD_LOGIC_VECTOR(2 DOWNTO 0);
			DIN : OUT STD_LOGIC_VECTOR(w-1 DOWNTO 0)
		);
END COMPONENT;
														 
CONSTANT ClkPeriod : TIME := 20 ns;
CONSTANT w : INTEGER := 8;
SIGNAL test_clk : STD_LOGIC := '0';
SIGNAL test_rst : STD_LOGIC := '1';
SIGNAL test_DOUT : STD_LOGIC_VECTOR(w-1 DOWNTO 0);
SIGNAL test_WR : STD_LOGIC := '0';
SIGNAL test_ADDR : STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL test_RD : STD_LOGIC := '0';
SIGNAL test_DIN : STD_LOGIC_VECTOR(w-1 DOWNTO 0);
SIGNAL test_sel : STD_LOGIC_VECTOR(2 DOWNTO 0) := "000";		
			
BEGIN								
	
	test_clk <= NOT test_clk AFTER ClkPeriod/2;
	
UUT : Homework4_Task6
	  GENERIC MAP(w => 8)
	  PORT MAP(
	  			rst=>test_rst,
				clk=>test_clk,
				DOUT=>test_DOUT,
				WR=>test_WR,
				ADDR=>test_ADDR,
				RD=>test_RD,
				sel=>test_sel,
				DIN=>test_DIN
			  ); 

			  

			  
veify: PROCESS
VARIABLE Count : INTEGER := 0; 
BEGIN	   	 						 
		
	WAIT FOR 20 ns;
	test_rst <= '0';
	test_DOUT <=  "11110000";
	test_WR <= '0';
	test_ADDR <= "1100100111110000";
	test_RD <= '0';
	test_sel <= "000";
	WAIT FOR 20 ns;
	test_WR <= '1';
	WAIT FOR 20 ns;
	test_RD <= '1';
	WAIT FOR 20 ns;
	test_sel <= "001";
	test_ADDR <= "1100101111110000";
	WAIT FOR 20 ns;
	test_sel <= "010";
	WAIT FOR 20 ns;
	test_sel <= "011";				
	test_ADDR <= "1100110111110000";
	WAIT FOR 20 ns;	
	test_ADDR <= "1100111111110000";
	WAIT FOR 20 ns;
	test_sel <= "100";
	WAIT FOR 20 ns;
	test_ADDR <= "1100110111110001";
	WAIT FOR 20 ns;
	test_ADDR <= "1100101111110010";
	WAIT FOR 20 ns;
	test_ADDR <= "1100100111110011";	  
	WAIT;
	
	
END PROCESS;

END arch_Homework4_Task6_tb;		 
