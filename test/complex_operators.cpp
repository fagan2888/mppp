// Copyright 2016-2020 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <algorithm>
#include <complex>
#include <type_traits>
#include <utility>

#include <mp++/complex.hpp>
#include <mp++/config.hpp>
#include <mp++/detail/mpfr.hpp>
#include <mp++/integer.hpp>
#include <mp++/rational.hpp>
#include <mp++/real.hpp>

#if defined(MPPP_WITH_QUADMATH)

#include <mp++/complex128.hpp>
#include <mp++/real128.hpp>

#endif

#include "catch.hpp"

using namespace mppp;

TEST_CASE("identity")
{
    complex r1{4, 5};
    REQUIRE(+r1 == r1);

    // Check stealing.
    const auto p = r1.get_prec();
    auto r2 = +std::move(r1);
    REQUIRE(r2.get_prec() == p);
    REQUIRE(r2 == complex{4, 5});
    REQUIRE(!r1.is_valid());
}

TEST_CASE("binary plus")
{
    // complex-complex.
    {
        complex r1{4, 5}, r2{-4, 7};
        const auto p = r1.get_prec();
        auto ret = r1 + r2;
        REQUIRE(std::is_same<complex, decltype(ret)>::value);
        REQUIRE(ret == complex{0, 12});
        REQUIRE(ret.get_prec() == r1.get_prec());

        // Test moves.
        ret = std::move(r1) + r2;
        REQUIRE(ret == complex{0, 12});
        REQUIRE(ret.get_prec() == p);
        REQUIRE(!r1.is_valid());

        r1 = complex{4, 5};
        ret = r1 + std::move(r2);
        REQUIRE(ret == complex{0, 12});
        REQUIRE(ret.get_prec() == p);
        REQUIRE(!r2.is_valid());

        r2 = complex{-4, 7};
        ret = std::move(r1) + std::move(r2);
        REQUIRE(ret == complex{0, 12});
        REQUIRE(ret.get_prec() == p);
        REQUIRE((!r1.is_valid() || !r2.is_valid()));
    }
    // complex-real.
    {
        complex c1{45, 6, complex_prec_t(128)};
        real r1{23, 10};
        auto ret = c1 + r1;
        REQUIRE(std::is_same<complex, decltype(ret)>::value);
        REQUIRE(ret == complex{68, 6, complex_prec_t(128)});
        REQUIRE(ret.get_prec() == 128);
        ret = r1 + c1;
        REQUIRE(std::is_same<complex, decltype(r1 + c1)>::value);
        REQUIRE(ret == complex{68, 6, complex_prec_t(128)});
        REQUIRE(ret.get_prec() == 128);

        // Try with higher precision on the non-complex argument.
        ret = c1 + real{23, 256};
        REQUIRE(ret == complex{68, 6, complex_prec_t(128)});
        REQUIRE(ret.get_prec() == 256);
        ret = real{23, 256} + c1;
        REQUIRE(ret == complex{68, 6, complex_prec_t(128)});
        REQUIRE(ret.get_prec() == 256);

        // Try with moves.
        auto c2 = c1;
        ret = complex{};
        ret = std::move(c1) + r1;
        REQUIRE(ret == complex{68, 6, complex_prec_t(128)});
        REQUIRE(ret.get_prec() == 128);
        REQUIRE(!c1.is_valid());

        c1 = c2;
        ret = complex{};
        ret = r1 + std::move(c1);
        REQUIRE(ret == complex{68, 6, complex_prec_t(128)});
        REQUIRE(ret.get_prec() == 128);
        REQUIRE(!c1.is_valid());
    }
    // complex-rv interoperable.
    {
        complex c1{45, 6, complex_prec_t(128)};
        auto ret = c1 + 6;
        REQUIRE(ret == complex{51, 6});
        REQUIRE(std::is_same<complex, decltype(ret)>::value);
        REQUIRE(ret.get_prec() == std::max<::mpfr_prec_t>(128, detail::real_deduce_precision(6)));
        ret = 6. + c1;
        REQUIRE(ret == complex{51, 6});
        REQUIRE(std::is_same<complex, decltype(6. + c1)>::value);
        REQUIRE(ret.get_prec() == std::max<::mpfr_prec_t>(128, detail::real_deduce_precision(6.)));

        // Try with higher precision on the non-complex argument.
        c1 = complex{1, 1, complex_prec_t(real_prec_min())};
        ret = c1 + 45_z1;
        REQUIRE(ret == complex{46, 1});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45_z1));
        ret = 45_q1 + c1;
        REQUIRE(ret == complex{46, 1});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45_q1));

        // Moves.
        auto c2 = c1;
        ret = std::move(c1) + 45;
        REQUIRE(ret == complex{46, 1});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45));
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(detail::real_deduce_precision(45) + 1)};
        ret = std::move(c1) + 45;
        REQUIRE(ret == complex{46, 1});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45) + 1);
        REQUIRE(!c1.is_valid());

        c1 = c2;
        ret = 45. + std::move(c1);
        REQUIRE(ret == complex{46, 1});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45.));
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(detail::real_deduce_precision(45.) + 1)};
        ret = 45. + std::move(c1);
        REQUIRE(ret == complex{46, 1});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45.) + 1);
        REQUIRE(!c1.is_valid());

