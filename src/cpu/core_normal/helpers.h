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

#define RMEbGb(inst)														\
	{																		\
		Bit8u rm = Fetchb();																\
		Bit8u *rmrb = Getrb(rm);											\
		if (rm >= 0xc0) {													\
			Bit8u* earb = GetEArb(rm);					                    \
			inst(*earb,*rmrb,LoadRb,SaveRb);								\
		} else {															\
			PhysPt eaa = core.ea_table[rm](); 														\
			inst(eaa,*rmrb,LoadMb,SaveMb);									\
		}																	\
	}

#define RMGbEb(inst)														\
	{																		\
		Bit8u rm = Fetchb();																\
		Bit8u *rmrb = Getrb(rm);											\
		if (rm >= 0xc0) {													\
			Bit8u* earb = GetEArb(rm);										\
			inst(*rmrb,*earb,LoadRb,SaveRb);								\
		} else {															\
			PhysPt eaa = core.ea_table[rm](); 														\
			inst(*rmrb,LoadMb(eaa),LoadRb,SaveRb);							\
		}				\
	}

#define RMEb(inst)															\
	{																		\
		if (rm >= 0xc0) {													\
			Bit8u *earb = GetEArb(rm);										\
			inst(*earb,LoadRb,SaveRb);										\
		}																	\
		else {																\
			PhysPt eaa = core.ea_table[rm](); 														\
			inst(eaa,LoadMb,SaveMb);										\
		}																	\
	}

#define RMEwGw(inst)														\
	{																		\
		Bit8u rm = Fetchb();																\
		Bit16u *rmrw = Getrw(rm);											\
		if (rm >= 0xc0) {													\
			Bit16u *earw = GetEArw(rm);										\
			inst(*earw,*rmrw,LoadRw,SaveRw);								\
		} else {															\
			PhysPt eaa = core.ea_table[rm](); 														\
			inst(eaa,*rmrw,LoadMw,SaveMw);									\
		}																	\
	}

#define RMEwGwOp3(inst,op3)													\
	{																		\
		Bit8u rm = Fetchb();																\
		Bit16u *rmrw = Getrw(rm);											\
		if (rm >= 0xc0) {													\
			Bit16u *earw = GetEArw(rm);										\
			inst(*earw,*rmrw,op3,LoadRw,SaveRw);							\
		}else {																\
			PhysPt eaa = core.ea_table[rm](); 														\
			inst(eaa,*rmrw,op3,LoadMw,SaveMw);								\
		}																	\
	}

#define RMGwEw(inst)														\
	{																		\
		Bit8u rm = Fetchb();																\
		Bit16u *rmrw = Getrw(rm);											\
		if (rm >= 0xc0) {													\
			Bit16u *earw = GetEArw(rm);										\
			inst(*rmrw,*earw,LoadRw,SaveRw);								\
		} else {															\
			PhysPt eaa = core.ea_table[rm](); 														\
			inst(*rmrw,LoadMw(eaa),LoadRw,SaveRw);							\
		}																	\
	}																

#define RMGwEwOp3(inst,op3)													\
	{																		\
		Bit8u rm = Fetchb();																\
		Bit16u *rmrw = Getrw(rm);											\
		if (rm >= 0xc0) {													\
			Bit16u *earw = GetEArw(rm);										\
			inst(*rmrw,*earw,op3,LoadRw,SaveRw);							\
		} else {															\
			PhysPt eaa = core.ea_table[rm](); 														\
			inst(*rmrw,LoadMw(eaa),op3,LoadRw,SaveRw);						\
		}																	\
	}																

#define RMEw(inst)															\
	{																		\
		if (rm >= 0xc0) {													\
			Bit16u *earw = GetEArw(rm);										\
			inst(*earw,LoadRw,SaveRw);										\
		} else {															\
			PhysPt eaa = core.ea_table[rm](); 														\
			inst(eaa,LoadMw,SaveMw);										\
		}																	\
	}

#define RMEdGd(inst)														\
	{																		\
		Bit8u rm = Fetchb();																\
		Bit32u *rmrd = Getrd(rm);											\
		if (rm >= 0xc0) {													\
			Bit32u *eard = GetEArd(rm);										\
			inst(*eard,*rmrd,LoadRd,SaveRd);								\
		} else {															\
			PhysPt eaa = core.ea_table[rm](); 														\
			inst(eaa,*rmrd,LoadMd,SaveMd);									\
		}																	\
	}

