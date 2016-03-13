#include <valarray>
#include <iterator>

#include <symengine/basic.h>
#include <symengine/ntheory.h>
#include <symengine/rational.h>
#include <symengine/mul.h>
#ifdef HAVE_SYMENGINE_ECM
#  include <ecm.h>
#endif // HAVE_SYMENGINE_ECM
#ifdef HAVE_SYMENGINE_PRIMESIEVE
#  include <primesieve.hpp>
#endif // HAVE_SYMENGINE_PRIMESIEVE
#ifdef HAVE_SYMENGINE_ARB
#  include "arb.h"
#  include "bernoulli.h"
#  include "rational.h"
#endif // HAVE_SYMENGINE_ARB
#include <symengine/dict.h>

namespace SymEngine {

// Basic number theoretic functions
RCP<const Integer> gcd(const Integer &a, const Integer &b)
{
    integer_class g;
    mp_gcd(g, a.as_mpz(), b.as_mpz());
    return integer(std::move(g));
}

void gcd_ext(const Ptr<RCP<const Integer>> &g, const Ptr<RCP<const Integer>> &s,
        const Ptr<RCP<const Integer>> &t, const Integer &a, const Integer &b)
{
    integer_class g_, s_, t_;
    mp_gcdext(g_, s_, t_, a.as_mpz(), b.as_mpz());
    *g = integer(std::move(g_));
    *s = integer(std::move(s_));
    *t = integer(std::move(t_));
}

RCP<const Integer> lcm(const Integer &a, const Integer &b)
{
    integer_class c;
    mp_lcm(c, a.as_mpz(), b.as_mpz());
    return integer(std::move(c));
}

int mod_inverse(const Ptr<RCP<const Integer>> &b, const Integer &a,
        const Integer &m)
{
    int ret_val;
    integer_class inv_t;
    ret_val = mp_invert(inv_t, a.as_mpz(), m.as_mpz());
    *b = integer(std::move(inv_t));
    return ret_val;
}

RCP<const Integer> mod(const Integer &n, const Integer &d)
{
    return integer(std::move(n.as_mpz() % d.as_mpz()));
}

RCP<const Integer> quotient(const Integer &n, const Integer &d)
{
    return integer(std::move(n.as_mpz() / d.as_mpz()));
}

void quotient_mod(const Ptr<RCP<const Integer>> &q, const Ptr<RCP<const Integer>> &r,
                    const Integer &n, const Integer &d)
{
    integer_class _q, _r;
    mp_tdiv_qr(_q, _r, n.as_mpz(), d.as_mpz());
    *q = integer(std::move(_q));
    *r = integer(std::move(_r));
}

RCP<const Integer> mod_f(const Integer &n, const Integer &d)
{
    integer_class q;
    mp_fdiv_r(q, n.as_mpz(), d.as_mpz());
    return integer(std::move(q));
}

RCP<const Integer> quotient_f(const Integer &n, const Integer &d)
{
    integer_class q;
    mp_fdiv_q(q, n.as_mpz(), d.as_mpz());
    return integer(std::move(q));
}

void quotient_mod_f(const Ptr<RCP<const Integer>> &q, const Ptr<RCP<const Integer>> &r,
                    const Integer &n, const Integer &d)
{
    integer_class _q, _r;
    mp_fdiv_qr(_q, _r, n.as_mpz(), d.as_mpz());
    *q = integer(std::move(_q));
    *r = integer(std::move(_r));
}

RCP<const Integer> fibonacci(unsigned long n)
{
    integer_class f;
    mp_fib_ui(f, n);
    return integer(std::move(f));
}

void fibonacci2(const Ptr<RCP<const Integer>> &g, const Ptr<RCP<const Integer>> &s,
        unsigned long n)
{
    integer_class g_t;
    integer_class s_t;
    mp_fib2_ui(g_t, s_t, n);
    *g = integer(std::move(g_t));
    *s = integer(std::move(s_t));
}

RCP<const Integer> lucas(unsigned long n)
{
    integer_class f;
    mp_lucnum_ui(f, n);
    return integer(std::move(f));
}

void lucas2(const Ptr<RCP<const Integer>> &g, const Ptr<RCP<const Integer>> &s,
        unsigned long n)
{
    integer_class g_t;
    integer_class s_t;
    mp_lucnum2_ui(g_t, s_t, n);
    *g = integer(std::move(g_t));
    *s = integer(std::move(s_t));
}

// Binomial Coefficient
RCP<const Integer> binomial(const Integer &n, unsigned long k)
{
    integer_class f;
    mp_bin_ui(f, n.as_mpz(), k);
    return integer(std::move(f));
}

// Factorial
RCP<const Integer> factorial(unsigned long n)
{
    integer_class f;
    mp_fac_ui(f, n);
    return integer(std::move(f));
}

// Returns true if `b` divides `a` without reminder
bool divides(const Integer &a, const Integer &b)
{
    return mp_divisible_p(a.as_mpz(), b.as_mpz()) != 0;
}

// Prime functions
int probab_prime_p(const Integer &a, unsigned reps)
{
    return mp_probab_prime_p(a.as_mpz(), reps);
}

RCP<const Integer> nextprime(const Integer &a)
{
    integer_class c;
    mp_nextprime(c, a.as_mpz());
    return integer(std::move(c));
}

namespace {
// Factoring by Trial division using primes only
int _factor_trial_division_sieve(integer_class &factor, const integer_class &N)
{
    integer_class sqrtN = mp_sqrt(N);
    unsigned long limit = mp_get_ui(sqrtN);
    if (limit > std::numeric_limits<unsigned>::max())
        throw std::runtime_error("N too large to factor");
    Sieve::iterator pi(limit);
    unsigned p;
    while ((p = pi.next_prime()) <= limit) {
        if (N % p == 0) {
            factor = p;
            return 1;
        }
    }
    return 0;
}
// Factor using lehman method.
int _factor_lehman_method(integer_class &rop, const integer_class &n)
{
    if (n < 21)
        throw std::runtime_error("Require n >= 21 to use lehman method");

    int ret_val = 0;
    integer_class u_bound;

    mp_root(u_bound, n, 3);
    u_bound = u_bound + 1;

    Sieve::iterator pi(mp_get_ui(u_bound));
    unsigned p;
    while ((p = pi.next_prime()) <= mp_get_ui(u_bound)) {
        if (n % p == 0) {
            rop = n / p;
            ret_val = 1;
            break;
        }
    }

    if (not ret_val) {

        integer_class k, a, b, l;

        k = 1;

        while (k <= u_bound) {
            a = mp_sqrt(4 * k * n);
            mp_root(b, n, 6);
            mp_root(l, k, 2);
            b = b / (4 * l);
            b = b + a;

            while (a <= b) {
                l = a * a - 4 * k * n;
                if (mp_perfect_square_p(l)) {
                    b = a + mp_sqrt(l);
                    mp_gcd(rop, n, b);
                    ret_val = 1;
                    break;
                }
                a = a + 1;
            }
            if (ret_val)
                break;
            k = k + 1;
        }
    }

    return ret_val;
}
} //anonymous namespace

int factor_lehman_method(const Ptr<RCP<const Integer>> &f, const Integer &n)
{
    int ret_val;
    integer_class rop;

    ret_val = _factor_lehman_method(rop, n.as_mpz());
    *f = integer(std::move(rop));
    return ret_val;
}

namespace {
// Factor using Pollard's p-1 method
int _factor_pollard_pm1_method(integer_class &rop, const integer_class &n,
            const integer_class &c, unsigned B)
{
    if (n < 4 or B < 3)
        throw std::runtime_error("Require n > 3 and B > 2 to use Pollard's p-1 method");

    integer_class m, _c;
    _c = c;

    Sieve::iterator pi(B);
    unsigned p;
    while ((p = pi.next_prime()) <= B) {
        m = 1;
        // calculate log(p, B), this can be improved
        while (m <= B / p) {
            m = m * p;
        }
        mp_powm(_c, _c, m, n);
    }
    _c = _c - 1;
    mp_gcd(rop, _c, n);

    if (rop == 1 or rop == n)
        return 0;
    else
        return 1;
}
}

int factor_pollard_pm1_method(const Ptr<RCP<const Integer>> &f, const Integer &n,
        unsigned B, unsigned retries)
{
    int ret_val = 0;
    integer_class rop, nm4, c;
    gmp_randstate_t state;

    gmp_randinit_default(state);
    gmp_randseed_ui(state, retries);
    nm4 = n.as_mpz() - 4;

    for (unsigned i = 0; i < retries and ret_val == 0; ++i) {
        mp_urandomm(c, state, nm4);
        c = c + 2;
        ret_val = _factor_pollard_pm1_method(rop, n.as_mpz(), c, B);
    }

    if (ret_val != 0)
        *f = integer(std::move(rop));
    gmp_randclear(state);
    return ret_val;
}

namespace {
// Factor using Pollard's rho method
int _factor_pollard_rho_method(integer_class &rop, const integer_class &n,
            const integer_class &a, const integer_class &s, unsigned steps = 10000)
{
    if (n < 5)
        throw std::runtime_error("Require n > 4 to use pollard's-rho method");

    integer_class u, v, g, m;
    u = s;
    v = s;

    for (unsigned i = 0; i < steps; ++i) {
        u = (u*u + a) % n;
        v = (v*v + a) % n;
        v = (v*v + a) % n;
        m = u - v;
        mp_gcd(g, m, n);

        if (g == n)
            return 0;
        if (g == 1)
            continue;
        rop = g;
        return 1;
    }
    return 0;
}
}

int factor_pollard_rho_method(const Ptr<RCP<const Integer>> &f,
        const Integer &n, unsigned retries)
{
    int ret_val = 0;
    integer_class rop, nm1, nm4, a, s;
    gmp_randstate_t state;

    gmp_randinit_default(state);
    gmp_randseed_ui(state, retries);
    nm1 = n.as_mpz() - 1;
    nm4 = n.as_mpz() - 4;

    for (unsigned i = 0; i < retries and ret_val == 0; ++i) {
        mp_urandomm(a, state, nm1);
        mp_urandomm(s, state, nm4);
        s = s + 1;
        ret_val = _factor_pollard_rho_method(rop, n.as_mpz(), a, s);
    }

    if (ret_val != 0)
        *f = integer(std::move(rop));
    gmp_randclear(state);
    return ret_val;
}

// Factorization
int factor(const Ptr<RCP<const Integer>> &f, const Integer &n, double B1)
{
    int ret_val = 0;
    integer_class _n, _f;

    _n = n.as_mpz();

#ifdef HAVE_SYMENGINE_ECM
    if (mp_perfect_power_p(_n)) {

        unsigned long int i = 1;
        integer_class m, rem;
        rem = 1; // Any non zero number
        m = 2; // set `m` to 2**i, i = 1 at the begining

        // calculate log2n, this can be improved
        for (; m < _n; ++i)
            m = m * 2;

        // eventually `rem` = 0 zero as `n` is a perfect power. `f_t` will
        // be set to a factor of `n` when that happens
        while (i > 1 and rem != 0) {
            mp_rootrem(_f, rem, _n, i);
            --i;
        }

        ret_val = 1;
    }
    else {

        if (mp_probab_prime_p(_n, 25) > 0) { // most probably, n is a prime
            ret_val = 0;
            _f = _n;
        }
        else {

            for (int i = 0; i < 10 and not ret_val; ++i)
                ret_val = ecm_factor(get_mpz_t(_f), get_mpz_t(_n), B1,
                        nullptr);
                mp_demote(_f);
            if (not ret_val)
                throw std::runtime_error("ECM failed to factor the given number");
        }
    }
#else
    // B1 is discarded if gmp-ecm is not installed
    ret_val = _factor_trial_division_sieve(_f, _n);
#endif // HAVE_SYMENGINE_ECM
    *f = integer(std::move(_f));

    return ret_val;
}

int factor_trial_division(const Ptr<RCP<const Integer>> &f, const Integer &n)
{
    int ret_val;
    integer_class factor;
    ret_val =_factor_trial_division_sieve(factor, n.as_mpz());
    if (ret_val == 1) *f = integer(std::move(factor));
    return ret_val;
}

void prime_factors(std::vector<RCP<const Integer>> &prime_list, const Integer &n)
{
    integer_class sqrtN;
    integer_class _n = n.as_mpz();
    if (_n == 0) return;
    if (_n < 0) _n *= -1;

    sqrtN = mp_sqrt(_n);
    auto limit = mp_get_ui(sqrtN);
    if (not mp_fits_ulong_p(sqrtN) or limit > std::numeric_limits<unsigned>::max())
        throw std::runtime_error("N too large to factor");
    Sieve::iterator pi(limit);
    unsigned p;

    while ((p = pi.next_prime()) <= limit) {
        while (_n % p == 0) {
            prime_list.push_back(integer(p));
            _n = _n / p;
        }
        if (_n == 1) break;
    }
    if (not (_n == 1))
        prime_list.push_back(integer(std::move(_n)));
}

void prime_factor_multiplicities(map_integer_uint &primes_mul, const Integer &n)
{
    integer_class sqrtN;
    integer_class _n = n.as_mpz();
    unsigned count;
    if (_n == 0) return;
    if (_n < 0) _n *= -1;

    sqrtN = mp_sqrt(_n);
    auto limit = mp_get_ui(sqrtN);
    if (not mp_fits_ulong_p(sqrtN) or limit > std::numeric_limits<unsigned>::max())
        throw std::runtime_error("N too large to factor");
    Sieve::iterator pi(limit);

    unsigned p;
    while ((p = pi.next_prime()) <= limit) {
        count = 0;
        while (_n % p == 0) { // when a prime factor is found, we divide
            ++count;                     // _n by that prime as much as we can
            _n = _n / p;
        }
        if (count > 0) {
            insert(primes_mul, integer(p), count);
            if (_n == 1) break;
        }
    }
    if (not (_n == 1))
        insert(primes_mul, integer(std::move(_n)), 1);
}

std::vector<unsigned> Sieve::_primes = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29};
bool Sieve::_clear = true;
unsigned Sieve::_sieve_size = 32 * 1024 * 8; //32K in bits

