--Author: Jeremy Barthelemy

LIBRARY ieee;
USE ieee.std_logic_1164.all;
use IEEE.std_logic_unsigned.all;
use ieee.STD_LOGIC_ARITH.all;

ENTITY DATAPATH IS
	PORT( 	
			m 				  : IN STD_LOGIC_VECTOR(31 DOWNTO 0);
			last_block 	  	  : IN STD_LOGIC_VECTOR(0 DOWNTO 0);			 
			clk 			  : IN STD_LOGIC;
			rst 			  : IN STD_LOGIC;
			sh 				  : IN STD_LOGIC;
			sn 				  : IN STD_LOGIC;
			eh 				  : IN STD_LOGIC;
			en 				  : IN STD_LOGIC;
			er 				  : IN STD_LOGIC;
			ei 				  : IN STD_LOGIC;
			el 				  : IN STD_LOGIC; 
			last_block_stored : OUT STD_LOGIC_VECTOR(0 DOWNTO 0) := "0";
			zi 				  : OUT STD_LOGIC := '0';			
			y 				  : OUT STD_LOGIC_VECTOR(31 DOWNTO 0);
			a_debug			  : OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
			b_debug			  : OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
			c_debug			  : OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
			d_debug			  : OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
			r_debug			  : OUT STD_LOGIC_VECTOR(31 DOWNTO 0)
	
	);
END DATAPATH;

ARCHITECTURE dat_arch OF DATAPATH IS
SIGNAL MUX_0_OUT : STD_LOGIC_VECToR(7 DOWNTO 0);
SIGNAL MUX_1_OUT : STD_LOGIC_VECToR(7 DOWNTO 0);
SIGNAL MUX_2_OUT : STD_LOGIC_VECToR(7 DOWNTO 0);
SIGNAL MUX_3_OUT : STD_LOGIC_VECToR(7 DOWNTO 0);
SIGNAL MUX_4_OUT : STD_LOGIC_VECToR(7 DOWNTO 0);
SIGNAL MUX_5_OUT : STD_LOGIC_VECToR(7 DOWNTO 0);
SIGNAL MUX_6_OUT : STD_LOGIC_VECToR(7 DOWNTO 0);
SIGNAL MUX_7_OUT : STD_LOGIC_VECToR(7 DOWNTO 0);
SIGNAL h0 : STD_LOGIC_VECTOR(7 DOWNTO 0);
SIGNAL h1 : STD_LOGIC_VECTOR(7 DOWNTO 0);
SIGNAL h2 : STD_LOGIC_VECTOR(7 DOWNTO 0);
SIGNAL h3 : STD_LOGIC_VECTOR(7 DOWNTO 0);	  	  
SIGNAL A : STD_LOGIC_VECTOR(7 DOWNTO 0); 
SIGNAL B : STD_LOGIC_VECTOR(7 DOWNTO 0);
SIGNAL C : STD_LOGIC_VECTOR(7 DOWNTO 0);
SIGNAL D : STD_LOGIC_VECTOR(7 DOWNTO 0);
SIGNAL AP : STD_LOGIC_VECTOR(7 DOWNTO 0); 
SIGNAL BP : STD_LOGIC_VECTOR(7 DOWNTO 0);
SIGNAL CP : STD_LOGIC_VECTOR(7 DOWNTO 0);
SIGNAL DP : STD_LOGIC_VECTOR(7 DOWNTO 0);
SIGNAL I : STD_LOGIC_VECTOR(5 DOWNTO 0); 
SIGNAL R : STD_LOGIC_VECTOR(31 DOWNTO 0);
SIGNAL iv0 : STD_LOGIC_VECTOR(7 DOWNTO 0) := "10001001";
SIGNAL iv1 : STD_LOGIC_VECTOR(7 DOWNTO 0) := "10101011";
SIGNAL iv2 : STD_LOGIC_VECTOR(7 DOWNTO 0) := "11001101";
SIGNAL iv3 : STD_LOGIC_VECTOR(7 DOWNTO 0) := "11101111";
SIGNAL A_PLUS_H0 : STD_LOGIC_VECTOR(7 DOWNTO 0);
SIGNAL B_PLUS_H1 : STD_LOGIC_VECTOR(7 DOWNTO 0);
SIGNAL C_PLUS_H2 : STD_LOGIC_VECTOR(7 DOWNTO 0);
SIGNAL D_PLUS_H3 : STD_LOGIC_VECTOR(7 DOWNTO 0);  
SIGNAL A_PLUS_H0_TEMP : STD_LOGIC_VECTOR(8 DOWNTO 0);
SIGNAL B_PLUS_H1_TEMP : STD_LOGIC_VECTOR(8 DOWNTO 0);
SIGNAL C_PLUS_H2_TEMP : STD_LOGIC_VECTOR(8 DOWNTO 0);
SIGNAL D_PLUS_H3_TEMP : STD_LOGIC_VECTOR(8 DOWNTO 0); 
SIGNAL A_TEMP : STD_LOGIC_VECTOR(8 DOWNTO 0); 
SIGNAL B_TEMP : STD_LOGIC_VECTOR(8 DOWNTO 0); 
SIGNAL C_TEMP : STD_LOGIC_VECTOR(8 DOWNTO 0); 
SIGNAL D_TEMP : STD_LOGIC_VECTOR(8 DOWNTO 0); 
SIGNAL h0_TEMP : STD_LOGIC_VECTOR(8 DOWNTO 0); 
SIGNAL h1_TEMP : STD_LOGIC_VECTOR(8 DOWNTO 0); 
SIGNAL h2_TEMP : STD_LOGIC_VECTOR(8 DOWNTO 0); 
SIGNAL h3_TEMP : STD_LOGIC_VECTOR(8 DOWNTO 0);	 

