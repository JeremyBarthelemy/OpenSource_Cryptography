LIBRARY ieee;
USE ieee.std_logic_1164.ALL;

USE ieee.std_logic_unsigned.ALL;  
 
ENTITY testing_sboxes IS
END testing_sboxes;
 
ARCHITECTURE behavior OF testing_sboxes IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT ROUND
    PORT(
         test_addr : IN  std_logic_vector(5 downto 0);
         test_dout : OUT  std_logic_vector(3 downto 0);
         Clk : IN  std_logic
        );
    END COMPONENT;
    

   --Inputs
   signal test_addr : std_logic_vector(5 downto 0) := (others => '0');
   signal Clk : std_logic := '0';

 	--Outputs
   signal test_dout : std_logic_vector(3 downto 0);

   -- Clock period definitions
   constant Clk_period : time := 10 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: ROUND PORT MAP (
          test_addr => test_addr,
          test_dout => test_dout,
          Clk => Clk
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
		test_addr <= test_addr + 1;
		
   end process;

END;
