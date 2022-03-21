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

    void abs(){
        isNeg = false;
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
//    friend BigInteger operator*(const BigInteger &b1, const BigInteger &b2);


    BigInteger& operator*=(const BigInteger &b){
        size_t b1S = this->body.size();
        size_t b2S = b.body.size();
        BigInteger ans(b1S + b2S + 1);
        BigInteger t(b1S + 1,0);
        for (size_t iR = 0; iR < b2S; ++iR) {
            ans<<=1;
            memcpy(&t.body[0], &(*this).body[0], sizeof(size_t) * b1S);
            t.body[b1S] = 0;
            BigInteger::mul_to_short(t, b.body[b2S - 1 - iR]);
            ans+=t;
        }
        ans.isNeg = this->isNeg != b.isNeg;
        *this = ans;
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

//    friend std::istream &operator>>(std::istream &in, BigInteger &b); // ?
//    friend std::ostream &operator<<(std::ostream &out, const BigInteger &b); // ?

    bool IsNeg() const{
        return isNeg;
    }

    explicit operator bool() const {
        return !(body.empty() || (body.size() == 1 && body[0] == 0));
    } // variant of static conversion

//    std::vector<size_t> GetBody() const{ return body; }


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
    BigInteger copy = b2;
    copy *= b1;
    return copy;
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
//    friend std::istream &operator>>(std::istream &in, Rational &b);
//    friend std::istream &operator<<(std::istream &in, Rational &b);
    explicit operator bool() const {
        return (bool)num;
    } // variant of static conversion
    std::string toString() const{
        Rational t = *this;
        std::string ans = "";
        if (!num) ans = "0";
        else{
            if (isNeg) ans+='-';
            ans += num.toString();
            if (den != 1){
                ans+='/';
                ans += den.toString();
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
