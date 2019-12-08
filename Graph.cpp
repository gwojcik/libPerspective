/*
    This file is part of libPerspective.
    Copyright (C) 2019  Grzegorz Wójcik

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include <atomic>
#include <string_view>
#include <iostream>
#include <stack>

#include "Graph.h"

#include "python2.7/Python.h"

namespace {
    std::string roleToString(VPRole role) {
        std::string result;
        switch(role) {
            case VPRole::NORMAL:
                result = "NORMAL";
                break;
            case VPRole::SPACE_KEY:
                result = "SPACE";
                break;
        }
        return result;
    }
    
    std::string nodeRelationToString(NodeRelation relation) {
        std::string result;
        switch (relation) {
            case NodeRelation::CHILD:
                result = "CHILD";
                break;
            case NodeRelation::PARENT:
                result = "PARENT";
                break;
            case NodeRelation::VIEW:
                result = "VIEW";
                break;
            case NodeRelation::COMPUTE:
                result = "COMPUTE";
                break;
            case NodeRelation::COMPUTE_SRC:
                result = "COMPUTE_SRC";
                break;
            case NodeRelation::ANCESTOR_TYPE: [[fallthrough]];
            case NodeRelation::COMPUTE_TYPE:
                result = "BAD_NODE_RELATION_TYPE";
                // TODO exception
        }
        return result;
    }

    class QuaternionAsVector3 : public Quaternion {
    public:
        explicit QuaternionAsVector3(const Quaternion & q) : Quaternion(q){};
    };

    struct pyDictWriter {
    private:
        PyObject * dict = nullptr;
    public:
        explicit pyDictWriter() {
            this->dict = PyDict_New();
        }
        void operator()(std::string_view name,  int i) {
            add(name, PyInt_FromLong(i));
        }
        void operator()(std::string_view name,  long i) {
            add(name, PyInt_FromLong(i));
        }
        void operator()(std::string_view name, const std::string & s) {
            add(name, PyString_FromStringAndSize(s.data(), s.size()));
        }
        void operator()(std::string_view name, const char * s) {
            add(name, PyString_FromString(s));
        }
        void operator()(std::string_view name, bool v) {
            add(name, PyBool_FromLong(v));
        }
        void operator()(std::string_view name, PyObject * py) {
            add(name, py);
        }
        void operator()(std::string_view name, QuaternionAsVector3 q) {
            auto tuple = PyTuple_New(3);
            PyTuple_SetItem(tuple, 0, PyFloat_FromDouble(q.x));
            PyTuple_SetItem(tuple, 1, PyFloat_FromDouble(q.y));
            PyTuple_SetItem(tuple, 2, PyFloat_FromDouble(q.z));
            add(name, tuple);
        }
        // TODO DRY
        void operator()(std::string_view name, Quaternion q) {
            auto tuple = PyTuple_New(4);
            PyTuple_SetItem(tuple, 0, PyFloat_FromDouble(q.x));
            PyTuple_SetItem(tuple, 1, PyFloat_FromDouble(q.y));
            PyTuple_SetItem(tuple, 2, PyFloat_FromDouble(q.z));
            PyTuple_SetItem(tuple, 3, PyFloat_FromDouble(q.w));
            add(name, tuple);
        }
        void operator()(std::string_view name, Complex c) {
            auto tuple = PyTuple_New(2);
            PyTuple_SetItem(tuple, 0, PyFloat_FromDouble(c.real()));
            PyTuple_SetItem(tuple, 1, PyFloat_FromDouble(c.imag()));
            add(name, tuple);
        }
        PyObject * result() {
            return dict;
        }
        private:
        void add(std::string_view key, PyObject * value) {
            auto pyKey = PyString_FromStringAndSize(key.data(), key.size());
            PyDict_SetItem(dict, pyKey, value);
        }
    };

    struct pyListWriter {
    private:
        PyObject * list;
    public:
        pyListWriter() {
            list = PyList_New(0);
        }
        void operator()(PyObject * item) {
            PyList_Append(list, item);
        }
        void operator()(long value) {
            PyList_Append(list, PyLong_FromDouble(value));
        }
        void operator()(int value) {
            PyList_Append(list, PyLong_FromDouble(value));
        }
        void operator()(precission value) {
            PyList_Append(list, PyFloat_FromDouble(value));
        }
        PyObject * result() {
            return list;
        }
    };

    struct pyDictReader;
    
    struct pyListReader {
    private:
        PyObject * list;
    public:
        explicit pyListReader(PyObject * list) {
            if (PyList_Check(list)) {
                this->list = list;
            } else {
                std::cout << "bad list";
            }
        }
        template<class T, class F> void forAll(F fct) {
            int size = getSize();
            for (int i = 0; i < size; i++) {
                auto value = get<T>(i);
                fct(value);
            }
        }
        template<class T> T get (int id);
        pyDictReader getDict(int id);
        int getSize() {
            return PyList_Size(list);
        }
    private:
        PyObject * getItem(int id) {
            PyObject * item = PyList_GetItem(list, id);
            if (item == nullptr) {
                std::cout << "no list item with id = '" << id << "'\nprocess will die\n";
                //TODO exception
            }
            return item;
        }
    };
    template<> precission pyListReader::get(int id){
        return PyFloat_AsDouble(getItem(id));
    }
    template<> std::string pyListReader::get(int id){
        PyObject * item = getItem(id);
        if (!PyString_Check(item)) {
            std::cout << "bad type\n";
        }
        return std::string(PyString_AsString(item));
    }

    struct pyDictReader {
    private:
        PyObject * dict;
    public:
        explicit pyDictReader(PyObject * dict) {
            if (PyDict_Check(dict)) {
                this->dict = dict;
            } else {
                std::cout << "bad dict\n";
            }
        }
        bool has(const std::string & key) {
            auto pyKey = PyString_FromStringAndSize(key.data(), key.size());
            PyObject * item = PyDict_GetItem(dict, pyKey);
            return item != nullptr;
        }
        template<class T> T get (const std::string & key);
        pyListReader getList(const std::string & key) {
            PyObject * item = getItem(key);
            if (!PyList_Check(item)) {
                std::cout << "bad type\n";
            }
            return pyListReader(item);
        }
    private:
        PyObject * getItem(const std::string & key) {
            auto pyKey = PyString_FromStringAndSize(key.data(), key.size());
            PyObject * item = PyDict_GetItem(dict, pyKey);
            if (item == nullptr) {
                std::cout << "no key '" << key << "'\nprocess will die\n";
                //TODO exception
            }
            return item;
        }
    };
    template<> int pyDictReader::get(const std::string& key){
        PyObject * item = getItem(key);
        if (!PyIndex_Check(item)) {
            std::cout << "bad type\n";
        }
        return PyInt_AsLong(item);
    }
    template<> precission pyDictReader::get(const std::string& key){
        return PyFloat_AsDouble(getItem(key));
    }
    template<> bool pyDictReader::get(const std::string& key){
        return PyObject_IsTrue(getItem(key));
    }
    template<> std::string pyDictReader::get(const std::string& key){
        PyObject * item = getItem(key);
        if (!PyString_Check(item)) {
            std::cout << "bad type\n";
        }
        return std::string(PyString_AsString(item));
    }
    template<> Complex pyDictReader::get(const std::string& key){
        PyObject * item = getItem(key);
        if (!PyTuple_Check(item)) {
            std::cout << "bad type\n";
        }
        return Complex (
            PyFloat_AsDouble(PyTuple_GetItem(item, 0)),
            PyFloat_AsDouble(PyTuple_GetItem(item, 1))
        );
    }
    template<> Quaternion pyDictReader::get(const std::string& key){
        PyObject * item = getItem(key);
        if (!PyTuple_Check(item)) {
            std::cout << "bad type\n";
        }
        auto size = PyTuple_Size(item);
        if (size == 3) {
            return Quaternion (
                PyFloat_AsDouble(PyTuple_GetItem(item, 0)),
                PyFloat_AsDouble(PyTuple_GetItem(item, 1)),
                PyFloat_AsDouble(PyTuple_GetItem(item, 2))
            );
        } else if (size == 4) {
            return Quaternion (
                PyFloat_AsDouble(PyTuple_GetItem(item, 0)),
                PyFloat_AsDouble(PyTuple_GetItem(item, 1)),
                PyFloat_AsDouble(PyTuple_GetItem(item, 2)),
                PyFloat_AsDouble(PyTuple_GetItem(item, 3))
            );
        } else {
            std::cout << "bad type\n";
            return Quaternion();
        }
    }
    
    pyDictReader pyListReader::getDict(int id){
        return pyDictReader(getItem(id));
    }

    template<class F> void forEachDict( pyListReader & dictReader, F fct) {
        int size = dictReader.getSize();
        for (int i =0; i < size; i++) {
            fct(dictReader.getDict(i));
        }
    }
    
    struct TraverseItem {
        NodeWrapper* parent;
        NodeWrapper* node;
    };

    template<typename F> void forEachNode(NodeWrapper * firstNode, F fct) {
        std::stack<TraverseItem> nodes;
        nodes.push(TraverseItem{
            .parent = nullptr,
            .node = firstNode,
        });
        while (!nodes.empty()) {
            TraverseItem item = nodes.top();
            nodes.pop();
            fct(item.node, item.parent);
            for (auto && child : item.node->get_children()) {
                nodes.push(TraverseItem{
                    .parent = item.node,
                    .node = child,
                });
            }
        }
    }

    CurvilinearPerspective curvilinear_from_python(pyDictReader & node){
        pyDictReader reader = pyDictReader(node);
        Complex left = reader.get<Complex>("left");
        Complex right = reader.get<Complex>("right");
        return CurvilinearPerspective(left, right);
    }

    RectilinearProjection rectilinear_from_python(pyDictReader & node) {
        pyDictReader reader = pyDictReader(node);
        Complex left = reader.get<Complex>("left");
        Complex right = reader.get<Complex>("right");
        return RectilinearProjection(left, right);
    }

    VanishingPoint vp_from_python(pyDictReader & node){
        pyDictReader reader = pyDictReader(node);
        if (reader.has("direction")) {
            Quaternion direction = reader.get<Quaternion>("direction");
            if (reader.has("direction_local")) {
                Quaternion directionLocal = reader.get<Quaternion>("direction_local");
                return VanishingPoint(direction, directionLocal);
            } else {
                return VanishingPoint(direction);
            }
        } else if (reader.has("position")) {
            Complex position = reader.get<Complex>("position");
            return VanishingPoint(position);
        } else {
            std::cout << "bad structure\n";
            std::exit(1); // TODO exception
        }
    }

    PerspectiveSpace space_from_python(pyDictReader & node){
        pyDictReader reader = pyDictReader(node);
        if (!reader.has("rotation") && !reader.has("up")) {
            std::cout << "missing rotation for perspective space\n";
            std::exit(1);
        } else if (reader.has("up") && !reader.has("rotation_angle")) {
            // FIXME rotation_angle is unused
            std::cout << "missing roation for up vector for perspective space\n";
            std::exit(1);
        } else if (reader.has("rotation") && !reader.has("rotation_local")) {
            std::cout << "missing local rotation for perspective space\n";
            std::exit(1);
        }
        
        if (reader.has("up")) {
            Quaternion up = reader.get<Quaternion>("up");
            Quaternion defaultUp = Quaternion(0, 1, 0);
            return PerspectiveSpace(rotationBetwenVectors(up, defaultUp));
        } else {
            Quaternion rotation = reader.get<Quaternion>("rotation");
            Quaternion rotationLocal = reader.get<Quaternion>("rotation_local");
            return PerspectiveSpace(rotation, rotationLocal);
        }
    }

    PerspectiveGroup group_from_python() {
        PerspectiveGroup group;
        return group;
    }

    Plane plane_from_python() {
        Plane plane;
        return plane;
    }

    std::shared_ptr<NodeWrapper> node_from_python(pyDictReader & node)    {
        pyDictReader reader = pyDictReader(node);
        std::string type = reader.get<std::string>("type");
        std::string name = reader.get<std::string>("name");
        if (type == "VP") {
            auto element = vp_from_python(node);
            return std::make_shared<NodeWrapper>(element, name);
        } else if ( type == "RectilinearProjection") {
            auto element = rectilinear_from_python(node);
            return std::make_shared<NodeWrapper>(element, name);
        } else if ( type == "CurvilinearPerspective") {
            auto element = curvilinear_from_python(node);
            return std::make_shared<NodeWrapper>(element, name);
        } else if ( type == "Group") {
            auto element = group_from_python();
            return std::make_shared<NodeWrapper>(element, name);
        } else if ( type == "Plane") {
            auto element = plane_from_python();
            return std::make_shared<NodeWrapper>(element, name);
        } else if ( type == "Space") {
            auto element = space_from_python(node);
            return std::make_shared<NodeWrapper>(element, name);
        } else {
            std::cout << "unknown perspective element type\n";
            std::exit(1);
        }
    }
}

int NodeWrapper::getNextUID(){
    static std::atomic<int> uid { 0 };
    return ++uid;
}

void NodeWrapper::compute(GraphBase* graph) {
    std::cout << "c++ compute\n";
    std::vector<NodeWrapper*> src;
    for (auto && relation : _relations) {
        if (relation.relation == NodeRelation::COMPUTE_SRC) {
            src.push_back(relation.node);
        }
    }
    if (src.size() == 0) {
        return;
    }
    if (_compute == nullptr) {
        return;
    }
    (this->*_compute)(src, _compute_additional_params);
    if (is_view() || is_space()) {
        std::vector<NodeWrapper*> computeNodes = graph->update_groups(this);
        for (auto && computeNode : computeNodes) {
            computeNode->compute(graph);
        }
    } else {
        auto view = get_view();
        if (view) {
            view->update_child(this);
        }
    }
}

std::vector<NodeWrapper *> GraphBase::get_points(NodeWrapper* nodeToDraw) {
    std::cout << "c++ get_points\n";
    std::cout << "c++ get_points node.uid = " << nodeToDraw->uid << "\n";
    std::stack<NodeWrapper*> nodes;
    nodes.push(nodeToDraw);
    std::vector<NodeWrapper*> result;
    while (!nodes.empty()) {
        std::cout << "c++ get_points - while\n";
        NodeWrapper * node = nodes.top();
        nodes.pop();
        if (node->is_point()) {
            result.push_back(node);
        }
        for (auto && child : node->get_children()) {
            nodes.push(child);
        }
    }
    return result;
}

std::vector<NodeWrapper *> GraphBase::get_all_enabled_points(bool skipLocked){
    std::stack<NodeWrapper*> nodes;
    nodes.push(get_root());
    std::vector<NodeWrapper*> result;
    while (!nodes.empty()) {
        NodeWrapper * node = nodes.top();
        nodes.pop();
        if (! node->enabled) {
            continue;
        }
        if (node->locked && skipLocked) {
            continue;
        }
        if (node->is_point()) {
            result.push_back(node);
        }
        for (auto && child : node->get_children()) {
            nodes.push(child);
        }
    }
    return result;
}

std::vector<NodeWrapper *> GraphBase::get_all_nodes(NodeWrapper* parent){
    std::vector<NodeWrapper*> result;
    forEachNode(parent, [&result](NodeWrapper * node, NodeWrapper * parent){
        (void) parent;
        result.push_back(node);
    });
    return result;
}


std::vector<NodeWrapper *> GraphBase::get_logic_children(NodeWrapper* node){
    std::stack<NodeWrapper*> groups;
    groups.push(node);
    std::vector<NodeWrapper*> result;
    while (!groups.empty()) {
        NodeWrapper* group = groups.top();
        groups.pop();
        for (auto && child : group->get_children()) {
            // TODO add comments
            if (child->is_UI_only()) {
                groups.push(child);
            } else {
                result.push_back(child);
            }
        }
    }
    return result;
}

std::vector<NodeWrapper *> GraphBase::update_groups(NodeWrapper* group) {
    std::vector<NodeWrapper*> computeNodes;
    NodeWrapper * view = nullptr;
    NodeWrapper * space = nullptr;
    if (group->is_space()) {
        space = group;
        view = group->get_view();
    } else {
        view = group;
    }
    for (auto && child : get_logic_children(group)) {
        for (auto && toCompute : child->get_compute_children()) {
            computeNodes.push_back(toCompute);
        }
        if (child->is_space()) {
            if (space) {
                space->update_subspace(child);
            }
            auto tmpCN = update_groups(child);
            computeNodes.insert(computeNodes.end(), tmpCN.begin(), tmpCN.end());
        } else if (child->is_view()) {
            // pass
        } else {
            if (child->is_compute()) {
                continue;
            }
            if (child->is_point() && space) {
                space->update_child_dir(child);
            }
            view->update_child(child);
        }
    }
    return computeNodes;
}

void GraphBase::update(NodeWrapper* node, Complex pos) {
    std::stack<NodeWrapper*> computeNodes;
    NodeWrapper * space = find_parent_space(node);
    NodeWrapper * view = node->get_view();
    if (node->is_key() && space != nullptr) {
        Quaternion newDir = view->as_projection()->calc_direction(pos);
        space->update_space(node->as_vanishingPoint(), newDir);
        std::vector<NodeWrapper*> tmpCN = update_groups(space);
        for (auto && tmp : tmpCN) {
            computeNodes.push(tmp);
        }
    } else {
        if (space != nullptr) {
            view->update_child(node, pos);
            space->as_space().move_child_to_space(node->as_vanishingPoint());
        } else {
            view->update_child(node,pos);
        }
        for (auto && toCompute : node->get_compute_children()) {
            computeNodes.push(toCompute);
        }
    }
    
    while (!computeNodes.empty()) {
        NodeWrapper * computeNode = computeNodes.top();
        computeNodes.pop();
        computeNode->compute(this);
        for (auto && toCompute : computeNode->get_compute_children()) {
            computeNodes.push(toCompute);
        }
    }
}

GraphBase::NewElementData GraphBase::get_group_for_new_element(){
    NodeWrapper * chosen = chosen_point;
    NodeWrapper * group = main_view;
    if (chosen != nullptr) {
        if (chosen->is_grouping()) {
            group = chosen;
        } else {
            group = chosen->get_parent();
        }
    }
    NodeWrapper * space = nullptr;
    NodeWrapper * view;
    if (group->is_view()) {
        view = group;
    } else if (group->is_space()) {
        view = group->get_view();
        space = group;
    } else {
        view = group->get_view();
        space = find_parent_space(group);
    }
    return NewElementData{
        .group = group,
        .view = view,
        .space = space,
    };
}

/** connect sub graph with root in local_rot as child of currently selected element */
NodeWrapper * GraphBase::connect_sub_graph(NodeWrapper* localRoot){
    auto data = get_group_for_new_element();
    
    if (localRoot->is_point()) {
        data.view->update_child(localRoot, localRoot->get_position());
        if (data.space != nullptr) {
            data.space->as_space().move_child_to_space(localRoot->as_vanishingPoint());
        }
    }
    
    if (localRoot->is_space()) {
        if (data.space != nullptr) {
            data.space->as_space().move_subspace_to_space(localRoot->as_space());
        }
    }
    
    data.group->add_child(localRoot);
    localRoot->add_parent(data.group);
    
    std::stack<NodeWrapper*> nodes;
    nodes.push(localRoot);
    while (!nodes.empty()) {
        NodeWrapper * node = nodes.top();
        nodes.pop();
        if (node->get_view() == nullptr) {
            node->add_view(data.view);
            if (node->is_vanishing_point()) {
                data.view->update_child(node);
            }
        } else if (node->is_vanishing_point()) {
            node->get_view()->update_child(node);
        }
        for (auto && child : node->get_children()) {
            nodes.push(child);
        }
    }
    return localRoot;
}


