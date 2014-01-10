
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;


entity Top is
GENERIC( op_size : INTEGER := 1024; w : INTEGER := 32; e : INTEGER := 32); 
PORT(
			--Inputs
			CLOCK : IN STD_LOGIC;
			RESET : IN STD_LOGIC;
			ENABLE : IN STD_LOGIC;
			X_in : IN STD_LOGIC;
			Y_in : IN STD_LOGIC_VECTOR(w-1 DOWNTO 0);
			M_in : IN STD_LOGIC_VECTOR(w-1 DOWNTO 0);
			shift : IN STD_LOGIC;
			--Outputs
			S_out : OUT STD_LOGIC_VECTOR(w-1 DOWNTO 0)
			
	  );
end Top;

architecture top_arch of Top is
SIGNAL Y_SIPO_OUT : STD_LOGIC_VECTOR(op_size-1 DOWNTO 0);
SIGNAL M_SIPO_OUT : STD_LOGIC_VECTOR(op_size-1 DOWNTO 0);
SIGNAL S_PISO_IN : STD_LOGIC_VECTOR(op_size-1 DOWNTO 0);
begin

--Y SIPO
Y_SIPO : ENTITY work.SIPO(sipo_arch)
		   GENERIC MAP(w => w, op_size => op_size)
			PORT MAP(
				CLOCK=>CLOCK,
				ENABLE=>ENABLE,
				RESET=>RESET,
				Input=>Y_in,
            Output=>Y_SIPO_OUT
				);

--M SIPO
M_SIPO : ENTITY work.SIPO(sipo_arch)
		   GENERIC MAP(w => w, op_size => op_size)
			PORT MAP(
				CLOCK=>CLOCK,
				ENABLE=>ENABLE,
				RESET=>RESET,
				Input=>M_in,
            Output=>M_SIPO_OUT
				);



ARCH2 : ENTITY work.Architecture2(arch2_arch)
		  GENERIC MAP(w => w, op_size => op_size)
		  PORT MAP(
			CLOCK=>CLOCK,
			RESET=>RESET,
			ENABLE=>ENABLE,
			X_in=>X_in,
			Y_in=>Y_SIPO_OUT,
			M_in=>M_SIPO_OUT,
			S_out=>S_PISO_IN
			
		  );
		  



--S PISO
S_PISO : ENTITY work.PISO(piso_arch)
		   GENERIC MAP(w => w, op_size => op_size)
			PORT MAP(
				Clock=>CLOCK,
				ENABLE=>ENABLE,
				RESET=>RESET,
				Input=>S_PISO_IN,
            Output=>S_out,
				shift=>shift
				);


end top_arch;