void Sieve::set_clear(bool clear)
{
    _clear = clear;
}

void Sieve::clear()
{
    _primes.erase(_primes.begin() + 10, _primes.end());
}

void Sieve::set_sieve_size(unsigned size) {
#ifdef HAVE_SYMENGINE_PRIMESIEVE
    primesieve::set_sieve_size(size);
#else
    _sieve_size = size * 1024 * 8; //size in bits
#endif
}

void Sieve::_extend(unsigned limit)
{
#ifdef HAVE_SYMENGINE_PRIMESIEVE
    if (_primes.back() < limit)
        primesieve::generate_primes(_primes.back() + 1, limit, &_primes);
#else
    const unsigned sqrt_limit = static_cast<unsigned>(std::sqrt(limit));
    unsigned start = _primes.back() + 1;
    if (limit <= start)
        return;
    if (sqrt_limit >= start) {
        _extend(sqrt_limit);
        start = _primes.back() + 1;
    }

    unsigned segment = _sieve_size;
    std::valarray<bool> is_prime(segment);
    for (; start <= limit; start += 2 * segment) {
        unsigned finish = std::min(start + segment * 2 + 1, limit);
        is_prime[std::slice(0, segment, 1)] = true;
        //considering only odd integers. An odd number n corresponds to n-start/2 in the array.
        for (unsigned index = 1; index < _primes.size() and
                    _primes[index] * _primes[index] <= finish; ++index) {
            unsigned n = _primes[index];
            unsigned multiple = (start / n + 1) * n;
            if (multiple % 2 == 0)
                multiple += n;
            if (multiple > finish)
                continue;
            std::slice sl = std::slice((multiple-start)/ 2, 1 + (finish - multiple) / (2 * n), n);
            //starting from n*n, all the odd multiples of n are marked not prime.
            is_prime[sl] = false;
        }
        for (unsigned n = start + 1; n <= finish; n += 2) {
            if (is_prime[(n - start) / 2])
                _primes.push_back(n);
        }
    }
#endif
}

