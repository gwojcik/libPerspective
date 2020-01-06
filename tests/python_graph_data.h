#pragma once

const char * test_point_py = R"({'nodes': [{
    'type': 'VP',
    'id': '1',
    'locked': True,
}], 'visualizations': [{
    'type': 'point',
    'nodes': ['1']
}], 'edges': [], 'root': "1"})";

const char * test_space_py = R"({'nodes': [{
    'type': 'Space',
    'id': 'space',
    'is_UI': 1
}, {
    'type': 'VP',
    'id': 'Forward',
    'name': 'Forward',
    'direction': [0, 0, 1],
}, {
    'type': 'VP',
    'id': 'Side',
    'name': 'Side',
    'direction': [1, 0, 0],
}, {
    'type': 'VP',
    'id': 'Up',
    'name': 'Up',
    'direction': [0, 1, 0],
    'locked': True
}, {
    'type': 'Group',
    'id': 'config',
    'name': 'Configuration',
    'enabled': False
}], 'visualizations': [{
    'type': 'point',
    'nodes': ['Forward']
}, {
    'type': 'point',
    'nodes': ['Side']
}, {
    'type': 'point',
    'nodes': ['Up']
}, {
    'type': 'space',
    'nodes': ['space', 'config']
}], 'edges': [{
    'src': 'space',
    'dst': 'Forward'
}, {
    'src': 'space',
    'dst': 'Side'
}, {
    'src': 'space',
    'dst': 'Up',
}, {
    'src': 'space',
    'dst': 'config'
}], 'root': 'space'})";

const char * test_group_py = R"({'nodes': [{
    'type': 'Group',
    'id': 'group',
    'name': 'group'
}], 'edges': [
], 'root': 'group'})";

const char * test_horizon_py = R"({'nodes': [{
    'type': 'Plane',
    'id': 'horizon',
    'name': 'horizon',
    'is_compute': True,
    'compute_fct': 'plane',
    'compute_params': []
}], 'edges': [
], 'root': 'horizon'})";

const char * test_projection_py = R"({
    'nodes': [{
        'type': 'Group',
        'id': 'Root',
        'name': 'Root'
    }, {
        'type': 'RectilinearProjection',
        'id': 'View',
        'name': 'View',
    }, {
        'type': 'VP',
        'id': 'R',
        'name': 'Right',
        'direction': [1, 0, 1],
        'locked': True
    }, {
        'type': 'VP',
        'id': 'L',
        'name': 'Left',
        'direction': [-1, 0, 1],
        'locked': True
    }, {
        'type': 'VP',
        'id': 'C',
        'name': 'Center',
        'direction': [0, 0, 1],
        'locked': True
    }, {
        'type': 'Plane',
        'id': 'implicit_plane',
        'name': 'implicit plane',
        'tag': '__implicit_plane',
        'is_compute': True,
        'compute_fct': 'plane',
        'compute_params': []
    }, {
        'type': 'Group',
        'id': 'SP_Config',
        'name': 'Configuration',
        'enabled': False
    }], 'visualizations': [{
        'type': 'projection',
        'nodes': ['View', 'SP_Config']
    }, {
        'type': 'point',
        'nodes': ['R']
    }, {
        'type': 'point',
        'nodes': ['L']
    }, {
        'type': 'point',
        'nodes': ['C']
    }], 'edges': [{
        'src': 'Root',
        'dst': 'View'
    }, {
        'src': 'View',
        'dst': 'R'
    }, {
        'src': 'View',
        'dst': 'L'
    }, {
        'src': 'View',
        'dst': 'C'
    }, {
        'src': 'View',
        'dst': 'implicit_plane'
    }, {
        'src': 'R',
        'dst': 'View',
        'type': 'VIEW'
    }, {
        'src': 'L',
        'dst': 'View',
        'type': 'VIEW'
    }, {
        'src': 'C',
        'dst': 'View',
        'type': 'VIEW'
    }, {
        'src': 'implicit_plane',
        'dst': 'View',
        'type': 'VIEW'
    }, {
        'src': 'Root',
        'dst': 'SP_Config'
    }, {
        'src': 'SP_Config',
        'dst': 'View',
        'type': 'VIEW'
    }], 'root': 'Root'
})";
