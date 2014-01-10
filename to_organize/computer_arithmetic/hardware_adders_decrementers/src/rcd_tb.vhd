LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
 
-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--USE ieee.numeric_std.ALL;
 
ENTITY rcd_tb IS
END rcd_tb;
 
ARCHITECTURE behavior OF rcd_tb IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT RCD
    PORT(
         CLK : IN  std_logic;
         X : IN  std_logic_vector(3 downto 0);
         S : OUT  std_logic_vector(3 downto 0);
         C : OUT  std_logic;
         V : OUT  std_logic
        );
    END COMPONENT;
    

   --Inputs
   signal CLK : std_logic := '0';
   signal X : std_logic_vector(3 downto 0) := (others => '0');

 	--Outputs
   signal S : std_logic_vector(3 downto 0);
   signal C : std_logic;
   signal V : std_logic; 

   -- Clock period definitions
   constant CLK_period : time := 10 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: RCD PORT MAP (
          CLK => CLK,
          X => X,
          S => S,
          C => C,
          V => V
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
		X <= "1010";
      wait for CLK_period*10;

      -- insert stimulus here 

      wait;
   end process;

END;