#if defined(MPPP_WITH_QUADMATH)
        c1 = c2;
        ret = 45_rq + std::move(c1);
        REQUIRE(ret == complex{46, 1});
        REQUIRE(ret.get_prec() == 113);
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(114)};
        ret = 45_rq + std::move(c1);
        REQUIRE(ret == complex{46, 1});
        REQUIRE(ret.get_prec() == 114);
        REQUIRE(!c1.is_valid());
#endif
    }
    // complex-unsigned integral.
    {
        complex c1{45, 6, complex_prec_t(128)};
        auto ret = c1 + 6u;
        REQUIRE(ret == complex{51, 6});
        REQUIRE(std::is_same<complex, decltype(ret)>::value);
        REQUIRE(ret.get_prec() == std::max<::mpfr_prec_t>(128, detail::real_deduce_precision(6u)));
        ret = 6u + c1;
        REQUIRE(ret == complex{51, 6});
        REQUIRE(std::is_same<complex, decltype(6u + c1)>::value);
        REQUIRE(ret.get_prec() == std::max<::mpfr_prec_t>(128, detail::real_deduce_precision(6u)));

        // Try with higher precision on the non-complex argument.
        c1 = complex{1, 1, complex_prec_t(real_prec_min())};
        ret = c1 + 45u;
        REQUIRE(ret == complex{46, 1});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45u));
        ret = 45u + c1;
        REQUIRE(ret == complex{46, 1});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45u));

        // Moves.
        auto c2 = c1;
        ret = std::move(c1) + 45u;
        REQUIRE(ret == complex{46, 1});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45u));
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(detail::real_deduce_precision(45u) + 1)};
        ret = std::move(c1) + 45u;
        REQUIRE(ret == complex{46, 1});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45u) + 1);
        REQUIRE(!c1.is_valid());

        c1 = c2;
        ret = 45u + std::move(c1);
        REQUIRE(ret == complex{46, 1});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45u));
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(detail::real_deduce_precision(45u) + 1)};
        ret = 45u + std::move(c1);
        REQUIRE(ret == complex{46, 1});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(45u) + 1);
        REQUIRE(!c1.is_valid());

#if defined(MPPP_HAVE_GCC_INT128)
        // Try with a large integral.
        c1 = c2;
        ret = __uint128_t(-1) + std::move(c1);
        REQUIRE(ret == complex{1_z1 + __uint128_t(-1), 1, complex_prec_t(128)});
        REQUIRE(ret.get_prec() == 128);
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(129)};
        ret = __uint128_t(-1) + std::move(c1);
        REQUIRE(ret == complex{1_z1 + __uint128_t(-1), 1, complex_prec_t(129)});
        REQUIRE(ret.get_prec() == 129);
        REQUIRE(!c1.is_valid());

        c1 = c2;
        ret = std::move(c1) + __uint128_t(-1);
        REQUIRE(ret == complex{1_z1 + __uint128_t(-1), 1, complex_prec_t(128)});
        REQUIRE(ret.get_prec() == 128);
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(129)};
        ret = std::move(c1) + __uint128_t(-1);
        REQUIRE(ret == complex{1_z1 + __uint128_t(-1), 1, complex_prec_t(129)});
        REQUIRE(ret.get_prec() == 129);
        REQUIRE(!c1.is_valid());