void Sieve::generate_primes(std::vector<unsigned> &primes, unsigned limit)
{
    _extend(limit);
    auto it = std::upper_bound (_primes.begin(), _primes.end(), limit);
    //find the first position greater than limit and reserve space for the primes
    primes.reserve(it - _primes.begin());
    std::copy(_primes.begin(), it, std::back_inserter(primes));
    if (_clear)
        clear();
}

Sieve::iterator::iterator(unsigned max)
{
    _limit = max;
    _index = 0;
}

Sieve::iterator::iterator()
{
    _limit = 0;
    _index = 0;
}

Sieve::iterator::~iterator()
{
    if (_clear)
        Sieve::clear();
}

unsigned Sieve::iterator::next_prime()
{
    if (_index >= _primes.size()) {
        unsigned extend_to = _primes[_index - 1] * 2;
        if (_limit > 0 and _limit < extend_to) {
            extend_to = _limit;
        }
        _extend(extend_to);
        if (_index >= _primes.size()) {     //the next prime is greater than _limit
            return _limit + 1;
        }
    }
    return SymEngine::Sieve::_primes[_index++];
}

RCP<const Number> bernoulli(unsigned long n) {
#ifdef HAVE_SYMENGINE_ARB
    fmpq_t res;
    fmpq_init(res);
    bernoulli_fmpq_ui(res, n);
    mpq_t a;
    mpq_init(a);
    fmpq_get_mpq(a, res);
    rational_class b(a);
    fmpq_clear(res);
    mpq_clear(a);
    return Rational::from_mpq(std::move(b));
#else
    // TODO: implement a faster algorithm
    std::vector<rational_class> v(n+1);
    for (unsigned m = 0; m <= n; ++m) {
        v[m] = rational_class(1, m + 1);

        for (unsigned j = m; j >= 1; --j) {
            v[j - 1] = j * (v[j - 1] - v[j]);
        }
    }
    return Rational::from_mpq(v[0]);
#endif
}

