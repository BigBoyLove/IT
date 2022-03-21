 
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <cmath>
#include <iostream>
#include <vector>
#include <string>
#include <memory.h>

class BigInteger {
public:
    static const long long BASE = 1ull << 32;

    BigInteger() = default;

    BigInteger(int x) :isNeg(x<0) {
        body.push_back(x * (1 + (-2) * isNeg));
    }

    explicit BigInteger(unsigned long long x) {
        x *= (1 + (-2) * isNeg);
        body.push_back(x % BASE);
        x >>= 32;
        if (x) body.push_back(x);
    }

    explicit BigInteger(size_t n) {
        body.reserve(n);
        body.push_back(0);
    }

    BigInteger(size_t n, size_t val) {
        body.resize(n, val);
    }

    explicit BigInteger(const char *s, unsigned radix = 10){
        if (s[0] == '-') {
            isNeg = true;
            ++s;
        }
        body.push_back(0);
        while (*s != '\0') {
            mul_to_short(*this, radix);
            body[0] += (unsigned) (*s - '0');
            ++s;
        }
    }

    BigInteger(const std::string &s){
        size_t i = 0;
        if (s[0] == '-') {
            isNeg = true;
            ++i;
        }
        body.push_back(0);
        for (; i < s.size(); ++i) {
            mul_to_short(*this, 10);
            body[0] += (unsigned) (s[i] - '0');
        }
    }

    BigInteger(const BigInteger &b): body(std::vector<size_t>(b.body.size())),isNeg(b.isNeg) {
        memcpy(&body[0], &b.body[0], sizeof(size_t) * b.body.size());
    }

    BigInteger& operator=(const BigInteger & b){
        BigInteger copy = b;
        std::swap(body,copy.body);
        std::swap(isNeg,copy.isNeg);
        return *this;
    }

    void abs(){
        isNeg = false;
    }

    std::string toString() const{
        std::string ans;
        if (!(*this)){ // == 0
            ans='0';
        }
        else{
            if (isNeg) ans+='-';
            ans += toStringUnsigned();
        }
        return ans;
    }

    std::string toStringUnsigned() const{
        BigInteger t = *this;
        std::string ans;
        if (!t){
            ans='0';
        }
        else {
            while (t) {
                ans += "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[fast_div_to_short_and_get_mod(t, 10)];
            }
        }
        Reverse(ans);
        return ans;
    }

    BigInteger operator -() const {
        BigInteger copy(*this);
        copy.isNeg = !copy.isNeg;
        return copy;
    }

    BigInteger &operator+=(const BigInteger &b) {
        if (isNeg == b.isNeg){
            addToUnsigned(*this, b);
            return *this;
        }
        else {
            subToUnsigned(*this, b);
            return *this;
        }

    }
    BigInteger &operator-=(const BigInteger &b) {
        if (isNeg == b.isNeg){
            subToUnsigned(*this, b);
            return *this;
        }
        else{
            addToUnsigned(*this, b);
            return *this;
        }
    }

    BigInteger &operator++() {
        *this+=1;
        return *this;
    } // prefix
    BigInteger operator++(int) {
        BigInteger copy = *this;
        *this+=1;
        return copy;
    } // postfix
    BigInteger &operator--() {
        *this-=1;
        return *this;
    } // prefix
    BigInteger operator--(int) {
        BigInteger copy = *this;
        *this-=1;
        return copy;
    } // postfix

    BigInteger &operator<<=(size_t shift) {
        for (size_t j = 0; j < shift; ++j) {
            size_t sz = body.size();
            if (body[sz - 1]) body.push_back(body[sz - 1]);
            for (size_t i = 1; i < sz; ++i) {
                body[sz - i] = body[sz - i - 1];
            }
            body[0] = 0;
        }
        return *this;
    }
    friend BigInteger operator*(const BigInteger &b1, const BigInteger &b2);


    BigInteger& operator*=(const BigInteger &b){
        BigInteger t = *this * b;
        *this = t;
        return *this;
    }

    BigInteger& operator/=(const BigInteger &b2){
        const BigInteger b1 = *this;
        size_t b1S = b1.body.size();
        BigInteger mod(b2.body.size() + 1);
        BigInteger t(b2.body.size() + 1); // for binary search
        long long curDivider, l, r, m;

        for (size_t i = 0; i < b1S; ++i) {
            mod<<=1;
            mod.body[0] = b1.body[b1S - 1 - i];
            curDivider = 0;
            l = 0;
            r = BigInteger::BASE;
            while (l <= r) {    // find divider
                m = (l + r) >> 1;
                BigInteger::fast_copy_body(t,b2);
                BigInteger::mul_to_short(t,m);
                if (BigInteger::cmpUnsigned(t, mod) <= 0) {
                    curDivider = m;
                    l = m + 1;
                } else r = m - 1;
            }
            body[b1S - 1 - i] = curDivider;
            BigInteger::fast_copy_body(t, b2);
            BigInteger::mul_to_short(t, curDivider);
            BigInteger::removeLeadingZeros(t);

            BigInteger::subToUnsigned(mod,t);
            BigInteger::removeLeadingZeros(mod);
        }
        BigInteger::removeLeadingZeros(*this);

        isNeg = b1.isNeg != b2.isNeg;
//        if (isNeg && mod) {
//            --*this;
//        }
        return *this;
    }

