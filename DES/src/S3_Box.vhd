library IEEE;
use IEEE.STD_LOGIC_1164.ALL;


ENTITY S3_Box IS
    Port ( Clk : IN  STD_LOGIC;
           ADDR : IN  STD_LOGIC_VECTOR (5 downto 0);
           DOUT : OUT  STD_LOGIC_VECTOR (3 downto 0));
end S3_Box;

ARCHITECTURE s3_arch OF S3_Box IS
type mem is array (0 to 2**6-1) of STD_LOGIC_VECTOR(3 downto 0);



CONSTANT my_rom : mem := (
	--Row 1
	0 => "1010",1 => "0000",2 => "1001",3 => "1110",4 => "0110",5 => "0011",6 => "1111",7 => "0101",
	8 => "0001",9 => "1101",10 => "1100",11 => "0111",12 => "1011",13 => "0100",14 => "0010",15 => "1000",
	--Row 2
	16 => "1101",17 => "0111",18 => "0000",19 => "1001",20 => "0011",21 => "0100",22 => "0110",23 => "1010",
	24 => "0010",25 => "1000",26 => "0101",27 => "1110",28 => "1100",29 => "1011",30 => "1111",31 => "0001",
	--Row 3
	32 => "1101",33 => "0110",34 => "0100",35 => "1001",36 => "1000",37 => "1111",38 => "0011",39 => "0000",
	40 => "1011",41 => "0001",42 => "0010",43 => "1100",44 => "0101",45 => "1010",46 => "1110",47 => "0111",
	--Row 4
	48 => "0001",49 => "1010",50 => "1101",51 => "0000",52 => "0110",53 => "1001",54 => "1000",55 => "0111",
	56 => "0100",57 => "1111",58 => "1110",59 => "0011",60 => "1011",61 => "0101",62 => "0010",63 => "1100");

begin


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


end s3_arch;

