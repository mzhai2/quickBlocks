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

#include <Python.h>
#include "etherlib.h"

static PyObject* init(PyObject* self, PyObject *args) {
    const char *baseURL;
    PyArg_ParseTuple(args, "s", &baseURL);
    getCurlContext()->baseURL = baseURL;
    etherlib_init();
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject* cleanup(PyObject* self, PyObject *args) {
    etherlib_cleanup();
    Py_INCREF(Py_None);
    return Py_None;
}

// my use case is not live so assuming all finalized
static PyObject* get_block(PyObject* self, PyObject *args) {
    int i;
    int cache;
    if (!PyArg_ParseTuple(args, "ip", &i, &cache)) {
        return NULL;
    }
    CBlock block;
    getBlock(block, i);
    block.finalized = true;
    if (cache) {
        writeBlockToBinary(block, getBinaryFilename(i));
    }
    return Py_BuildValue("s", block.Format().c_str());
}

static PyObject* check_block(PyObject* self, PyObject *args) {
    int i;
    int fix;
    if (!PyArg_ParseTuple(args, "ip", &i, &fix)) {
        return NULL;
    }
    CBlock block;
    getCurlContext()->provider = "local";
    queryBlock(block, asStringU(i), true, false);
    block.finalized = true;

    CBlock saved_block;
    getCurlContext()->provider = "binary";
    queryBlock(saved_block, asStringU(i), true, false);
    if (block != saved_block) {
        cout << i << "\nsaved";
        cout << saved_block << "\nnew";
        cout << block << "\n";
	if (fix) {
           writeBlockToBinary(block, getBinaryFilename(i));
        }
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef _quickblocks_methods[] = {
    {"get_block", get_block, METH_VARARGS, "get_block"},
    {"check_block", check_block, METH_VARARGS, "check_block"},
    {"init", init, METH_VARARGS, "init"},
    {"cleanup", cleanup, METH_VARARGS, "cleanup"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef cModPyDem =
{
    PyModuleDef_HEAD_INIT,
    "_quickblocks",
    "",
    -1,
    _quickblocks_methods
};

PyMODINIT_FUNC PyInit__quickblocks(void) {
    return PyModule_Create(&cModPyDem);
}
