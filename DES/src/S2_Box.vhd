library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity S2_Box is
    Port ( Clk : in  STD_LOGIC;
           ADDR : in  STD_LOGIC_VECTOR (5 downto 0);
           DOUT : out  STD_LOGIC_VECTOR (3 downto 0));
end S2_Box;

architecture s2_arch of S2_Box is

type mem is array (0 to 2**6-1) of STD_LOGIC_VECTOR(3 downto 0);

CONSTANT my_rom : mem := (
	--Row 1
	0 => "1111",1 => "0001",2 => "1000",3 => "1110",4 => "0110",5 => "1011",6 => "0011",7 => "0100",
	8 => "1001",9 => "0111",10 => "0010",11 => "1101",12 => "1100",13 => "0000",14 => "0101",15 => "1010",
	--Row 2
	16 => "0011",17 => "1101",18 => "0100",19 => "0111",20 => "1111",21 => "0010",22 => "1000",23 => "1110",
	24 => "1100",25 => "0000",26 => "0001",27 => "1010",28 => "0110",29 => "1001",30 => "1011",31 => "0101",
	--Row 3
	32 => "0000",33 => "1110",34 => "0111",35 => "1011",36 => "1010",37 => "0100",38 => "1101",39 => "0001",
	40 => "0101",41 => "1000",42 => "1100",43 => "0110",44 => "1001",45 => "0011",46 => "0010",47 => "1111",
	--Row 4
	48 => "1101",49 => "1000",50 => "1010",51 => "0001",52 => "0011",53 => "1111",54 => "0100",55 => "0010",
	56 => "1011",57 => "0110",58 => "0111",59 => "1100",60 => "0000",61 => "0101",62 => "1110",63 => "1001");


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


end s2_arch;

