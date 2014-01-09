library ieee;
use ieee.std_logic_1164.all;   
use ieee.numeric_std.all;
USE ieee.std_logic_unsigned.all;

entity sipo is
generic ( w : integer := 32;	op_size : integer := 1024 );
port(

CLOCK	: in std_logic;
RESET	: in std_logic;
ENABLE	: in std_logic;
Input : in std_logic_vector(w-1 downto 0);
Output : out std_logic_vector(op_size-1 downto 0)

);
end sipo;

architecture sipo_arch of sipo is

type output_array is array (op_size/w - 1 downto 0) of std_logic_vector(w-1 downto 0);

signal reg_in : output_array;
signal reg_out : output_array;

begin

-- Generate the registers for the SIPO
registers_gen_sipo: for i in 0 to op_size/w - 1 generate
sipo_reg: entity work.reg(reg_arch)
generic map (size => w)
port map(
D => reg_in(i),
Q => reg_out(i),
Reset => RESET,
Enable => ENABLE,
Clock => CLOCK
);
end generate;
-- Link the signals for the sipo
reg_in(op_size/w - 1) <= Input;
signals: for i in op_size/w - 2 downto 0 generate
reg_in(i) <= reg_out(i + 1);
end generate;
-- Link the outputs
outputs: for i in 0 to op_size/w - 1 generate
Output(op_size - (i * w) - 1 downto op_size - ((i + 1) * w)) <= reg_out(op_size/w - i - 1);
end generate;
end sipo_arch;