#endif
    }

    // Complex-std::complex.
    {
        complex c1{45, 6, complex_prec_t(128)};
        auto ret = c1 + std::complex<double>(6, 7);
        REQUIRE(ret == complex{51, 13});
        REQUIRE(std::is_same<complex, decltype(ret)>::value);
        REQUIRE(ret.get_prec() == std::max<::mpfr_prec_t>(128, detail::real_deduce_precision(6.)));
        ret = std::complex<double>(6, 7) + c1;
        REQUIRE(ret == complex{51, 13});
        REQUIRE(std::is_same<complex, decltype(std::complex<double>(6, 7) + c1)>::value);
        REQUIRE(ret.get_prec() == std::max<::mpfr_prec_t>(128, detail::real_deduce_precision(6.)));

        // Try with higher precision on the non-complex argument.
        c1 = complex{1, 1, complex_prec_t(real_prec_min())};
        ret = c1 + std::complex<double>(6, 7);
        REQUIRE(ret == complex{7, 8});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(6.));
        ret = std::complex<double>(6, 7) + c1;
        REQUIRE(ret == complex{7, 8});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(6.));

        // Moves.
        auto c2 = c1;
        ret = std::move(c1) + std::complex<double>(6, 7);
        REQUIRE(ret == complex{7, 8});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(6.));
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(detail::real_deduce_precision(6.) + 1)};
        ret = std::move(c1) + std::complex<double>(6, 7);
        REQUIRE(ret == complex{7, 8});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(6.) + 1);
        REQUIRE(!c1.is_valid());

        c1 = c2;
        ret = std::complex<double>(6, 7) + std::move(c1);
        REQUIRE(ret == complex{7, 8});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(6.));
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(detail::real_deduce_precision(6.) + 1)};
        ret = std::complex<double>(6, 7) + std::move(c1);
        REQUIRE(ret == complex{7, 8});
        REQUIRE(ret.get_prec() == detail::real_deduce_precision(6.) + 1);
        REQUIRE(!c1.is_valid());
    }

#if defined(MPPP_WITH_QUADMATH)
    // Complex-complex128.
    {
        complex c1{45, 6, complex_prec_t(128)};
        auto ret = c1 + complex128(6, 7);
        REQUIRE(ret == complex{51, 13});
        REQUIRE(std::is_same<complex, decltype(ret)>::value);
        REQUIRE(ret.get_prec() == 128);
        ret = complex128(6, 7) + c1;
        REQUIRE(ret == complex{51, 13});
        REQUIRE(std::is_same<complex, decltype(complex128(6, 7) + c1)>::value);
        REQUIRE(ret.get_prec() == 128);

        // Try with higher precision on the non-complex argument.
        c1 = complex{1, 1, complex_prec_t(real_prec_min())};
        ret = c1 + complex128(6, 7);
        REQUIRE(ret == complex{7, 8});
        REQUIRE(ret.get_prec() == 113);
        ret = complex128(6, 7) + c1;
        REQUIRE(ret == complex{7, 8});
        REQUIRE(ret.get_prec() == 113);

        // Moves.
        auto c2 = c1;
        ret = std::move(c1) + complex128(6, 7);
        REQUIRE(ret == complex{7, 8});
        REQUIRE(ret.get_prec() == 113);
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(114)};
        ret = std::move(c1) + complex128(6, 7);
        REQUIRE(ret == complex{7, 8});
        REQUIRE(ret.get_prec() == 114);
        REQUIRE(!c1.is_valid());

        c1 = c2;
        ret = complex128(6, 7) + std::move(c1);
        REQUIRE(ret == complex{7, 8});
        REQUIRE(ret.get_prec() == 113);
        REQUIRE(c1.is_valid());
        c1 = complex{1, 1, complex_prec_t(114)};
        ret = complex128(6, 7) + std::move(c1);
        REQUIRE(ret == complex{7, 8});
        REQUIRE(ret.get_prec() == 114);
        REQUIRE(!c1.is_valid());
    }
#endif
}

TEST_CASE("negation")
{
    complex r1{4, 5};
    REQUIRE(-r1 == complex{-4, -5});

    // Check stealing.
    const auto p = r1.get_prec();
    auto r2 = -std::move(r1);
    REQUIRE(r2.get_prec() == p);
    REQUIRE(r2 == complex{-4, -5});
    REQUIRE(!r1.is_valid());
}
