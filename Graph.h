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
#pragma once

#include <string>
#include <variant>
#include <map>
#include <memory>
#include "Space.h"
#include "Helpers.h"
#include "Point.h"
#include "Projection.h"

// TODO move to python specific file
struct _object;
typedef _object PyObject;
class NodeWrapper;
class GraphBase;
PyObject * to_object(GraphBase* graph);

/** Enumeration of the roles of vanishing points */
enum class VPRole : char {
    NORMAL = 0x0,
    SPACE_KEY = 0x1,
    KEY = 0x1,
};


/** Enumeration of the relations between nodes in graph */
enum class NodeRelation : char {
    CHILD = 0,
    PARENT = 1 << 0,
    VIEW = 1 << 1,
    COMPUTE = 1 << 2,
    COMPUTE_SRC = 1 << 3,
    ANCESTOR_TYPE = PARENT | VIEW | COMPUTE_SRC,
    COMPUTE_TYPE = COMPUTE | COMPUTE_SRC,
};


inline std::pair<precission,precission> vector_to_angle(const Quaternion & vector) {
    Quaternion normalized = normalize(vector);
    return {
        std::atan2(normalized.x, normalized.z),
        std::asin(normalized.y)
    };
}





class NodeWrapper {
private:
    struct RelationItem {
        NodeWrapper * node;
        NodeRelation relation;
    };
    /** VP position is computed from other nodes */
    std::variant<int,PerspectiveSpace,Plane,VanishingPoint,RectilinearProjection,CurvilinearPerspective,PerspectiveGroup> node = 0;
    std::vector<RelationItem> _relations;
    friend PyObject * to_object(GraphBase *);
    bool isCompute = false;
public:
    bool enabled = true;
    bool locked = false;
    VPRole role = VPRole::NORMAL;
    int uid;    // TODO make it immutable in c++
    std::string name;
    std::vector<NodeWrapper *> _children;
    std::vector<precission> _compute_additional_params;
    long color = 0; // rgba // TODO change to int?
    void (NodeWrapper::* _compute)(std::vector<NodeWrapper*>,std::vector<precission>) = nullptr;
    std::string compute_function_name;
    bool parent_enabled = true;
    bool parent_locked = false;
    bool _is_space = false;
    bool _is_vanishing_point = false;
    bool _is_point = false;
    bool _is_view = false;
    bool _is_projection = false;
    bool _is_group = false;
    bool _is_plane = false;
    bool _is_curvilinear = false;
    bool _is_rectilinear = false;
    bool _is_UI = false;
    bool _is_grouping = false;
    bool _is_UI_only = false;

private:
    static int getNextUID();
public:
    NodeWrapper(const std::string & name) {
        uid = getNextUID();
        this->name = name;
    }
    NodeWrapper(PerspectiveSpace &space, const std::string & name) : NodeWrapper(name) {
        node = space;
        _is_space = true;
        _is_grouping = true;
    }
    NodeWrapper(Plane &plane, const std::string & name) : NodeWrapper(name) {
        node = plane;
        _is_plane = true;
    }
    NodeWrapper(VanishingPoint &vp, const std::string & name) : NodeWrapper(name) {
        node = vp;
        _is_vanishing_point = true;
        _is_point = true;
        _is_UI = true;
    }
    NodeWrapper(RectilinearProjection &projection, const std::string & name) : NodeWrapper(name) {
        node = projection;
        _is_view = true;
        _is_projection = true;
        _is_rectilinear = true;
        _is_grouping = true;
    }
    NodeWrapper(CurvilinearPerspective &projection, const std::string & name) : NodeWrapper(name) {
        node = projection;
        _is_view = true;
        _is_projection = true;
        _is_curvilinear = true;
        _is_grouping = true;
    }
    NodeWrapper(PerspectiveGroup &group, const std::string & name) : NodeWrapper(name) {
        node = group;
        _is_group = true;
        _is_UI = true;
        _is_grouping = true;
        _is_UI_only = true;
    }
    PerspectiveSpace & as_space() {
        return std::get<PerspectiveSpace>(node);
    }
    Plane & as_plane() {
        return std::get<Plane>(node);
    }
    VanishingPoint & as_vanishingPoint() {
        return std::get<VanishingPoint>(node);
    }
    BaseProjection * as_projection() {
        if (auto projection = std::get_if<RectilinearProjection>(&node)) {
            return projection;
        } else {
            return &std::get<CurvilinearPerspective>(node);
        }
    }
    PerspectiveGroup & as_group() {
        return std::get<PerspectiveGroup>(node);
    }
    Complex get_position() {
        return std::get<VanishingPoint>(node).get_position();
    }
    /** Update perspective space using change in its node position */
    void update_space(const VanishingPoint & child_node, const Quaternion & new_dir) {
        as_space().update_space(child_node, new_dir);
    }
    void update_subspace(NodeWrapper * subspace) {
        as_space().update_subspace(subspace->as_space());
    }
    void update_child(NodeWrapper * child_node, const Complex & new_position) {
        as_projection()->update_child(child_node->as_vanishingPoint(), new_position);
    }
    void update_child(NodeWrapper * child_node) {
        if (!child_node->_is_plane) {
            as_projection()->update_child(child_node->as_vanishingPoint());
        }
    }
    void update_child_dir(NodeWrapper * child_node) {
        as_space().update_child_dir(child_node->as_vanishingPoint());
    }
    void add_child(NodeWrapper * child){
        _children.push_back(child);
    }
    std::vector<NodeWrapper*> & get_children() {
        return _children;
    }
    void add_relative(NodeWrapper * node, NodeRelation relation) {
        _relations.push_back(RelationItem{
            .node = node,
            .relation = relation,
        });
    }
    void remove_child(NodeWrapper * child) {
        for (auto it = _children.begin(); it != _children.end();) {
            if (*it == child) {
                _children.erase(it);
                break;
            } else {
                ++it;
            }
        }
    }
    // TODO DRY
    void add_parent(NodeWrapper * parent) {
        _relations.push_back(RelationItem{
            .node = parent,
            .relation = NodeRelation::PARENT,
        });
    }
    void set_parent(NodeWrapper * parent) {
        for (auto it = _relations.begin(); it != _relations.end();) {
            if (it->relation == NodeRelation::PARENT) {
                _relations.erase(it);
                break;
            }
        }
        add_relative(parent, NodeRelation::PARENT);
    }
    NodeWrapper * get_parent() {
        for (auto && item : _relations) {
            if (item.relation == NodeRelation::PARENT) {
                return item.node;
            }
        }
        return nullptr;
    }
    // TODO DRY
    void add_view(NodeWrapper * view) {
        _relations.push_back(RelationItem{
            .node = view,
            .relation = NodeRelation::VIEW,
        });
    }
    // TODO DRY
    NodeWrapper * get_view() {
        for (auto && item : _relations) {
            if (item.relation == NodeRelation::VIEW) {
                return item.node;
            }
        }
        return nullptr;
    }
    std::vector<NodeWrapper*> get_compute_children() {
        std::vector<NodeWrapper*> result;
        for (auto && item : _relations) {
            if (item.relation == NodeRelation::COMPUTE) {
                result.push_back(item.node);
            }
        }
        return result;
    }
    std::string get_angles_str() {
        if (!is_vanishing_point()) {
            return "";
        } else {
            auto angles = vector_to_angle(as_vanishingPoint().get_direction());
            // TODO move to python
            return std::to_string(angles.first) + "°, " + std::to_string(angles.second) + "°";
        }
    }
    PerspectiveLine * get_line(Complex origin) {
        NodeWrapper * view = get_view();
        return view->as_projection()->get_line(as_vanishingPoint(), origin);
    }
    void clear_compute_sources() {
        for (auto relationIt = _relations.begin(); relationIt != _relations.end();) {
            if (relationIt->relation == NodeRelation::COMPUTE_SRC) {
                NodeWrapper * node = relationIt->node;
                for (auto relation2it = node->_relations.begin(); relation2it != node->_relations.end();) {
                    if (relation2it->relation == NodeRelation::COMPUTE) {
                        if (relation2it->node == this) {
                            relation2it = node->_relations.erase(relation2it);
                            continue;
                        }
                    }
                    ++ relation2it;
                }
                relationIt = _relations.erase(relationIt);
            } else {
                ++relationIt;
            }
        }
    }
    void update_compute_point_source(GraphBase * graph, std::vector<NodeWrapper*> sources) {
        clear_compute_sources();
        for (auto && src : sources) {
            src->add_relative(this, NodeRelation::COMPUTE);
            this->add_relative(src, NodeRelation::COMPUTE_SRC);
        }
        compute(graph);
    }
    void set_compute_additional_params(precission param) {
        _compute_additional_params = {param};
    }
    void set_compute_fct_by_name(std::string name) {
        _compute = compute_functions(name);
        compute_function_name = name;
    }
    void compute(GraphBase * graph);
    void compute_plane(std::vector<NodeWrapper*> src,std::vector<precission> unused) {
        (void) unused;
        auto & plane = as_plane();
        auto source_size = src.size();
        if (source_size == 1) {
            plane.set_normal(src[0]->as_vanishingPoint().get_direction());
        } else if (source_size == 2) {
            Quaternion direction1 = src[0]->as_vanishingPoint().get_direction();
            Quaternion direction2 = src[1]->as_vanishingPoint().get_direction();
            Quaternion normal = normalize(cross(direction1, direction2));
            plane.set_normal(normal);
        } else {
            plane.set_normal(Quaternion(0, 1, 0, 0));
        }
    }
    /** compute horizontally mirrored point */
    void compute_mirrored_points(std::vector<NodeWrapper*> src,std::vector<precission> unused) {
        (void) unused;
        Quaternion srcVector = normalize(src[0]->as_vanishingPoint().get_direction());
        as_vanishingPoint().set_direction(Quaternion(
            srcVector.x,
            -srcVector.y,
            srcVector.z,
            srcVector.w
        ));
    }
    /** compute measure points */
    void compute_measure_points (std::vector<NodeWrapper*> src,std::vector<precission> params) {
        precission direction = params[0];
        Quaternion baseVector = Quaternion(direction, 0, 0, 0);
        Quaternion srcVector = src[0]->as_vanishingPoint().get_direction();
        _compute_measure_points(baseVector, srcVector);
    }
    /** compute measure points from 2 VPs */
    void compute_measure_points_2 (std::vector<NodeWrapper*> src,std::vector<precission> params) {
        precission direction = params[0];
        Quaternion baseVector = src[0]->as_vanishingPoint().get_direction().scalar_mul(direction);
        Quaternion srcVector = src[1]->as_vanishingPoint().get_direction();
        _compute_measure_points(baseVector, srcVector);
    }
    void compute_cross_product(std::vector<NodeWrapper*> src,std::vector<precission> unused) {
        (void) unused;
        auto sourceSize = src.size();
        if (sourceSize != 2) {
            return;
        } else {
            Quaternion direction1 = src[0]->as_vanishingPoint().get_direction();
            Quaternion direction2 = src[1]->as_vanishingPoint().get_direction();
            Quaternion normal = normalize(cross(direction1, direction2));
            as_vanishingPoint().set_direction(normal);
        }
    }
    void compute_2d_direction(std::vector<NodeWrapper*> src,std::vector<precission> unused) {
        (void) unused;
        auto sourceSize = src.size();
        if (sourceSize != 2) {
            return;
        } else {
            Complex new2dDir = _compute_2d_direction(src[0], src[1]);
            Quaternion direction = normalize(Quaternion(new2dDir.real(), new2dDir.imag(), 0, 0));
            as_vanishingPoint().set_direction(direction);
        }
    }
    void compute_2d_direction_90(std::vector<NodeWrapper*> src,std::vector<precission> unused) {
        (void) unused;
        auto sourceSize = src.size();
        if (sourceSize != 2) {
            return;
        } else {
            Complex dir2d = _compute_2d_direction(src[0], src[1]);
            dir2d = Complex(-dir2d.imag(), dir2d.real());   // 90° rotation
            Quaternion direction = Quaternion(dir2d.real(), dir2d.imag(), 0, 0);
            as_vanishingPoint().set_direction(direction);
        }
    }
    void compute_horizon_1(std::vector<NodeWrapper*> src,std::vector<precission> unused) {
        (void) unused;
        Plane & plane = as_plane();
        Quaternion elevation = src[0]->as_vanishingPoint().get_direction();
        Quaternion side = Quaternion(1, 0, 0, 0);
        Quaternion normal = normalize(cross(elevation, side));
        plane.set_normal(normal);
    }
    void compute_space_2p_rect(std::vector<NodeWrapper*> src,std::vector<precission> unused) {
        (void) unused;
        Plane & plane = src[0]->as_plane();
        NodeWrapper* base = src[1];
        NodeWrapper* direction = src[2];
        
        Quaternion planeNormal = plane.get_normal();
        auto * projection = get_view()->as_projection();
        
        Quaternion baseRay = projection->calc_direction(base->get_position());
        Quaternion dirRay = projection->calc_direction(direction->get_position());
        
        precission baseRayDotSign = dot(planeNormal, baseRay);
        precission dirRayDotSign = dot(planeNormal, dirRay);
        // test if points lie on same side of horizon
        if (baseRayDotSign * dirRayDotSign <= 0) {
            return;
        }
        
        Quaternion base3dPos = projection->intersect_view_ray_canvas(baseRay);
        Quaternion dir3dPos = intersect_view_ray_and_plane(planeNormal, base3dPos, dirRay);
        
        Quaternion dir3d = dir3dPos - base3dPos;
        
        Quaternion defaultUp = Quaternion(0, 1, 0, 0);
        Quaternion planeRotation = rotationBetwenVectors(defaultUp, planeNormal);
        Quaternion defaultForward = Quaternion(0, 0, 1, 0);
        Quaternion forwardDir = rotate(planeRotation, defaultForward);
        Quaternion rectRotation = rotationBetwenVectors(forwardDir, dir3d);
        
        PerspectiveSpace & space = as_space();
        space.update_global_rotation(rectRotation * planeRotation);
    }

