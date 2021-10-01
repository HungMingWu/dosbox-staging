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

#ifndef DOSBOX_PAGING_H
#define DOSBOX_PAGING_H

#include "dosbox.h"

#include "mem.h"

// disable this to reduce the size of the TLB
// NOTE: does not work with the dynamic core (dynrec is fine)

class PageDirectory;

#define MEM_PAGE_SIZE	(4096)
#define XMS_START		(0x110)

#define TLB_SIZE		(1024*1024)

#define PFLAG_READABLE		0x1
#define PFLAG_WRITEABLE		0x2
#define PFLAG_HASROM		0x4
#define PFLAG_HASCODE32		0x8				//Page contains 32-bit dynamic code
#define PFLAG_NOCODE		0x10			//No dynamic code can be generated here
#define PFLAG_INIT			0x20			//No dynamic code can be generated here
#define PFLAG_HASCODE16		0x40			//Page contains 16-bit dynamic code
#define PFLAG_HASCODE		(PFLAG_HASCODE32|PFLAG_HASCODE16)

#define LINK_START	((1024+64)/4)			//Start right after the HMA

//Allow 128 mb of memory to be linked
#define PAGING_LINKS (128*1024/4)

class PageHandler {
public:
	virtual ~PageHandler() = default;

	virtual Bitu readb(PhysPt addr);
	virtual Bitu readw(PhysPt addr);
	virtual Bitu readd(PhysPt addr);
	virtual void writeb(PhysPt addr,Bitu val);
	virtual void writew(PhysPt addr,Bitu val);
	virtual void writed(PhysPt addr,Bitu val);
	virtual HostPt GetHostReadPt(Bitu phys_page);
	virtual HostPt GetHostWritePt(Bitu phys_page);
	virtual bool readb_checked(PhysPt addr,Bit8u * val);
	virtual bool readw_checked(PhysPt addr,Bit16u * val);
	virtual bool readd_checked(PhysPt addr,Bit32u * val);
	virtual bool writeb_checked(PhysPt addr,Bitu val);
	virtual bool writew_checked(PhysPt addr,Bitu val);
	virtual bool writed_checked(PhysPt addr,Bitu val);

	Bitu flags = 0x0;
};

void PAGING_InitTLB(void);

/* This maps the page directly, only use when paging is disabled */
bool PAGING_ForcePageInit(Bitu lin_addr);

void MEM_SetLFB(Bitu page, Bitu pages, PageHandler *handler, PageHandler *mmiohandler);
void MEM_SetPageHandler(Bitu phys_page, Bitu pages, PageHandler * handler);
void MEM_ResetPageHandler(Bitu phys_page, Bitu pages);


#ifdef _MSC_VER
#pragma pack (1)
#endif
struct X86_PageEntryBlock{
#ifdef WORDS_BIGENDIAN
	Bit32u		base:20;
	Bit32u		avl:3;
	Bit32u		g:1;
	Bit32u		pat:1;
	Bit32u		d:1;
	Bit32u		a:1;
	Bit32u		pcd:1;
	Bit32u		pwt:1;
	Bit32u		us:1;
	Bit32u		wr:1;
	Bit32u		p:1;
#else
	Bit32u		p:1;
	Bit32u		wr:1;
	Bit32u		us:1;
	Bit32u		pwt:1;
	Bit32u		pcd:1;
	Bit32u		a:1;
	Bit32u		d:1;
	Bit32u		pat:1;
	Bit32u		g:1;
	Bit32u		avl:3;
	Bit32u		base:20;
#endif
} GCC_ATTRIBUTE(packed);
#ifdef _MSC_VER
#pragma pack ()
#endif

union X86PageEntry {
	Bit32u load;
	X86_PageEntryBlock block;
};

struct PagingBlock {
	Bitu			cr3;
	Bitu			cr2;
	struct {
		Bitu page;
		PhysPt addr;
	} base;
	struct {
		HostPt read[TLB_SIZE];
		HostPt write[TLB_SIZE];
		PageHandler * readhandler[TLB_SIZE];
		PageHandler * writehandler[TLB_SIZE];
		Bit32u	phys_page[TLB_SIZE];
	} tlb;
	struct {
		Bitu used;
		Bit32u entries[PAGING_LINKS];
	} links;
	Bit32u		firstmb[LINK_START];
	bool		enabled;

public:

	/* Some other functions */
	void Enable(bool enabled);
	bool Enabled();

	void clearTLB();

	void LinkPage(Bitu lin_page, Bitu phys_page, bool readOnly = false);
	void UnlinkPages(Bitu lin_page, Bitu pages);
	Bitu GetDirBase();
	void SetDirBase(Bitu cr3);

	/* This maps the page directly, only use when paging is disabled */
	void MapPage(Bitu lin_page, Bitu phys_page);
	bool MakePhysPage(Bitu &page);

	template <bool read>
	inline HostPt get_tlb(PhysPt address)
	{
		if constexpr (read) {
			return tlb.read[address >> 12];
		} else {
			return tlb.write[address >> 12];
		}
	}

	template <bool read>
	inline PageHandler* get_tlb_handler(PhysPt address)
	{
		if constexpr (read) {
			return tlb.readhandler[address >> 12];
		} else {
			return tlb.writehandler[address >> 12];
		}
	}

	/* Use these helper functions to access linear addresses in readX/writeX
	 * functions */
	inline PhysPt GetPhysicalPage(PhysPt linePage)
	{
		return (tlb.phys_page[linePage >> 12] << 12);
	}

	inline PhysPt GetPhysicalAddress(PhysPt linAddr)
	{
		return (tlb.phys_page[linAddr >> 12] << 12) | (linAddr & 0xfff);
	}
};

