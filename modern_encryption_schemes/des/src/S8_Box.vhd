library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity S8_Box is
    Port ( Clk : in  STD_LOGIC;
           ADDR : in  STD_LOGIC_VECTOR (5 downto 0);
           DOUT : out  STD_LOGIC_VECTOR (3 downto 0));
end S8_Box;

architecture s8_arch of S8_Box is

type mem is array (0 to 2**6-1) of STD_LOGIC_VECTOR(3 downto 0);

CONSTANT my_rom : mem := (

	--Row 1
	0 => "1101",1 => "0010",2 => "1000",3 => "0100",4 => "0110",5 => "1111",6 => "1011",7 => "0001",
	8 => "1010",9 => "1001",10 => "0011",11 => "1110",12 => "0101",13 => "0000",14 => "1100",15 => "0111",
	--Row 2
	16 => "0001",17 => "1111",18 => "1101",19 => "1000",20 => "1010",21 => "0011",22 => "0111",23 => "0100",
	24 => "1100",25 => "0101",26 => "0110",27 => "1011",28 => "0000",29 => "1110",30 => "1001",31 => "0010",
	--Row 3
	32 => "0111",33 => "1011",34 => "0100",35 => "0001",36 => "1001",37 => "1100",38 => "1110",39 => "0010",
	40 => "0000",41 => "0110",42 => "1010",43 => "1101",44 => "1111",45 => "0011",46 => "0101",47 => "1000",
	--Row 4
	48 => "0010",49 => "0001",50 => "1110",51 => "0111",52 => "0100",53 => "1010",54 => "1000",55 => "1101",
	56 => "1111",57 => "1100",58 => "1001",59 => "0000",60 => "0011",61 => "0101",62 => "0110",63 => "1011");

BEGIN


 PROCESS(ADDR, Clk)
   BEGIN
	IF rising_edge(Clk) THEN
     CASE ADDR IS
       WHEN "000000" => DOUT <= my_rom(0);
       WHEN "000010" => DOUT <= my_rom(1);
       WHEN "000100" => DOUT <= my_rom(2);
       WHEN "000110" => DOUT <= my_rom(3);
       WHEN "001000" => DOUT <= my_rom(4);
       WHEN "001010" => DOUT <= my_rom(5);
       WHEN "001100" => DOUT <= my_rom(6);
       WHEN "001110" => DOUT <= my_rom(7);
       WHEN "010000" => DOUT <= my_rom(8);
       WHEN "010010" => DOUT <= my_rom(9);
       WHEN "010100" => DOUT <= my_rom(10);
       WHEN "010110" => DOUT <= my_rom(11);
       WHEN "011000" => DOUT <= my_rom(12);
       WHEN "011010" => DOUT <= my_rom(13);
       WHEN "011100" => DOUT <= my_rom(14);
       WHEN "011110" => DOUT <= my_rom(15);
		 
		 WHEN "000001" => DOUT <= my_rom(16);
       WHEN "000011" => DOUT <= my_rom(17);
       WHEN "000101" => DOUT <= my_rom(18);
       WHEN "000111" => DOUT <= my_rom(19);
       WHEN "001001" => DOUT <= my_rom(20);
       WHEN "001011" => DOUT <= my_rom(21);
       WHEN "001101" => DOUT <= my_rom(22);
       WHEN "001111" => DOUT <= my_rom(23);
       WHEN "010001" => DOUT <= my_rom(24);
       WHEN "010011" => DOUT <= my_rom(25);
       WHEN "010101" => DOUT <= my_rom(26);
       WHEN "010111" => DOUT <= my_rom(27);
       WHEN "011001" => DOUT <= my_rom(28);
       WHEN "011011" => DOUT <= my_rom(29);
       WHEN "011101" => DOUT <= my_rom(30);
       WHEN "011111" => DOUT <= my_rom(31);
		 
		 WHEN "100000" => DOUT <= my_rom(32);
       WHEN "100010" => DOUT <= my_rom(33);
       WHEN "100100" => DOUT <= my_rom(34);
       WHEN "100110" => DOUT <= my_rom(35);
       WHEN "101000" => DOUT <= my_rom(36);
       WHEN "101010" => DOUT <= my_rom(37);
       WHEN "101100" => DOUT <= my_rom(38);
       WHEN "101110" => DOUT <= my_rom(39);
       WHEN "110000" => DOUT <= my_rom(40);
       WHEN "110010" => DOUT <= my_rom(41);
       WHEN "110100" => DOUT <= my_rom(42);
       WHEN "110110" => DOUT <= my_rom(43);
       WHEN "111000" => DOUT <= my_rom(44);
       WHEN "111010" => DOUT <= my_rom(45);
       WHEN "111100" => DOUT <= my_rom(46);
       WHEN "111110" => DOUT <= my_rom(47);
		 
		 WHEN "100001" => DOUT <= my_rom(48);
       WHEN "100011" => DOUT <= my_rom(49);
       WHEN "100101" => DOUT <= my_rom(50);
       WHEN "100111" => DOUT <= my_rom(51);
       WHEN "101001" => DOUT <= my_rom(52);
       WHEN "101011" => DOUT <= my_rom(53);
       WHEN "101101" => DOUT <= my_rom(54);
       WHEN "101111" => DOUT <= my_rom(55);
       WHEN "110001" => DOUT <= my_rom(56);
       WHEN "110011" => DOUT <= my_rom(57);
       WHEN "110101" => DOUT <= my_rom(58);
       WHEN "110111" => DOUT <= my_rom(59);
       WHEN "111001" => DOUT <= my_rom(60);
       WHEN "111011" => DOUT <= my_rom(61);
       WHEN "111101" => DOUT <= my_rom(62);
       WHEN "111111" => DOUT <= my_rom(63);
		 
       WHEN OTHERS => DOUT <= "0000";
	 END CASE;
	 END IF;
  END PROCESS;


end s8_arch;

