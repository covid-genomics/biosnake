#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "src/src/commons/Application.h"
#include "src/src/output.h"

namespace py = pybind11;

mmseqs_output call_mmseqs_proxy(mmseqs_call_args args) {
    pybind11::gil_scoped_release release;
    return call_mmseqs(args);
}

PYBIND11_PLUGIN(mmseqs_native) {
  py::module m("mmseqs_native", R"doc(
        Python module
        -----------------------
        .. currentmodule:: unafold_python
        .. autosummary::
           :toctree: _generate
           
           add
           subtract
    )doc");

    pybind11::class_<mmseqs_call_args>(m, "MMSeqsCallArgs")
    .def(pybind11::init<>())
    .def_readwrite("cli_args", &mmseqs_call_args::cli_args);

    pybind11::class_<mmseqs_output>(m, "MMSeqsCallOutput")
    .def(pybind11::init<>())
    .def_readwrite("vars_str", &mmseqs_output::vars_str);

  m.def("_call_mmseqs", &call_mmseqs_proxy, R"doc(
        Run mmseqs2
    )doc");

  return m.ptr();
}