    void (NodeWrapper::* compute_functions(std::string name))(std::vector<NodeWrapper*>,std::vector<precission>)  {
        if (name == "plane") {
            return &NodeWrapper::compute_plane;
        } else if (name == "compute_mirrored_points") {
            return &NodeWrapper::compute_mirrored_points;
        } else if (name == "compute_measure_points") {
            return &NodeWrapper::compute_measure_points;
        } else if (name == "compute_measure_points_2") {
            return &NodeWrapper::compute_measure_points_2;
        } else if (name == "cross_product") {
            return &NodeWrapper::compute_cross_product;
        } else if (name == "2d_direction") {
            return &NodeWrapper::compute_2d_direction;
        } else if (name == "2d_direction_90") {
            return &NodeWrapper::compute_2d_direction_90;
        } else if (name == "horizon_1") {
            return &NodeWrapper::compute_horizon_1;
        } else if (name == "space_2p_rect") {
            return &NodeWrapper::compute_space_2p_rect;
        } else {
            // TODO exception
        }
        return nullptr;
    }
    // TODO private
    /** compute measure points */
    void _compute_measure_points(const Quaternion & base_vector, const Quaternion & src_vector) {
        Quaternion normalizedSrc = normalize(src_vector); // TODO unnecessary normalization?
        Quaternion dstDirection = normalizedSrc - base_vector;
        if (length(dstDirection) < 0.000001) {
            dstDirection = Quaternion(0, 0, 1, 0);
        }
        dstDirection = normalize(dstDirection);
        as_vanishingPoint().set_direction(dstDirection);
    }
    // TODO private
    Complex _compute_2d_direction(NodeWrapper * src0, NodeWrapper * src1) {
        Quaternion direction_1 = src0->as_vanishingPoint().get_direction();
        Quaternion direction_2 = src1->as_vanishingPoint().get_direction();
        Quaternion normal = normalize(cross(direction_1, direction_2));
        as_vanishingPoint().set_direction(normal);
        Complex pos_1 = src0->as_vanishingPoint().get_position();
        Complex pos_2 = src1->as_vanishingPoint().get_position();
        return Complex(pos_2.real() - pos_1.real(), pos_2.imag() - pos_1.imag());
    }
    void update_toggle_from_parent() {
        NodeWrapper * parent = get_parent();
        if (parent) {
            parent_enabled = parent->enabled && parent->parent_enabled;
        }
    }
    void update_lock_from_parent() {
        NodeWrapper * parent = get_parent();
        if (parent) {
            parent_locked = parent->locked || parent->parent_locked;
        }
    }
    bool is_point() {
        return _is_point;
    }
    bool is_vanishing_point() {
        return _is_vanishing_point;
    }
    bool is_UI() {
        return _is_UI;
    }
    void set_UI(bool value) {
        _is_UI = value;
    }
    bool is_UI_only() {
        return _is_UI_only;
    }
    bool is_space() {
        return _is_space;
    }
    bool is_view() {
        return _is_view;
    }
    bool is_projection() {
        return _is_projection;
    }
    bool is_grouping() {
        return _is_grouping;
    }
    bool is_compute() {
        return isCompute;
    }
    void set_compute(bool compute) {
        isCompute = compute;
    }
    void toggle() {
        enabled ^= true;
    }
    void lock() {
        locked ^= true;
    }
    bool is_key() const {
        return role == VPRole::KEY;
    }
    void set_role(VPRole role) {
        this->role = role;
    }
};