RCP<const Number> harmonic(unsigned long n, long m) {
    rational_class res(0);
    if (m == 1) {
        for (unsigned i = 1; i <= n; ++i) {
            res += rational_class(1, i);
        }
        return Rational::from_mpq(res);
    } else {
        for (unsigned i = 1; i <= n; ++i) {
            if (m > 0) {
                rational_class t(1u, i);
                mp_pow_ui(get_den(t), get_den(t), m);
                res += t;
            } else {
                integer_class t(i);
                mp_pow_ui(t, t, -m);
                res += t;
            }
        }
        return Rational::from_mpq(res);
    }
}

// References : Cohen H., A course in computational algebraic number theory (1996), page 21.
bool crt(const Ptr<RCP<const Integer>> &R, const std::vector<RCP<const Integer>> &rem,
       const std::vector<RCP<const Integer>> &mod)
{
    if (mod.size() > rem.size())
        throw std::runtime_error("Too few remainders");
    if (mod.size() == 0)
        throw std::runtime_error("Moduli vector cannot be empty");

    integer_class m, r, g, s, t;
    m = mod[0]->as_mpz();
    r = rem[0]->as_mpz();

    for (unsigned i = 1; i < mod.size(); ++i) {
        mp_gcdext(g, s, t, m, mod[i]->as_mpz());
        // g = s * m + t * mod[i]
        t = rem[i]->as_mpz() - r;
        if (not mp_divisible_p(t, g))
            return false;
        r += m * s * (t / g);           // r += m * (m**-1 mod[i]/g)* (rem[i] - r) / g
        m *= mod[i]->as_mpz() / g;
        mp_fdiv_r(r, r, m);
    }
    *R = integer(std::move(r));
    return true;
}

namespace {
// Crt over a cartesian product of vectors (Assuming that moduli are pairwise relatively prime).
void _crt_cartesian(std::vector<RCP<const Integer>> &R, const std::vector<std::vector<RCP<const Integer>>> &rem,
            const std::vector<RCP<const Integer>> &mod)
{
    if (mod.size() > rem.size())
        throw std::runtime_error("Too few remainders");
    if (mod.size() == 0)
        throw std::runtime_error("Moduli vector cannot be empty");
    integer_class m, _m, r, s, t;
    m = mod[0]->as_mpz();
    R = rem[0];

    for (unsigned i = 1; i < mod.size(); ++i) {
        std::vector<RCP<const Integer>> rem2;
        mp_invert(s, m, mod[i]->as_mpz());
        _m = m;
        m *= mod[i]->as_mpz();
        for (auto & elem : R) {
            for (auto & _k : rem[i]) {
                r = elem->as_mpz();
                r += _m * s * (_k->as_mpz() - r);
                mp_fdiv_r(r, r, m);
                rem2.push_back(integer(r));
            }
        }
        R = rem2;
    }
}

// Tests whether n is a prime power and finds a prime p and e such that n = p**e.
bool _prime_power(integer_class &p, integer_class &e, const integer_class &n)
{
    if (n < 2)
        return false;
    integer_class _n = n, temp;
    e = 1;
    unsigned i = 2;
    while (mp_perfect_power_p(_n) and _n >= 2) {
        if (mp_root(temp, _n, i)) {
            e *= i;
            _n = temp;
        } else {
            ++i;
        }
    }
    if (mp_probab_prime_p(_n, 25)) {
        p = _n;
        return true;
    }
    return false;
}

// Computes a primitive root modulo p**e or 2*p**e where p is an odd prime.
// References : Cohen H., A course in computational algebraic number theory (2009), pages 25-27.
void _primitive_root(integer_class &g, const integer_class &p, const integer_class &e,
            bool even = false)
{
    std::vector<RCP<const Integer>> primes;
    prime_factors(primes, *integer(p - 1));

    integer_class t;
    g = 2;
    while (g < p) {
        bool root = true;
        for (const auto &it: primes) {
            t = it->as_mpz();
            t = (p - 1)/t;
            mp_powm(t, g, t, p);
            if (t == 1) {           // If g**(p-1)/q is 1 then g is not a primitive root.
                root = false;
                break;
            }
        }
        if (root)
            break;
        ++g;
    }

    if (e > 1) {
        t = p * p;
        integer_class pm1 = p - 1;
        mp_powm(t, g, pm1, t);
        if (t == 1) {               // If g**(p-1) mod (p**2) == 1 then g + p is a primitive root.
            g += p;
        }
    }
    if (even and g % 2 == 0) {
        mp_pow_ui(t, p, mp_get_ui(e));
        g += t;                     // If g is even then root of 2*p**e is g + p**e.
    }
}

} //anonymous namespace

bool primitive_root(const Ptr<RCP<const Integer>> &g, const Integer &n)
{
    integer_class _n = n.as_mpz();
    if (_n < 0)
        _n = -_n;
    if (_n <= 1)
        return false;
    if (_n < 5) {
        *g = integer(_n - 1);
        return true;
    }
    bool even = false;
    if (_n % 2 == 0) {
        if (_n % 4 == 0) {
            return false;    // If n mod 4 == 0 and n > 4, then no primitive roots.
        }
        _n /= 2;
        even = true;
    }
    integer_class p, e;
    if (not _prime_power(p, e, _n))
        return false;
    _primitive_root(_n, p, e, even);
    *g = integer(std::move(_n));
    return true;
}

