#include <catch2/catch.hpp>
#include "../PythonGraph.h"
#include "python_graph_data.h"
#include <Python.h>

namespace {
    PyObject * prepare_py_globals() {
        PyObject * globals = PyDict_New();
#if PYTHON_ABI_VERSION != 3
        PyDict_SetItemString(globals, "__builtins__", PyEval_GetBuiltins());
#endif
        return globals;
    }
}

TEST_CASE ( "Graph python" ) {
    using namespace Catch::literals;
    Py_Initialize();
    SECTION ( "python to raw" ) {
        PyObject * globals = prepare_py_globals();
        PyObject * locals = PyDict_New();
        SECTION("point") {
            PyObject * dataPy = PyRun_String(test_point_py, Py_eval_input, globals, locals);
            RawGraph data = python_to_raw_data(dataPy);
            Py_DecRef(dataPy);
            REQUIRE(data.edges.size() == 0);
            REQUIRE(data.root == "1");
            REQUIRE(data.visualizations != nullptr);
            REQUIRE(data.visualizations->size() == 1);
            REQUIRE(data.nodes.size() == 1);
            REQUIRE(data.nodes[0].name == nullptr);
        }
        SECTION("space") {
            PyObject * dataPy = PyRun_String(test_space_py, Py_eval_input, globals, locals);
            RawGraph data = python_to_raw_data(dataPy);
            Py_DecRef(dataPy);
            REQUIRE(data.edges.size() == 4);
            REQUIRE(data.root == "space");
            REQUIRE(data.visualizations != nullptr);
            REQUIRE(data.visualizations->size() == 4);
            REQUIRE(data.nodes.size() == 5);
            REQUIRE(data.nodes[0].name == nullptr);
            REQUIRE(data.nodes[1].name != nullptr);
            REQUIRE(*data.nodes[1].name == "Forward");
        }
        SECTION("group") {
            PyObject * dataPy = PyRun_String(test_group_py, Py_eval_input, globals, locals);
            RawGraph data = python_to_raw_data(dataPy);
            Py_DecRef(dataPy);
            REQUIRE(data.edges.size() == 0);
            REQUIRE(data.root == "group");
            REQUIRE(data.visualizations == nullptr);
            REQUIRE(data.nodes.size() == 1);
            REQUIRE(data.nodes[0].name != nullptr);
            REQUIRE(*data.nodes[0].name == "group");
        }
        SECTION("horizon") {
            PyObject * dataPy = PyRun_String(test_horizon_py, Py_eval_input, globals, locals);
            RawGraph data = python_to_raw_data(dataPy);
            Py_DecRef(dataPy);
            REQUIRE(data.edges.size() == 0);
            REQUIRE(data.root == "horizon");
            REQUIRE(data.visualizations == nullptr);
            REQUIRE(data.nodes.size() == 1);
            REQUIRE(data.nodes[0].name != nullptr);
            REQUIRE(*data.nodes[0].name == "horizon");
        }
        SECTION("projection") {
            PyObject * dataPy = PyRun_String(test_projection_py, Py_eval_input, globals, locals);
            RawGraph data = python_to_raw_data(dataPy);
            Py_DecRef(dataPy);
            REQUIRE(data.edges.size() == 11);
            REQUIRE(data.root == "Root");
            REQUIRE(data.visualizations != nullptr);
            REQUIRE(data.visualizations->size() == 4);
            REQUIRE(data.nodes.size() == 7);
            REQUIRE(data.nodes[0].name != nullptr);
            REQUIRE(*data.nodes[0].name == "Root");
        }
        Py_DecRef(locals);
        Py_DecRef(globals);
    }
    // TODO test all fields in node, edge and visualization
    SECTION ("raw -> python -> raw") {
        SECTION("empty") {
            RawGraph data = {
                .nodes = {},
                .edges = {},
                .root = "test",
                .version = nullptr,
                .visualizations = {},
            };
            PyObject * pyData = raw_data_to_python(data);
            RawGraph data2 = python_to_raw_data(pyData);
            Py_DecRef(pyData);
            REQUIRE(data2.nodes.size() == 0);
            REQUIRE(data2.edges.size() == 0);
            REQUIRE(data.root == data2.root);
            REQUIRE(data2.version.get() == nullptr);
            REQUIRE(data2.visualizations->size() == 0);
        }
    }
    SECTION ("python -> internal -> python -> raw") {
        PyObject * globals = prepare_py_globals();
        PyObject * locals = PyDict_New();
        PyObject * dataPy = PyRun_String(test_projection_py, Py_eval_input, globals, locals);
        RawGraph rawIn = python_to_raw_data(dataPy);
        Py_DecRef(dataPy);
        REQUIRE(rawIn.nodes[1].type == "RectilinearProjection");
        rawIn.nodes[1].left = std::make_unique<Complex>(-100,0);
        rawIn.nodes[1].right = std::make_unique<Complex>(100,0);
        PyObject * dataPyFixed = raw_data_to_python(rawIn);
        PythonGraph graph;
        REQUIRE_NOTHROW(graph.initialize_from_structure(dataPyFixed));
        Py_DecRef(dataPyFixed);
        PyObject * outData = nullptr;
        REQUIRE_NOTHROW(outData = graph.to_object());
        RawGraph rawOut = python_to_raw_data(outData);
        Py_DecRef(outData);
        Py_DecRef(locals);
        Py_DecRef(globals);
    }
}
