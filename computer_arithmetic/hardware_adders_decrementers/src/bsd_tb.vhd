LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
 
-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--USE ieee.numeric_std.ALL;
 
ENTITY bsd_tb IS
END bsd_tb;
 
ARCHITECTURE behavior OF bsd_tb IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT BSD
    PORT(
         X : IN  std_logic;
         START : IN  std_logic;
         CLK : IN  std_logic;
         C : OUT  std_logic;
         V : OUT  std_logic;
         S : OUT  std_logic
        );
    END COMPONENT;
    

   --Inputs
   signal X : std_logic := '0';
   signal START : std_logic := '0';
   signal CLK : std_logic := '0';

 	--Outputs
   signal C : std_logic;
   signal V : std_logic;
   signal S : std_logic;

   -- Clock period definitions
   constant CLK_period : time := 10 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: BSD PORT MAP (
          X => X,
          START => START,
          CLK => CLK,
          C => C,
          V => V,
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
 

   --  Stimulus process
   stim_proc: process
   begin		
      -- hold reset state for 100 ns.
		WAIT FOR CLK_PERIOD*10;
      wait for 100 ns;			
		START <= '1';
		X <= '0';
		WAIT FOR CLK_PERIOD;
		START <= '0';
		X <= '0';
		WAIT for CLK_PERIOD;
		X <= '0';
		WAIT FOR CLK_PERIOD;
		X <= '1';

      -- insert stimulus here 

      wait;
   end process;

END;