    BigInteger& operator%=(const BigInteger &b);

    friend std::istream &operator>>(std::istream &in, BigInteger &b); // ?
    friend std::ostream &operator<<(std::ostream &out, const BigInteger &b); // ?

    bool IsNeg() const{
        return isNeg;
    }

    explicit operator bool() const {
        return !(body.empty() || (body.size() == 1 && body[0] == 0));
    } // variant of static conversion

    static unsigned fast_div_to_short_and_get_mod(BigInteger &b, unsigned x) {
        size_t old_mod = 0, t = 0;
        size_t bU = b.body.size();
        for (size_t i = 0; i < bU; ++i) {
            t = b.body[bU - 1 - i] + (old_mod << 32);
            b.body[bU - 1 - i] = t / x;
            old_mod = t % x;
        }
        removeLeadingZeros(b);
        return t % x;
    }


    std::vector<size_t> GetBody() const{ return body; }

    static long long cmpUnsigned(const BigInteger& left,const BigInteger& right) {
        size_t lU = left.body.size(), rU = right.body.size();
        if (lU < rU) return -1;
        if (lU == rU) {
            return (cmpBodiesEqualSz(left.body, right.body, lU));
        }
        return 1;
    }

    static void swap(BigInteger &b1, BigInteger &b2){
        std::swap(b1.body,b2.body);
        std::swap(b1.isNeg,b2.isNeg);
    }

private:

    static void Reverse(std::string &s){
        size_t n = s.size();
        for (size_t i = 0; i < n / 2; ++i) {
            std::swap(s[i],s[n- i - 1]);
        }
    }

    std::vector<size_t> body; // from min to max degree
    bool isNeg = false;


    static void removeLeadingZeros(BigInteger &b) {
        size_t bU = b.body.size();
        size_t i = 0;
        while (i < bU - 1 && b.body[bU - 1 - i] == 0){
            b.body.pop_back();
            i++;
        }
    }

    static void mul_to_short(BigInteger &b, size_t x) {
        size_t carry = 0;
        for (size_t t = 0, i = 0; i < b.body.size(); ++i) {
            t = b.body[i] * x + carry;
            b.body[i] = t % BASE;
            carry = t >> 32;
        }
        if (carry >= 1) {
            b.body.push_back(carry);
        }
    }

    static void fast_copy_body(BigInteger &b,const BigInteger &orig) {
        b.body.resize(orig.body.size());
        memcpy(&b.body[0], &orig.body[0], sizeof(size_t) * orig.body.size());
//        for (size_t i = orig.body.size(); i < b.body.size(); ++i) {
//            b.body[i] = 0;
//        }
    }

//    static long long* castToLL(const std::vector<size_t> &v){
//        auto *nV = new long long[v.size()];
//        memcpy(nV, &v[0], sizeof(size_t) * v.size());
//        return nV;
//    }

    static long long cmpBodiesEqualSz(const std::vector<size_t> &v1, const std::vector<size_t> &v2, size_t sz) {
        for (long long i = (long long) sz - 1; i >= 0; --i) {
            long long t = static_cast<long long>(v1[i]) - static_cast<long long>(v2[i]);
            if (t != 0){
                return t;
            }
        }
        return 0;
    }



    static void addToUnsigned(BigInteger &to, const BigInteger &b){
        const std::vector<size_t> & minBody = to.body.size() < b.body.size() ? to.body : b.body;
        const std::vector<size_t> & maxBody = to.body.size() >= b.body.size() ? to.body : b.body;
        //        std::vector<size_t> BigInteger::* pMinBody = body.size() >= b.body.size() ? this.&BigInteger::body : b.&BigInteger::body;
        size_t i = 0;
        unsigned carry = 0;
        for (; i < minBody.size(); ++i) {
            size_t t = to.body[i] + b.body[i] + carry;
            to.body[i] = t % BASE;
            carry = t >> 32;
        }
        size_t mBS = maxBody.size();
        to.body.resize(mBS + 1);
        while (i < mBS) {
            size_t t = maxBody[i] + carry;
            to.body[i] = t % BASE;
            carry = t >> 32;
            ++i;
        }
        to.body[mBS] = carry;
        removeLeadingZeros(to);
    }

