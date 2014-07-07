/*
 * Multi2Sim
 * Copyright (C) 2014 Agamemnon Despopoulos (agdespopoulos@gmail.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <lib/cpp/String.h>

#include "Bank.h"
#include "Rank.h"


namespace dram
{


Rank::Rank(int id,
		Channel *parent,
		int num_banks,
		int num_rows,
		int num_columns,
		int num_bits)
		:
		id(id),
		channel(parent)
{
	// Create the banks for this rank.
	for (int i = 0; i < num_banks; i++)
	{
		auto bank = std::make_shared<Bank>(i, this, num_rows,
				num_columns, num_bits);
		banks.push_back(bank);
	}
}


void Rank::dump(std::ostream &os) const
{
	os << misc::fmt("\t\tDumping Rank %d\n", id);
	os << misc::fmt("\t\t%d Banks\n\t\tBank dump:\n", (int) banks.size());

	for (auto bank : banks)
	{
		bank->dump(os);
	}
}


}  // namespace dram