#define RMEdGdOp3(inst,op3)													\
	{																		\
		Bit8u rm = Fetchb();																\
		Bit32u *rmrd = Getrd(rm);											\
		if (rm >= 0xc0) {													\
			Bit32u *eard = GetEArd(rm);										\
			inst(*eard,*rmrd,op3,LoadRd,SaveRd);							\
		} else {															\
			PhysPt eaa = core.ea_table[rm](); 														\
			inst(eaa,*rmrd,op3,LoadMd,SaveMd);								\
		}																	\
	}


#define RMGdEd(inst)														\
	{																		\
		Bit8u rm = Fetchb();																\
		Bit32u *rmrd = Getrd(rm);											\
		if (rm >= 0xc0) {													\
			Bit32u *eard = GetEArd(rm);										\
			inst(*rmrd,*eard,LoadRd,SaveRd);								\
		} else {															\
			PhysPt eaa = core.ea_table[rm](); 														\
			inst(*rmrd,LoadMd(eaa),LoadRd,SaveRd);							\
		}																	\
	}																

#define RMGdEdOp3(inst,op3)													\
	{																		\
		Bit8u rm = Fetchb();																\
		Bit32u *rmrd = Getrd(rm);											\
		if (rm >= 0xc0) {													\
			Bit32u *eard = GetEArd(rm);										\
			inst(*rmrd,*eard,op3,LoadRd,SaveRd);							\
		} else {															\
			PhysPt eaa = core.ea_table[rm](); 														\
			inst(*rmrd,LoadMd(eaa),op3,LoadRd,SaveRd);						\
		}																	\
	}																

#define RMEw(inst)															\
	{																		\
		if (rm >= 0xc0) {													\
			Bit16u *earw = GetEArw(rm);										\
			inst(*earw,LoadRw,SaveRw);										\
		} else {															\
			PhysPt eaa = core.ea_table[rm](); 														\
			inst(eaa,LoadMw,SaveMw);										\
		}																	\
	}

#define RMEd(inst)															\
	{																		\
		if (rm >= 0xc0) {													\
			Bit32u *eard = GetEArd(rm);										\
			inst(*eard,LoadRd,SaveRd);										\
		} else {															\
			PhysPt eaa = core.ea_table[rm](); 														\
			inst(eaa,LoadMd,SaveMd);										\
		}																	\
	}

#define ALIb(inst)															\
	{ inst(reg_al,Fetchb(),LoadRb,SaveRb)}

#define AXIw(inst)															\
	{ inst(reg_ax,Fetchw(),LoadRw,SaveRw);}

#define EAXId(inst)															\
	{ inst(reg_eax,Fetchd(),LoadRd,SaveRd);}

#define FPU_ESC(code) {														\
	Bit8u rm=Fetchb();														\
	if (rm >= 0xc0) {														\
		FPU_ESC ## code ## _Normal(rm);										\
	} else {																\
		PhysPt eaa = core.ea_table[rm](); 															\
		FPU_ESC ## code ## _EA(rm,eaa);										\
	}																		\
}

#define CASE_W(_WHICH)							\
	case (OPCODE_NONE+_WHICH):

#define CASE_D(_WHICH)							\
	case (OPCODE_SIZE+_WHICH):

#define CASE_B(_WHICH)							\
	CASE_W(_WHICH)								\
	CASE_D(_WHICH)

#define CASE_0F_W(_WHICH)						\
	case ((OPCODE_0F|OPCODE_NONE)+_WHICH):

#define CASE_0F_D(_WHICH)						\
	case ((OPCODE_0F|OPCODE_SIZE)+_WHICH):

#define CASE_0F_B(_WHICH)						\
	CASE_0F_W(_WHICH)							\
	CASE_0F_D(_WHICH)

#define FixEA16 \
	do { \
		switch (rm & 7) { \
		case 6: \
			if (rm < 0x40) \
				break; \
			[[fallthrough]]; \
		case 2: \
		case 3: BaseDS = BaseSS; \
		} \
		eaa = BaseDS + (Bit16u)(eaa - BaseDS); \
	} while (0)
