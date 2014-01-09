----------------------------------------------------------------------------------
-- ECE 645 
-- 
--
--
----------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;

package montyMult_pkg is		
	
	-- Constant declarations.
	
	-- Fucntion declarations.	
	-- Calculates ceiling(log base 2) of an integer.
	function log2(constant s : integer) return integer;

   -- Components declarations.
	-- Processing Element for Architecture 1.
	component PE_arch1
		generic (W : integer := 64);
		port(
			CLK, RSTN, EN, QEN : in std_logic;
			Y, M : in std_logic_vector(W-1 downto 0);
			S_IN : in std_logic_vector(W-1 downto 0);
			X : in std_logic;		
			S_OUT : out std_logic_vector(W-1 downto 0)
		);
	end component;
   
	-- TaskD Cell.
	component TaskD
		generic(W : integer := 32);
		port(
			CLK, RSTN, EN : in std_logic;
			Xi, S1 : in std_logic;
			Y, M : in std_logic_vector(W-1 downto 0);
			S0_in : in std_logic_vector(W-1 downto 1);
			Qi : out std_logic;
			C : out std_logic_vector(1 downto 0);
			S0_out : out std_logic_vector(W-1 downto 0)
		);
	end component;

	-- TaskE Cell.
	component TaskE
		generic(W : integer := 32);
		port(
			CLK, RSTN, EN : in std_logic;
			Qi, Xi, S0_in : in std_logic;
			C_in : in std_logic_vector(1 downto 0);
			Y, M : in std_logic_vector(W-1 downto 0);
			Sj_in : in std_logic_vector(W-1 downto 1);
			C_out : out std_logic_vector(1 downto 0);
			Sj_out : out std_logic_vector(W-1 downto 0)
		);
	end component;

	-- TaskF Cell.
	component TaskF
		generic(W : integer := 32);
		port(
			CLK, RSTN, EN : in std_logic;
			Qi, Xi, Ce_in : in std_logic;
			C_in : std_logic_vector(1 downto 0);
			Y, M : in std_logic_vector(W-1 downto 0);
			Se_in : in std_logic_vector(W-1 downto 1);
			C_out : out std_logic_vector(1 downto 0);
			Se_out : out std_logic_vector(W-1 downto 0)
		);
	end component;
	
	-- RAM.
	component ramMxN
		generic (M : integer := 16;  N : integer := 64);
		port(
			CLK, WE : in std_logic;
			A_IN : in std_logic_vector(N-1 downto 0);
			ADDR_A : in std_logic_vector(M-1 downto 0);
			A_OUT : out std_logic_vector(N-1 downto 0)
		);
	end component;	
	
	-- Single bit shift register.
	component bitShiftReg
		generic(N : integer := 32);
		port(
			CLK, EN, RSTN : in std_logic;
			DIN : in std_logic;
			LOAD : in std_logic_vector(N-1 downto 0);
			DOUT : out std_logic_vector(N-1 downto 0)
		);
	end component;
	
	-- Register.
	component nBitRegSynch
		generic(N : integer := 8);
		port(
			D : in std_logic_vector(N-1 downto 0);
			RSTN, CLK, EN : in std_logic;
			Q : out std_logic_vector(N-1 downto 0)
		);		
	end component;
	
	-- Flip Flop.
	component DFlipFlop is
		port(
			CLK, EN, D : in std_logic;
			Q : out std_logic
		);
	end component;
	
end montyMult_pkg;

package body montyMult_pkg is

	-- Functions descriptions.
	-- Calculates ceiling(log base 2) of an integer.
	function log2(constant s : integer) return integer is
		variable m, n : integer;
	begin
		m := 0;
		n := 1;
		while(n < s) loop
			m := m + 1;
			n := n*2;
		end loop;
		return m;
	end log2;
		
end package body montyMult_pkg;