PyObject * to_object(GraphBase* graph) {
    std::cout << "to_object_edges\n";
    pyListWriter nodes;
    pyListWriter edges;
    std::map<int, std::string> tagMap;
    for (auto && tag : graph->tags) {
        tagMap[tag.second->uid] = tag.first;
    }
    forEachNode(graph->get_root(), [&edges, &nodes, &tagMap](NodeWrapper * node, NodeWrapper * parent){
        if (parent) {
            pyDictWriter childEdge;
            childEdge("src", parent->uid);
            childEdge("dst", node->uid);
            childEdge("type", nodeRelationToString(NodeRelation::CHILD));
            edges(childEdge.result());
        }
        for (auto && relation : node->_relations) {
            pyDictWriter relationEdge;
            relationEdge("src", node->uid);
            relationEdge("dst", relation.node->uid);
            relationEdge("type", nodeRelationToString(relation.relation));
            edges(relationEdge.result());
        }
        pyDictWriter writer;
        writer("id", node->uid);
        writer("name", node->name);
        writer("locked", node->locked);
        writer("enabled", node->enabled);
        writer("parent_enabled", node->parent_enabled);
        writer("parent_locked", node->parent_locked);
        writer("is_compute", node->is_compute());
        writer("color", node->color);
        if (node->is_compute()) {
            pyListWriter params;
            for (auto && p : node->_compute_additional_params) {
                params(p);
            }
            writer("compute_params", params.result());
            writer("compute_fct", node->compute_function_name);
        }
        
        if (tagMap.count(node->uid)) {
            writer("tag", tagMap[node->uid]);
        }
        
        if (node->_is_group) {
            writer("type", "Group");
        } else if (node->is_point()) {
            writer("type", "VP");
            writer("role", roleToString(node->role));
            writer("direction", QuaternionAsVector3(node->as_vanishingPoint().get_direction()));
            writer("direction_local", QuaternionAsVector3(node->as_vanishingPoint().get_direction_local()));
        } else if (node->is_projection()) {
            if (node->_is_curvilinear) {
                writer("type", "CurvilinearPerspective");
            } else {
                writer("type", "RectilinearProjection");
            }
            writer("right", node->as_projection()->calc_pos_from_dir(Quaternion(1, 0, 1, 0)));
            writer("left", node->as_projection()->calc_pos_from_dir(Quaternion(-1, 0, 1, 0)));
        } else if (node->is_space()) {
            writer("type", "Space");
            writer("is_UI", 1);
            writer("rotation", node->as_space().get_rotation());
            writer("rotation_local", node->as_space().get_rotation_local());
        } else if (node->_is_plane) {
            writer("type", "Plane");
        } else {
            // TODO exception
        }
        nodes(writer.result());
    });

    pyListWriter visualizations;
    for (auto && visualization : graph->visualizations) {
        pyDictWriter visualizationData;
        visualizationData("type", visualization.type);
        pyListWriter visNodes;
        for (auto && node : visualization.nodes) {
            visNodes(node);
        }
        visualizationData("nodes", visNodes.result());
        visualizations(visualizationData.result());
    }
    
    pyDictWriter result;
    result("nodes", nodes.result());
    result("edges", edges.result());
    result("root", graph->get_root()->uid);
    result("visualizations", visualizations.result());
    result("version", "0.3.0");
    return result.result();
}