namespace {
// Computes primitive roots modulo p**e or 2*p**e where p is an odd prime.
// References :
// [1] Cohen H., A course in computational algebraic number theory (1996), pages 25-27.
// [2] Hackman P., Elementary number theory (2009), page 28.
void _primitive_root_list(std::vector<RCP<const Integer>> &roots, const integer_class &p,
            const integer_class &e, bool even = false)
{
    integer_class g, h, d, t, pe2, n, pm1;
    _primitive_root(g, p, integer_class(1), false); // Find one primitive root for p.
    h = 1;
    pm1 = p - 1;
    // Generate other primitive roots for p. h = g**i and gcd(i, p-1) = 1. Ref[2]
    mp_pow_ui(n, p, mp_get_ui(e));
    for (unsigned long i = 1; i < p; ++i) {
        h *= g;
        h %= p;
        mp_gcd(d, pm1, integer_class(i));
        if (d == 1) {
            if (e == 1) {
                if (even and h % 2 == 0)
                    roots.push_back(integer(h + n));
                else
                    roots.push_back(integer(h));
            } else {
                integer_class pp = p * p;
                // Find d such that (h + d*p)**(p-1) mod (p**2) == 1. Ref[1]
                // h**(p-1) - 1 = d*p*h**(p-2)
                // d = (h - h**(2-p)) / p
                t = 2 - p;
                mp_powm(d, h, t, pp);
                d = ((h - d) / p + p) % p;
                t = h;
                // t = h + i * p + j * p * p and i != d
                mp_pow_ui(pe2, p, mp_get_ui(e) - 2);
                for (unsigned long j = 0; j < pe2; ++j) {
                    for (unsigned long i = 0; i < p; ++i) {
                        if (i != d) {
                            if (even and t % 2 == 0)
                                roots.push_back(integer(t + n));
                            else
                                roots.push_back(integer(t));
                        }
                        t += p;
                    }
                }
            }
        }
    }
} //_primitive_root_list
} //anonymous namespace

void primitive_root_list(std::vector<RCP<const Integer>> &roots, const Integer &n)
{
    integer_class _n = n.as_mpz();
    if (_n < 0)
        _n = -_n;
    if (_n <= 1)
        return;
    if (_n < 5) {
        roots.push_back(integer(_n - 1));
        return;
    }
    bool even = false;
    if (_n % 2 == 0) {
        if (_n % 4 == 0) {
            return;    // If n%4 == 0 and n > 4, then no primitive roots.
        }
        _n /= 2;
        even = true;
    }
    integer_class p, e;
    if (not _prime_power(p, e, _n))
        return;
    _primitive_root_list(roots, p, e, even);
    std::sort(roots.begin(), roots.end(), SymEngine::RCPIntegerKeyLess());
    return;
}

RCP<const Integer> totient(const RCP<const Integer> &n) {
    if (n->is_zero())
        return integer(1);

    integer_class phi = n->as_mpz(), p;
    if (phi < 0)
        phi = -phi;
    map_integer_uint prime_mul;
    prime_factor_multiplicities(prime_mul, *n);

    for (const auto &it: prime_mul) {
        p = it.first->as_mpz();
        mp_divexact(phi, phi, p);
        // phi is exactly divisible by p.
        phi *= p - 1;
    }
    return integer(std::move(phi));
}

RCP<const Integer> carmichael(const RCP<const Integer> &n) {
    if (n->is_zero())
        return integer(1);

    map_integer_uint prime_mul;
    integer_class lambda, t, p;
    unsigned multiplicity;

    prime_factor_multiplicities(prime_mul, *n);
    lambda = 1;
    for (const auto it : prime_mul) {
        p = it.first->as_mpz();
        multiplicity = it.second;
        if (p == 2 and multiplicity > 2) {     // For powers of 2 greater than 4 divide by 2.
            multiplicity--;
        }
        t = p - 1;
        mp_lcm(lambda, lambda, t);
        mp_pow_ui(t, p, multiplicity - 1);
        // lambda and p are relatively prime.
        lambda = lambda * t;
    }
    return integer(std::move(lambda));
}

// References : Cohen H., A course in computational algebraic number theory (1996), page 25.
bool multiplicative_order(const Ptr<RCP<const Integer>> &o, const RCP<const Integer> &a,
        const RCP<const Integer> &n)
{
    integer_class order, p, t;
    integer_class _a = a->as_mpz(), _n = mp_abs(n->as_mpz());
    mp_gcd(t, _a, _n);
    if (t != 1)
        return false;

    RCP<const Integer> lambda = carmichael(n);
    map_integer_uint prime_mul;
    prime_factor_multiplicities(prime_mul, *lambda);
    _a %= _n;
    order = lambda->as_mpz();

    for (const auto it : prime_mul) {
        p = it.first->as_mpz();
        mp_pow_ui(t, p, it.second);
        mp_divexact(order, order, t);
        mp_powm(t, _a, order, _n);
        while (t != 1) {
            mp_powm(t, t, p, _n);
            order *= p;
        }
    }
    *o = integer(std::move(order));
    return true;
}
int legendre(const Integer &a, const Integer &n)
{
    return mp_legendre(a.as_mpz(), n.as_mpz());
}

int jacobi(const Integer &a, const Integer &n)
{
    return mp_jacobi(a.as_mpz(), n.as_mpz());
}

int kronecker(const Integer &a, const Integer &n)
{
    return mp_kronecker(a.as_mpz(), n.as_mpz());
}

