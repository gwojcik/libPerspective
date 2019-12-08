#include "Quaternion.h"
#include <assert.h>
#include <iostream>
#include <iomanip>

// TODO use test framework

constexpr bool nearQual(const double value, const double expected) {
    return value > (expected - std::abs(expected) * 0.0000001) && value < (expected + std::abs(expected) * 0.0000001);
}

int main() {
#ifdef NDEBUG
    static_assert(false, "NDEBUG (assert) required for tests");
#endif
    {
        constexpr auto q1 = Quaternion{1,1,1,1};
        const auto len = length(q1);
        assert(nearQual(len, 1.73205080756887729352));
        std::cout << "OK - Quaternion - length - test 1\n";
    }
    {
        constexpr auto q1 = Quaternion{1,1,1,1};
        constexpr auto q2 = Quaternion{2,2,2,1};
        assert(normalize(q1).x == normalize(q2).x);
        assert(normalize(q1).y == normalize(q2).y);
        assert(normalize(q1).z == normalize(q2).z);
        assert(normalize(q1).w == normalize(q2).w);
        std::cout << "OK - Quaternion - normalize - test 1\n";
    }
    {
        constexpr auto q1 = Quaternion{1,2,3,1};
        constexpr auto q2 = Quaternion{2,4,6,1};
        assert(normalize(q1).x == normalize(q2).x);
        assert(normalize(q1).y == normalize(q2).y);
        assert(normalize(q1).z == normalize(q2).z);
        assert(normalize(q1).w == normalize(q2).w);
        std::cout << "OK - Quaternion - normalize - test 2\n";
    }
    {
        constexpr auto q1 = Quaternion{1,2,3,4};
        constexpr auto q2 = Quaternion{0,0,0,1};
        {
            constexpr auto q12 = q1 * q2;
            assert(q12.x == 1.0);
            assert(q12.y == 2.0);
            assert(q12.z == 3.0);
            assert(q12.w == 4.0);
            std::cout << "OK - Quaternion - quaternion mul - test 1\n";
        }
        {
            constexpr auto q21 = q2 * q1;
            assert(q21.x == 1.0);
            assert(q21.y == 2.0);
            assert(q21.z == 3.0);
            assert(q21.w == 4.0);
            std::cout << "OK - Quaternion - quaternion mul - test 2\n";
        }
    }
    {
        constexpr auto q1 = Quaternion{1,2,3,4};
        constexpr auto r = q1 * 3;
        assert(r.x == 3.0);
        assert(r.y == 6.0);
        assert(r.z == 9.0);
        assert(r.w == 12.0);
        std::cout << "OK - Quaternion - scalar mul - test 1\n";
    }
    {
        constexpr auto q1 = Quaternion{1,2,3,4};
        constexpr auto r = conjugate(q1);
        assert(r.x == -1.0);
        assert(r.y == -2.0);
        assert(r.z == -3.0);
        assert(r.w == 4.0);
        std::cout << "OK - Quaternion - conjugate - test 1\n";
    }
    {
        constexpr auto vector = Quaternion{1,2,3,0};
        constexpr auto rotation = Quaternion{0,1,0,0};
        constexpr auto result = rotate(rotation, vector);
        assert(result.x == -1.0);
        assert(result.y == 2.0);
        assert(result.z == -3.0);
        assert(result.w == 0.0);
        std::cout << "OK - Quaternion - rotate vector - test 1\n";
    }
    {
        constexpr auto a = Quaternion{1,0,0,0};
        constexpr auto b = Quaternion{0,1,0,0};
        {
            constexpr auto resultAB = cross(a, b);
            assert(resultAB.x == 0);
            assert(resultAB.y == 0);
            assert(resultAB.z == 1);
            assert(resultAB.w == 0);
            std::cout << "OK - Quaternion - cross - test 1\n";
        }
        {
            constexpr auto resultBA = cross(b, a);
            assert(resultBA.x == 0);
            assert(resultBA.y == 0);
            assert(resultBA.z == -1);
            assert(resultBA.w == 0);
            std::cout << "OK - Quaternion - cross - test 2\n";
        }
    }
    {
        constexpr auto a = Quaternion{1,0,0,0};
        constexpr auto b = Quaternion{0,1,0,0};
        constexpr auto c = Quaternion{1,1,0,0};
        assert(dot(a,b) == 0);
        std::cout << "OK - Quaternion - dot - test 1\n";
        assert(dot(b,c) == 1);
        std::cout << "OK - Quaternion - dot - test 2\n";
    }
    {
        constexpr auto a = Quaternion{1,0,0,0};
        constexpr auto b = Quaternion{0,1,0,0};
        constexpr auto c = Quaternion{1,1,0,0};
        {
            auto x = rotationBetwenVectors(a, b);
            assert(x.x == 0);
            assert(x.y == 0);
            assert(nearQual(x.z, 0.707106769));
            assert(nearQual(x.w, 0.707106769));
            std::cout << "OK - Quaternion - rotationBetwenVectors - test 1\n";
        }
        {
            auto x = rotationBetwenVectors(b, a);
            assert(x.x == 0);
            assert(x.y == 0);
            assert(nearQual(x.z, -0.707106769));
            assert(nearQual(x.w, 0.707106769));
            std::cout << "OK - Quaternion - rotationBetwenVectors - test 2\n";
        }
        {
            auto x = rotationBetwenVectors(a, c);
            assert(x.x == 0);
            assert(x.y == 0);
            // std::cout << std::setprecision(9) << x.w << "\n";
            assert(nearQual(x.z, 0.382683426));
            assert(nearQual(x.w, 0.923879504));
            std::cout << "OK - Quaternion - rotationBetwenVectors - test 3\n";
        }
    }
    return 1;
}