    static void subToUnsigned(BigInteger &to, const BigInteger &b){
        bool thisLesserB = cmpUnsigned(to, b) < 0;
        const std::vector<size_t> & minBody = thisLesserB ? to.body : b.body;
        const std::vector<size_t> & maxBody = !thisLesserB ? to.body : b.body;
        to.isNeg = thisLesserB ? !to.isNeg : to.isNeg;

//        if (maxBodyLL == nullptr || maxBodyLL == nullptr);

        int own = 0;
        long long t;
        size_t i = 0;
        for (; i < minBody.size(); ++i) {
            t = static_cast<long long>(maxBody[i]) - static_cast<long long>(minBody[i]) - own;
            int tLesserZero = t < 0;
            own = 1 * tLesserZero;
            t += BASE * tLesserZero;
            to.body[i] = t;
        }
        size_t mBS = maxBody.size();
        to.body.resize(mBS);
        while (i < mBS) {
            t = static_cast<long long>(maxBody[i]) - own;
            int tLesserZero = t < 0;
            own = 1 * tLesserZero;
            t += BASE * tLesserZero;
            to.body[i] = t;
            ++i;
        }
        removeLeadingZeros(to);
    }
};

BigInteger operator+(const BigInteger &b1, const BigInteger &b2) {
    BigInteger copy = b1;
    copy += b2;
    return copy;
}

BigInteger operator-(const BigInteger &b1, const BigInteger &b2) {
    BigInteger copy = b1;
    copy -= b2;
    return copy;
}

BigInteger operator*(const BigInteger &b1, const BigInteger &b2){
    size_t b1S = b1.body.size();
    size_t b2S = b2.body.size();
    BigInteger ans(b1S + b2S + 1);
    BigInteger t(b1S + 1,0);
    for (size_t iR = 0; iR < b2S; ++iR) {
        ans<<=1;
        memcpy(&t.body[0], &b1.body[0], sizeof(size_t) * b1S);
        t.body[b1S] = 0;
        BigInteger::mul_to_short(t, b2.body[b2S - 1 - iR]);
        ans+=t;
    }
    ans.isNeg = b1.isNeg != b2.isNeg;
    return ans;
}

BigInteger operator/(const BigInteger &b1, const BigInteger &b2){
    BigInteger copy = b1;
    copy /= b2;
    return copy;
}

BigInteger& BigInteger::operator%=(const BigInteger &b){
    *this -= (*this / b) * b;
    return *this;
}

BigInteger operator%(const BigInteger &b1, const BigInteger &b2){
    BigInteger copy = b1;
    copy %= b2;
    return copy;
}

bool operator<(const BigInteger &b1, const BigInteger &b2) {
    if (b1.IsNeg() == b2.IsNeg()){
        if(!b1.IsNeg()) return BigInteger::cmpUnsigned(b1,b2) < 0;
        return BigInteger::cmpUnsigned(b1,b2) > 0;
    }
    else{
        return b1.IsNeg();
    }
}

bool operator>(const BigInteger &b1, const BigInteger &b2) {
    return b2 < b1;
}

bool operator==(const BigInteger &b1, const BigInteger &b2) {
    if ((b1.IsNeg() != b2.IsNeg()) || BigInteger::cmpUnsigned(b1,b2) != 0) return false;
    return true;
}
bool operator!=(const BigInteger &b1, const BigInteger &b2) {
    return !(b1 == b2);
}

bool operator<=(const BigInteger &b1, const BigInteger &b2) {
    return !(b1 > b2);
}

bool operator>=(const BigInteger &b1, const BigInteger &b2) {
    return !(b1 < b2);
}

std::ostream &operator<<(std::ostream &out, const BigInteger &b) {
//    BigInteger copy = b;
//    std::string s;
//    if (!b) out << 0;
//    else {
//        if (b.isNeg) out << '-';
//        while (copy) {
//            s += "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[BigInteger::fast_div_to_short_and_get_mod(copy, 10)];
//        }
//        size_t sz = s.size();
//        for (size_t i = 0; i < sz; ++i) {
//            out<<s[sz - 1 - i];
//        }
//    }
    out<<b.toString();
    return out;
}

std::istream &operator>>(std::istream &in, BigInteger &b) {
    std::string s;
    in >> s;
//    size_t i = 0;
//    if (s[0] == '-') {
//        b.isNeg = true;
//        ++i;
//    }
//    b.body.push_back(0);
//    for (; i < s.size(); ++i) {
//        BigInteger::mul_to_short(b, 10);
//        b.body[0] += (unsigned) (s[i] - '0');
//    }
    b = s;
    return in;
}

BigInteger operator "" _bi(unsigned long long x){
    BigInteger b(x);
    return b;
}