--        final int iv0 = 0x89, iv1 = 0xAB, iv2 = 0xCD, iv3 = 0xEF;
--        final int k[] = {0x01, 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89, 0x9A, 0xAB, 0xBC, 0xCD, 0xDE, 0xEF, 0xF0};


BEGIN

RND : ENTITY work.ROUND(ROUND_arch)
	  PORT MAP(a => A, b => B, c => C, d => D, i => I, r => R, ap => AP, bp => BP, cp => CP, dp => DP);

REG_R : ENTITY work.A_REG(a_reg_arch)
		GENERIC MAP(N => 32)
		PORT MAP(D => m, Q => R, ENABLE => er, RESET => rst, CLOCK => clk);
	
REG_LB : ENTITY work.A_REG(a_reg_arch)
		 GENERIC MAP(N => 1)
		 PORT MAP(D => last_block, Q => last_block_stored, ENABLE => el, RESET => rst, CLOCK => clk);

Count_I : ENTITY work.S_COUNTER(s_counter_arch)
		  PORT MAP(Q => I, ENABLE => ei, RESET => rst, CLOCK => clk);

REG_h0 : ENTITY work.A_REG(a_reg_arch)
	  	 PORT MAP(D => mux_0_out, Q => h0, ENABLE => eh, RESET => rst, CLOCK => clk);	
	
REG_h1 : ENTITY work.A_REG(a_reg_arch)
		 PORT MAP(D => mux_1_out, Q => h1, ENABLE => eh, RESET => rst, CLOCK => clk);
	
REG_h2 : ENTITY work.A_REG(a_reg_arch)
		 PORT MAP(D => mux_2_out, Q => h2, ENABLE => eh, RESET => rst, CLOCK => clk);
	
REG_h3 : ENTITY work.A_REG(a_reg_arch)
		 PORT MAP(D => mux_3_out, Q => h3, ENABLE => eh, RESET => rst, CLOCK => clk);
	
REG_A : ENTITY work.A_REG(a_reg_arch)
		PORT MAP(D=> mux_4_out,Q=> A,ENABLE=> en,RESET=> rst,CLOCK=> clk);
REG_B : ENTITY work.A_REG(a_reg_arch)
		PORT MAP(D=> mux_5_out,Q=> B,ENABLE=> en,RESET=> rst,CLOCK=> clk);
REG_C : ENTITY work.A_REG(a_reg_arch)
		PORT MAP(D=> mux_6_out,Q=> C,ENABLE=> en,RESET=> rst,CLOCK=> clk);
REG_D : ENTITY work.A_REG(a_reg_arch)
		PORT MAP(D=> mux_7_out,Q=> D,ENABLE=> en,RESET=> rst,CLOCK=> clk);

	  	 
--mux0
WITH sh SELECT
mux_0_out <= A_PLUS_H0 WHEN '0',
			 iv0 WHEN OTHERS;
--mux1
WITH sh SELECT
mux_1_out <= B_PLUS_H1 WHEN '0',
			iv1 WHEN OTHERS;
--mux2
WITH sh SELECT
mux_2_out <= C_PLUS_H2 WHEN '0',
			 iv2 WHEN OTHERS;
--mux3		 				 
WITH sh SELECT
mux_3_out <= D_PLUS_H3 WHEN '0',
			 iv3 WHEN OTHERS;

--mux4
WITH sn SELECT
mux_4_out <= h0 WHEN '0',
			 AP WHEN OTHERS;
--mux5
WITH sn SELECT
mux_5_out <= h1 WHEN '0',
			 BP WHEN OTHERs;
--mux6
WITH sn SELECT
mux_6_out <= h2 WHEN '0',
			 CP WHEN OTHERS;
--mux7
WITH sn SELECT
mux_7_out <= h3 WHEN '0',
			 DP WHEN OTHERS;
			 
A_temp <= '0' & A;
B_temp <= '0' & B;
C_temp <= '0' & C;
D_temp <= '0' & D;
h0_temp <= '0' & h0;
h1_temp <= '0' & h1;
h2_temp <= '0' & h2;
h3_temp <= '0' & h3;
A_PLUS_H0_TEMP <= (A_temp + h0_temp);
B_PLUS_H1_TEMP <= (B_temp + h1_temp);
C_PLUS_H2_TEMP <= (C_temp + h2_temp);
D_PLUS_H3_TEMP <= (D_temp + h3_temp);

A_PLUS_H0 <= (A_PLUS_H0_TEMP(7 DOWNTO 0));
B_PLUS_H1 <= (B_PLUS_H1_TEMP(7 DOWNTO 0));
C_PLUS_H2 <= (C_PLUS_H2_TEMP(7 DOWNTO 0));
D_PLUS_H3 <= (D_PLUS_H3_TEMP(7 DOWNTO 0));

a_debug <= A;
b_debug <= B;
c_debug <= C;
d_debug <= D;
r_debug <= R;	   

zi <= '1' WHEN (I = "111111")
		  ELSE '0';

y <= h0 & h1 & h2 & h3;	   			--y := h0 || h1 || h2 || h3

		
END dat_arch;
