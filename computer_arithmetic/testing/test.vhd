
LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
use IEEE.std_logic_unsigned.all;
 
-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--USE ieee.numeric_std.ALL;
 
ENTITY test IS
END test;
 
ARCHITECTURE behavior OF test IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT Problem6
    PORT(
         Clk : IN  std_logic;
         a : IN  std_logic_vector(3 downto 0);
         b : IN  std_logic;
         AND_OUT : OUT  std_logic_vector(3 downto 0);
         XOR_OUT : OUT  std_logic_vector(3 downto 0)
        );
    END COMPONENT;
    

   --Inputs
   signal Clk : std_logic := '0';
   signal a : std_logic_vector(3 downto 0) := (others => '0');
   signal b : std_logic := '0';

 	--Outputs
   signal AND_OUT : std_logic_vector(3 downto 0);
   signal XOR_OUT : std_logic_vector(3 downto 0);

   -- Clock period definitions
   constant Clk_period : time := 10 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: Problem6 PORT MAP (
          Clk => Clk,
          a => a,
          b => b,
          AND_OUT => AND_OUT,
          XOR_OUT => XOR_OUT
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
      wait for 100 ns;	
		a <= "0000";
		b <= '0';
		WAIT FOR Clk_period;
		a <= "0001";
		WAIT FOR Clk_period;
		a <= "0010";
		WAIT FOR Clk_period;
		a <= "0011";
		WAIT FOR Clk_period;
		a <= "0100";
		WAIT FOR Clk_period;
		a <= "0101";
		WAIT FOR Clk_period;
		a <= "0110";
		WAIT FOR Clk_period;
		a <= "0111";
		WAIT FOR Clk_period;
		a <= "1000";
		WAIT FOR Clk_period;
		a <= "1001";
		WAIT FOR Clk_period;
		a <= "1010";
		WAIT FOR Clk_period;
		a <= "1011";
		WAIT FOR Clk_period;
		a <= "1100";
		WAIT FOR Clk_period;
		a <= "1101";
		WAIT FOR Clk_period;
		a <= "1110";
		WAIT FOR Clk_period;
		a <= "1111";
		WAIT FOR Clk_period;
		a <= "0000";
		b <= '1';
		WAIT FOR Clk_period;
		a <= "0001";
		WAIT FOR Clk_period;
		a <= "0010";
		WAIT FOR Clk_period;
		a <= "0011";
		WAIT FOR Clk_period;
		a <= "0100";
		WAIT FOR Clk_period;
		a <= "0101";
		WAIT FOR Clk_period;
		a <= "0110";
		WAIT FOR Clk_period;
		a <= "0111";
		WAIT FOR Clk_period;
		a <= "1000";
		WAIT FOR Clk_period;
		a <= "1001";
		WAIT FOR Clk_period;
		a <= "1010";
		WAIT FOR Clk_period;
		a <= "1011";
		WAIT FOR Clk_period;
		a <= "1100";
		WAIT FOR Clk_period;
		a <= "1101";
		WAIT FOR Clk_period;
		a <= "1110";
		WAIT FOR Clk_period;
		a <= "1111";
		
	WAIT;
		

   end process;

END;
