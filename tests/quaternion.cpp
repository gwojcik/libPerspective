#include <catch2/catch.hpp>
#include "../Quaternion.h"

TEST_CASE ( "Quaternion math" )
{
    using namespace Catch::literals;
    SECTION ( "length" ) {
        REQUIRE ( length ( Quaternion ( 1,1,1,1 ) ) == 1.73205080756887729352_a );
    }
    SECTION ( "normalize" ) {
        SECTION ( "test 1" ) {
            constexpr auto q1 = Quaternion{1,1,1,1};
            constexpr auto q2 = Quaternion{2,2,2,1};
            const auto n1 = normalize ( q1 );
            const auto n2 = normalize ( q2 );
            REQUIRE ( n1.x == n2.x );
            REQUIRE ( n1.y == n2.y );
            REQUIRE ( n1.z == n2.z );
            REQUIRE ( n1.w == n2.w );
        }
        SECTION ( "test 2" ) {
            constexpr auto q1 = Quaternion{1,2,3,1};
            constexpr auto q2 = Quaternion{2,4,6,1};
            const auto n1 = normalize ( q1 );
            const auto n2 = normalize ( q2 );
            REQUIRE ( n1.x == n2.x );
            REQUIRE ( n1.y == n2.y );
            REQUIRE ( n1.z == n2.z );
            REQUIRE ( n1.w == n2.w );
        }
    }
    SECTION ( "multiplication" ) {
        constexpr auto q1 = Quaternion{1,2,3,4};
        constexpr auto q2 = Quaternion{0,0,0,1};
        SECTION ( "test multiplication with identity" ) {
            SECTION ( "q * identity" ) {
                constexpr auto q12 = q1 * q2;
                REQUIRE ( q12.x == 1.0 );
                REQUIRE ( q12.y == 2.0 );
                REQUIRE ( q12.z == 3.0 );
                REQUIRE ( q12.w == 4.0 );
            }
            SECTION ( "identity * q" ) {
                constexpr auto q21 = q2 * q1;
                REQUIRE ( q21.x == 1.0 );
                REQUIRE ( q21.y == 2.0 );
                REQUIRE ( q21.z == 3.0 );
                REQUIRE ( q21.w == 4.0 );
            }
        }
    }
    SECTION ( "scalar multiplication" ) {
        constexpr auto q1 = Quaternion{1,2,3,4};
        constexpr auto r = q1 * 3;
        REQUIRE ( r.x == 3.0 );
        REQUIRE ( r.y == 6.0 );
        REQUIRE ( r.z == 9.0 );
        REQUIRE ( r.w == 12.0 );
    }
    SECTION ( "conjugate" ) {
        constexpr auto q1 = Quaternion{1,2,3,4};
        constexpr auto r = conjugate ( q1 );
        REQUIRE ( r.x == -1.0 );
        REQUIRE ( r.y == -2.0 );
        REQUIRE ( r.z == -3.0 );
        REQUIRE ( r.w == 4.0 );
    }
    SECTION ( "rotate vector" ) {
        constexpr auto vector = Quaternion{1,2,3,0};
        constexpr auto rotation = Quaternion{0,1,0,0};
        constexpr auto result = rotate ( rotation, vector );
        REQUIRE ( result.x == -1.0 );
        REQUIRE ( result.y == 2.0 );
        REQUIRE ( result.z == -3.0 );
        REQUIRE ( result.w == 0.0 );
    }
    SECTION ( "cross product" ) {
        constexpr auto a = Quaternion{1,0,0,0};
        constexpr auto b = Quaternion{0,1,0,0};
        SECTION ( "test 1" ) {
            constexpr auto resultAB = cross ( a, b );
            REQUIRE ( resultAB.x == 0 );
            REQUIRE ( resultAB.y == 0 );
            REQUIRE ( resultAB.z == 1 );
            REQUIRE ( resultAB.w == 0 );
        }
        SECTION ( "test 2" ) {
            constexpr auto resultBA = cross ( b, a );
            REQUIRE ( resultBA.x == 0 );
            REQUIRE ( resultBA.y == 0 );
            REQUIRE ( resultBA.z == -1 );
            REQUIRE ( resultBA.w == 0 );
        }
    }
    SECTION ( "dot product" ) {
        constexpr auto a = Quaternion{1,0,0,0};
        constexpr auto b = Quaternion{0,1,0,0};
        constexpr auto c = Quaternion{1,1,0,0};
        REQUIRE ( dot ( a,b ) == 0 );
        REQUIRE ( dot ( b,c ) == 1 );
    }
    SECTION ( "rotationBetwenVectors" ) {
        constexpr auto a = Quaternion{1,0,0,0};
        constexpr auto b = Quaternion{0,1,0,0};
        constexpr auto c = Quaternion{1,1,0,0};
        SECTION ( "test 1" ) {
            auto x = rotationBetwenVectors ( a, b );
            REQUIRE ( x.x == 0 );
            REQUIRE ( x.y == 0 );
            REQUIRE ( x.z == 0.707106769_a );
            REQUIRE ( x.w == 0.707106769_a );
        }
        SECTION ( "test 2" ) {
            auto x = rotationBetwenVectors ( b, a );
            REQUIRE ( x.x == 0 );
            REQUIRE ( x.y == 0 );
            REQUIRE ( x.z == -0.707106769_a );
            REQUIRE ( x.w == 0.707106769_a );
        }
        SECTION ( "test 3" ) {
            auto x = rotationBetwenVectors ( a, c );
            REQUIRE ( x.x == 0 );
            REQUIRE ( x.y == 0 );
            REQUIRE ( x.z == 0.382683426_a );
            REQUIRE ( x.w == 0.923879504_a );
        }
    }
}
