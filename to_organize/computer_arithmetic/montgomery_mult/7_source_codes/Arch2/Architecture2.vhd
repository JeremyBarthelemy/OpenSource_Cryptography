library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity Architecture2 is
GENERIC( op_size : INTEGER := 1024; w : INTEGER := 32; e : INTEGER := 32);
PORT(
			--Inputs
			CLOCK : IN STD_LOGIC;
			RESET : IN STD_LOGIC;
			ENABLE : IN STD_LOGIC;
			X_in : IN STD_LOGIC;
			Y_in : IN STD_LOGIC_VECTOR(op_size-1 DOWNTO 0);
			M_in : IN STD_LOGIC_VECTOR(op_size-1 DOWNTO 0);
			--Outputs
--			C_out : OUT STD_LOGIC_VECTOR();
			S_out : OUT STD_LOGIC_VECTOR(op_size-1 DOWNTO 0)
			
	  );
end Architecture2;

architecture arch2_arch of Architecture2 is
SIGNAL xi_shift_out : STD_LOGIC_VECTOR(e-1 DOWNTO 0);
SIGNAL qi_shift_out : STD_LOGIC_VECTOR(e-2 DOWNTO 0);
SIGNAL q0_to_shift_reg : STD_LOGIC; --q0 to be fed into the qi shift register
SIGNAL C_OUT_SIGNALS : STD_LOGIC_VECTOR(2*w-3 DOWNTO 0);
SIGNAL S_PassBacks : STD_LOGIC_VECTOR(w-1 DOWNTO 1);

BEGIN

qi_shift : ENTITY work.shift_reg(shift_arch)
			  GENERIC MAP(size=>e-1)
			  PORT MAP(D=>q0_to_shift_reg,Q_OF_REGS=>qi_shift_out,ENABLE=>ENABLE,RESET=>RESET,CLOCK=>CLOCK);
			  
xi_shift : ENTITY work.shift_reg(shift_arch)
			  GENERIC MAP(size=>e)
			  PORT MAP(D=>X_in,Q_OF_REGS=>xi_shift_out,ENABLE=>ENABLE,RESET=>RESET,CLOCK=>CLOCK);
			  

--D Processing Element
D_Proc : ENTITY work.PE0(Behavioral)
	 GENERIC MAP(w => 32)
	 PORT MAP( 
				--Inputs
				xi=>xi_shift_out(0),
				Y_in=>Y_in(w-1 DOWNTO 0),
				M_in=>M_in(w-1 DOWNTO 0),
				REG_EN=>ENABLE,
				RESET=>RESET,
				CLOCK=>CLOCK,
				S_Next_In=>S_PassBacks(1),
				--Outputs
				qi=>q0_to_shift_reg,
				C_out=>C_OUT_SIGNALS(1 DOWNTO 0),
				S_out=> S_out(w-1 DOWNTO 0)

	  );


--E Processing Element Generator
GEN1 : FOR i IN 1 TO e-2 GENERATE
	E_Proc : ENTITY work.PEj(Behavioral)
		GENERIC MAP(w => 32)
		PORT MAP( 
					--Inputs
					xi=>xi_shift_out(i),
					Y_in=>Y_in(i*w+w-1 DOWNTO i*w),
					M_in=>M_in(i*w+w-1 DOWNTO i*w),
					C_In=>C_OUT_SIGNALS(2*i-1 DOWNTO 2*i-2),
					REG_EN=>ENABLE,
					qi=>qi_shift_out(i-1),
					RESET=>RESET,
					CLOCK=>CLOCK,
					S_Next_In=>S_PassBacks(i+1),
					--Outputs
					S_PassBack=>S_PassBacks(i),
					C_out=>C_OUT_SIGNALS(2*i+1 DOWNTO 2*i),
					S_out=>S_out(i*w + w-1 DOWNTO i*w)
		);
END GENERATE;
	  
--F Processing Element
F_Proc : ENTITY work.PElast(Behavioral)
	 GENERIC MAP(w => 32)
	 PORT MAP( 
				--Inputs
				xi=>xi_shift_out(e-1),
				Y_in=>Y_in(op_size-1 DOWNTO (op_size-w)),
				M_in=>M_in(op_size-1 DOWNTO (op_size-w)),
				C_In=>C_OUT_SIGNALS(2*w-3 DOWNTO 2*w-4),
				qi=>qi_shift_out(e-2),
				REG_EN=>ENABLE,
				RESET=>RESET,
				CLOCK=>CLOCK,
				--Outputs
				S_PassBack=>S_PassBacks(w-1),
				--C_out=>C_OUT_SIGNALS(2*w-1 DOWNTO 2*w-2),
				S_out=>S_out(op_size-1 DOWNTO op_size-w)
	  );
	  
end arch2_arch;

