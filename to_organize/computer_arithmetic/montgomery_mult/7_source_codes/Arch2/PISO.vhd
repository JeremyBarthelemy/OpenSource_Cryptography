library ieee;
use ieee.std_logic_1164.all;   
use ieee.numeric_std.all;
USE ieee.std_logic_unsigned.all;

entity piso is
GENERIC(w : integer := 32; op_size : integer := 1024);

PORT(
	CLOCK	: in std_logic;
	RESET	: in std_logic;
	ENABLE	: in std_logic;
	shift : in std_logic;
	Input : in std_logic_vector(op_size-1 downto 0);
	Output : out std_logic_vector(w-1 downto 0)

);
end piso;

architecture piso_arch of piso is

type signal_array is array (0 to op_size/w - 1) of std_logic_vector(w-1 downto 0);

signal din : signal_array;
signal dout : signal_array;

begin

-- Generate the registers for the SIPO
regs_gen_piso: for i in 0 to op_size/w - 1 generate
piso_reg: entity work.reg(reg_arch) 
generic map (size => w)
port map(
D => din(i),
Q => dout(i),
Reset => reset,
Enable => enable,
Clock => clock
);
end generate;
-- Link the input signals for the sipo
din(0) <= input(op_size - 1 downto op_size - w);
signals: for i in 1 to op_size/w - 1 generate
din(i) <= input(op_size - i * w - 1 downto op_size - (i + 1) * w ) when shift = '0'
else dout(i - 1);
end generate;
-- PISO output
Output <= dout(op_size/w - 1);
end piso_arch;