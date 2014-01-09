LIBRARY ieee;
USE ieee.std_logic_1164.all;
--USE ieee.numeric_std.all;
use IEEE.std_logic_unsigned.all;
use ieee.STD_LOGIC_ARITH.all;

ENTITY ROUND IS
	PORT(
		a : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
		b : IN STD_LOGIC_VECTOR(7 DOWNTO 0); 
		c :	IN STD_LOGIC_VECTOR(7 DOWNTO 0);
		d :	IN STD_LOGIC_VECTOR(7 DOWNTO 0);
		i : IN STD_LOGIC_VECTOR(5 DOWNTO 0);
		r : IN STD_LOGIC_VECTOR(31 DOWNTO 0);																						
		ap : OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
		bp : OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
		cp : OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
		dp : OUT STD_LOGIC_VECTOR(7 DOWNTO 0)	
	);
END ROUND;	

ARCHITECTURE ROUND_arch OF ROUND IS
--8-bit intermediate values
SIGNAL ki : STD_LOGIC_VECTOR(7 DOWNTO 0);
SIGNAL ri : STD_LOGIC_VECTOR(7 DOWNTO 0);
SIGNAL e : STD_LOGIC_VECTOR(7 DOWNTO 0);				    
SIGNAL f : STD_LOGIC_VECTOR(7 DOWNTO 0);
SIGNAL X : STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL Y : STD_LOGIC_VECTOR(17 DOWNTO 0);
SIGNAL Z : STD_LOGIC_VECTOR(18 DOWNTO 0);

BEGIN
					 
ROM : ENTITY work.ROM(arch_ROM)
	PORT MAP(ADDRESS_IN => i(3 DOWNTO 0), DOUT =>ki);			   				--ki := k[i mod 16]
	
ROT_1 : ENTITY work.Variable_Rotator(var_arch)
	PORT MAP(var => b, DIR => '0', amount => "011", output => cp);	   			--c := b <<< 3 

ROT_2 : ENTITY work.Variable_Rotator(var_arch)
	PORT MAP(var => a, DIR => '1', amount => i(2 DOWNTO 0), output =>bp);	 	--b := a >>> i

ap <= e XOR ki;		 															--a := e XOR ki	


WITH i(1 DOWNTO 0) SELECT					   									--ri := r[i mod 4]
	ri <=  r(7 DOWNTO 0) WHEN "00",
	r(15 DOWNTO 8) WHEN "01",
	r(23 DOWNTO 16) WHEN "10",
	r(31 DOWNTO 24) WHEN OTHERS; 


X <= (ri*"10"*"10"*"10"*"10" + ri);	
f <= X(7 DOWNTO 0);											 --f := (17*ri) mod 2^8

Y <= ("10"*d+"1")*f;
e <= Y(7 DOWNTO 0);												 --e := (2d+1)*f mod 2^8

Z <= ("100"*c+"10")*c;
dp <= Z(7 DOWNTO 0);												--d := (4c+2)*c mod 2^8


END ROUND_arch;