extern PagingBlock paging; 

/* Some support functions */

PageHandler * MEM_GetPageHandler(Bitu phys_page);


/* Unaligned address handlers */
Bit16u mem_unalignedreadw(PhysPt address);
Bit32u mem_unalignedreadd(PhysPt address);
void mem_unalignedwritew(PhysPt address,Bit16u val);
void mem_unalignedwrited(PhysPt address,Bit32u val);

bool mem_unalignedreadw_checked(PhysPt address,Bit16u * val);
bool mem_unalignedreadd_checked(PhysPt address,Bit32u * val);
bool mem_unalignedwritew_checked(PhysPt address,Bit16u val);
bool mem_unalignedwrited_checked(PhysPt address,Bit32u val);

/* Special inlined memory reading/writing */

static INLINE Bit8u mem_readb_inline(PhysPt address) {
	HostPt tlb_addr = paging.get_tlb<true>(address);
	if (tlb_addr) return host_read<Bit8u>(tlb_addr+address);
	else return (Bit8u)(paging.get_tlb_handler<true>(address))->readb(address);
}

static INLINE Bit16u mem_readw_inline(PhysPt address) {
	if ((address & 0xfff)<0xfff) {
		HostPt tlb_addr = paging.get_tlb<true>(address);
		if (tlb_addr) return host_read<uint16_t>(tlb_addr+address);
		else return (Bit16u)(paging.get_tlb_handler<true>(address))->readw(address);
	} else return mem_unalignedreadw(address);
}

static INLINE uint32_t mem_readd_inline(PhysPt address)
{
	if ((address & 0xfff) < 0xffd) {
		HostPt tlb_addr = paging.get_tlb<true>(address);
		if (tlb_addr)
			return host_read<uint32_t>(tlb_addr + address);
		else
			return static_cast<uint32_t>((paging.get_tlb_handler<true>(address))->readd(address));
	} else {
		return mem_unalignedreadd(address);
	}
}

static INLINE void mem_writeb_inline(PhysPt address,Bit8u val) {
	HostPt tlb_addr = paging.get_tlb<false>(address);
	if (tlb_addr) host_write<uint8_t>(tlb_addr+address,val);
	else (paging.get_tlb_handler<false>(address))->writeb(address,val);
}

static INLINE void mem_writew_inline(PhysPt address,Bit16u val) {
	if ((address & 0xfff)<0xfff) {
		HostPt tlb_addr = paging.get_tlb<false>(address);
		if (tlb_addr) host_write<uint16_t>(tlb_addr+address,val);
		else (paging.get_tlb_handler<false>(address))->writew(address,val);
	} else mem_unalignedwritew(address,val);
}

static INLINE void mem_writed_inline(PhysPt address,Bit32u val) {
	if ((address & 0xfff)<0xffd) {
		HostPt tlb_addr = paging.get_tlb<false>(address);
		if (tlb_addr) host_write<uint32_t>(tlb_addr+address,val);
		else (paging.get_tlb_handler<false>(address))->writed(address,val);
	} else mem_unalignedwrited(address,val);
}


static INLINE bool mem_readb_checked(PhysPt address, Bit8u * val) {
	HostPt tlb_addr = paging.get_tlb<true>(address);
	if (tlb_addr) {
		*val=host_read<Bit8u>(tlb_addr+address);
		return false;
	} else return (paging.get_tlb_handler<true>(address))->readb_checked(address, val);
}

static INLINE bool mem_readw_checked(PhysPt address, Bit16u * val) {
	if ((address & 0xfff)<0xfff) {
		HostPt tlb_addr = paging.get_tlb<true>(address);
		if (tlb_addr) {
			*val=host_read<uint16_t>(tlb_addr+address);
			return false;
		} else return (paging.get_tlb_handler<true>(address))->readw_checked(address, val);
	} else return mem_unalignedreadw_checked(address, val);
}

static INLINE bool mem_readd_checked(PhysPt address, Bit32u * val) {
	if ((address & 0xfff)<0xffd) {
		HostPt tlb_addr = paging.get_tlb<true>(address);
		if (tlb_addr) {
			*val=host_read<uint32_t>(tlb_addr+address);
			return false;
		} else return (paging.get_tlb_handler<true>(address))->readd_checked(address, val);
	} else return mem_unalignedreadd_checked(address, val);
}

static INLINE bool mem_writeb_checked(PhysPt address,Bit8u val) {
	HostPt tlb_addr = paging.get_tlb<false>(address);
	if (tlb_addr) {
		host_write<uint8_t>(tlb_addr+address,val);
		return false;
	} else return (paging.get_tlb_handler<false>(address))->writeb_checked(address,val);
}

static INLINE bool mem_writew_checked(PhysPt address,Bit16u val) {
	if ((address & 0xfff)<0xfff) {
		HostPt tlb_addr = paging.get_tlb<false>(address);
		if (tlb_addr) {
			host_write<uint16_t>(tlb_addr+address,val);
			return false;
		} else return (paging.get_tlb_handler<false>(address))->writew_checked(address,val);
	} else return mem_unalignedwritew_checked(address,val);
}

static INLINE bool mem_writed_checked(PhysPt address,Bit32u val) {
	if ((address & 0xfff)<0xffd) {
		HostPt tlb_addr = paging.get_tlb<false>(address);
		if (tlb_addr) {
			host_write<uint32_t>(tlb_addr+address,val);
			return false;
		} else return (paging.get_tlb_handler<false>(address))->writed_checked(address,val);
	} else return mem_unalignedwrited_checked(address,val);
}


#endif