class Rational{
public:
    Rational() = default;
    Rational(int x): num(BigInteger(abs(x))), den(BigInteger(1)), isNeg(x<0){}
    Rational(BigInteger b):num(b),den(BigInteger(1)), isNeg(b.IsNeg()){
        num.abs();
    }

    Rational(const Rational &r):num(r.num),den(r.den), isNeg(r.isNeg){
        num.abs();
        den.abs();
    }

    Rational& operator=(const Rational & r){
        Rational copy = r;
        std::swap(num,copy.num);
        std::swap(den,copy.den);
        std::swap(isNeg,copy.isNeg);
        return *this;
    }

    Rational &operator+=(const Rational &r) {
        if (isNeg == r.isNeg){
            addToUnsigned(*this, r);
            return *this;
        }
        else {
            subToUnsigned(*this, r);
            return *this;
        }

    }
    Rational &operator-=(const Rational &r) {
        if (isNeg == r.isNeg){
            subToUnsigned(*this, r);
            return *this;
        }
        else{
            addToUnsigned(*this, r);
            return *this;
        }
    }

    Rational &operator*=(const Rational &r){
        num*=r.num;
        den*=r.den;
        simplify(num,den);
        isNeg^=r.isNeg;
        return *this;
    }
    Rational &operator/=(const Rational &r){
        num*=r.den;
        den*=r.num;
        simplify(num,den);
        isNeg^=r.isNeg;
        return *this;
    }

    Rational operator -() const {
        Rational copy(*this);
        copy.isNeg = !copy.isNeg;
        return copy;
    }
    friend std::istream &operator>>(std::istream &in, Rational &b);
    friend std::istream &operator<<(std::istream &in, Rational &b);
    explicit operator bool() const {
        return (bool)num;
    } // variant of static conversion
    std::string toString() const{
        Rational t = *this;
        std::string ans = "";
        if (!num) ans = "0";
        else{
            if (isNeg) ans+='-';
            ans += num.toStringUnsigned();
            if (den != 1){
                ans+='/';
                ans += den.toStringUnsigned();
            }
        }
        return ans;
    }

    std::string asDecimal(size_t precision=0) const{
        BigInteger t = this->num;
        for (size_t i = 0; i <= precision; ++i) {
            t *= 10;
        }
        t /= this->den;
        std::string ans;
        bool hasDigitsGreaterZero = false;
        if(precision){
            unsigned tmp = BigInteger::fast_div_to_short_and_get_mod(t,10) / 5;
            for (size_t i = 0; i < precision; ++i) {
                tmp += BigInteger::fast_div_to_short_and_get_mod(t,10);
                hasDigitsGreaterZero |= (tmp != 0);
                ans += std::to_string(tmp%10);
                tmp/=10;
            }
            t+=static_cast<int>(tmp);
            hasDigitsGreaterZero |= (t != 0);
            ans+='.';
        }
        if (!t){
            ans+='0';
        }
        else {
            while (t) {
                ans += "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[BigInteger::fast_div_to_short_and_get_mod(t, 10)];
            }
        }
        if (isNeg && hasDigitsGreaterZero) {
            ans += '-';
        }
        Reverse(ans);
//        std::cerr<<(*this).toString()<<'\n';
//        std::cerr<<ans<<'\n';

        return ans;
    }
//    explicit operator double() const {
//        BigInteger t = this->num;
//
//        for (size_t i = 0; i <= 16; ++i) {
//            t *= 10;
//        }
//        t /= this->den;
//        double ans = static_cast<double>(BigInteger::fast_div_to_short_and_get_mod(t,10) / 5 + BigInteger::fast_div_to_short_and_get_mod(t,10));
//        ans/=10;
//        for (size_t i = 0; i < 15; ++i) {
//            ans += BigInteger::fast_div_to_short_and_get_mod(t,10);
//            ans/=10;
//        }
//        size_t deg10 = 1;
//        while (t != 0) {
//            ans += static_cast<double>(deg10 * BigInteger::fast_div_to_short_and_get_mod(t,10));
//            deg10 *= 10;
//        }
//        if (isNeg) {
//            ans = -ans;
//        }
//        return ans;
//    }

    explicit operator double() const {
        return std::stod(this->asDecimal(16));
    }

    bool IsNeg() const{
        return isNeg;
    }

private:
    BigInteger num;
    BigInteger den;
    bool isNeg = false;

    static BigInteger gcd(const BigInteger &b1, const BigInteger &b2){
        BigInteger copy1 = b1;
        BigInteger copy2 = b2;
        while (copy2){
            copy1 %= copy2;
            BigInteger::swap(copy1,copy2);
        }
        return copy1;
    }

    static void Reverse(std::string &s){
        size_t n = s.size();
        for (size_t i = 0; i < n / 2; ++i) {
            std::swap(s[i],s[n- i - 1]);
        }
    }

