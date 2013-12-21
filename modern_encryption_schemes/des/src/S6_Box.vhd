
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;


entity S6_Box is
    Port ( Clk : in  STD_LOGIC;
           ADDR : in  STD_LOGIC_VECTOR (5 downto 0);
           DOUT : out  STD_LOGIC_VECTOR (3 downto 0));
end S6_Box;

architecture s6_arch of S6_Box is
type mem is array (0 to 2**6-1) of STD_LOGIC_VECTOR(3 downto 0);

CONSTANT my_rom : mem := (
	--Row 1
	0 => "1100",1 => "0001",2 => "1010",3 => "1111",4 => "1001",5 => "0010",6 => "0110",7 => "1000",
	8 => "0000",9 => "1101",10 => "0011",11 => "0100",12 => "1110",13 => "0111",14 => "0101",15 => "1011",
	--Row 2
	16 => "1010",17 => "1111",18 => "0100",19 => "0010",20 => "0111",21 => "1100",22 => "1001",23 => "0101",
	24 => "0110",25 => "0001",26 => "1101",27 => "1110",28 => "0000",29 => "1011",30 => "0011",31 => "1000",
	--Row 3
	32 => "1001",33 => "1110",34 => "1111",35 => "0101",36 => "0010",37 => "1000",38 => "1100",39 => "0011",
	40 => "0111",41 => "0000",42 => "0100",43 => "1010",44 => "0001",45 => "1101",46 => "1011",47 => "0110",
	--Row 4
	48 => "0100",49 => "0011",50 => "0010",51 => "1100",52 => "1001",53 => "0101",54 => "1111",55 => "1010",
	56 => "1011",57 => "1110",58 => "0001",59 => "0111",60 => "0110",61 => "0000",62 => "1000",63 => "1101");

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



end s6_arch;

