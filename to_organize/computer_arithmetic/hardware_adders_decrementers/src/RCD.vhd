--Ripple Carry Decrementer, Implementation of Task 1				   

LIBRARY ieee;
USE ieee.std_logic_1164.all;
use IEEE.std_logic_signed.all;

ENTITY RCD IS
	GENERIC(k : INTEGER);
	PORT(
			--CLK : IN STD_LOGIC;
			X : IN STD_LOGIC_VECTOR(k-1 DOWNTO 0);
			S : OUT STD_LOGIC_VECTOR(k-1 DOWNTO 0);
			C : OUT STD_LOGIC;
			V : OUT STD_LOGIC
		);
END RCD;

ARCHITECTURE rcd_arch OF RCD IS
SIGNAL S_Temp : STD_LOGIC_VECTOR(k DOWNTO 0);
SIGNAL X_Temp : STD_LOGIC_VECTOR(k DOWNTO 0);
BEGIN

X_Temp <= X(k-1) & X;
S_Temp <= X_Temp - 1;

C <= X_Temp(k) XOR S_Temp(k); --Positive msb of X_Temp to a negative msb of S_Temp...Simple XOR suffices since we know
--that X_Temp(k) is always positive and C will only result in a '1' if S_Temp(k) is "negative"
V <=  (X_Temp(k-1) AND (NOT S_TEMP(k-1))); --X_Temp(k-1) must be "negative" and S_Temp(k-1) must be "positive"

S <= S_Temp(k-1 DOWNTO 0);
	
END rcd_arch;
