"""
QuickBlocks - Decentralized, useful, and detailed data from Ethereum blockchains
Copyright (c) 2018 Great Hill Corporation (http://quickblocks.io)
This program is free software: you may redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation, either
version 3 of the License, or (at your option) any later version. This program is
distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details. You should have received a copy of the GNU General
Public License along with this program. If not, see http://www.gnu.org/licenses/.
"""

import sys
from quickblocks import QuickBlocks
from tqdm import tqdm
from multiprocessing.dummy import Pool as ThreadPool 

qb = QuickBlocks(sys.argv[1], cache=True)
for i in tqdm(range(int(sys.argv[2]), int(sys.argv[3]))):
    block = qb.check_block(i)
# n_workers = 8
# chunk = 1000
# pool = ThreadPool(n_workers) 
# for i in tqdm(range(int(sys.argv[2]), int(sys.argv[3]), chunk)):
#     pool.map(qb.__getItem__, range(i, i+chunk))
