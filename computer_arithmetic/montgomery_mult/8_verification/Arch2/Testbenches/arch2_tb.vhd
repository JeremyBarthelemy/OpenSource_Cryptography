
LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;
use ieee.std_logic_textio.all;

library std;
use std.textio.all;
 
entity arch2_tb is
end arch2_tb;
 
ARCHITECTURE behavior OF arch2_tb IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT Architecture2
	 GENERIC( op_size : INTEGER := 1024; w : INTEGER := 32; e : INTEGER := 32); --e is the number of processing elements required i.e. size(operand)/size(word)
	 PORT(
			--Inputs
			CLOCK : IN STD_LOGIC;
			RESET : IN STD_LOGIC;
			ENABLE : IN STD_LOGIC;
			X_in : IN STD_LOGIC;
			Y_in : IN STD_LOGIC_VECTOR(1023 DOWNTO 0);
			M_in : IN STD_LOGIC_VECTOR(1023 DOWNTO 0);
			--Outputs
--			C_out : OUT STD_LOGIC_VECTOR();
			S_out : OUT STD_LOGIC_VECTOR(1023 DOWNTO 0)
        );
    END COMPONENT;

   --Inputs
	SIGNAL CLOCK : STD_LOGIC;
	SIGNAL RESET : STD_LOGIC;
	SIGNAL ENABLE : STD_LOGIC;
	SIGNAL X_in : STD_LOGIC;
	SIGNAL Y_in : STD_LOGIC_VECTOR(1023 DOWNTO 0);
	SIGNAL M_in : STD_LOGIC_VECTOR(1023 DOWNTO 0);
	
	type vector is record
		y_in : std_logic_vector(1023 downto 0);
		m_in : std_logic_vector(1023 downto 0);
	end record;
		
	type vectorArray is array (0 to 63) of vector;
	constant X : std_logic_vector(64 downto 0) := '0' & x"0000000000000001";
	constant tv_array : vectorArray := (
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
	
	--Outputs
	SIGNAL S_out : STD_LOGIC_VECTOR(1023 DOWNTO 0);
	SIGNAL s_comp : STD_LOGIC_VECTOR(1023 DOWNTO 0);

   -- Clock period definitions
   constant CLOCK_period : time := 10 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: Architecture2 PORT MAP (
	
				CLOCK =>CLOCK,
			RESET =>RESET,
			ENABLE =>ENABLE,
			X_in =>X_in,
			Y_in =>Y_in,
			M_in =>M_in,
			--Outputs
			S_out =>S_out
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
	variable myLine : line;
   begin		
		RESET <= '1';
      -- hold reset state for 100 ns.
      wait for 100 ns;	
		
		ENABLE <= '1';
		RESET <= '0';
--		m <= (OTHERS =>'0');
--		y <= (others =>'1');
		for index in 0 to 15 loop		
			y_in <= tv_array(index).y_in;
			m_in <= tv_array(index).m_in;
			wait for CLOCK_PERIOD;
		end loop;
		
		for i in 0 to 16 loop
			s_comp((15 + (16*i)) downto (16*i)) <= S_out;
			wait for CLOCK_PERIOD;
		end loop;
		
		for i in 0 to 16 loop			
			if(s_comp((15 + (16*i)) downto (16*i)) /= S_out) then
				report "Actual word of S_out does the same as expected value!!!!!!"
					severity error;
				write(myLine, string'(" Actual word : "));
				hwrite(myline, S_out);
				writeline(output, myLine);
				write(myLine, string'("Expected word: "));
				hwrite(myline, s_comp((1023 + (1024*i)) downto (1024*i)));
				writeline(output, myLine);
			end if;
			wait for CLOCK_PERIOD;
		end loop;
		
      wait;
   end process;

END;