namespace {
bool _sqrt_mod_tonelli_shanks(integer_class &rop, const integer_class &a, const integer_class &p)
{
    gmp_randstate_t state;
    gmp_randinit_default(state);
    gmp_randseed(state, get_mpz_t(p));

    integer_class n, y, b, q, pm1, t(1);
    pm1 = p - 1;
    unsigned e, m;
    e = mp_scan1(pm1);
    q = pm1 >> e;    //p - 1 = 2**e*q

    while (t != -1) {
        mp_urandomm(n, state, p);
        mp_demote(n);
        t = mp_legendre(n, p);
    }
    mp_powm(y, n, q, p);   //y = n**q mod p
    mp_powm(b, a, q, p);   //b = a**q mod p
    t = (q + 1) / 2;
    mp_powm(rop, a, t, p);   //rop = a**((q + 1) / 2) mod p

    while (b != 1) {
        m = 0;
        t = b;
        while (t != 1) {
            mp_powm(t, t, integer_class(2), p);
            ++m;                                                            //t = t**2 = b**2**(m)
        }
        if(m == e)
            return false;
        mp_pow_ui(q, integer_class(2), e - m - 1);    //q = 2**(e - m - 1)
        mp_powm(t, y, q, p);   // t = y**(2**(e - m - 1))
        mp_powm(y, t, integer_class(2), p);    //y = t**2
        e = m;
        rop = (rop * t) % p;
        b = (b * y) % p;
    }
    return true;
}

bool _sqrt_mod_prime(integer_class &rop, const integer_class &a, const integer_class &p)
{
    if (p == 2) {
        rop = a % p;
        return true;
    }
    int l = mp_legendre(a, p);
    integer_class t;
    if (l == -1) {
        return false;
    } else if (l == 0) {
        rop = 0;
    } else if (p % 4 == 3) {
        t = (p + 1) / 4;
        mp_powm(rop, a, t, p);
    } else if (p % 8 == 5) {
        t = (p - 1) / 4;
        mp_powm(t, a, t, p);
        if (t == 1) {
            t = (p + 3) / 8;
            mp_powm(rop, a, t, p);
        } else {
            t = (p - 5) / 8;
            integer_class t1 = 4 * a;
            mp_powm(t, t1, t, p);
            rop = (2 * a * t) % p;
        }
    } else {
        if(p < 10000) { // If p < 10000, brute force is faster.
            integer_class sq = integer_class(1), _a;
            mp_fdiv_r(_a, a, p);
            for(unsigned i = 1; i < p; ++i) {
                if (sq == _a) {
                    rop = i;
                    return true;
                }
                sq += 2 * i + 1;
                mp_fdiv_r(sq, sq, p);
            }
            return false;
        } else {
            return _sqrt_mod_tonelli_shanks(rop, a, p);
        }
    }
    return true;
}

// References : Menezes, Alfred J., Paul C. Van Oorschot, and Scott A. Vanstone. Handbook of applied cryptography. CRC press, 2010. pages 104 - 108
// Calculates log = x mod q**k where g**x == a mod p and order(g, p) = n.
void _discrete_log(integer_class &log, const integer_class &a, const integer_class &g, const integer_class &n,
            const integer_class &q, const unsigned &k, const integer_class &p)
{
    log = 0;
    integer_class gamma = a, alpha, _n, t, beta, qj(1), m, l;
    _n = n / q;
    mp_powm(alpha, g, _n, p);
    mp_sqrtrem(m, t, q);
    if (t != 0)
        ++m;    // m = ceiling(sqrt(q)).
    map_integer_uint table; // Table for lookup in baby-step giant-step algorithm
    integer_class alpha_j(1), d, s;
    s = -m;
    mp_powm(s, alpha, s, p);

    for (unsigned j = 0; j < m; ++j) {
        insert(table, integer(alpha_j), j);
        alpha_j = (alpha_j * alpha) % p;
    }

    for (unsigned long j = 0; j < k; ++j) { // Pohlig-Hellman
        mp_powm(beta, gamma, _n, p);
        // Baby-step giant-step algorithm for l = log_alpha(beta)
        d = beta;
        bool found = false;
        for (unsigned i = 0; not found &&i < m; ++i) {
            if (table.find(integer(d)) != table.end()) {
                l = i * m + table[integer(d)];
                found = true;
                break;
            }
            d = (d * s) % p;
        }
        _n /= q;
        t = -l * qj;

        log -= t;
        mp_powm(t, g, t, p);
        gamma *= t; //gamma *= g ** (-l * (q ** j))
        qj *= q;
    }
}

// References : Johnston A., A generalised qth root algorithm.
// Solution for x**n == a mod p**k where a != 0 mod p and p is an odd prime.
bool _nthroot_mod1(std::vector<RCP<const Integer>> &roots, const integer_class &a, const integer_class &n,
            const integer_class &p, const unsigned k, bool all_roots = false)
{
    integer_class _n, r, root, s, t, g(0), pk, m, phi;
    mp_pow_ui(pk, p, k);
    phi = pk * (p - 1) / p;
    mp_gcd(m, phi, n);
    t = phi / m;
    mp_powm(t, a, t, pk);
    // Check whether a**(phi / gcd(phi, n)) == 1 mod p**k.
    if (t != 1) {
        return false;
    }
    // Solve x**n == a mod p first.
    t = p - 1;
    mp_gcdext(_n, r, s, n, t);
    if (r < 0) {
        mp_fdiv_r(r, r, t / _n);
    }
    mp_powm(s, a, r, p);

    // Solve x**(_n) == s mod p where _n | p - 1.
    if (_n == 1) {
        root = s;
    } else if (_n == 2) {
        _sqrt_mod_prime(root, s, p);
    } else { // Ref[1]
        map_integer_uint prime_mul;
        prime_factor_multiplicities(prime_mul, *integer(_n));
        integer_class h, q, qt, z, v, x, s1 = s;
        _primitive_root(g, p, integer_class(2));
        unsigned c;
        for (const auto &it: prime_mul) {
            q = it.first->as_mpz();
            mp_pow_ui(qt, q, it.second);
            h = (p - 1) / q;
            c = 1;
            while (h % q == 0) {
                ++c;
                h /= q;
            }
            mp_invert(t, h, qt);
            z = t * -h;
            x = (1 + z) / qt;
            mp_powm(v, s1, x, p);

            if (c == it.second) {
                s1 = v;
            } else {
                mp_powm(x, s1, h, p);
                t = h * qt;
                mp_powm(r, g, t, p);
                mp_pow_ui(qt, q, c - it.second);
                _discrete_log(t, x, r, qt, q, c - it.second, p);
                t = -z * t;
                mp_powm(r, g, t, p);
                v *= r;
                mp_fdiv_r(v, v, p);
                s1 = v;
            }
        }
        root = s1;
    }
    r = n;
    unsigned c = 0;
    while(r % p == 0) {
        mp_divexact(r, r, p);
        ++c;
    }

    // Solve s == x**r mod p**k where (x**r)**(p**c)) == a mod p**k
    integer_class pc = n / r, pd = pc * p;
    if (c >= 1) {
        mp_powm(s, root, r, p);
        // s == root**r mod p. Since s**(p**c) == 1 == a mod p**(c + 1), lift until p**k.
        for (unsigned d = c + 2; d <= k; ++d) {
            t = 1 - pc;
            pd *= p;
            mp_powm(t, s, t, pd);
            t = (a * t - s) / pc;
            s += t;
        }
    } else {
        s = a;
    }

    // Solve x**r == s mod p**k given that root**r == s mod p and r % p != 0.
    integer_class u;
    pd = p;
    for (unsigned d = 2; d < 2 * k; d *= 2) { // Hensel lifting
        t = r - 1;
        pd *= pd;
        if (d > k)
            pd = pk;
        mp_powm(u, root, t, pd);
        t = r * u;
        mp_invert(t, t, pd);
        root += (s - u * root) * t;
        mp_fdiv_r(root, root, pd);
    }
    if (m != 1 and all_roots) {
        // All roots are generated by root*(g**(phi / gcd(phi , n)))**j
        if (n == 2) {
            t = -1;
        } else {
            if (g == 0)
                _primitive_root(g, p, integer_class(2));
            t = phi / m;
            mp_powm(t, g, t, pk);
        }
        for (unsigned j = 0; j < m; ++j) {
            roots.push_back(integer(root));
            root *= t;
            mp_fdiv_r(root, root, pk);
        }
    } else {
        roots.push_back(integer(root));
    }
    return true;
}

// Solution for x**n == a mod p**k.
bool _nthroot_mod_prime_power(std::vector<RCP<const Integer>> &roots, const integer_class &a, const integer_class &n,
            const integer_class &p, const unsigned k, bool all_roots = false)
{
    integer_class pk, root;
    std::vector<RCP<const Integer>> _roots;
    if (a % p != 0) {
        if (p == 2) {
            integer_class r = n, t, s, pc, pj;
            pk = integer_class(1) << k;
            unsigned c = mp_scan1(n);
            r = n >> c; // n = 2**c * r where r is odd.

            // Handle special cases of k = 1 and k = 2.
            if (k == 1) {
                roots.push_back(integer(1));
                return true;
            }
            if (k == 2) {
                if (c > 0 and a % 4 == 3) {
                    return false;
                }
                roots.push_back(integer(std::move(a % 4)));
                if (all_roots and c > 0)
                    roots.push_back(integer(3));
                return true;
            }
            if (c >= k - 2) {
                c = k - 2;  // Since x**(2**c) == x**(2**(k - 2)) mod 2**k, let c = k - 2.
            }
            t = integer_class(1) << (k - 2);
            pc = integer_class(1) << c;

            mp_invert(s, r, t);
            if (c == 0) {
                // x**r == a mod 2**k and x**2**(k - 2) == 1 mod 2**k, implies x**(r * s) == x == a**s mod 2**k.
                mp_powm(root, a, s, pk);
                roots.push_back(integer(root));
                return true;
            }

            // First, solve for y**2**c == a mod 2**k where y == x**r
            t = integer_class(1) << (c + 2);
            mp_fdiv_r(t, a, t);
            // Check for a == y**2**c == 1 mod 2**(c + 2).
            if (t != 1)
                return false;
            root = 1;
            pj = pc * 4;
            // 1 is a root of x**2**c == 1 mod 2**(c + 2). Lift till 2**k.
            for (unsigned j = c + 2; j < k; ++j) {
                pj *= 2;
                mp_powm(t, root, pc, pj);
                t -= a;
                if (t % pj != 0)
                    // Add 2**(j - c).
                    root += integer_class(1) << (j - c);
            }
            // Solve x**r == root mod 2**k.
            mp_powm(root, root, s, pk);

            if (all_roots) {
                // All roots are generated by, root * (j * (2**(k - c) +/- 1)).
                t = pk / pc * root;
                for (unsigned i = 0 ; i < 2; ++i) {
                    for (unsigned long j = 0; j < pc; ++j) {
                        roots.push_back(integer(root));
                        root += t;
                    }
                    root = t - root;
                }
            } else {
                roots.push_back(integer(root));
            }
            return true;
        } else {
            return _nthroot_mod1(roots, a, n, p, k, all_roots);
        }
    } else {
        integer_class _a;
        mp_pow_ui(pk, p, k);
        _a = a % pk;
        unsigned m;
        integer_class pm;
        if (_a == 0) {
            if (not all_roots) {
                roots.push_back(integer(0));
                return true;
            }
            _roots.push_back(integer(0));
            if (n >= k)
                m = k - 1;
            else
                m = k - 1 - (k - 1) / mp_get_ui(n);
            mp_pow_ui(pm, p, m);
        } else {
            unsigned r = 1;
            mp_divexact(_a, _a, p);
            while (_a % p == 0) {
                mp_divexact(_a, _a, p);
                ++r;
            }
            if (r < n or r % n != 0 or not _nthroot_mod_prime_power(_roots, _a, n, p, k - r, all_roots)) {
                return false;
            }
            m = r / mp_get_ui(n);
            mp_pow_ui(pm, p, m);
            if (not all_roots) {
                roots.push_back(integer(_roots.back()->as_mpz() * pm));
                return true;
            }
            for (auto &it : _roots) {
                it = integer (it->as_mpz() * pm);
            }
            m = r - r / mp_get_ui(n);
            mp_pow_ui(pm, p, m);
        }
        integer_class pkm;
        mp_pow_ui(pkm, p, k - m);

        for (const auto &it : _roots) {
            root = it->as_mpz();
            for (unsigned long i = 0; i < pm; ++i) {
                roots.push_back(integer(root));
                root += pkm;
            }
        }
    }
    return true;
}
} //anonymous namespace