struct VisualizationData {
    std::string type;
    std::vector<int> nodes;
};


class GraphBase {
private:
    std::map<int, NodeWrapper*> nodeMap;
    std::map<std::string, NodeWrapper*> tags;
    std::vector<std::shared_ptr<NodeWrapper>> nodes;
    std::vector<VisualizationData> visualizations;
    friend PyObject * to_object(GraphBase *);
public:
    NodeWrapper * _root = nullptr;
    NodeWrapper * main_view = nullptr;
    NodeWrapper * chosen_point = nullptr;
    bool _is_empty = true;

private:
    struct NewElementData {
        NodeWrapper * group;
        NodeWrapper * view;
        NodeWrapper * space;
    };

    NewElementData get_group_for_new_element();
    void createRoot() {
        PerspectiveGroup tmpRoot = PerspectiveGroup();
        auto rootPtr = std::shared_ptr<NodeWrapper>(new NodeWrapper(tmpRoot, "root"));
        nodes.push_back(rootPtr);
        _root = rootPtr.get();
    }
public:
    GraphBase() {
        clear();
    }

    void clear() {
        _is_empty = true;
        _root = nullptr;
        main_view = nullptr;
        chosen_point = nullptr;
        nodes.clear();
        nodeMap.clear();
        tags.clear();
        visualizations.clear();
        createRoot();
    }
    
