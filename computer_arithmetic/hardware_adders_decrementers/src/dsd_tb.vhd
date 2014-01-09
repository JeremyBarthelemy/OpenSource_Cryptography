LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
 
-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--USE ieee.numeric_std.ALL;
 
ENTITY dsd_tb IS
END dsd_tb;
 
ARCHITECTURE behavior OF dsd_tb IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT DSD
    PORT(
         X : IN  std_logic_vector(3 downto 0);
         START : IN  std_logic;
         CLK : IN  std_logic;
         C : OUT  std_logic;
         V : OUT  std_logic;
         S : OUT  std_logic_vector(3 downto 0)
        );
    END COMPONENT;
    

   --Inputs
   signal X : std_logic_vector(3 downto 0) := (others => '0');
   signal START : std_logic := '0';
   signal CLK : std_logic := '0';

 	--Outputs
   signal C : std_logic;
   signal V : std_logic;
   signal S : std_logic_vector(3 downto 0);

   -- Clock period definitions
   constant CLK_period : time := 10 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: DSD PORT MAP (
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
 

   -- Stimulus process
   stim_proc: process
   begin		
      -- hold reset state for 100 ns.
      wait for 100 ns;	

      wait for CLK_period*10;
		X <= "0000";
		START <= '1';
		WAIT FOR CLK_PERIOD;
		START <= '0';
		X <= "0000";
		WAIT FOR CLK_PERIOD;
		X <= "1000";
		WAIT FOR CLK_PERIOD;

      -- insert stimulus here 

      wait;
   end process;

END;