bool nthroot_mod(const Ptr<RCP<const Integer>> &root, const RCP<const Integer> &a,
        const RCP<const Integer> &n, const RCP<const Integer> &mod)
{
    if (mod->as_mpz() <= 0) {
        return false;
    } else if (mod->as_mpz() == 1) {
        *root = integer(0);
        return true;
    }
    map_integer_uint prime_mul;
    prime_factor_multiplicities(prime_mul, *mod);
    std::vector<RCP<const Integer>> moduli;
    bool ret_val;

    std::vector<RCP<const Integer>> rem;
    for (const auto &it: prime_mul) {
        integer_class _mod;
        mp_pow_ui(_mod, it.first->as_mpz(), it.second);
        moduli.push_back(integer(std::move(_mod)));
        ret_val = _nthroot_mod_prime_power(rem, a->as_mpz(), n->as_mpz(), it.first->as_mpz(), it.second, false);
        if(not ret_val) return false;
    }
    crt(root, rem, moduli);
    return true;
}

void nthroot_mod_list(std::vector<RCP<const Integer>> &roots, const RCP<const Integer> &a,
        const RCP<const Integer> &n, const RCP<const Integer> &mod)
{
    if (mod->as_mpz() <= 0) {
        return;
    } else if (mod->as_mpz() == 1) {
        roots.push_back(integer(0));
        return;
    }
    map_integer_uint prime_mul;
    prime_factor_multiplicities(prime_mul, *mod);
    std::vector<RCP<const Integer>> moduli;
    bool ret_val;

    std::vector<std::vector<RCP<const Integer>>> rem;
    for (const auto &it: prime_mul) {
        integer_class _mod;
        mp_pow_ui(_mod, it.first->as_mpz(), it.second);
        moduli.push_back(integer(std::move(_mod)));
        std::vector<RCP<const Integer>> rem1;
        ret_val = _nthroot_mod_prime_power(rem1, a->as_mpz(), n->as_mpz(), it.first->as_mpz(), it.second, true);
        if(not ret_val) return;
        rem.push_back(rem1);
    }
    _crt_cartesian(roots, rem, moduli);
    std::sort(roots.begin(), roots.end(), SymEngine::RCPIntegerKeyLess());
}

