/*-------------------------------------------------------------------------------------------
 * QuickBlocks - Decentralized, useful, and detailed data from Ethereum blockchains
 * Copyright (c) 2018 Great Hill Corporation (http://quickblocks.io)
 *
 * This program is free software: you may redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version. This program is
 * distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details. You should have received a copy of the GNU General
 * Public License along with this program. If not, see http://www.gnu.org/licenses/.
 *-------------------------------------------------------------------------------------------*/
#include "options.h"

//---------------------------------------------------------------------------------------------------
CParams params[] = {
    CParams("~block_list",       "a space-separated list of one or more blocks to retrieve"),
    CParams("-raw",              "pull the block data from the running Ethereum node (no cache)"),
    CParams("-h(a)shes",         "display only transaction hashes, default is to display full transaction detail"),
//    CParams("-trac(e)s",         "include transaction traces in the export"),
    CParams("-check",            "compare results between qblocks and Ethereum node, report differences, if any"),
    CParams("-latest",           "display the latest blocks at both the node and the cache"),
    CParams("@force",            "force a re-write of the block to the cache"),
    CParams("@quiet",            "do not print results to screen, used for speed testing and data checking"),
    CParams("@source:[c|r]",     "either :c(a)che or :(r)aw, source for data retrival. (shortcuts -c = qblocks, -r = node)"),
    CParams("@fields:[a|m|c|r]", "either :(a)ll, (m)ini, (c)ache or :(r)aw; which fields to include in output (all is default)"),
    CParams("@normalize",        "normalize (remove un-common fields and sort) for comparison with other results (testing)"),
    CParams("",                  "Returns block(s) from local cache or directly from a running node.\n"),
};
uint32_t nParams = sizeof(params) / sizeof(CParams);

//---------------------------------------------------------------------------------------------------
bool COptions::parseArguments(string_q& command) {

    if (!standardOptions(command))
        return false;

    Init();
    blknum_t latestBlock = getLatestBlockFromClient();
    bool isLatest = false;
    while (!command.empty()) {

        string_q arg = nextTokenClear(command, ' ');

        // shortcuts
        if (arg == "-r" || arg == "--raw")   { arg = "--source:raw";   }
        if (arg == "-a" || arg == "--cache") { arg = "--source:cache"; }

        // do not collapse
        if (arg == "-c" || arg == "--check") {
            setenv("TEST_MODE", "true", true);
            isCheck = true;
            quiet++; // if both --check and --quiet are present, be very quiet...
            expContext().spcs = 4;
            expContext().hexNums = true;
            expContext().quoteNums = true;
            CRuntimeClass *pClass = GETRUNTIME_CLASS(CBlock);
            if (pClass) {
                CFieldData *pField = pClass->FindField("blockNumber");
                if (pField)
                    pField->setName("number");
            }
            pClass = GETRUNTIME_CLASS(CBlock);
            if (pClass) {
                CFieldData *pField = pClass->FindField("hash");
                if (pField)
                    pField->setName("blockHash");
            }
            GETRUNTIME_CLASS(CBlock)->sortFieldList();
            GETRUNTIME_CLASS(CTransaction)->sortFieldList();
            GETRUNTIME_CLASS(CReceipt)->sortFieldList();

        } else if (arg == "-o" || arg == "--force") {
            etherlib_init("binary");
            latestBlock = getLatestBlockFromClient();
            force = true;

        } else if (arg == "--normalize") {
            normalize = true;

        } else if (arg == "-l" || arg == "--latest") {
            uint64_t lastUpdate = toUnsigned(asciiFileToString("/tmp/getBlock_junk.txt"));
            uint64_t cache=NOPOS, client=NOPOS;
            getLatestBlocks(cache, client);
            uint64_t diff = cache > client ? 0 : client - cache;
            stringToAsciiFile("/tmp/getBlock_junk.txt", asStringU(diff)); // for next time

            cout << "Latest block in cache:  " << cYellow << (isTestMode() ? "--cache--"  : padNum8T(cache))  << cOff << "\n";
            cout << "Latest block at client: " << cYellow << (isTestMode() ? "--client--" : padNum8T(client)) << cOff << "\n";
            cout << "Difference:             " << cYellow << (isTestMode() ? "--diff--"   : padNum8T(diff));
            if (!isTestMode() && lastUpdate) {
                uint64_t diffDiff = lastUpdate - diff;
                cout << " (+" << diffDiff << ")";
            }
            cout << cOff << "\n";
            isLatest = true;

        } else if (startsWith(arg, "-s:") || startsWith(arg, "--source:")) {
            string_q mode = arg.Substitute("-s:","").Substitute("--source:","");
            if (mode == "r" || mode == "raw") {
                isRaw = true;

            } else if (mode == "c" || mode == "cache") {
                latestBlock = getLatestBlockFromClient();
                isCache = true;

            } else if (mode == "r" || mode == "remote") {
                etherlib_init("remote");
                latestBlock = getLatestBlockFromClient();

            } else {
                return usage("Invalide source. Must be either '(r)aw' or '(c)ache'. Quitting...");
            }

        } else if (arg == "-a" || arg == "--hashes") {
            hashes = true;

        } else if (arg == "-e" || arg == "--traces") {
            traces = true;

        } else if (arg == "-q" || arg == "--quiet") {
            quiet++; // if both --check and --quiet are present, be very quiet...

        } else if (startsWith(arg, "-f:") || startsWith(arg, "--fields:")) {
            string_q mode = arg.Substitute("-f:","").Substitute("--fields:","");

            if (mode == "a" || mode == "all") {
                SHOW_ALL_FIELDS(CBlock);
                SHOW_ALL_FIELDS(CTransaction);
                SHOW_ALL_FIELDS(CReceipt);

            } else if (mode == "m" || mode == "mini") {
                HIDE_ALL_FIELDS(CBlock);
                HIDE_ALL_FIELDS(CTransaction);
                HIDE_ALL_FIELDS(CReceipt);
                UNHIDE_FIELD(CBlock, "blockNumber");
                UNHIDE_FIELD(CBlock, "timestamp");
                UNHIDE_FIELD(CBlock, "transactions");
                UNHIDE_FIELD(CTransaction, "receipt");
                UNHIDE_FIELD(CTransaction, "transactionIndex");
                UNHIDE_FIELD(CTransaction, "gasPrice");
                UNHIDE_FIELD(CTransaction, "gas");
                UNHIDE_FIELD(CTransaction, "isError");
                UNHIDE_FIELD(CTransaction, "from");
                UNHIDE_FIELD(CTransaction, "to");
                UNHIDE_FIELD(CTransaction, "value");
                UNHIDE_FIELD(CReceipt, "gasUsed");

            } else if (mode == "r" || mode == "raw") {
            } else if (mode == "c" || mode == "cache") {
                HIDE_ALL_FIELDS(CBlock);
                HIDE_ALL_FIELDS(CTransaction);
                HIDE_ALL_FIELDS(CReceipt);
                UNHIDE_FIELD(CBlock, "blockNumber");
                UNHIDE_FIELD(CBlock, "timestamp");
                UNHIDE_FIELD(CBlock, "transactions");
                UNHIDE_FIELD(CTransaction, "receipt");
                UNHIDE_FIELD(CTransaction, "transactionIndex");
                UNHIDE_FIELD(CTransaction, "gasPrice");
                UNHIDE_FIELD(CTransaction, "gas");
                UNHIDE_FIELD(CTransaction, "isError");
                UNHIDE_FIELD(CTransaction, "from");
                UNHIDE_FIELD(CTransaction, "to");
                UNHIDE_FIELD(CTransaction, "value");
                UNHIDE_FIELD(CReceipt, "gasUsed");
            }

        } else if (startsWith(arg, '-')) {  // do not collapse

            if (!builtInCmd(arg)) {
                return usage("Invalid option: " + arg);
            }

        } else {

            string_q ret = blocks.parseBlockList(arg, latestBlock);
            if (endsWith(ret, "\n")) {
                cerr << "\n  " << ret << "\n";
                return false;
            } else if (!ret.empty()) {
                return usage(ret);
            }
        }
    }

    if (expContext().isParity) {
        HIDE_FIELD(CTransaction, "cumulativeGasUsed");
        HIDE_FIELD(CTransaction, "gasUsed");
        HIDE_FIELD(CTransaction, "timestamp");
        CRuntimeClass *pClass = GETRUNTIME_CLASS(CBlock);
        if (pClass) {
            CFieldData *pField = pClass->FindField("blockNumber");
            if (pField)
                pField->setName("number");
            pClass->sortFieldList();
        }
    }

    if (hashes && !isRaw)
        return usage("The --hashes option works only with --raw. Quitting...");

    if (!blocks.hasBlocks() && !isLatest)
        return usage("You must specify at least one block.");

    format = getGlobalConfig()->getDisplayStr(false, "");
    if (contains(format, "{PRICE:CLOSE}")) {
//        priceBlocks = true;
    }

    return true;
}