std::shared_ptr<NodeWrapper> create_from_structure__part_10(pyDictReader & nodeData) {
    std::shared_ptr<NodeWrapper> node = node_from_python(nodeData);

    pyDictReader reader = pyDictReader(nodeData);
    std::string type = reader.get<std::string>("type");
    
    if (reader.has("is_UI")) {
        node->set_UI(reader.get<int>("is_UI"));
    }
    
    if (reader.has("enabled")) {
        node->enabled = reader.get<bool>("enabled");
    } else {
        node->enabled = true;
    }
    
    if (reader.has("parent_enabled")) {
        node->parent_enabled = reader.get<bool>("enabled");
    } else {
        node->parent_enabled = true;
    }

    if (reader.has("locked")) {
        node->locked = reader.get<bool>("locked");
    } else {
        node->locked = false;
    }

    if (reader.has("parent_locked")) {
        node->parent_locked = reader.get<bool>("parent_locked");
    } else {
        node->parent_locked = false;
    }
    
    if (reader.has("is_compute")) {
        bool is_compute = reader.get<bool>("is_compute");
        node->set_compute(is_compute);
        if (is_compute) {
            std::string fctName = reader.get<std::string>("compute_fct");
            node->set_compute_fct_by_name(fctName);
            if (reader.has("compute_params")) {
                auto computeParams = reader.getList("compute_params");
                if (computeParams.getSize()) {
                    node->set_compute_additional_params(computeParams.get<precission>(0));
                }
            }
        }
    } else {
        node->set_compute(false);
    }
    
    if (type == "VP") {
        if (reader.has("role")) {
            std::string role = reader.get<std::string>("role");
            if (role == "NORMAL") {
                node->set_role(VPRole::NORMAL);
            } else if (role == "SPACE") {
                node->set_role(VPRole::SPACE_KEY);
            } else {
                std::cout << "bad VP role\n";
                std::exit(1); // TODO exception
            }
        }
        
        if (reader.has("color")) {
            node->color = reader.get<int>("color");
        }
    }

    return node; 
}