bool powermod(const Ptr<RCP<const Integer>> &powm, const RCP<const Integer> &a,
        const RCP<const Number> &b, const RCP<const Integer> &m)
{
    if(is_a<Integer>(*b)) {
        integer_class t = rcp_static_cast<const Integer>(b)->as_mpz();
        if (b->is_negative())
            t *= -1;
        mp_powm(t, a->as_mpz(), t, m->as_mpz());
        if (b->is_negative()) {
            bool ret_val = mp_invert(t, t, m->as_mpz());
            if (not ret_val)
                return false;
        }
        *powm = integer(std::move(t));
        return true;
    } else if (is_a<Rational>(*b)) {
        RCP<const Integer> num, den, r;
        get_num_den(static_cast<const Rational &>(*b), outArg(num), outArg(den));
        if (den->is_negative()) {
            den = den->mulint(*minus_one);
            num = num->mulint(*minus_one);
        }
        integer_class t = mp_abs(num->as_mpz());
        mp_powm(t, a->as_mpz(), t, m->as_mpz());
        if (num->is_negative()) {
            bool ret_val = mp_invert(t, t, m->as_mpz());
            if (not ret_val)
                return false;
        }
        r = integer(std::move(t));
        return nthroot_mod(powm, r, den, m);
    }
    return false;
}

void powermod_list(std::vector<RCP<const Integer>> &pows, const RCP<const Integer> &a,
        const RCP<const Number> &b, const RCP<const Integer> &m)
{
    if(is_a<Integer>(*b)) {
        integer_class t = mp_abs(rcp_static_cast<const Integer>(b)->as_mpz());
        mp_powm(t, a->as_mpz(), t, m->as_mpz());
        if (b->is_negative()) {
            bool ret_val = mp_invert(t, t, m->as_mpz());
            if (not ret_val)
                return;
        }
        pows.push_back(integer(std::move(t)));
    } else if (is_a<Rational>(*b)) {
        RCP<const Integer> num, den, r;
        get_num_den(static_cast<const Rational &>(*b), outArg(num), outArg(den));
        if (den->is_negative()) {
            den = den->mulint(*integer(-1));
            num = num->mulint(*integer(-1));
        }
        integer_class t = num->as_mpz();
        if (num->is_negative())
            t *= -1;
        mp_powm(t, a->as_mpz(), t, m->as_mpz());
        if (num->is_negative()) {
            bool ret_val = mp_invert(t, t, m->as_mpz());
            if (not ret_val)
                return;
        }
        r = integer(t);
        nthroot_mod_list(pows, r, den, m);
    }
}

int mobius(const Integer &a)
{
    if (a.as_int() <= 0) {
        throw std::runtime_error("mobius: Integer <= 0");
    }
    map_integer_uint prime_mul;
    bool is_square_free = true;
    prime_factor_multiplicities(prime_mul, a);
    int num_prime_factors = prime_mul.size();
    for (const auto &it: prime_mul) {
        int p_freq = it.second;
        if (p_freq > 1) {
            is_square_free = false;
            break;
        }
    }
    if (!is_square_free) {
        return 0;
    } else if (num_prime_factors % 2 == 0) {
        return 1;
    } else {
        return -1;
    }
}

long mertens(const unsigned long a)
{
    long mertens = 0;
    for (unsigned long i = 1; i<= a; ++i) {
        mertens += mobius(*(integer(i)));
    }
    return mertens;
}
} // SymEngine