    NodeWrapper * get_root() {
        return _root;
    }
    
    NodeWrapper * connect_sub_graph(NodeWrapper * localRoot);

    /** create sub graph from data and attach it as child of currently selected element */
    NodeWrapper * add_sub_graph(PyObject * data) {
        NodeWrapper * localRoot = create_from_structure(data);
        connect_sub_graph(localRoot);
        return localRoot;
    }
    
    /** initialize graph from data */
    void initialize_from_structure(PyObject * data) {
        NodeWrapper * localRoot = create_from_structure(data);
        _root = localRoot;
        std::vector<NodeWrapper *> computeNodes;
        for (auto && child : localRoot->get_children()) {
            std::vector<NodeWrapper*> tmp = update_groups(child);
            computeNodes.insert(computeNodes.end(), tmp.begin(), tmp.end());
        }
        for (auto && computeNode :computeNodes) {
            computeNode->compute(this);
        }
    }

    NodeWrapper * create_from_structure(PyObject * data);
    
    NodeWrapper * get_by_tag(const std::string & tag) {
        if (tags.count(tag)) {
            return tags[tag];
        } else {
            return nullptr;
        }
    }
    
    NodeWrapper * get_by_uid(int uid) {
        if (nodeMap.count(uid)) {
            return nodeMap[uid];
        } else {
            return nullptr;
        }
    }
    