    static void simplify(BigInteger &num, BigInteger &den){
        BigInteger gcd = Rational::gcd(num, den);
        num /= gcd;
        den /= gcd;
    }

    static void addToUnsigned(Rational &to, const Rational &r){
        to.num*=r.den;
        to.num += (to.den*r.num);
        to.den *= r.den;

        simplify(to.num,to.den);
    }

    static void subToUnsigned(Rational &to, const Rational &r){
        to.num*=r.den;
        to.num -= (to.den*r.num);
        to.isNeg = to.num.IsNeg() ? !to.isNeg : to.isNeg;
        to.num.abs();
        to.den *= r.den;

        simplify(to.num,to.den);
    }
};

Rational operator+(const Rational &r1,const Rational &r2){
    Rational copy = r1;
    copy += r2;
    return copy;
}

Rational operator-(const Rational &r1,const Rational &r2){
    Rational copy = r1;
    copy -= r2;
    return copy;
}

Rational operator*(const Rational &r1,const Rational &r2){
    Rational copy = r1;
    copy *= r2;
    return copy;
}

Rational operator/(const Rational &r1,const Rational &r2){
    Rational copy = r1;
    copy /= r2;
    return copy;
}

bool operator<(const Rational &b1, const Rational &b2) {
    Rational t = b1 - b2;
    return t.IsNeg();
}

bool operator>(const Rational &b1, const Rational &b2) {
    return b2 < b1;
}

bool operator==(const Rational &b1, const Rational &b2) {
    return !bool(b1 - b2);
}
bool operator!=(const Rational &b1, const Rational &b2) {
    return !(b1 == b2);
}

bool operator<=(const Rational &b1, const Rational &b2) {
    return !(b1 > b2);
}

bool operator>=(const Rational &b1, const Rational &b2) {
    return !(b1 < b2);
}

// SQRT and IsPrimeHelper for IsPrime. O(log n)
template<size_t N, size_t K>
struct SQRT {
    static const size_t val = (K * K < N) ? (K + 1) : SQRT<N, K - 1>::val;
};

template<size_t N>
struct SQRT<N, 2> {
    static const size_t val = 2;
};

template<>
struct SQRT<1, 1> {
    static const size_t val = 1;
};

template<size_t N, size_t K>
struct IsPrimeHelper {
    static const bool val = N % K != 0 && IsPrimeHelper<N, K - 1>::val;
};

template<size_t N>
struct IsPrimeHelper<N, 1> {
    static const bool val = true;
};

template<>
struct IsPrimeHelper<2, 2> {
    static const bool val = true;
};

template<>
struct IsPrimeHelper<1, 1> {
    static const bool val = false;
};

template<size_t N>
struct IsPrime {
    static const bool val = IsPrimeHelper<N, SQRT<N, N>::val>::val;
};

// Example of Field. What can do: arithmetic and cout <<
template<size_t N>
class Residue {
public:
    explicit Residue(int x) {
        val = (x % (long long) N + N) % N;
    }

    Residue() : val(0) {}

    explicit operator int() const {
        return val;
    }

    Residue(const Residue &orig) : val(orig.val) {}

    Residue<N> &operator=(const Residue<N> &b) {
        val = b.val;
        return *this;
    }

    Residue<N> operator-() const {
        return (Residue<N>(0) - *this);
    }

    Residue<N> &operator+=(const Residue<N> &b) {
        val = (val + b.val) % N;
        return *this;
    }

    Residue<N> operator+(const Residue<N> &b) const {
        Residue<N> copy(*this);
        copy += b;
        return copy;
    }

    Residue<N> &operator-=(const Residue<N> &b) {
        val = (val + N - b.val) % N;
        return *this;
    }

    Residue<N> operator-(const Residue<N> &b) const {
        Residue<N> copy(*this);
        copy -= b;
        return copy;
    }

    size_t toString() const {
        return val;
    }

    Residue<N> &operator++() {
        *this += 1;
        return *this;
    } // prefix
    Residue<N> operator++(int) {
        Residue<N> copy = *this;
        *this += 1;
        return copy;
    } // postfix
    Residue<N> &operator--() {
        *this -= 1;
        return *this;
    } // prefix
    Residue<N> operator--(int) {
        Residue<N> copy = *this;
        *this -= 1;
        return copy;
    } // postfix

    Residue<N> &operator*=(const Residue<N> &b) {
        val = (val * b.val) % N;
        return *this;
    }

    Residue<N> operator*(const Residue<N> &b) const {
        Residue<N> copy(*this);
        copy *= b;
        return copy;
    }

    Residue<N> &operator/=(const Residue<N> &b) {
        static_assert(IsPrime<N>::val, "The Residue is not prime"); // for CE if N is not prime(Only Prime has reversible division)
        *this *= pow(b, N - 2);
        return *this;
    }

