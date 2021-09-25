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

extern Bit8u  * lookupRMregb[];
extern Bit16u * lookupRMregw[];
extern Bit32u * lookupRMregd[];
extern Bit8u  * lookupRMEAregb[256];
extern Bit16u * lookupRMEAregw[256];
extern Bit32u * lookupRMEAregd[256];


inline Bit8u *Getrb(Bit8u rm) { return lookupRMregb[rm]; }
inline Bit16u *Getrw(Bit8u rm) { return lookupRMregw[rm]; }
inline Bit32u *Getrd(Bit8u rm) { return lookupRMregd[rm]; }

inline Bit8u *GetEArb(Bit8u rm) { return lookupRMEAregb[rm]; }
inline Bit16u *GetEArw(Bit8u rm) { return lookupRMEAregw[rm]; }
inline Bit32u *GetEArd(Bit8u rm) { return lookupRMEAregd[rm]; }