//---------------------------------------------------------------------------------------------------
void COptions::Init(void) {
    paramsPtr  = params;
    nParamsRef = nParams;
    pOptions = this;

    isCheck     = false;
    isRaw       = false;
    hashes      = false;
    traces      = false;
    force       = false;
    normalize   = false;
    isCache     = false;
    quiet       = 0; // quiet has levels
    format      = "";
    priceBlocks = false;
    blocks.Init();
}

//---------------------------------------------------------------------------------------------------
COptions::COptions(void) {
    // Mimics python -m json.tool indenting.
    expContext().spcs = 4;
    expContext().hexNums = false;
    expContext().quoteNums = false;

    // will sort the fields in these classes if --parity is given
    sorts[0] = GETRUNTIME_CLASS(CBlock);
    sorts[1] = GETRUNTIME_CLASS(CTransaction);
    sorts[2] = GETRUNTIME_CLASS(CReceipt);

    Init();
}

//--------------------------------------------------------------------------------
COptions::~COptions(void) {
}

//--------------------------------------------------------------------------------
bool COptions::isMulti(void) const {
    return ((blocks.stop - blocks.start) > 1 || blocks.hashList.getCount() > 1 || blocks.numList.getCount() > 1);
}

//--------------------------------------------------------------------------------
string_q COptions::postProcess(const string_q& which, const string_q& str) const {

    if (which == "options") {
        return str.Substitute("block_list", "<block> [block...]");

    } else if (which == "notes" && (verbose || COptions::isReadme)) {

        string_q ret;
        ret += "[{block_list}] is a space-separated list of values, a start-end range, a [{special}], or any combination.\n";
        ret += "This tool retrieves information from the local node or the ${FALLBACK} node, if configured (see documentation).\n";
        ret += "[{special}] blocks are detailed under " + cTeal + "[{whenBlock --list}]" + cOff + ".\n";
        return ret;
    }
    return str;
}

