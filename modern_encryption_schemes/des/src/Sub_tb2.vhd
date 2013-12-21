LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
 
USE ieee.std_logic_unsigned.ALL; 
 
ENTITY Sub_tb2 IS
END Sub_tb2;
 
ARCHITECTURE behavior OF Sub_tb2 IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT Sub_Box
    PORT(
         Clk : IN  std_logic;
         Input : IN  std_logic_vector(47 downto 0);
         Output : OUT  std_logic_vector(31 downto 0)
        );
    END COMPONENT;
    

   --Inputs
   signal Clk : std_logic := '0';
   signal Input : std_logic_vector(47 downto 0) := (others => '0');

 	--Outputs
   signal Output : std_logic_vector(31 downto 0);

   -- Clock period definitions
   constant Clk_period : time := 10 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: Sub_Box PORT MAP (
          Clk => Clk,
          Input => Input,
          Output => Output
        );

   -- Clock process definitions
   Clk_process :process
   begin
		Clk <= '0';
		wait for Clk_period/2;
		Clk <= '1';
		wait for Clk_period/2;
   end process;
 

   -- Stimulus process
   stim_proc: process
   begin		
      -- hold reset state for 100 ns.
      wait for 10 ns;	

      wait for Clk_period*1;

      -- insert stimulus here 
		Input <= Input + 1;
		
   end process;

END;
