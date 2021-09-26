/*
 *  Copyright (C) 2002-2021  The DOSBox Team
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
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

	CASE_0F_D(0x00)												/* GRP 6 Exxx */
		{
			if ((reg_flags & FLAG_VM) || (!cpu.pmode)) goto illegal_opcode;
			Bit8u rm = Fetchb();Bitu which=(rm>>3)&7;
			switch (which) {
			case 0x00:	/* SLDT */
			case 0x01:	/* STR */
				{
					Bitu saveval;
					if (!which) saveval=CPU_SLDT();
					else saveval=CPU_STR();
					if (rm >= 0xc0) {
						Bit16u *earw = GetEArw(rm);
						*earw = (Bit16u)saveval;
					} else {
						PhysPt eaa = core.ea_table[rm](); 
						SaveMw(eaa, saveval);
					}
				}
				break;
			case 0x02:case 0x03:case 0x04:case 0x05:
				{
					/* Just use 16-bit loads since were only using selectors */
					Bitu loadval;
					if (rm >= 0xc0) {
						Bit16u *earw = GetEArw(rm);
						loadval = *earw;
					} else {
						PhysPt eaa = core.ea_table[rm](); 
						loadval = LoadMw(eaa);
					}
					switch (which) {
					case 0x02:
						if (cpu.cpl) EXCEPTION(EXCEPTION_GP);
						if (CPU_LLDT(loadval)) RUNEXCEPTION();
						break;
					case 0x03:
						if (cpu.cpl) EXCEPTION(EXCEPTION_GP);
						if (CPU_LTR(loadval)) RUNEXCEPTION();
						break;
					case 0x04:
						CPU_VERR(loadval);
						break;
					case 0x05:
						CPU_VERW(loadval);
						break;
					}
				}
				break;
			default:
				LOG(LOG_CPU,LOG_ERROR)("GRP6:Illegal call %2X",static_cast<uint32_t>(which));
				goto illegal_opcode;
			}
		}
		break;
	CASE_0F_D(0x01)												/* Group 7 Ed */
		{
			Bit8u rm = Fetchb();Bitu which=(rm>>3)&7;
			if (rm < 0xc0)	{ //First ones all use EA
				PhysPt eaa = core.ea_table[rm](); 
				Bitu limit;
				switch (which) {
				case 0x00:										/* SGDT */
					SaveMw(eaa,(Bit16u)CPU_SGDT_limit());
					SaveMd(eaa+2,(Bit32u)CPU_SGDT_base());
					break;
				case 0x01:										/* SIDT */
					SaveMw(eaa,(Bit16u)CPU_SIDT_limit());
					SaveMd(eaa+2,(Bit32u)CPU_SIDT_base());
					break;
				case 0x02:										/* LGDT */
					if (cpu.pmode && cpu.cpl) EXCEPTION(EXCEPTION_GP);
					CPU_LGDT(LoadMw(eaa),LoadMd(eaa+2));
					break;
				case 0x03:										/* LIDT */
					if (cpu.pmode && cpu.cpl) EXCEPTION(EXCEPTION_GP);
					CPU_LIDT(LoadMw(eaa),LoadMd(eaa+2));
					break;
				case 0x04:										/* SMSW */
					SaveMw(eaa,(Bit16u)CPU_SMSW());
					break;
				case 0x06:										/* LMSW */
					limit=LoadMw(eaa);
					if (CPU_LMSW((Bit16u)limit)) RUNEXCEPTION();
					break;
				case 0x07:										/* INVLPG */
					if (cpu.pmode && cpu.cpl) EXCEPTION(EXCEPTION_GP);
					PAGING_ClearTLB();
					break;
				}
			} else {
				Bit32u *eard = GetEArd(rm);
				switch (which) {
				case 0x02:										/* LGDT */
					if (cpu.pmode && cpu.cpl) EXCEPTION(EXCEPTION_GP);
					goto illegal_opcode;
				case 0x03:										/* LIDT */
					if (cpu.pmode && cpu.cpl) EXCEPTION(EXCEPTION_GP);
					goto illegal_opcode;
				case 0x04:										/* SMSW */
					*eard=(Bit32u)CPU_SMSW();
					break;
				case 0x06:										/* LMSW */
					if (CPU_LMSW(*eard)) RUNEXCEPTION();
					break;
				default:
					LOG(LOG_CPU,LOG_ERROR)("Illegal group 7 RM subfunction %d",static_cast<int>(which));
					goto illegal_opcode;
					break;
				}

			}
		}
		break;
	CASE_0F_D(0x02)												/* LAR Gd,Ed */
		{
			if ((reg_flags & FLAG_VM) || (!cpu.pmode)) goto illegal_opcode;
			Bit8u rm = Fetchb();
			Bit32u *rmrd = Getrd(rm);
			Bitu ar = *rmrd;
			if (rm >= 0xc0) {
				Bit16u *earw = GetEArw(rm);
				CPU_LAR(*earw, ar);
			} else {
				PhysPt eaa = core.ea_table[rm](); 
				CPU_LAR(LoadMw(eaa), ar);
			}
			*rmrd=(Bit32u)ar;
		}
		break;
	CASE_0F_D(0x03)												/* LSL Gd,Ew */
		{
			if ((reg_flags & FLAG_VM) || (!cpu.pmode)) goto illegal_opcode;
	        Bit8u rm = Fetchb();
			Bit32u *rmrd = Getrd(rm);
			Bitu limit=*rmrd;
			/* Just load 16-bit values for selectors */
			if (rm >= 0xc0) {
				Bit16u *earw = GetEArw(rm);
				CPU_LSL(*earw, limit);
			} else {
				PhysPt eaa = core.ea_table[rm](); 
				CPU_LSL(LoadMw(eaa), limit);
			}
			*rmrd=(Bit32u)limit;
		}
		break;
	CASE_0F_D(0x80)												/* JO */
		JumpCond32_d(TFLG_O);break;
	CASE_0F_D(0x81)												/* JNO */
		JumpCond32_d(TFLG_NO);break;
	CASE_0F_D(0x82)												/* JB */
		JumpCond32_d(TFLG_B);break;
	CASE_0F_D(0x83)												/* JNB */
		JumpCond32_d(TFLG_NB);break;
	CASE_0F_D(0x84)												/* JZ */
		JumpCond32_d(TFLG_Z);break;
	CASE_0F_D(0x85)												/* JNZ */
		JumpCond32_d(TFLG_NZ);break;
	CASE_0F_D(0x86)												/* JBE */
		JumpCond32_d(TFLG_BE);break;
	CASE_0F_D(0x87)												/* JNBE */
		JumpCond32_d(TFLG_NBE);break;
	CASE_0F_D(0x88)												/* JS */
		JumpCond32_d(TFLG_S);break;
	CASE_0F_D(0x89)												/* JNS */
		JumpCond32_d(TFLG_NS);break;
	CASE_0F_D(0x8a)												/* JP */
		JumpCond32_d(TFLG_P);break;
	CASE_0F_D(0x8b)												/* JNP */
		JumpCond32_d(TFLG_NP);break;
	CASE_0F_D(0x8c)												/* JL */
		JumpCond32_d(TFLG_L);break;
	CASE_0F_D(0x8d)												/* JNL */
		JumpCond32_d(TFLG_NL);break;
	CASE_0F_D(0x8e)												/* JLE */
		JumpCond32_d(TFLG_LE);break;
	CASE_0F_D(0x8f)												/* JNLE */
		JumpCond32_d(TFLG_NLE);break;
	
	CASE_0F_D(0xa0)												/* PUSH FS */		
		CPU_Push32(SegValue(fs));break;
	CASE_0F_D(0xa1)												/* POP FS */		
		if (CPU_PopSeg(fs,true)) RUNEXCEPTION();
		break;
	CASE_0F_D(0xa3)												/* BT Ed,Gd */
		{
			FillFlags();
			Bit8u rm = Fetchb();
			Bit32u *rmrd = Getrd(rm);	
			Bit32u mask = 1 << (*rmrd & 31);
			if (rm >= 0xc0) {
				Bit32u *eard = GetEArd(rm);
				SETFLAGBIT(FLAG_CF, (*eard & mask));
			} else {
				PhysPt eaa = core.ea_table[rm](); 
				eaa += (((Bit32s)*rmrd) >> 5) * 4;
				if (!TEST_PREFIX_ADDR) FixEA16;
				Bit32u old = LoadMd(eaa);
				SETFLAGBIT(FLAG_CF, (old & mask));
			}
			break;
		}
	CASE_0F_D(0xa4)												/* SHLD Ed,Gd,Ib */
		RMEdGdOp3(DSHLD,Fetchb());
		break;
	CASE_0F_D(0xa5)												/* SHLD Ed,Gd,CL */
		RMEdGdOp3(DSHLD,reg_cl);
		break;
	CASE_0F_D(0xa8)												/* PUSH GS */		
		CPU_Push32(SegValue(gs));break;
	CASE_0F_D(0xa9)												/* POP GS */		
		if (CPU_PopSeg(gs,true)) RUNEXCEPTION();
		break;
	CASE_0F_D(0xab)												/* BTS Ed,Gd */
		{
			FillFlags();
			Bit8u rm = Fetchb();
			Bit32u *rmrd = Getrd(rm);	
			Bit32u mask=1 << (*rmrd & 31);
			if (rm >= 0xc0) {
				Bit32u *eard = GetEArd(rm);
				SETFLAGBIT(FLAG_CF, (*eard & mask));
				*eard |= mask;
			} else {
				PhysPt eaa = core.ea_table[rm](); 
				eaa += (((Bit32s)*rmrd) >> 5) * 4;
				if (!TEST_PREFIX_ADDR) FixEA16;
				Bit32u old = LoadMd(eaa);
				SETFLAGBIT(FLAG_CF, (old & mask));
				SaveMd(eaa,old | mask);
			}
			break;
		}
	
	CASE_0F_D(0xac)												/* SHRD Ed,Gd,Ib */
		RMEdGdOp3(DSHRD,Fetchb());
		break;
	CASE_0F_D(0xad)												/* SHRD Ed,Gd,CL */
		RMEdGdOp3(DSHRD,reg_cl);
		break;
	CASE_0F_D(0xaf)												/* IMUL Gd,Ed */
		{
			RMGdEdOp3(DIMULD,*rmrd);
			break;
		}
	CASE_0F_D(0xb1)												/* CMPXCHG Ed,Gd */
		{	
			if (CPU_ArchitectureType<CPU_ARCHTYPE_486NEWSLOW) goto illegal_opcode;
			FillFlags();
			Bit8u rm = Fetchb();
			Bit32u *rmrd = Getrd(rm);
			if (rm >= 0xc0) {
				Bit32u *eard = GetEArd(rm);
				if (*eard == reg_eax) {
					*eard = *rmrd;
					SETFLAGBIT(FLAG_ZF, true);
				} else {
					reg_eax = *eard;
					SETFLAGBIT(FLAG_ZF, false);
				}
			} else {
				PhysPt eaa = core.ea_table[rm](); 
				Bit32u val=LoadMd(eaa);
				if (val==reg_eax) {
					SaveMd(eaa,*rmrd);
					SETFLAGBIT(FLAG_ZF, true);
				} else {
					SaveMd(eaa,val);	// cmpxchg always issues a write
					reg_eax=val;
					SETFLAGBIT(FLAG_ZF, false);
				}
			}
			break;
		}
	CASE_0F_D(0xb2)												/* LSS Ed */
		{
			Bit8u rm = Fetchb();
			Bit32u *rmrd = Getrd(rm);	
			if (rm >= 0xc0) goto illegal_opcode;
			PhysPt eaa = core.ea_table[rm](); 
			if (CPU_SetSegGeneral(ss,LoadMw(eaa+4))) RUNEXCEPTION();
			*rmrd=LoadMd(eaa);
			break;
		}
	CASE_0F_D(0xb3)												/* BTR Ed,Gd */
		{
			FillFlags();
			Bit8u rm = Fetchb();
			Bit32u *rmrd = Getrd(rm);
			Bit32u mask = 1 << (*rmrd & 31);
			if (rm >= 0xc0) {
				Bit32u *eard = GetEArd(rm);
				SETFLAGBIT(FLAG_CF, (*eard & mask));
				*eard &= ~mask;
			} else {
				PhysPt eaa = core.ea_table[rm](); 
				eaa += (((Bit32s)*rmrd) >> 5) * 4;
				if (!TEST_PREFIX_ADDR) FixEA16;
				Bit32u old = LoadMd(eaa);
				SETFLAGBIT(FLAG_CF, (old & mask));
				SaveMd(eaa, old & ~mask);
			}
			break;
		}
	CASE_0F_D(0xb4)												/* LFS Ed */
		{	
			Bit8u rm = Fetchb();
	        Bit32u *rmrd = Getrd(rm);
			if (rm >= 0xc0) goto illegal_opcode;
			PhysPt eaa = core.ea_table[rm](); 
			if (CPU_SetSegGeneral(fs,LoadMw(eaa+4))) RUNEXCEPTION();
			*rmrd=LoadMd(eaa);
			break;
		}
	CASE_0F_D(0xb5)												/* LGS Ed */
		{	
			Bit8u rm = Fetchb();
	        Bit32u *rmrd = Getrd(rm);	
			if (rm >= 0xc0) goto illegal_opcode;
			PhysPt eaa = core.ea_table[rm](); 
			if (CPU_SetSegGeneral(gs,LoadMw(eaa+4))) RUNEXCEPTION();
			*rmrd=LoadMd(eaa);
			break;
		}
	CASE_0F_D(0xb6)												/* MOVZX Gd,Eb */
		{
	        Bit8u rm = Fetchb();
	        Bit32u *rmrd = Getrd(rm);															
			if (rm >= 0xc0) {
				Bit8u *earb = GetEArb(rm);
				*rmrd = *earb;
			} else {
				PhysPt eaa = core.ea_table[rm](); 
				*rmrd = LoadMb(eaa);
			}
			break;
		}
	CASE_0F_D(0xb7)												/* MOVXZ Gd,Ew */
		{
	        Bit8u rm = Fetchb();
	        Bit32u *rmrd = Getrd(rm);
			if (rm >= 0xc0) {
				Bit16u *earw = GetEArw(rm);
				*rmrd = *earw;
			} else {
				PhysPt eaa = core.ea_table[rm](); 
				*rmrd = LoadMw(eaa);
			}
			break;
		}
	CASE_0F_D(0xba)												/* GRP8 Ed,Ib */
		{
			FillFlags();Bit8u rm = Fetchb();
			if (rm >= 0xc0 ) {
				Bit32u *eard = GetEArd(rm);
				Bit32u mask=1 << (Fetchb() & 31);
				SETFLAGBIT(FLAG_CF, (*eard & mask));
				switch (rm & 0x38) {
				case 0x20:											/* BT */
					break;
				case 0x28:											/* BTS */
					*eard|=mask;
					break;
				case 0x30:											/* BTR */
					*eard&=~mask;
					break;
				case 0x38:											/* BTC */
					if (GETFLAG(CF)) *eard&=~mask;
					else *eard|=mask;
					break;
				default:
					E_Exit("CPU:66:0F:BA:Illegal subfunction %X",rm & 0x38);
				}
			} else {
				PhysPt eaa = core.ea_table[rm](); 
				Bit32u old = LoadMd(eaa);
				Bit32u mask=1 << (Fetchb() & 31);
				SETFLAGBIT(FLAG_CF, (old & mask));
				switch (rm & 0x38) {
				case 0x20:											/* BT */
					break;
				case 0x28:											/* BTS */
					SaveMd(eaa,old|mask);
					break;
				case 0x30:											/* BTR */
					SaveMd(eaa,old & ~mask);
					break;
				case 0x38:											/* BTC */
					if (GETFLAG(CF)) old&=~mask;
					else old|=mask;
					SaveMd(eaa,old);
					break;
				default:
					E_Exit("CPU:66:0F:BA:Illegal subfunction %X",rm & 0x38);
				}
			}
			break;
		}
	CASE_0F_D(0xbb)												/* BTC Ed,Gd */
		{
			FillFlags();
	        Bit8u rm = Fetchb();
			Bit32u *rmrd = Getrd(rm);
			Bit32u mask = 1 << (*rmrd & 31);
			if (rm >= 0xc0) {
				Bit32u * eard = GetEArd(rm);
				SETFLAGBIT(FLAG_CF, (*eard & mask));
				*eard ^= mask;
			} else {
				PhysPt eaa = core.ea_table[rm](); 
				eaa += (((Bit32s)*rmrd) >> 5)*4;
				if (!TEST_PREFIX_ADDR) FixEA16;
				Bit32u old = LoadMd(eaa);
				SETFLAGBIT(FLAG_CF, (old & mask));
				SaveMd(eaa, old ^ mask);
			}
			break;
		}
	CASE_0F_D(0xbc)												/* BSF Gd,Ed */
		{
	        Bit8u rm = Fetchb();
	        Bit32u *rmrd = Getrd(rm);
			Bit32u result,value;
			if (rm >= 0xc0) {
				Bit32u *eard = GetEArd(rm);
				value = *eard;
			} else {
				PhysPt eaa = core.ea_table[rm](); 
				value = LoadMd(eaa);
			}
			if (value==0) {
				SETFLAGBIT(FLAG_ZF, true);
			} else {
				result = 0;
				while ((value & 0x01)==0) { result++; value>>=1; }
				SETFLAGBIT(FLAG_ZF, false);
				*rmrd = result;
			}
			lflags.type=t_UNKNOWN;
			break;
		}
	CASE_0F_D(0xbd)												/*  BSR Gd,Ed */
		{
	        Bit8u rm = Fetchb();
	        Bit32u *rmrd = Getrd(rm);
			Bit32u result,value;
			if (rm >= 0xc0) {
				Bit32u *eard = GetEArd(rm);
				value = *eard;
			} else {
				PhysPt eaa = core.ea_table[rm](); 
				value = LoadMd(eaa);
			}
			if (value==0) {
				SETFLAGBIT(FLAG_ZF, true);
			} else {
				result = 31;	// Operandsize-1
				while ((value & 0x80000000)==0) { result--; value<<=1; }
				SETFLAGBIT(FLAG_ZF,false);
				*rmrd = result;
			}
			lflags.type=t_UNKNOWN;
			break;
		}
	CASE_0F_D(0xbe)												/* MOVSX Gd,Eb */
		{
	        Bit8u rm = Fetchb();
	        Bit32u *rmrd = Getrd(rm);														
			if (rm >= 0xc0) {
				Bit8u *earb = GetEArb(rm);
				*rmrd = *(Bit8s *)earb;
			} else {
				PhysPt eaa = core.ea_table[rm](); 
				*rmrd = LoadMbs(eaa);
			}
			break;
		}
	CASE_0F_D(0xbf)												/* MOVSX Gd,Ew */
		{
	        Bit8u rm = Fetchb();
	        Bit32u *rmrd = Getrd(rm);														
			if (rm >= 0xc0) {
				Bit16u *earw = GetEArw(rm);
				*rmrd = *(Bit16s *)earw;
			} else {
				PhysPt eaa = core.ea_table[rm](); 
				*rmrd = LoadMws(eaa);
			}
			break;
		}
	CASE_0F_D(0xc1)												/* XADD Gd,Ed */
		{
			if (CPU_ArchitectureType<CPU_ARCHTYPE_486OLDSLOW) goto illegal_opcode;
	        Bit8u rm = Fetchb();
			Bit32u *rmrd = Getrd(rm);	
			Bit32u oldrmrd=*rmrd;
			if (rm >= 0xc0) {
				Bit32u *eard = GetEArd(rm);
				*rmrd = *eard;
				*eard += oldrmrd;
			} else {
				PhysPt eaa = core.ea_table[rm](); 
				*rmrd = LoadMd(eaa);
				SaveMd(eaa, LoadMd(eaa) + oldrmrd);
			}
			break;
		}
	CASE_0F_D(0xc8)												/* BSWAP EAX */
		if (CPU_ArchitectureType<CPU_ARCHTYPE_486OLDSLOW) goto illegal_opcode;
		BSWAPD(reg_eax);break;
	CASE_0F_D(0xc9)												/* BSWAP ECX */
		if (CPU_ArchitectureType<CPU_ARCHTYPE_486OLDSLOW) goto illegal_opcode;
		BSWAPD(reg_ecx);break;
	CASE_0F_D(0xca)												/* BSWAP EDX */
		if (CPU_ArchitectureType<CPU_ARCHTYPE_486OLDSLOW) goto illegal_opcode;
		BSWAPD(reg_edx);break;
	CASE_0F_D(0xcb)												/* BSWAP EBX */
		if (CPU_ArchitectureType<CPU_ARCHTYPE_486OLDSLOW) goto illegal_opcode;
		BSWAPD(reg_ebx);break;
	CASE_0F_D(0xcc)												/* BSWAP ESP */
		if (CPU_ArchitectureType<CPU_ARCHTYPE_486OLDSLOW) goto illegal_opcode;
		BSWAPD(reg_esp);break;
	CASE_0F_D(0xcd)												/* BSWAP EBP */
		if (CPU_ArchitectureType<CPU_ARCHTYPE_486OLDSLOW) goto illegal_opcode;
		BSWAPD(reg_ebp);break;
	CASE_0F_D(0xce)												/* BSWAP ESI */
		if (CPU_ArchitectureType<CPU_ARCHTYPE_486OLDSLOW) goto illegal_opcode;
		BSWAPD(reg_esi);break;
	CASE_0F_D(0xcf)												/* BSWAP EDI */
		if (CPU_ArchitectureType<CPU_ARCHTYPE_486OLDSLOW) goto illegal_opcode;
		BSWAPD(reg_edi);break;
