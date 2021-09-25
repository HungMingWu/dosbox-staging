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

	CASE_0F_W(0x00)												/* GRP 6 Exxx */
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
						*earw = saveval;
					} else {
						GetEAa;
						SaveMw(eaa, saveval);
					}
				}
				break;
			case 0x02:case 0x03:case 0x04:case 0x05:
				{
					Bitu loadval;
					if (rm >= 0xc0) {
						Bit16u *earw = GetEArw(rm);
						loadval = *earw;
					} else {
						GetEAa;
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
				goto illegal_opcode;
			}
		}
		break;
	CASE_0F_W(0x01)												/* Group 7 Ew */
		{
			Bit8u rm = Fetchb();Bitu which=(rm>>3)&7;
			if (rm < 0xc0)	{ //First ones all use EA
				GetEAa;Bitu limit;
				switch (which) {
				case 0x00:										/* SGDT */
					SaveMw(eaa,CPU_SGDT_limit());
					SaveMd(eaa+2,CPU_SGDT_base());
					break;
				case 0x01:										/* SIDT */
					SaveMw(eaa,CPU_SIDT_limit());
					SaveMd(eaa+2,CPU_SIDT_base());
					break;
				case 0x02:										/* LGDT */
					if (cpu.pmode && cpu.cpl) EXCEPTION(EXCEPTION_GP);
					CPU_LGDT(LoadMw(eaa),LoadMd(eaa+2) & 0xFFFFFF);
					break;
				case 0x03:										/* LIDT */
					if (cpu.pmode && cpu.cpl) EXCEPTION(EXCEPTION_GP);
					CPU_LIDT(LoadMw(eaa),LoadMd(eaa+2) & 0xFFFFFF);
					break;
				case 0x04:										/* SMSW */
					SaveMw(eaa,CPU_SMSW());
					break;
				case 0x06:										/* LMSW */
					limit=LoadMw(eaa);
					if (CPU_LMSW(limit)) RUNEXCEPTION();
					break;
				case 0x07:										/* INVLPG */
					if (cpu.pmode && cpu.cpl) EXCEPTION(EXCEPTION_GP);
					PAGING_ClearTLB();
					break;
				}
			} else {
				Bit16u *earw = GetEArw(rm);
				switch (which) {
				case 0x02:										/* LGDT */
					if (cpu.pmode && cpu.cpl) EXCEPTION(EXCEPTION_GP);
					goto illegal_opcode;
				case 0x03:										/* LIDT */
					if (cpu.pmode && cpu.cpl) EXCEPTION(EXCEPTION_GP);
					goto illegal_opcode;
				case 0x04:										/* SMSW */
					*earw=CPU_SMSW();
					break;
				case 0x06:										/* LMSW */
					if (CPU_LMSW(*earw)) RUNEXCEPTION();
					break;
				default:
					goto illegal_opcode;
				}
			}
		}
		break;
	CASE_0F_W(0x02)												/* LAR Gw,Ew */
		{
			if ((reg_flags & FLAG_VM) || (!cpu.pmode)) goto illegal_opcode;
			Bit8u rm = Fetchb();
	        Bit16u *rmrw = Getrw(rm);
			Bitu ar = *rmrw;
			if (rm >= 0xc0) {
				Bit16u *earw = GetEArw(rm);
				CPU_LAR(*earw, ar);
			} else {
				GetEAa;
				CPU_LAR(LoadMw(eaa), ar);
			}
			*rmrw=(Bit16u)ar;
		}
		break;
	CASE_0F_W(0x03)												/* LSL Gw,Ew */
		{
			if ((reg_flags & FLAG_VM) || (!cpu.pmode)) goto illegal_opcode;
	        Bit8u rm = Fetchb();
			Bit16u *rmrw = Getrw(rm);
			Bitu limit=*rmrw;
			if (rm >= 0xc0) {
				Bit16u *earw = GetEArw(rm);
				CPU_LSL(*earw, limit);
			} else {
				GetEAa;
				CPU_LSL(LoadMw(eaa), limit);
			}
			*rmrw=(Bit16u)limit;
		}
		break;
	CASE_0F_B(0x06)												/* CLTS */
		if (cpu.pmode && cpu.cpl) EXCEPTION(EXCEPTION_GP);
		cpu.cr0&=(~CR0_TASKSWITCH);
		break;
	CASE_0F_B(0x08)												/* INVD */
	CASE_0F_B(0x09)												/* WBINVD */
		if (CPU_ArchitectureType<CPU_ARCHTYPE_486OLDSLOW) goto illegal_opcode;
		if (cpu.pmode && cpu.cpl) EXCEPTION(EXCEPTION_GP);
		break;
	CASE_0F_B(0x20)												/* MOV Rd.CRx */
		{
			Bit8u rm = Fetchb();
			Bitu which=(rm >> 3) & 7;
			if (rm < 0xc0 ) {
				rm |= 0xc0;
				LOG(LOG_CPU,LOG_ERROR)("MOV XXX,CR%u with non-register", static_cast<uint32_t>(which));
			}
			Bit32u *eard = GetEArd(rm);
			Bit32u crx_value;
			if (CPU_READ_CRX(which,crx_value)) RUNEXCEPTION();
			*eard=crx_value;
		}
		break;
	CASE_0F_B(0x21)												/* MOV Rd,DRx */
		{
			Bit8u rm = Fetchb();
			Bitu which=(rm >> 3) & 7;
			if (rm < 0xc0 ) {
				rm |= 0xc0;
				LOG(LOG_CPU,LOG_ERROR)("MOV XXX,DR%u with non-register", static_cast<uint32_t>(which));
			}
			Bit32u *eard = GetEArd(rm);
			Bit32u drx_value;
			if (CPU_READ_DRX(which,drx_value)) RUNEXCEPTION();
			*eard=drx_value;
		}
		break;
	CASE_0F_B(0x22)												/* MOV CRx,Rd */
		{
			Bit8u rm = Fetchb();
			Bitu which=(rm >> 3) & 7;
			if (rm < 0xc0 ) {
				rm |= 0xc0;
				LOG(LOG_CPU,LOG_ERROR)("MOV XXX,CR%u with non-register", static_cast<uint32_t>(which));
			}
			Bit32u *eard = GetEArd(rm);
			if (CPU_WRITE_CRX(which,*eard)) RUNEXCEPTION();
		}
		break;
	CASE_0F_B(0x23)												/* MOV DRx,Rd */
		{
			Bit8u rm = Fetchb();
			Bitu which=(rm >> 3) & 7;
			if (rm < 0xc0 ) {
				rm |= 0xc0;
				LOG(LOG_CPU,LOG_ERROR)("MOV DR%u,XXX with non-register", static_cast<uint32_t>(which));
			}
			Bit32u *eard = GetEArd(rm);
			if (CPU_WRITE_DRX(which,*eard)) RUNEXCEPTION();
		}
		break;
	CASE_0F_B(0x24)												/* MOV Rd,TRx */
		{
			Bit8u rm = Fetchb();
			Bitu which=(rm >> 3) & 7;
			if (rm < 0xc0 ) {
				rm |= 0xc0;
				LOG(LOG_CPU,LOG_ERROR)("MOV XXX,TR%u with non-register", static_cast<uint32_t>(which));
			}
			Bit32u *eard = GetEArd(rm);
			Bit32u trx_value;
			if (CPU_READ_TRX(which,trx_value)) RUNEXCEPTION();
			*eard=trx_value;
		}
		break;
	CASE_0F_B(0x26)												/* MOV TRx,Rd */
		{
			Bit8u rm = Fetchb();
			Bitu which=(rm >> 3) & 7;
			if (rm < 0xc0 ) {
				rm |= 0xc0;
				LOG(LOG_CPU,LOG_ERROR)("MOV TR%u,XXX with non-register", static_cast<uint32_t>(which));
			}
			Bit32u *eard = GetEArd(rm);
			if (CPU_WRITE_TRX(which,*eard)) RUNEXCEPTION();
		}
		break;
	CASE_0F_B(0x31)												/* RDTSC */
		{
			if (CPU_ArchitectureType<CPU_ARCHTYPE_PENTIUMSLOW)
				goto illegal_opcode;
			/* Use a fixed number when in auto cycles mode as else the reported value changes constantly */
	                Bit64s tsc = (Bit64s)(PIC_FullIndex() *
	                                      static_cast<double>(CPU_CycleAutoAdjust
	                                                                  ? 70000
	                                                                  : CPU_CycleMax));
	                reg_edx = (Bit32u)(tsc >> 32);
			reg_eax = (Bit32u)(tsc & 0xffffffff);
		}
		break;
	CASE_0F_W(0x80) /* JO */
		JumpCond16_w(TFLG_O);
		break;
	CASE_0F_W(0x81) /* JNO */
		JumpCond16_w(TFLG_NO);break;
	CASE_0F_W(0x82)												/* JB */
		JumpCond16_w(TFLG_B);break;
	CASE_0F_W(0x83)												/* JNB */
		JumpCond16_w(TFLG_NB);break;
	CASE_0F_W(0x84)												/* JZ */
		JumpCond16_w(TFLG_Z);break;
	CASE_0F_W(0x85)												/* JNZ */
		JumpCond16_w(TFLG_NZ);break;
	CASE_0F_W(0x86)												/* JBE */
		JumpCond16_w(TFLG_BE);break;
	CASE_0F_W(0x87)												/* JNBE */
		JumpCond16_w(TFLG_NBE);break;
	CASE_0F_W(0x88)												/* JS */
		JumpCond16_w(TFLG_S);break;
	CASE_0F_W(0x89)												/* JNS */
		JumpCond16_w(TFLG_NS);break;
	CASE_0F_W(0x8a)												/* JP */
		JumpCond16_w(TFLG_P);break;
	CASE_0F_W(0x8b)												/* JNP */
		JumpCond16_w(TFLG_NP);break;
	CASE_0F_W(0x8c)												/* JL */
		JumpCond16_w(TFLG_L);break;
	CASE_0F_W(0x8d)												/* JNL */
		JumpCond16_w(TFLG_NL);break;
	CASE_0F_W(0x8e)												/* JLE */
		JumpCond16_w(TFLG_LE);break;
	CASE_0F_W(0x8f)												/* JNLE */
		JumpCond16_w(TFLG_NLE);break;
	CASE_0F_B(0x90)												/* SETO */
		SETcc(TFLG_O);break;
	CASE_0F_B(0x91)												/* SETNO */
		SETcc(TFLG_NO);break;
	CASE_0F_B(0x92)												/* SETB */
		SETcc(TFLG_B);break;
	CASE_0F_B(0x93)												/* SETNB */
		SETcc(TFLG_NB);break;
	CASE_0F_B(0x94)												/* SETZ */
		SETcc(TFLG_Z);break;
	CASE_0F_B(0x95)												/* SETNZ */
		SETcc(TFLG_NZ);	break;
	CASE_0F_B(0x96)												/* SETBE */
		SETcc(TFLG_BE);break;
	CASE_0F_B(0x97)												/* SETNBE */
		SETcc(TFLG_NBE);break;
	CASE_0F_B(0x98)												/* SETS */
		SETcc(TFLG_S);break;
	CASE_0F_B(0x99)												/* SETNS */
		SETcc(TFLG_NS);break;
	CASE_0F_B(0x9a)												/* SETP */
		SETcc(TFLG_P);break;
	CASE_0F_B(0x9b)												/* SETNP */
		SETcc(TFLG_NP);break;
	CASE_0F_B(0x9c)												/* SETL */
		SETcc(TFLG_L);break;
	CASE_0F_B(0x9d)												/* SETNL */
		SETcc(TFLG_NL);break;
	CASE_0F_B(0x9e)												/* SETLE */
		SETcc(TFLG_LE);break;
	CASE_0F_B(0x9f)												/* SETNLE */
		SETcc(TFLG_NLE);break;

	CASE_0F_W(0xa0)												/* PUSH FS */		
		Push_16(SegValue(fs));break;
	CASE_0F_W(0xa1)												/* POP FS */	
		if (CPU_PopSeg(fs,false)) RUNEXCEPTION();
		break;
	CASE_0F_B(0xa2)												/* CPUID */
		if (!CPU_CPUID()) goto illegal_opcode;
		break;
	CASE_0F_W(0xa3)												/* BT Ew,Gw */
		{
			FillFlags();
			Bit8u rm = Fetchb();
			Bit16u *rmrw = Getrw(rm);
			Bit16u mask = 1 << (*rmrw & 15);
			if (rm >= 0xc0) {
				Bit16u *earw = GetEArw(rm);
				SETFLAGBIT(FLAG_CF, (*earw & mask));
			} else {
				GetEAa;
				eaa += (((Bit16s)*rmrw) >> 4) * 2;
				if (!TEST_PREFIX_ADDR) FixEA16;
				Bit16u old = LoadMw(eaa);
				SETFLAGBIT(FLAG_CF, (old & mask));
			}
			break;
		}
	CASE_0F_W(0xa4)												/* SHLD Ew,Gw,Ib */
		RMEwGwOp3(DSHLW,Fetchb());
		break;
	CASE_0F_W(0xa5)												/* SHLD Ew,Gw,CL */
		RMEwGwOp3(DSHLW,reg_cl);
		break;
	CASE_0F_W(0xa8)												/* PUSH GS */		
		Push_16(SegValue(gs));break;
	CASE_0F_W(0xa9)												/* POP GS */		
		if (CPU_PopSeg(gs,false)) RUNEXCEPTION();
		break;
	CASE_0F_W(0xab)												/* BTS Ew,Gw */
		{
			FillFlags();
			Bit8u rm = Fetchb();
			Bit16u *rmrw = Getrw(rm);
			Bit16u mask = 1 << (*rmrw & 15);
			if (rm >= 0xc0) {
				Bit16u *earw = GetEArw(rm);
				SETFLAGBIT(FLAG_CF, (*earw & mask));
				*earw |= mask;
			} else {
				GetEAa;
				eaa += (((Bit16s)*rmrw) >> 4) * 2;
				if (!TEST_PREFIX_ADDR) FixEA16;
				Bit16u old=LoadMw(eaa);
				SETFLAGBIT(FLAG_CF, (old & mask));
				SaveMw(eaa,old | mask);
			}
			break;
		}
	CASE_0F_W(0xac)												/* SHRD Ew,Gw,Ib */
		RMEwGwOp3(DSHRW,Fetchb());
		break;
	CASE_0F_W(0xad)												/* SHRD Ew,Gw,CL */
		RMEwGwOp3(DSHRW,reg_cl);
		break;
	CASE_0F_W(0xaf)												/* IMUL Gw,Ew */
		RMGwEwOp3(DIMULW,*rmrw);
		break;
	CASE_0F_B(0xb0) 										/* cmpxchg Eb,Gb */
		{
			if (CPU_ArchitectureType<CPU_ARCHTYPE_486OLDSLOW) goto illegal_opcode;
			FillFlags();
			Bit8u rm = Fetchb();
			Bit8u *rmrb = Getrb(rm);	
			if (rm >= 0xc0) {
				Bit8u *earb = GetEArb(rm);
				if (reg_al == *earb) {
					*earb = *rmrb;
					SETFLAGBIT(FLAG_ZF,true);
				} else {
					reg_al = *earb;
					SETFLAGBIT(FLAG_ZF,false);
				}
			} else {
   				GetEAa;
				Bit8u val = LoadMb(eaa);
				if (reg_al == val) { 
					SaveMb(eaa,*rmrb);
					SETFLAGBIT(FLAG_ZF, true);
				} else {
					SaveMb(eaa,val);	// cmpxchg always issues a write
					reg_al = val;
					SETFLAGBIT(FLAG_ZF, false);
				}
			}
			break;
		}
	CASE_0F_W(0xb1) 									/* cmpxchg Ew,Gw */
		{
			if (CPU_ArchitectureType<CPU_ARCHTYPE_486OLDSLOW) goto illegal_opcode;
			FillFlags();
	        Bit8u rm = Fetchb();
	        Bit16u *rmrw = Getrw(rm);
			if (rm >= 0xc0) {
				Bit16u *earw = GetEArw(rm);
				if(reg_ax == *earw) { 
					*earw = *rmrw;
					SETFLAGBIT(FLAG_ZF, true);
				} else {
					reg_ax = *earw;
					SETFLAGBIT(FLAG_ZF, false);
				}
			} else {
   				GetEAa;
				Bit16u val = LoadMw(eaa);
				if(reg_ax == val) { 
					SaveMw(eaa,*rmrw);
					SETFLAGBIT(FLAG_ZF, true);
				} else {
					SaveMw(eaa,val);	// cmpxchg always issues a write
					reg_ax = val;
					SETFLAGBIT(FLAG_ZF, false);
				}
			}
			break;
		}

	CASE_0F_W(0xb2)												/* LSS Ew */
		{
			Bit8u rm = Fetchb();
	        Bit16u *rmrw = Getrw(rm);
			if (rm >= 0xc0) goto illegal_opcode;
			GetEAa;
			if (CPU_SetSegGeneral(ss,LoadMw(eaa+2))) RUNEXCEPTION();
			*rmrw=LoadMw(eaa);
			break;
		}
	CASE_0F_W(0xb3)												/* BTR Ew,Gw */
		{
			FillFlags();
	        Bit8u rm = Fetchb();
	        Bit16u *rmrw = Getrw(rm);
			Bit16u mask = 1 << (*rmrw & 15);
			if (rm >= 0xc0 ) {
				Bit16u *earw = GetEArw(rm);
				SETFLAGBIT(FLAG_CF, (*earw & mask));
				*earw &= ~mask;
			} else {
				GetEAa;
				eaa += (((Bit16s)*rmrw) >> 4)*2;
				if (!TEST_PREFIX_ADDR) FixEA16;
				Bit16u old = LoadMw(eaa);
				SETFLAGBIT(FLAG_CF,(old & mask));
				SaveMw(eaa, old & ~mask);
			}
			break;
		}
	CASE_0F_W(0xb4)												/* LFS Ew */
		{	
			Bit8u rm = Fetchb();
	        Bit16u *rmrw = Getrw(rm);
			if (rm >= 0xc0) goto illegal_opcode;
			GetEAa;
			if (CPU_SetSegGeneral(fs,LoadMw(eaa+2))) RUNEXCEPTION();
			*rmrw=LoadMw(eaa);
			break;
		}
	CASE_0F_W(0xb5)												/* LGS Ew */
		{	
			Bit8u rm = Fetchb();
	        Bit16u *rmrw = Getrw(rm);
			if (rm >= 0xc0) goto illegal_opcode;
			GetEAa;
			if (CPU_SetSegGeneral(gs,LoadMw(eaa+2))) RUNEXCEPTION();
			*rmrw=LoadMw(eaa);
			break;
		}
	CASE_0F_W(0xb6)												/* MOVZX Gw,Eb */
		{
	        Bit8u rm = Fetchb();
	        Bit16u *rmrw = Getrw(rm);														
			if (rm >= 0xc0) {
				Bit8u *earb = GetEArb(rm);
				*rmrw = *earb;
			} else {
				GetEAa;
				*rmrw = LoadMb(eaa);
			}
			break;
		}
	CASE_0F_W(0xb7)												/* MOVZX Gw,Ew */
	CASE_0F_W(0xbf)												/* MOVSX Gw,Ew */
		{
	        Bit8u rm = Fetchb();
	        Bit16u *rmrw = Getrw(rm);														
			if (rm >= 0xc0) {
				Bit16u *earw = GetEArw(rm);
				*rmrw = *earw;
			} else {
				GetEAa;
				*rmrw = LoadMw(eaa);
			}
			break;
		}
	CASE_0F_W(0xba)												/* GRP8 Ew,Ib */
		{
			FillFlags();Bit8u rm = Fetchb();
			if (rm >= 0xc0 ) {
				Bit16u *earw = GetEArw(rm);
				Bit16u mask=1 << (Fetchb() & 15);
				SETFLAGBIT(FLAG_CF, (*earw & mask));
				switch (rm & 0x38) {
				case 0x20:										/* BT */
					break;
				case 0x28:										/* BTS */
					*earw|=mask;
					break;
				case 0x30:										/* BTR */
					*earw&= ~mask;
					break;
				case 0x38:										/* BTC */
					*earw^=mask;
					break;
				default:
					E_Exit("CPU:0F:BA:Illegal subfunction %X",rm & 0x38);
				}
			} else {
				GetEAa;Bit16u old=LoadMw(eaa);
				Bit16u mask=1 << (Fetchb() & 15);
				SETFLAGBIT(FLAG_CF, (old & mask));
				switch (rm & 0x38) {
				case 0x20:										/* BT */
					break;
				case 0x28:										/* BTS */
					SaveMw(eaa,old|mask);
					break;
				case 0x30:										/* BTR */
					SaveMw(eaa,old & ~mask);
					break;
				case 0x38:										/* BTC */
					SaveMw(eaa,old ^ mask);
					break;
				default:
					E_Exit("CPU:0F:BA:Illegal subfunction %X",rm & 0x38);
				}
			}
			break;
		}
	CASE_0F_W(0xbb)												/* BTC Ew,Gw */
		{
			FillFlags();
			Bit8u rm = Fetchb();
			Bit16u *rmrw = Getrw(rm);
			Bit16u mask = 1 << (*rmrw & 15);
			if (rm >= 0xc0) {
				Bit16u *earw = GetEArw(rm);
				SETFLAGBIT(FLAG_CF, (*earw & mask));
				*earw ^= mask;
			} else {
				GetEAa;eaa += (((Bit16s)*rmrw) >> 4) * 2;
				if (!TEST_PREFIX_ADDR) FixEA16;
				Bit16u old = LoadMw(eaa);
				SETFLAGBIT(FLAG_CF, (old & mask));
				SaveMw(eaa,old ^ mask);
			}
			break;
		}
	CASE_0F_W(0xbc)												/* BSF Gw,Ew */
		{
	        Bit8u rm = Fetchb();
	        Bit16u *rmrw = Getrw(rm);
			Bit16u result,value;
			if (rm >= 0xc0) {
				Bit16u *earw = GetEArw(rm);
				value = *earw;
			} else {
				GetEAa;
				value = LoadMw(eaa);
			}
			if (value==0) {
				SETFLAGBIT(FLAG_ZF, true);
			} else {
				result = 0;
				while ((value & 0x01)==0) { result++; value>>=1; }
				SETFLAGBIT(FLAG_ZF, false);
				*rmrw = result;
			}
			lflags.type=t_UNKNOWN;
			break;
		}
	CASE_0F_W(0xbd)												/* BSR Gw,Ew */
		{
	        Bit8u rm = Fetchb();
	        Bit16u *rmrw = Getrw(rm);
			Bit16u result,value;
			if (rm >= 0xc0) {
				Bit16u *earw = GetEArw(rm);
				value = *earw;
			} else { 
				GetEAa; 
				value = LoadMw(eaa);
			}
			if (value==0) {
				SETFLAGBIT(FLAG_ZF,true);
			} else {
				result = 15;	// Operandsize-1
				while ((value & 0x8000)==0) { result--; value<<=1; }
				SETFLAGBIT(FLAG_ZF,false);
				*rmrw = result;
			}
			lflags.type=t_UNKNOWN;
			break;
		}
	CASE_0F_W(0xbe)												/* MOVSX Gw,Eb */
		{
	        Bit8u rm = Fetchb();
	        Bit16u *rmrw = Getrw(rm);														
			if (rm >= 0xc0) {
				Bit8u *earb = GetEArb(rm);
				*rmrw = *(Bit8s *)earb;
			} else {
				GetEAa;
				*rmrw = LoadMbs(eaa);
			}
			break;
		}
	CASE_0F_B(0xc0)												/* XADD Gb,Eb */
		{
			if (CPU_ArchitectureType<CPU_ARCHTYPE_486OLDSLOW) goto illegal_opcode;
			Bit8u rm = Fetchb();
			Bit8u *rmrb = Getrb(rm);
			Bit8u oldrmrb = *rmrb;
			if (rm >= 0xc0) {
				Bit8u *earb = GetEArb(rm);
				*rmrb = *earb;
				*earb += oldrmrb;
			} else {
				GetEAa;
				*rmrb = LoadMb(eaa);
				SaveMb(eaa, LoadMb(eaa) + oldrmrb);
			}
			break;
		}
	CASE_0F_W(0xc1)												/* XADD Gw,Ew */
		{
			if (CPU_ArchitectureType<CPU_ARCHTYPE_486OLDSLOW) goto illegal_opcode;
	        Bit8u rm = Fetchb();
	        Bit16u *rmrw = Getrw(rm);
			Bit16u oldrmrw = *rmrw;
			if (rm >= 0xc0) {
				Bit16u *earw = GetEArw(rm);
				*rmrw = *earw;
				*earw += oldrmrw;
			} else {
				GetEAa;
				*rmrw = LoadMw(eaa);
				SaveMw(eaa, LoadMw(eaa) + oldrmrw);
			}
			break;
		}
	CASE_0F_W(0xc8)												/* BSWAP AX */
		if (CPU_ArchitectureType<CPU_ARCHTYPE_486OLDSLOW) goto illegal_opcode;
		BSWAPW(reg_ax);break;
	CASE_0F_W(0xc9)												/* BSWAP CX */
		if (CPU_ArchitectureType<CPU_ARCHTYPE_486OLDSLOW) goto illegal_opcode;
		BSWAPW(reg_cx);break;
	CASE_0F_W(0xca)												/* BSWAP DX */
		if (CPU_ArchitectureType<CPU_ARCHTYPE_486OLDSLOW) goto illegal_opcode;
		BSWAPW(reg_dx);break;
	CASE_0F_W(0xcb)												/* BSWAP BX */
		if (CPU_ArchitectureType<CPU_ARCHTYPE_486OLDSLOW) goto illegal_opcode;
		BSWAPW(reg_bx);break;
	CASE_0F_W(0xcc)												/* BSWAP SP */
		if (CPU_ArchitectureType<CPU_ARCHTYPE_486OLDSLOW) goto illegal_opcode;
		BSWAPW(reg_sp);break;
	CASE_0F_W(0xcd)												/* BSWAP BP */
		if (CPU_ArchitectureType<CPU_ARCHTYPE_486OLDSLOW) goto illegal_opcode;
		BSWAPW(reg_bp);break;
	CASE_0F_W(0xce)												/* BSWAP SI */
		if (CPU_ArchitectureType<CPU_ARCHTYPE_486OLDSLOW) goto illegal_opcode;
		BSWAPW(reg_si);break;
	CASE_0F_W(0xcf)												/* BSWAP DI */
		if (CPU_ArchitectureType<CPU_ARCHTYPE_486OLDSLOW) goto illegal_opcode;
		BSWAPW(reg_di);break;
		
