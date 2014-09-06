/*
 *  Multi2Sim
 *  Copyright (C) 2014  Sida Gu (gu.sid@husky.neu.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <lib/cpp/Misc.h>
#include <memory/Memory.h>

#include "Context.h"
#include "Emu.h"
#include "Regs.h"

namespace MIPS
{
Context::ExecuteInstFn Context::execute_inst_fn[InstOpcodeCount] =
{
	nullptr  // For InstOpcodeNone
#define DEFINST(name, _fmt_str, _op0, _op1, _op2, _op3) \
		 , &Context::ExecuteInst_##name
#include <arch/mips/asm/Inst.def>
#undef DEFINST
};

void Context::mips_isa_branch(unsigned int dest)
{
	target_eip = dest;
}

void Context::mips_isa_rel_branch(unsigned int dest)
{
	target_eip = regs.getPC() + dest + 4;
}

void Context::ExecuteInst_J()
{
	// Read Operands
	unsigned int dest;
	unsigned int target = inst.getTarget();

	// Perform Operation
	dest = (misc::getBits32(regs.getPC()+4, 31, 28)) << 28 | (target << 2);
	mips_isa_branch(dest);
}


void Context::ExecuteInst_JAL()
{
	// Read Operands
	unsigned int reg_no = 31;
	unsigned int value = regs.getPC() + 8;
	unsigned int target = inst.getTarget();

	// Perform Operation
	unsigned int dest = (misc::getBits32(regs.getPC()+4, 31, 28)) << 28 | (target << 2);
	regs.setGPR(reg_no, value);
	mips_isa_branch(dest);
}


void Context::ExecuteInst_BEQ()
{
	// Read Operands
	unsigned int rs = inst.getBytes()->standard.rs;
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int imm = inst.getBytes()->offset_imm.offset;

	// Perform Operation
	if(regs.getGPR(rs) == regs.getGPR(rt))
		mips_isa_rel_branch(misc::SignExtend32(imm << 2, 16));
}


void Context::ExecuteInst_BNE()
{
	// Read Operands
	unsigned int rs = inst.getBytes()->standard.rs;
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int imm = inst.getBytes()->offset_imm.offset;

	// Perform Operation
	if (regs.getGPR(rs) != regs.getGPR(rt))
		mips_isa_rel_branch(misc::SignExtend32(imm << 2, 16));
}


void Context::ExecuteInst_BLEZ()
{
	// Read Operands
	unsigned int rs = inst.getBytes()->standard.rs;
	unsigned int imm = inst.getBytes()->offset_imm.offset;

	// Perform Operation
	if ((int)regs.getGPR(rs) <= 0)
		mips_isa_rel_branch(misc::SignExtend32(imm << 2, 16));
}


void Context::ExecuteInst_BGTZ()
{
	// Read Operands
	unsigned int imm = inst.getBytes()->offset_imm.offset;

	// Perform Operation
	mips_isa_rel_branch(misc::SignExtend32(imm << 2, 16));
}


void Context::ExecuteInst_ADDI()
{
	// Read Operands
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int rs = inst.getBytes()->standard.rs;
	unsigned int offset = inst.getBytes()->offset_imm.offset;

	// Perform Operation
	regs.setGPR(rt, ((int)regs.getGPR(rs) + (int)offset));
}


void Context::ExecuteInst_ADDIU()
{
	// Read Operands
	unsigned int IMM = inst.getBytes()->offset_imm.offset;
	unsigned int RS = inst.getBytes()->standard.rs;
	unsigned int RT = inst.getBytes()->standard.rt;

	// Perform Operation
	regs.setGPR(RT,regs.getGPR(RS) + misc::SignExtend32(IMM,16));
}


void Context::ExecuteInst_SLTI()
{
	// Read Operands
	unsigned int RT = inst.getBytes()->standard.rt;
	int IMM = inst.getBytes()->offset_imm.offset;

	// Perform Operation
	if ((int)(regs.getGPR(RT)) < IMM)
		regs.setGPR(RT, 1);
	else
		regs.setGPR(RT, 0);

}


void Context::ExecuteInst_SLTIU()
{
	// Read Operands
	unsigned int RT = inst.getBytes()->standard.rt;
	unsigned int IMM = inst.getBytes()->offset_imm.offset;

	// Perform Operation
	if ((unsigned int)(regs.getGPR(RT)) < IMM)
		regs.setGPR(RT, 1);
	else
		regs.setGPR(RT, 0);
}


void Context::ExecuteInst_ANDI()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_ORI()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_XORI()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_LUI()
{
	// Read Operands
	unsigned int IMM = inst.getBytes()->offset_imm.offset;
	unsigned int rt =inst.getBytes()->standard.rt;

	// Perform Operation
	regs.setGPR(rt,(unsigned int)(IMM << 16));
}


void Context::ExecuteInst_BEQL()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_BNEL()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_BLEZL()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_BGTZL()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_LB()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_LH()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_LWL()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_LW()
{
	// read operands from instruction
	unsigned int rs = inst.getBytes()->standard.rs;
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int IMM = inst.getBytes()->offset_imm.offset;

	// Perform operation LW
	unsigned int addr = regs.getGPR(rs) + misc::SignExtend32((signed)IMM,16);
	char temp;
	if ((misc::getBits32(addr, 1, 0) | 0) == 1 )
		misc::panic("LW: address error, effective address must be naturallty-aligned\n");
	memory->Read(addr, 4, &temp);
	regs.setGPR(rt, (int)temp);
}


void Context::ExecuteInst_LBU()
{
	// Read Operands from instruction
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int base = inst.getBytes()->offset_imm.base;
	unsigned int IMM = inst.getBytes()->offset_imm.offset;

	// Perform operation LBU
	unsigned int vAddr = regs.getGPR(base) + misc::SignExtend32((signed)IMM, 16);
	char temp;
	memory->Read(vAddr, sizeof(unsigned char), &temp);
	regs.setGPR(rt, (int)temp);
}


void Context::ExecuteInst_LHU()
{
	// Read Operands from instruction
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int base = inst.getBytes()->offset_imm.base;
	unsigned int IMM = inst.getBytes()->offset_imm.offset;

	// Perform operation LHU
	char temp;
	unsigned int addr = regs.getGPR(base) + misc::SignExtend32((signed)IMM, 16);
	memory->Read(addr, sizeof(char), &temp);
	regs.setGPR(rt, (int)temp);
}


void Context::ExecuteInst_LWR()
{
	// Read Operands from instruction
	char src[4];
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int base = inst.getBytes()->offset_imm.base;
	unsigned int IMM = inst.getBytes()->offset_imm.offset;
	unsigned int rt_value = regs.getGPR(rt);
	unsigned char *dst = (unsigned char *) & rt_value;

	// Perform operation LWR
	unsigned int addr = regs.getGPR(base) + misc::SignExtend32((signed)IMM, 16);
	int i, size = 1 + (addr & 3);
	memory->Read(addr-size+1, size, src);
	for (i = 0; i < size; i++)
		dst[size - i - 1]= src[i];
}


void Context::ExecuteInst_SB()
{
	// Read operands from instruction
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int base = inst.getBytes()->offset_imm.base;
	unsigned int IMM = inst.getBytes()->offset_imm.offset;

	// Perform operation SB
	char temp = regs.getGPR(rt);
	unsigned int addr = regs.getGPR(base) + misc::SignExtend32((signed)IMM, 16);
	memory->Write(addr, sizeof(unsigned char), &temp);
}


void Context::ExecuteInst_SH()
{
	// Read operands from instruction
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int base = inst.getBytes()->offset_imm.base;
	unsigned int IMM = inst.getBytes()->offset_imm.offset;

	// Perform operation SH
	const char temp = regs.getGPR(rt);
	unsigned int addr = regs.getGPR(base) + misc::SignExtend32((signed)IMM, 16);
	memory->Write(addr, sizeof(unsigned short int), &temp);
}


void Context::ExecuteInst_SWL()
{
	// Read operands from instruction
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int base = inst.getBytes()->offset_imm.base;
	unsigned int IMM = inst.getBytes()->offset_imm.offset;
	unsigned int rt_value = regs.getGPR(rt);
	unsigned char *src = (unsigned char *) & rt_value;
	char dst[4];

	// Perform operation SWL
	unsigned int addr = regs.getGPR(base) + misc::SignExtend32((signed)IMM, 16);
	int i, size = 4 - (addr & 3);
	for (i = 0; i < size; i++)
		dst[i] = src[3 -i];
	memory->Write(addr, size, dst);
}


void Context::ExecuteInst_SW()
{
	// Read operands from instruction
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int temp = regs.getGPR(rt);
	unsigned int base = inst.getBytes()->offset_imm.base;
	unsigned int IMM = inst.getBytes()->offset_imm.offset;
	unsigned int addr = regs.getGPR(base) + misc::SignExtend32((signed)IMM, 16);

	// Perform operation SW
	char tmp = (char)temp;
	memory->Write(addr, 4, &tmp);
}


void Context::ExecuteInst_SWR()
{
	// Read operands from instruction
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int base = inst.getBytes()->offset_imm.base;
	unsigned int IMM = inst.getBytes()->offset_imm.offset;
	unsigned int rt_value = regs.getGPR(rt);
	unsigned char *src = (unsigned char *) & rt_value;
	char dst[4];

	// Perform operation SWR
	unsigned int addr = regs.getGPR(base) + misc::SignExtend32((signed)IMM, 16);
	int i, size = 1 + (addr & 3);
	for (i = 0; i < size; i++)
		dst[i] = src[size - i -1];
	memory->Write(addr - size + 1, size, dst);
}


void Context::ExecuteInst_CACHE()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_LL()
{
	// Read operands from instruction
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int base = inst.getBytes()->offset_imm.base;
	unsigned int IMM = inst.getBytes()->offset_imm.offset;
	char temp;

	// Perform operation LL
	unsigned int addr = regs.getGPR(base) + misc::SignExtend32((signed)IMM, 16);
	memory->Read(addr, 4, &temp);
	regs.setGPR(rt, temp);
	// FIXME: set LLbit = 1
}


void Context::ExecuteInst_LWC1()
{
	// Read operands from instruction
	unsigned int base = inst.getBytes()->offset_imm.base;
	unsigned int IMM = inst.getBytes()->offset_imm.offset;
	unsigned int ft = inst.getBytes()->offset_imm.rt;
	char temp;
	float f;

	// Perform operation LWC1
	unsigned int addr = regs.getGPR(base) + misc::SignExtend32((signed)IMM, 16);
	memory->Read(addr, 4, &temp);
	f = (float)temp;
	regs.setFPR(ft, f);
}


void Context::ExecuteInst_LWC2()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_PREF()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_LDC1()
{
	// Read operands from instruction
	unsigned int base = inst.getBytes()->offset_imm.base;
	unsigned int IMM = inst.getBytes()->offset_imm.offset;
	unsigned int ft =inst.getBytes()->offset_imm.rt;
	char temp;
	float f;

	// Perform operation LDC1
	unsigned int addr = regs.getGPR(base) + misc::SignExtend32((signed)IMM, 16);
	memory->Read(addr, 4, &temp);
	f = (float)temp;
	regs.setFPR(ft, f);
}


void Context::ExecuteInst_LDC2()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SC()
{
	// Read operands from instruction
	unsigned int base = inst.getBytes()->offset_imm.base;
	unsigned int IMM = inst.getBytes()->offset_imm.offset;
	unsigned int rt = inst.getBytes()->offset_imm.rt;
	unsigned int temp = regs.getGPR(rt);

	// Perform operation SC
	unsigned int addr = regs.getGPR(base) + misc::SignExtend32((signed)IMM, 16);
	char tmp = (char) temp;
	memory->Write(addr, 4, &tmp);
	regs.setGPR(rt, 1);
}


void Context::ExecuteInst_SWC1()
{
	// Read operands from instruction
	unsigned int base = inst.getBytes()->offset_imm.base;
	unsigned int IMM = inst.getBytes()->offset_imm.offset;
	unsigned int ft =inst.getBytes()->offset_imm.rt;
	char temp;

	// Perform operation SWC1
	unsigned int addr = regs.getGPR(base) + misc::SignExtend32((signed)IMM, 16);
	memory->Read(addr, 4, &temp);
	float f = float(temp);
	regs.setFPR(ft, f);
}


void Context::ExecuteInst_SWC2()
{

}


void Context::ExecuteInst_SDC1()
{
	// Read operands from instruction
	unsigned int base = inst.getBytes()->offset_imm.base;
	unsigned int IMM = inst.getBytes()->offset_imm.offset;
	unsigned int ft = inst.getBytes()->standard.rt;
	double dbl;

	// Perform operation SDC1
	dbl = (double)regs.getFPR(ft);
	unsigned char temp = * (unsigned char *) &dbl;
	unsigned int addr = regs.getGPR(base) + misc::SignExtend32((signed)IMM, 16);
	memory->Write(addr, sizeof(unsigned int), reinterpret_cast<const char*>(&temp));
}


void Context::ExecuteInst_SDC2()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SLL()
{
	// Read operands from instruction
	unsigned int rd = inst.getBytes()->standard.rd;
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int sa = inst.getBytes()->standard.sa;

	// Perform operation SLL
	regs.setGPR(rd,(regs.getGPR(rt) << sa));
}


void Context::ExecuteInst_MOVF()
{
	// FIXME: need to implement float point
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MOVT()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SRL()
{
	// Read operands from instruction
	unsigned int rd = inst.getBytes()->standard.rd;
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int sa = inst.getBytes()->standard.sa;

	// Perform operation SRL
	unsigned int temp = regs.getGPR(rt) >> sa;
	regs.setGPR(rd, temp);
}


void Context::ExecuteInst_ROR()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SRA()
{
	// Read operands from instruction
	unsigned int rd = inst.getBytes()->standard.rd;
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int sa = inst.getBytes()->standard.sa;

	// Perform operation SRA
	unsigned int temp = regs.getGPR(rt) >> sa;
	regs.setGPR(rd, temp);
}


void Context::ExecuteInst_SLLV()
{
	// Read operands from instruction
	unsigned int rd = inst.getBytes()->standard.rd;
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int rs = inst.getBytes()->standard.rs;

	// Perform operation SLLV
	unsigned int temp = regs.getGPR(rt) << rs;
	regs.setGPR(rd, temp);
}


void Context::ExecuteInst_SRLV()
{
	// Read operands from instruction
	unsigned int rd = inst.getBytes()->standard.rd;
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int rs = inst.getBytes()->standard.rs;

	// Perform operation SRLV
	unsigned int s = misc::getBits32(regs.getGPR(rs), 4, 0);
	regs.setGPR(rd, regs.getGPR(rt) >> s);
}


void Context::ExecuteInst_ROTRV()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SRAV()
{
	// Read operands from instruction
	unsigned int rd = inst.getBytes()->standard.rd;
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int rs = inst.getBytes()->standard.rs;

	// Perform operation SRAV
	unsigned int s = misc::getBits32(regs.getGPR(rs), 4, 0);
	regs.setGPR(rd, (int)regs.getGPR(rt) >> s);
}


void Context::ExecuteInst_JR()
{
	// Read operands from instruction
	unsigned int rs = inst.getBytes()->standard.rs;

	// Perform operation JR
	mips_isa_branch(regs.getGPR(rs));
}


void Context::ExecuteInst_JALR()
{
	// Read operands from instruction
	unsigned int rs = inst.getBytes()->standard.rs;
	unsigned int rd = inst.getBytes()->standard.rd;

	// Perform operation JALR
	mips_isa_branch(regs.getGPR(rs));
	regs.setGPR(rd, (regs.getPC() + 8));
}


void Context::ExecuteInst_MOVZ()
{
	// Read operands from instruction
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int rd = inst.getBytes()->standard.rd;
	unsigned int rs = inst.getBytes()->standard.rs;

	// Perform operation MOVZ
	if (regs.getGPR(rt) == 0)
		regs.setGPR(rd, regs.getGPR(rs));
}


void Context::ExecuteInst_MOVN()
{
	// Read operands from instruction
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int rd = inst.getBytes()->standard.rd;
	unsigned int rs = inst.getBytes()->standard.rs;

	// Perform operation MOVZ
	if (regs.getGPR(rt) != 0)
		regs.setGPR(rd, regs.getGPR(rs));
}


void Context::ExecuteInst_SYSCALL()
{
	ExecuteSyscall();
}


void Context::ExecuteInst_BREAK()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SYNC()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MFHI()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MTHI()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MFLO()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MTLO()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MULT()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MULTU()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_DIV()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_DIVU()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_ADD()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_ADDU()
{
	// read operands from instruction
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int rs = inst.getBytes()->standard.rs;
	unsigned int rd = inst.getBytes()->standard.rd;

	// perform operation ADDU
	unsigned int value = regs.getGPR(rt) + regs.getGPR(rs);
	regs.setGPR(rd, value);
}


void Context::ExecuteInst_SUB()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SUBU()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_AND()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_OR()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_XOR()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_NOR()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SLT()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SLTU()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_TGE()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_TGEU()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_TLT()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_TLTU()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_TEQ()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_TNE()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_BLTZ()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_BGEZ()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_BLTZL()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_BGEZL()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_TGEI()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_TGEIU()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_TLTI()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_TLTIU()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_TEQI()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_TNEI()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_BLTZAL()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_BGEZAL()
{
	unsigned int pc = regs.getPC();
	std::cout<<"pc = "<<pc;
}


void Context::ExecuteInst_BLTZALL()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_BGEZALL()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SYNCI()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MADD()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MADDU()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MUL()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MSUB()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MSUBU()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_CLZ()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_CLO()
{
	throw misc::Panic("Unimplemented instruction");
}









void Context::ExecuteInst_EXT()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_INS()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_WSBH()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SEB()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SEH()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_RDHWR()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MFCO()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MTCO()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_RDPGPR()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_DI()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_EI()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_WRPGPR()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_TLBR()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_TLBWI()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_TLBWR()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_TLBP()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_ERET()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_DERET()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_WAIT()
{
	throw misc::Panic("Unimplemented instruction");
}






void Context::ExecuteInst_MFC1()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_CFC1()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MFHC1()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MTC1()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_CTC1()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MTHC1()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_BC1F()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_BC1FL()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_BC1T()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_BC1TL()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_ADD_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SUB_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MUL_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_DIV_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SQRT_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_ABS_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MOV_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_NEG_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_ROUND_L_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_TRUNC_L_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_CEIL_L_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_FLOOR_L_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_ROUND_W_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_TRUNC_W_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_CEIL_W_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_FLOOR_W_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MOVF_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MOVT_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MOVZ_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MOVN_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_RECIP_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_RSQRT_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_CVT_D_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_CVT_W_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_CVT_L_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_CVT_PS_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_ADD_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SUB_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MUL_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_DIV_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SQRT_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_ABS_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MOV_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_NEG_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_ROUND_L_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_TRUNC_L_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_CEIL_L_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_FLOOR_L_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_ROUND_W_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_TRUNC_W_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_CEIL_W_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_FLOOR_W_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MOVF_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MOVT_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MOVZ_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MOVN_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_RECIP_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_RSQRT_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_CVT_S_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_CVT_W_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_CVT_L_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_C_F_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_C_UN_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_C_EQ_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_C_UEQ_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_C_OLT_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_C_ULT_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_C_OLE_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_C_ULE_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_C_SF_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_C_NGLE_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_C_SEQ_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_C_NGL_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_C_LT_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_C_NGE_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_C_LE_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_C_NGT_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_CVT_S_W()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_CVT_D_W()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_ADD_PS()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SUB_PS()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MUL_PS()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_ABS_PS()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MOV_PS()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_NEG_PS()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MOVF_PS()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MOVT_PS()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MOVZ_PS()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MOVN_PS()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_CVT_S_PU()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_CVT_S_PL()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_PLL_PS()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_PLU_PS()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_PUL_PS()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_PUU_PS()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MFC2()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_CFC2()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MFHC2()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MTC2()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_CTC2()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MTHC2()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_BC2F()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_BC2FL()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_BC2T()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_BC2TL()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_COP2()
{
	throw misc::Panic("Unimplemented instruction");
}
}