    Residue<N> operator/(const Residue<N> &b) const {
        Residue<N> copy(*this);
        copy /= b;
        return copy;
    }

    bool operator==(const Residue<N> &b) const {
        return val == b.val;
    }

    bool operator!=(const Residue<N> &b) const {
        return !(val == b.val);
    }

    bool operator<(const Residue<N> &b) const {
        return val < b.val;
    }

    bool operator>(const Residue<N> &b) const {
        return (b < *this);
    }

    bool operator<=(const Residue<N> &b) const {
        return !(*this > b);
    }

    bool operator>=(const Residue<N> &b) const {
        return !(*this < b);
    }

    template<size_t N1> // without that won't see operator<<
    friend std::ostream &operator<<(std::ostream &out, const Residue<N1> &r);

private:
    size_t val;

    static Residue<N> pow(const Residue<N> &b, size_t p) { // binary
        Residue<N> copy(b);
        Residue<N> ans(1);
        while (p) {
            if (p & 1)
                ans *= copy;
            copy *= copy;
            p >>= 1;
        }
        return ans;
    }
};

template<size_t N>
std::ostream &operator<<(std::ostream &out, const Residue<N> &r) {
    out << r.val;
    return out;
}

template<size_t M, size_t N, typename Field=Rational> // m rows n cols
class Matrix {
public:
    Matrix(){
        static_assert(N==M,"Nelzia");
        body = createE(N);
    }

    template<typename T>
    explicit Matrix(const std::vector<std::vector<T>> &v) : body(std::vector<std::vector<Field>>(M)) {
        for (size_t i = 0; i < M; ++i) {
            body[i] = std::vector<Field>(N);
            for (size_t j = 0; j < N; ++j) {
                body[i][j] = Field(v[i][j]);
            }
        }
    }

    explicit Matrix(size_t t) : body(std::vector<std::vector<Field>>(M)) {
        for (size_t i = 0; i < M; ++i) {
            body[i] = std::vector<Field>(N, Field(t));
        }
    }

    Matrix<M, N, Field>(const Matrix<M, N, Field> &orig) : body(std::vector<std::vector<Field>>(M)) {
        body = orig.body;
    }

    template<typename T>
    Matrix(const std::initializer_list<std::initializer_list<T>> &list) : body(std::vector<std::vector<Field>>(M)) {
        size_t i = 0;
        for (auto it1 = list.begin(); it1 != list.end(); ++it1, ++i) {
            size_t j = 0;
            body[i] = std::vector<Field>(N);
            for (auto it2 = it1->begin(); it2 != it1->end(); ++it2, ++j)
                body[i][j] = Field(*it2);
        }
    }

    size_t rank() const {
        std::vector<std::vector<Field>> copy;
        copy = body;
        return gaussForwardAndGetRang(copy);
    }

    Matrix<N, M, Field> transposed() const {
        std::vector<std::vector<Field>> v(N);
        for (size_t i = 0; i < N; ++i) {
            v[i] = std::vector<Field>();
            v[i] = getColumn(i);
        }
        return Matrix<N, M, Field>(v);
//        Matrix<N,M,Field> ans;
//        for (size_t i = 0; i < N; ++i) {
//            for (size_t j = 0; j < M; ++j) {
//                ans[i][j] = body[j][i];
//            }
//        }
//        return ans;
    }

    Field trace() const {
        Field ans = Field(0);
        size_t minim = M + (N - M) * (N < M);

        for (size_t i = 0; i < minim; ++i) {
            ans += body[i][i];
        }
        return ans;
    }

    Matrix<M, N, Field> &operator+=(const Matrix<M, N, Field> &m) {
        for (size_t i = 0; i < M; ++i) {
            for (size_t j = 0; j < N; ++j) {
                body[i][j] += m[i][j];
            }
        }
        return *this;
    }

    Matrix<M, N, Field> &operator-=(const Matrix<M, N, Field> &m) {
        for (size_t i = 0; i < M; ++i) {
            for (size_t j = 0; j < N; ++j) {
                body[i][j] -= m[i][j];
            }
        }
        return *this;
    }

    Matrix<M, N, Field> &operator*=(const Field &x) {
        for (size_t i = 0; i < M; ++i) {
            for (size_t j = 0; j < N; ++j) {
                body[i][j] *= x;
            }
        }
        return *this;
    }

    Matrix<M, N, Field> operator*(const Field &x) const {
        Matrix<M, N, Field> copy = *this;
        copy *= x;
        return copy;
    }

    std::vector<Field> getRow(unsigned id) const {
//        std::vector<Field> row(N);
//        for (size_t i = 0; i < N; ++i) {
//            row.push_back(body[id][i]);
//        }
//        return row;
//        std::cerr<<body[id][0].toString()<<'\n';
        return body[id];
    }

