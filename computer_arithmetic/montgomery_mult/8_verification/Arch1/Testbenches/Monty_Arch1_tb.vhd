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
 
ENTITY Monty_Arch1_tb IS
END Monty_Arch1_tb;
 
ARCHITECTURE behavior OF Monty_Arch1_tb IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT MontyMult_Arch1
    PORT(
         CLK : IN  std_logic;
         WE : IN  std_logic;
         EN : IN  std_logic;
         RSTN : IN  std_logic;
         QEN : IN  std_logic_vector(64 downto 0);
         Y : IN  std_logic_vector(15 downto 0);
         M : IN  std_logic_vector(15 downto 0);
         X : IN  std_logic_vector(64 downto 0);
         ADDR : IN  std_logic_vector(6 downto 0);
         S : OUT  std_logic_vector(15 downto 0)
        );
    END COMPONENT;
    

   --Inputs
   signal CLK : std_logic := '0';
   signal WE : std_logic := '0';
   signal EN : std_logic := '0';
   signal RSTN : std_logic := '0';
	signal QEN : std_logic_vector(64 downto 0) := (others => '0');
   signal Y : std_logic_vector(15 downto 0) := (others => '0');
   signal M : std_logic_vector(15 downto 0) := (others => '0');
   signal X : std_logic_vector(64 downto 0) := (others => '0');
   signal ADDR : std_logic_vector(6 downto 0) := (others => '0');

	type vector is record
		y_in : std_logic_vector(15 downto 0);
		m_in : std_logic_vector(15 downto 0);
	end record;	
		
	type vectorArray is array (0 to 63) of vector;
	constant X_first : std_logic_vector(64 downto 0) := '0' & x"0000000000000001";
	constant MontyMult_vectorTable_first : vectorArray := (
		(y_in => x"0000", m_in => x"0003"),		(y_in => x"A15C", m_in => x"0000"),		(y_in => x"5005", m_in => x"0000"),		(y_in => x"38DD", m_in => x"0000"),
		(y_in => x"1580", m_in => x"0000"),		(y_in => x"1111", m_in => x"0000"),		(y_in => x"0013", m_in => x"84A0"),		(y_in => x"48A0", m_in => x"0100"),
		(y_in => x"0000", m_in => x"001A"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),
		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),
		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),
		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),
		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),
		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),
		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),
		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),
		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),
		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),
		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),
		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),
		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),
		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000")
	);	
	constant X_second : std_logic_vector(64 downto 0) := '0' & x"0000000000010000";
	constant MontyMult_vectorTable_second : vectorArray := (
		(y_in => x"A15C", m_in => x"0003"),		(y_in => x"5005", m_in => x"0000"),		(y_in => x"38DD", m_in => x"0000"),		(y_in => x"1580", m_in => x"0000"),
		(y_in => x"1111", m_in => x"0000"),		(y_in => x"0013", m_in => x"0000"),		(y_in => x"48A0", m_in => x"84A0"),		(y_in => x"0000", m_in => x"0100"),
		(y_in => x"0000", m_in => x"001A"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),
		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),
		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),
		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),
		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),
		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),
		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),
		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),
		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),
		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),
		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),
		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),
		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),
		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000"),		(y_in => x"0000", m_in => x"0000")
	);
 	--Outputs
   signal S : std_logic_vector(15 downto 0);
	signal s_comp : std_logic_vector(1023 downto 0) := (others => '0');

   -- Clock period definitions
   constant CLK_period : time := 10 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: MontyMult_Arch1 PORT MAP (
          CLK => CLK,
          WE => WE,
          EN => EN,
          RSTN => RSTN,
			 QEN => QEN,
          Y => Y,
          M => M,
          X => X,
          ADDR => ADDR,
          S => S
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
		RSTN <= '1';
      -- hold reset state for 100 ns.
      wait for 100 ns;	
		
		WE <= '1';
		RSTN <= '0';
		
		for index in 0 to 63 loop
			ADDR <= std_logic_vector(to_unsigned(index, 7));			
			Y <= MontyMult_vectorTable_first(index).y_in;
			M <= MontyMult_vectorTable_first(index).m_in;
			wait for clk_period;
		end loop;

		WE <= '0';
		EN <= '1';
		
		X <= X_first;				
		for i in 0 to 64 loop
			QEN <= (i => '1', others => '0');
			ADDR <= std_logic_vector(to_unsigned(i, 7));
			wait for clk_period;
		end loop;
		
		for i in 0 to 16 loop
			s_comp((15 + (16*i)) downto (16*i)) <= S;
			wait for clk_period;
		end loop;
		
		EN <= '0';
		RSTN <= '1';
      -- hold reset state for 100 ns.
      wait for 100 ns;	
		
		WE <= '1';
		RSTN <= '0';

		for index in 0 to 63 loop
			ADDR <= std_logic_vector(to_unsigned(index, 7));			
			Y <= MontyMult_vectorTable_second(index).y_in;
			M <= MontyMult_vectorTable_second(index).m_in;
			wait for clk_period;
		end loop;

		WE <= '0';
		EN <= '1';
		
		X <= X_second;								
		for i in 0 to 64 loop
			QEN <= (i => '1', others => '0');
			ADDR <= std_logic_vector(to_unsigned(i, 7));
			wait for clk_period;
		end loop;
		
		for i in 0 to 16 loop			
			if(s_comp((15 + (16*i)) downto (16*i)) /= S) then
				report "Actual word of S does not equal expected!"
					severity error;
				write(myLine, string'(" Actual word : "));
				hwrite(myline, S);
				writeline(output, myLine);
				write(myLine, string'("Expected word: "));
				hwrite(myline, s_comp((15 + (16*i)) downto (16*i)));
				writeline(output, myLine);
			end if;			
			wait for clk_period;
		end loop;
		
		-- Done.
      wait;
   end process;

END;