    NodeWrapper * remove_by_uid(int uid) {
        NodeWrapper * node = get_by_uid(uid);
        nodeMap.erase(uid);
        if (node) {
            NodeWrapper * parent = node->get_parent();
            if (parent) {
                parent->remove_child(node);
                return node;
            }
        }
        return nullptr;
    }
    
    std::vector<NodeWrapper *> get_points(NodeWrapper * nodeToDraw);
    
    std::vector<NodeWrapper *> get_all_enabled_points(bool skipLocked = false);
    
    std::vector<NodeWrapper *> get_all_nodes(NodeWrapper * parent);
    
    std::vector<VisualizationData> get_visualizations() {
        return visualizations;
    }

    NodeWrapper * find_parent_space(NodeWrapper * node) {
        NodeWrapper * parent = node->get_parent();
        while (parent != nullptr) {
            if (parent->is_space()) {
                return parent;
            } else {
                parent = parent->get_parent();
            }
        }
        return nullptr;
    }

    // TODO unused?
    // TODO DRY
    NodeWrapper * find_parent_view(NodeWrapper * node) {
        NodeWrapper * parent = node->get_parent();
        while (parent != nullptr) {
            if (parent->is_view()) {
                return parent;
            } else {
                parent = parent->get_parent();
            }
        }
        return nullptr;
    }
    
    // TODO DRY
    NodeWrapper * find_UI_parent(NodeWrapper * node) {
        NodeWrapper * parent = node->get_parent();
        while (parent != nullptr) {
            if (parent->is_UI()) {
                return parent;
            } else {
                parent = parent->get_parent();
            }
        }
        return nullptr;
    }

    std::vector<NodeWrapper *> get_logic_children(NodeWrapper * node);

    std::vector<NodeWrapper *> update_groups(NodeWrapper * group);

    void update(NodeWrapper * node, Complex pos);

    /** convert dst to compute point */
    void convert_to_compute_node(NodeWrapper * dst, std::vector<NodeWrapper*> & sources, const std::string & fctName, precission value) {
        for (auto && src : sources) {
            src->add_relative(dst, NodeRelation::COMPUTE);
            dst->add_relative(src, NodeRelation::COMPUTE_SRC);
        }
        dst->set_compute(true);
        dst->set_compute_additional_params(value);
        dst->set_compute_fct_by_name(fctName);
        dst->compute(this);
    }

    bool is_empty() {
        return _is_empty;
    }
};