    std::vector<Field> getColumn(unsigned id) const {
        std::vector<Field> col(M);
        for (size_t i = 0; i < M; ++i) {
            col[i] = body[i][id];
        }
        return col;
    }

    std::vector<Field> &operator[](size_t id) {
        return body[id];
    }

    std::vector<Field> operator[](size_t id) const {
        return body[id];
    }

    std::vector<std::vector<Field>>
    getBody() const {                                                     // really need?
        return body;
    }

    template<size_t M1, size_t N1, typename Field1>
    friend std::ostream &operator<<(std::ostream &out, const Matrix<M1, N1, Field1> &m);

    std::vector<std::vector<Field>> body;

    void static Print(const std::vector<std::vector<Field>> &v) {
        for (size_t i = 0; i < v.size(); ++i) {
            for (size_t j = 0; j < v[0].size(); ++j) {
                std::cout << v[i][j].toString() << " ";
            }
            std::cout << '\n';
        }
    }


    void invert() {

        static_assert(N==M,"Nelzia");
//        if (std::is_same_v<Field, Rational>)
//            std::cerr<<*this;
        std::vector<std::vector<Field>> copy = createE(N);

        std::swap(copy, body);
//        Matrix<N,N,Field>::Print(copy);
//        std::cout<<'\n';
//        Matrix<N,N,Field>::Print(body);
        size_t rows = copy.size();
        size_t cols = copy[0].size();
        for (size_t col = 0, row = 0; col < cols && row < rows; ++col) {
            size_t iPivot = row; // pivot = elem in row not zero
            while ((iPivot < rows) && (copy[iPivot][col] == Field(0))) {
                ++iPivot;
            }
            if (iPivot == rows) continue; // no not-zero elems in col

            if (iPivot != row) {
                for (size_t i = col; i < cols; ++i) {
                    std::swap(copy[iPivot][i], copy[row][i]);
                }
                for (size_t i = 0; i < cols; ++i) {
                    std::swap(body[iPivot][i], body[row][i]);
                }
            }


            for (size_t i = 0; i < rows; ++i) {
                if (i != row) {
                    Field c = copy[i][col] / copy[row][col];
                    for (size_t j = col; j < cols; ++j) {
                        copy[i][j] -= copy[row][j] * c;
                    }
                    for (size_t j = 0; j < cols; ++j) {
                        body[i][j] -= body[row][j] * c;
                    }
                } else {
                    Field pivot = copy[row][col];
                    for (size_t j = col; j < cols; ++j) {
                        copy[i][j] /= pivot;
                    }
                    for (size_t j = 0; j < cols; ++j) {
                        body[i][j] /= pivot;
                    }
                }
            }
//            std::cout<<'\n';
//            std::cout<<"_______________"<<'\n';
//            std::cout<<"row "<<row<<'\n';
//            Matrix<N,N,Field>::Print(copy);
//            std::cout<<'\n';
//            Matrix<N,N,Field>::Print(body);
            ++row;
        }
    }

    Matrix<N, N, Field> inverted() const {

        static_assert(N==M,"Nelzia");
        Matrix inv(*this);
        inv.invert();
        return inv;
    }

    Field det() const {

        static_assert(N==M,"Nelzia");
        std::vector<std::vector<Field>> copy;
        copy = body;

        size_t rows = copy.size();
        size_t cols = copy[0].size();
        size_t row = 0; // in the end = rang
        Field det = Field(1);
        for (size_t col = 0; col < cols && row < rows; ++col) {
            size_t iPivot = row; // pivot = elem in row not zero
            while ((iPivot < rows) && (copy[iPivot][col] == Field(0))) {
                ++iPivot;
            }
            if (iPivot == rows) continue; // no not-zero elems in col

            if (iPivot != row) {
                for (size_t i = col; i < cols; ++i)
                    std::swap(copy[iPivot][i], copy[row][i]);
                det *= Field(-1);  // det changes sign when we swap strings

            }

            for (size_t i = row + 1; i < rows; ++i) {   // det doesn't change when we add do T(c)
                Field c = copy[i][col] / copy[row][col];
                for (size_t j = col; j < cols; ++j)
                    copy[i][j] -= copy[row][j] * c;
            }
//            std::cout<<"row: "<< row<<'\n';
//            Print(v);
            ++row;
        }

        if (row < N) return Field(0);

        for (size_t i = 0; i < N; ++i) {
            det *= copy[i][i];
        }
        return det;
    }

