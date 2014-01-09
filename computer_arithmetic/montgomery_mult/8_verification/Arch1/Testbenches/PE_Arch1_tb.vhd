----------------------------------------------------------------------------------
-- ECE 645 
-- 
--
--
----------------------------------------------------------------------------------
LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;
use ieee.std_logic_textio.all;

library std;
use std.textio.all;
 
ENTITY PE_Arch1_tb IS
END PE_Arch1_tb;
 
ARCHITECTURE behavior OF PE_Arch1_tb IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT PE_arch1
    PORT(
         CLK : IN  std_logic;
         RSTN : IN  std_logic;
         EN : IN  std_logic;
         QEN : IN  std_logic;
         Y : IN  std_logic_vector(15 downto 0);
         M : IN  std_logic_vector(15 downto 0);
         S_IN : IN  std_logic_vector(15 downto 0);
         X : IN  std_logic;
         S_OUT : OUT  std_logic_vector(15 downto 0)
        );
    END COMPONENT;
    

   --Inputs
   signal CLK : std_logic := '0';
   signal RSTN : std_logic := '0';
   signal EN : std_logic := '0';
   signal QEN : std_logic := '0';
   signal Y : std_logic_vector(15 downto 0) := (others => '0');
   signal M : std_logic_vector(15 downto 0) := (others => '0');
   signal S_IN : std_logic_vector(15 downto 0) := (others => '0');
   signal X : std_logic := '0';

	type vector is record
		s_input : std_logic_vector(15 downto 0);
		y_in : std_logic_vector(15 downto 0);
		m_in : std_logic_vector(15 downto 0);
		s_result : std_logic_vector(15 downto 0);
	end record;	
		
	type vectorArray is array (0 to 5) of vector;
	constant PE_vectorTable : vectorArray := (
		(s_input => x"0000", y_in => x"0005", m_in => x"0051", s_result => x"0000"),		
		(s_input => x"0001", y_in => x"25A0", m_in => x"0158", s_result => x"0005"),		
		(s_input => x"0000", y_in => x"FFFF", m_in => x"FFFF", s_result => x"A6F8"),	
		(s_input => x"0001", y_in => x"FFFF", m_in => x"FFFF", s_result => x"FFFE"),		
		(s_input => x"0000", y_in => x"1010", m_in => x"0101", s_result => x"8000"),	
		(s_input => x"0001", y_in => x"0000", m_in => x"0000", s_result => x"1012")
	);

 	--Outputs
   signal S_OUT : std_logic_vector(15 downto 0);

   -- Clock period definitions
   constant CLK_period : time := 10 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: PE_arch1 PORT MAP (
          CLK => CLK,
          RSTN => RSTN,
          EN => EN,
          QEN => QEN,
          Y => Y,
          M => M,
          S_IN => S_IN,
          X => X,
          S_OUT => S_OUT
        );

   -- Clock process definitions
   CLK_process :process
   begin
		CLK <= '0';
		wait for CLK_period/2;
		CLK <= '1';
		wait for CLK_period/2;
   end process;
 

   -- Stimulus process
   stim_proc: process
	variable myLine : line;
   begin		
      -- hold reset state for 100 ns.
		RSTN <= '1';
      wait for 100 ns;	
		
		RSTN <= '0';
		X <= '1';
		EN <= '1';
		QEN <= '1';
		
		for index in 0 to 5 loop
			S_IN <= PE_vectorTable(index).s_input;
			Y <= PE_vectorTable(index).y_in;
			M <= PE_vectorTable(index).m_in;
			
			if(PE_vectorTable(index).s_result /= S_OUT) then
				report "Actual S does not equal expected!"
					severity error;
				write(myLine, string'(" Actual word : "));
				hwrite(myline, S_OUT);
				writeline(output, myLine);
				write(myLine, string'("Expected word: "));
				hwrite(myline, PE_vectorTable(index).s_result);
				writeline(output, myLine);
			end if;			
			wait for CLK_period;
		end loop;
		
      wait;
   end process;

END;
