LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE IEEE.std_logic_unsigned.all;
USE ieee.std_logic_textio.all;

LIBRARY std;
USE std.textio.all;
 
ENTITY top_tb IS
END top_tb;
 
ARCHITECTURE behavior OF top_tb IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT level4_top
    PORT(
         CLOCK : IN  std_logic;
         X : IN  std_logic_vector(31 downto 0);
         Y : IN  std_logic_vector(31 downto 0);
         S : OUT  std_logic_vector(31 downto 0);
         Cin : IN  std_logic;
         Cout : OUT  std_logic;
         MUX_SEL : IN  std_logic;
         g_out : OUT  std_logic;
         p_out : OUT  std_logic
        );
    END COMPONENT;
    

   --Inputs
   signal CLOCK : std_logic := '0';
   signal X : std_logic_vector(31 downto 0) := (others => '0');
   signal Y : std_logic_vector(31 downto 0) := (others => '0');
	signal test_sum : STD_LOGIC_VECTOR(31 DOWNTO 0) := (others => '0');
   signal Cin : std_logic := '0';
   signal MUX_SEL : std_logic := '0';

 	--Outputs
   signal S : std_logic_vector(31 downto 0);
   signal Cout : std_logic;
   signal g_out : std_logic;
   signal p_out : std_logic;

   -- Clock period definitions
   constant CLOCK_period : time := 10 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: level4_top PORT MAP (
          CLOCK => CLOCK,
          X => X,
          Y => Y,
          S => S,
          Cin => Cin,
          Cout => Cout,
          MUX_SEL => MUX_SEL,
          g_out => g_out,
          p_out => p_out
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
   verify: process
	variable ErrorMsg: LINE;
   BEGIN		
      -- hold reset state for 100 ns.
      wait for 100 ns;	
		Cin <= '0';
		Y <= "01110101010100110101001010111010";
		WAIT FOR CLOCK_PERIOD*9;
		MUX_SEL <= '1';
      wait for CLOCK_period*10;
		test_sum <= X + Y;
		IF S /= test_sum THEN
        write(ErrorMsg, STRING'("Test Failed "));
        write(ErrorMsg, now);
        writeline(output, ErrorMsg);
		END IF;
		WAIT FOR CLOCK_PERIOD;
		X <= X + 1;
		MUX_SEL <= '0';
		--Some delay issues with the SIPO and PISO with regard to the test_sum (and thus there are false positive failures),
		--although the values given as output were tested and verified for the case without the two
   END PROCESS;

END;