    Matrix<N, N, Field> &operator*=(const Matrix<N, N, Field> &m) {

        static_assert(N==M,"Nelzia");
        Matrix<N, N, Field> copy = *this;
        *this = copy * m;
        return *this;
    }

private:
//    using Matrix<N, N, Field>::body;
    size_t static gaussForwardAndGetRang(std::vector<std::vector<Field>> &v) {
        size_t rows = v.size();
        size_t cols = v[0].size();
        size_t row = 0; // in the end = rang
        for (size_t col = 0; col < cols && row < rows; ++col) {
            size_t iPivot = row; // pivot = elem in row not zero
            while ((iPivot < rows) && (v[iPivot][col] == Field(0))) {
                ++iPivot;
            }
            if (iPivot == rows) continue; // no not-zero elems in col

            if (iPivot != row) {
                for (size_t i = col; i < cols; ++i)
                    std::swap(v[iPivot][i], v[row][i]);
            }

            for (size_t i = row + 1; i < rows; ++i) {
                Field c = v[i][col] / v[row][col];
                for (size_t j = col; j < cols; ++j)
                    v[i][j] -= v[row][j] * c;
            }
//            std::cout<<"row: "<< row<<'\n';
//            Print(v);
            ++row;
        }
        return row;
    }

    void gauss(std::vector<std::vector<Field>> &v) {
        static_assert(N==M,"Nelzia");
        size_t rows = v.size();
        size_t cols = v[0].size();
        for (size_t col = 0, row = 0; col < cols && row < rows; ++col) {
            size_t iPivot = row; // pivot = elem in row not zero
            while ((iPivot < rows) && (v[iPivot][col] == 0)) {
                ++iPivot;
            }
            if (iPivot == rows) continue; // no not-zero elems in col

            for (size_t i = col; i < cols; ++i) // if has found
                swap(v[iPivot][i], v[row][i]);

            for (size_t i = 0; i < rows; ++i)
                if (i != row) {
                    Field c = v[i][col] / v[row][col];
                    for (size_t j = col; j < cols; ++j)
                        v[i][j] -= v[row][j] * c;
                }
            ++row;
        }
    }

    std::vector<std::vector<Field>> createE(size_t sz) const {
        std::vector<std::vector<Field>> copy(sz);
        for (size_t i = 0; i < sz; ++i) {
            copy[i] = std::vector<Field>(sz, Field(0));
        }
        for (size_t i = 0; i < sz; ++i) {
            copy[i][i] = Field(1);
        }
        return copy;
    }
};

template<size_t M, size_t N, typename Field>
std::ostream &operator<<(std::ostream &out, const Matrix<M, N, Field> &m) {
    std::cout.precision(16);
    for (size_t i = 0; i < M; ++i) {
        for (size_t j = 0; j < N; ++j) {
            out << m[i][j].toString() << " ";
        }
        out << '\n';
    }
    return out;
}

template<size_t M, size_t N, typename Field=Rational>
Matrix<M, N, Field> operator+(const Matrix<M, N, Field> &m1, const Matrix<M, N, Field> &m2) {
    Matrix<M, N, Field> copy = m1;
    copy += m2;
    return copy;
}

template<size_t M, size_t N, typename Field=Rational>
Matrix<M, N, Field> operator-(const Matrix<M, N, Field> &m1, const Matrix<M, N, Field> &m2) {
    Matrix<M, N, Field> copy = m1;
    copy -= m2;
    return copy;
}

template<size_t M, size_t N, size_t P, typename Field=Rational>
Matrix<M, P, Field> operator*(const Matrix<M, N, Field> &m1, const Matrix<N, P, Field> &m2) {
    Matrix<M, P, Field> m(0);
    for (size_t i = 0; i < M; ++i) {
        for (size_t j = 0; j < P; ++j) {
            for (size_t k = 0; k < N; ++k) {
                m[i][j] += (m1[i][k] * m2[k][j]);
            }
        }
    }
    return m;
}
std::istream &operator>>(std::istream &in, Rational &b) {
    std::string s;
    in >> s;
    if(s[0] == '-'){
        b.isNeg = true;
        s = s.substr(1,s.size());
    }
    b.num = s;
    b.den = 1;
    return in;
}

template<size_t M, size_t N, typename Field=Rational>
Matrix<M, N, Field> operator*(const Field &x, const Matrix<M, N, Field> &m) {
    return m * x;
}

template<size_t M, size_t N, typename Field=Rational>
bool operator==(const Matrix<M, N, Field> &m1, const Matrix<M, N, Field> &m2) {
    bool notEqual = false;
    for (size_t i = 0; i < M; ++i) {
        for (size_t j = 0; j < N; ++j) {
            notEqual |= (m1[i][j] != m2[i][j]);
        }
    }
    return !notEqual;
}

template<size_t M, size_t N, typename Field=Rational>
bool operator!=(const Matrix<M, N, Field> &m1, const Matrix<M, N, Field> &m2) {
    return !(m1 == m2);
}

template<size_t N, typename Field = Rational>
using SquareMatrix = Matrix<N,N,Field>;
