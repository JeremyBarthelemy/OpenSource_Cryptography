library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

--think of the entity as the interface -- inputs and outputs
entity AND_gate is
    Port ( 
	        A : in  STD_LOGIC_VECTOR(15 downto 0);
           B : in  STD_LOGIC_VECTOR(15 downto 0);
           C : out  STD_LOGIC_VECTOR(15 downto 0)
			 );
end AND_gate;

--architecture is used to define the guts of the module...what is happening on the inside!
architecture and_gate_out of AND_gate is
--Define your signals here, between architecture declaration and begin!
begin

--C takes the value of A AND B, as in the typical AND gate logic
C <= A AND B;

end and_gate_out;