void create_from_structure__part_11(NodeWrapper* dstElement, NodeWrapper* srcElement, pyDictReader & reader){
    std::string type = "CHILD";
    if (reader.has("type")) {
        type = reader.get<std::string>("type");
    }
    if (type == "CHILD") {
        srcElement->add_child(dstElement);
        dstElement->add_parent(srcElement);
    } else if ( type == "PARENT") {
        // pass added in CHILD
    } else if ( type == "VIEW") {
        if (!dstElement->is_view()) {
            std::cout << "incorrect view relation\n";
            std::exit(1); // TODO exception
        }
        srcElement->add_view(dstElement);
    } else if ( type == "COMPUTE_SRC") {
        srcElement->add_relative(dstElement, NodeRelation::COMPUTE_SRC);
    } else if (type == "COMPUTE") {
        srcElement->add_relative(dstElement, NodeRelation::COMPUTE);
    } else {
        std::cout << "unknown edge element type\n";
        std::exit(1); // TODO exception
    }
}

NodeWrapper * GraphBase::create_from_structure(PyObject* data){
    std::cout << "c++ create_from_structure\n";
    pyDictReader allData(data);

    _is_empty = false;
    if (allData.has("version")) {
        std::string version = allData.get<std::string>("version");
        std::cout << "version: " << version << "\n";
        if (version != "0.3.0") {
            // TODO better version checking
            std::cout << "unsupported version: " << version << "\n";
            std::exit(1); // TODO exception
        }
    }
    if (!(allData.has("nodes") && allData.has("edges") && allData.has("root"))) {
        std::cout << "bad structure\n";
        std::exit(1); // TODO exception
    }
    
    std::string root = allData.get<std::string>("root");

    pyListReader nodes = allData.getList("nodes");
    std::map<std::string, int> idMap;
    forEachDict(nodes, [&idMap,this](pyDictReader nodeData){
        std::string dataId = nodeData.get<std::string>("id");
        std::shared_ptr<NodeWrapper> node = create_from_structure__part_10(nodeData);
        idMap[dataId] = node->uid;

        this->nodes.push_back(node);
        this->nodeMap[node->uid] = node.get();
        if (nodeData.has("tag")) {
            std::string tag = nodeData.get<std::string>("tag");
            this->tags[tag] = node.get();
        }
        
        std::string type = nodeData.get<std::string>("type");
        if ((type == "RectilinearProjection" || type == "CurvilinearPerspective") && this->main_view == nullptr) {
            this->main_view = node.get();
        }
    });

    pyListReader edges = allData.getList("edges");
    forEachDict(edges, [&idMap, this](pyDictReader edgeData) {
        if (!edgeData.has("src") || !edgeData.has("dst")) {
            std::cout << "edge format is incorrect\n";
            std::exit(1);
        }
        std::string src = edgeData.get<std::string>("src");
        std::string dst = edgeData.get<std::string>("dst");
        create_from_structure__part_11(
            get_by_uid(idMap[dst]), get_by_uid(idMap[src]), edgeData
        );
    });
    
    if (allData.has("visualizations")) {
        pyListReader visualizations = allData.getList("visualizations");
        forEachDict(visualizations, [&idMap, this](pyDictReader visualizationData) {
            VisualizationData visualization;
            visualization.type = visualizationData.get<std::string>("type");
            pyListReader nodes = visualizationData.getList("nodes");
            nodes.forAll<std::string>([&idMap, &visualization](std::string nodeIdentifier){
                visualization.nodes.push_back(idMap[nodeIdentifier]);
            });
            this->visualizations.push_back(visualization);
        });
    }
    
    return get_by_uid(idMap[root]);
}
