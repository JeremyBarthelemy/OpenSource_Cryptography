----------------------------------------------------------------------------------
-- ECE 645 
-- 
--
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use work.montyMult_pkg.all;

entity MontyMult_Arch1 is
	generic(K : integer := 1024; W : integer := 16; P : integer := 65; ADDR_BITS : integer := 7);
	port(
		CLK, WE, EN, RSTN : in std_logic;
		QEN : in std_logic_vector(P-1 downto 0);
		Y, M : in std_logic_vector(W-1 downto 0);
		X : in std_logic_vector(P-1 downto 0);		
		ADDR : in std_logic_vector(ADDR_BITS-1 downto 0);
		S : out std_logic_vector(W-1 downto 0)
	);
end MontyMult_Arch1;

architecture MontyMult_Arch1_behavioral of MontyMult_Arch1 is
	
	-- Array type declarations.
	type wBitArray is array(P downto 0) of std_logic_vector(W-1 downto 0);
	
	-- Intermediate signal declarations.
	signal y_word, m_word : std_logic_vector(W-1 downto 0);
	signal preRegBusY, preRegBusM, sInPEBus : wBitArray;
	--signal q_load, q_out : std_logic_vector(P-1 downto 0);
	
	-- QUEUE related types and signals.
	--type queueArray is array(QUEUE_LENGTH-1 downto 0) of std_logic_vector(W-1 downto 0);
	--constant QUEUE_LENGTH : integer := (P - (K / W) - 1);
	--signal queueBus : queueArray;
	
begin
	-- Handle Q enable internally.
	-- 	Attempted to make QEN controlled internally. Caused simulation problems despite code logic being sound.
	--q_load <= ((P-1) => '1', others => '0');
	--q_bits : bitShiftReg generic map(P) port map(CLK, EN, RSTN, '0', q_load, q_out);

	sInPEBus(0) <= (others => '0');

	-- RAMs for M and Y.
	Y_Ram : ramMxN generic map(ADDR_BITS,W) port map(CLK, WE, Y, ADDR, y_word);
	M_Ram : ramMxN generic map(ADDR_BITS,W) port map(CLK, WE, M, ADDR, m_word);
	
	-- Generate pre-PE registers.
	preReg_YStart : nBitRegSynch generic map(W) port map(y_word, RSTN, CLK, EN, preRegBusY(0));
	preReg_MStart : nBitRegSynch generic map(W) port map(m_word, RSTN, CLK, EN, preRegBusM(0));
	Gen_Pre_Regs : for i in 0 to P-2 generate 
		preReg_Y : nBitRegSynch generic map(W) port map(preRegBusY(i), RSTN, CLK, EN, preRegBusY(i+1));
		preReg_M : nBitRegSynch generic map(W) port map(preRegBusM(i), RSTN, CLK, EN, preRegBusM(i+1));
	end generate;
	
	-- Generate PEs.
	Gen_PEs : for i in 0 to P-1 generate 
		--pes : PE_arch1 generic map(W) port map(CLK, RSTN, EN, q_out(P-1-i), preRegBusY(i), preRegBusM(i), sInPEBus(i), X(i), sInPEBus(i+1));
		pes : PE_arch1 generic map(W) port map(CLK, RSTN, EN, QEN(i), preRegBusY(i), preRegBusM(i), sInPEBus(i), X(i), sInPEBus(i+1));
	end generate;
	
	-- Generate "Queue" for S outputs.
	-- Do this only when there are not enough PEs.
	--queueReg_Start : nBitRegSynch generic map(W) port map(sInPEBus(P), RSTN, CLK, EN, queueBus(0));
	--Gen_Queue_Regs : for i in 0 to QUEUE_LENGTH-2 generate 
	--	queueReg : nBitRegSynch generic map(W) port map(queueBus(i), RSTN, CLK, EN, queueBus(i+1));
	--end generate;
	--queueReg_End : nBitRegSynch generic map(W) port map(queueBus(QUEUE_LENGTH-1), RSTN, CLK, EN, sInPEBus(0));
	
	-- Output.
	S <= sInPEBus(P);

end MontyMult_Arch1_behavioral;

