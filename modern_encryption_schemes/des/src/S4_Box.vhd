
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity S4_Box is
    Port ( Clk : in  STD_LOGIC;
           ADDR : in  STD_LOGIC_VECTOR (5 downto 0);
           DOUT : out  STD_LOGIC_VECTOR (3 downto 0));
end S4_Box;

architecture s4_arch of S4_Box is
type mem is array (0 to 2**6-1) of STD_LOGIC_VECTOR(3 downto 0);

CONSTANT my_rom : mem := (
	--Row 1
	0 => "0111",1 => "1101",2 => "1110",3 => "0011",4 => "0000",5 => "0110",6 => "1001",7 => "1010",
	8 => "0001",9 => "0010",10 => "1000",11 => "0101",12 => "1011",13 => "1100",14 => "0100",15 => "1111",
	--Row 2
	16 => "1101",17 => "1000",18 => "1011",19 => "0101",20 => "0110",21 => "1111",22 => "0000",23 => "0011",
	24 => "0100",25 => "0111",26 => "0010",27 => "1100",28 => "0001",29 => "1010",30 => "1110",31 => "1001",
	--Row 3
	32 => "1010",33 => "0110",34 => "1001",35 => "0000",36 => "1100",37 => "1011",38 => "0111",39 => "1101",
	40 => "1111",41 => "0001",42 => "0011",43 => "1110",44 => "0101",45 => "0010",46 => "1000",47 => "0100",
	--Row 4
	48 => "0011",49 => "1111",50 => "0000",51 => "0110",52 => "1010",53 => "0001",54 => "1101",55 => "1000",
	56 => "1001",57 => "0100",58 => "0101",59 => "1011",60 => "1100",61 => "0111",62 => "0010",63 => "1110");

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


end s4_arch;

