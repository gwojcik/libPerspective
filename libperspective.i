%module libperspective

%{
#include "Quaternion.h"
#include "Point.h"
#include "Projection.h"
#include "Helpers.h"
#include "Space.h"
#include "Graph.h"
%}

%begin %{
#define SWIG_PYTHON_2_UNICODE
%}

%immutable NodeWrapper::uid;

%include "std_complex.i"
%include "std_vector.i"
%include "std_string.i"

%include "Quaternion.h"
%include "Point.h"
%include "Projection.h"
%include "Helpers.h"
%include "Space.h"
%include "Graph.h"

%template(ComplexVector) std::vector<Complex>;
%template(QuaternionVector) std::vector<Quaternion>;
%template(NodeWrapperVector) std::vector<NodeWrapper*>;
%template(VisualizationDataVector) std::vector<VisualizationData>;
%template(IntVector) std::vector<int>;