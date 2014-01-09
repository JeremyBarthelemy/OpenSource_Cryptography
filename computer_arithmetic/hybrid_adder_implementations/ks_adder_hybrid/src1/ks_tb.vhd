LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
 
-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--USE ieee.numeric_std.ALL;
 
ENTITY ks_tb IS
END ks_tb;
 
ARCHITECTURE behavior OF ks_tb IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT KoggeStone
    PORT(
			CLOCK : IN STD_LOGIC;
         X : IN  std_logic_vector(255 downto 0);
         Y : IN  std_logic_vector(255 downto 0);
         S : OUT  std_logic_vector(255 downto 0);
         Cin : IN  std_logic;
         test_sum : OUT  std_logic_vector(255 downto 0);
         test_sig : OUT  std_logic_vector(255 downto 0);
         Cout : OUT  std_logic
        );
    END COMPONENT;
    
	 
   --Inputs
   signal X : std_logic_vector(255 downto 0) := (others => '0');
   signal Y : std_logic_vector(255 downto 0) := (others => '0');
   signal Cin : std_logic := '0';
	SIGNAL CLOCK : STD_LOGIC := '0';

 	--Outputs
   signal S : std_logic_vector(255 downto 0);
   signal test_sum : std_logic_vector(255 downto 0);
   signal test_sig : std_logic_vector(255 downto 0);
   signal Cout : std_logic;
   -- No clocks detected in port list. Replace CLOCK below with 
   -- appropriate port name 
 
   constant CLOCK_period : time := 10 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: KoggeStone PORT MAP (
			 CLOCK => CLOCK,
          X => X,
          Y => Y,
          S => S,
          Cin => Cin,
          test_sum => test_sum,
          test_sig => test_sig,
          Cout => Cout
        );

   -- Clock process definitions
   CLOCK_process :process
   begin
		CLOCK <= '0';
		wait for CLOCK_period/2;
		CLOCK <= '1';
		wait for CLOCK_period/2;
   end process;
 

   -- Stimulus process
   stim_proc: process
   begin		
      -- hold reset state for 100 ns.
      wait for 100 ns;	
		X <= "1010101010101010101010101010101010101010101010101010101010101010101010111001010011111111111111111111111111111111111111110001011001000000000000111010101010101010111010000001011010101010101010101010101010101010101010101011101010101010000000011010101010101010";
		Y <= "1111111000000000000000011101010101011111111010101010010101010101010101101110111011101110111011101110111011101110111011101110111011101110111011111101001000101011111011101110111111111100010100010011011110000111000100101010111011101111111111111110111011101110";
		Cin <= '0';
      wait for CLOCK_period*10;

      -- insert stimulus here 

      wait;
   end process;

END;
