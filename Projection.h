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
#include "Point.h"
#include <vector>
#include <memory>
#include "log.h"


/** Base class for lines in perspective */
class PerspectiveLine {
public:
    virtual ~PerspectiveLine(){}
    /** return distance between position and line */
    virtual precission get_distance(const Complex & position) = 0;

    /** return line points */
    virtual std::vector<Complex> get_line_points(const Complex & position) = 0;
};


class HorizonLineBase {
public:
    virtual ~HorizonLineBase(){}
    /** Return points inside bounding box */
    virtual std::vector<Complex> for_bbox(const Complex & corner_a, const Complex & corner_b) = 0;
};

/** Common methods for more complex perspective prjections */
class Projection {
protected:
    Complex center;
    Complex rotation;
    precission size;
public:
    virtual ~Projection(){}
    Projection(const Complex & left_pos, const Complex & right_pos) {
        Complex center = (left_pos + right_pos)/2.0;

        Complex diff = right_pos - center;
        precission size = std::hypot(diff.real(), diff.imag());
        Complex rotation = diff / size;

        this->center = center;
        this->rotation = rotation;
        this->size = size;
    }

    /** set internal variables */
    void set_all(const Complex & center, const Complex & rotation, precission size) {
        this->center = center;
        this->rotation = rotation;
        this->size = size;
    }

    /** return center */
    Complex get_center_complex() const {
        return this->center;
    }

    void set_center(Complex center) {
        this->center = center;
    }

    precission get_size() const {
        return this->size;
    }

    void set_size(precission size) {
        this->size = size;
    }

    /** return 2D rotaion around center */
    Complex get_rotation() const {
        return this->rotation;
    }

    void set_rotation(Complex rotation) {
        this->rotation = rotation;
    }

    Complex internal_position_to_model(const Complex & pos) const {
        return pos * rotation * size + center;
    }

    virtual void update_child(VanishingPoint & vp) {
        vp.set_position(this->calc_pos_from_dir(vp.get_direction()));
    }
    virtual void update_child(VanishingPoint & vp, const Complex & new_position) {
        vp.set_direction(this->calc_direction(new_position));
        vp.set_direction_local(vp.get_direction());
        vp.set_position(new_position);
    }

    virtual std::shared_ptr<PerspectiveLine> get_line(const VanishingPoint & vp, const Complex & start_position) const = 0;

    virtual std::shared_ptr<PerspectiveLine> get_line(const Quaternion & direction, const Complex & start_position) const = 0;

    virtual std::shared_ptr<HorizonLineBase> get_horizon_line(const Quaternion & up) const = 0;

    virtual Quaternion calc_direction(const Complex & pos) const = 0;

    virtual Complex calc_pos_from_dir(const Quaternion & direction) const = 0;

    virtual std::vector<Complex> project_on_canvas(const std::vector<Quaternion> & positions) const = 0;

    virtual Quaternion intersect_view_ray_canvas(const Quaternion & ray) const = 0;

    /**
     * convert position in model (pixel based) to internal space
     *  internal space is transformed with center, size and rotation properties
     */
    Complex model_position_to_internal(const Complex & position) const {
        return ((position - center) / size * std::conj(rotation));
    }
};


/**
 * Standart perspective projection for 1, 2 and 3 point perspective.
 * Defines center of view, size and rotation of perspective projection.
 * Size defines how far away are -45° and +45° vanishing points
 */
class RectilinearProjection : public Projection {
public:
    RectilinearProjection(const Complex & left_pos, const Complex & right_pos) : Projection(left_pos, right_pos) {}

    /** convert 2D model position to 3D direction vector */
    virtual Quaternion calc_direction(const Complex & model_pos) const override {
        Complex internal_pos = this->model_position_to_internal(model_pos);
        Quaternion view_ray = Quaternion(internal_pos.real(), internal_pos.imag(), 1, 0);
        view_ray = normalize(view_ray);
        return view_ray;
    }

    /** Take vector part of Quaternion and project it on canvas. */
    virtual Complex calc_pos_from_dir(const Quaternion & direction) const override {
        if (direction.z == 0) {
            return Complex(1024*1024*1024, 1024*1024*1024);
        }
        precission scale = size / direction.z;
        return Complex(direction.x * scale, direction.y * scale) * rotation + center;
    }

    Complex get_direction_2d(const VanishingPoint & vp, const Complex & start_position) const {
        Quaternion direction = vp.get_direction();
        Complex sp = start_position;
        sp -= center;
        Complex d = sp + Complex(direction.x, direction.y) * rotation;
        precission scale = size / (size + direction.z);
        d *= scale;
        Complex dir_vec = sp - d;
        precission vec_len = std::hypot(dir_vec.real(), dir_vec.imag());
        return dir_vec / vec_len;
    }

    virtual std::shared_ptr<PerspectiveLine> get_line(const VanishingPoint & vp, const Complex & start_position) const override;

    virtual std::shared_ptr<PerspectiveLine> get_line(const Quaternion & direction, const Complex & start_position) const override;

    virtual std::shared_ptr<HorizonLineBase> get_horizon_line(const Quaternion & up) const override;

    virtual Quaternion intersect_view_ray_canvas(const Quaternion & ray) const override {
        return ray.scalar_mul(1.0 / ray.z);
    }

    /**
     * @param position in view space
     */
    virtual std::vector<Complex> project_on_canvas(const std::vector<Quaternion> & positions) const override {
        std::vector<Complex> result;
        for( auto && pos : positions) {
            Quaternion intersection = this->intersect_view_ray_canvas(pos);
            Complex position_2d = Complex(intersection.x, intersection.y);
            Complex position = position_2d * size * rotation + center;
            result.push_back(position);
        }
        return result;
    }
};


/** Curvilinear Perspective for 4 and 5 point perspective */
class CurvilinearPerspective : public Projection {
public:
    CurvilinearPerspective(const Complex & left_pos, const Complex & right_pos) : Projection(left_pos, right_pos) {}

    virtual Complex calc_pos_from_dir(const Quaternion & direction) const override {
        Quaternion dirNormalized = normalize(direction);
        Complex internal_pos = Complex(dirNormalized.x, dirNormalized.y);
        return internal_position_to_model(internal_pos);
    }

    virtual Quaternion calc_direction(const Complex & pos) const override {
        Complex internal = model_position_to_internal(pos);
        precission r = std::hypot(internal.real(), internal.imag());
        if (r > 1.0) {
            r = 1.0;
            internal = Complex(internal.real() / r, internal.imag() / r);
        }
        precission z = std::sqrt(1 - r * r);
        return Quaternion(internal.real(), internal.imag(), z, 0);
    }

    virtual std::shared_ptr<PerspectiveLine> get_line(const VanishingPoint & vp, const Complex & start_position) const override;

    virtual std::shared_ptr<PerspectiveLine> get_line(const Quaternion & direction, const Complex & start_position) const override;

    virtual std::shared_ptr<HorizonLineBase> get_horizon_line(const Quaternion & up) const override;

    /** use view space */
    virtual Quaternion intersect_view_ray_canvas(const Quaternion & ray) const override {
        return ray;
    }

    virtual std::vector<Complex> project_on_canvas(const std::vector<Quaternion> & positions) const override {
        std::vector<Complex> result;
        for ( auto && pos : positions) {
            if (pos.z > 0) {
                result.push_back(calc_pos_from_dir(pos));
            }
        }
        return result;
    }
};
