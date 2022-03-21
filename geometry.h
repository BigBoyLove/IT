#include <iostream>
#include <vector>
#include <cmath>

const double EPS = 1e-5;

class Line;

struct Point {
    Point() = default;

    Point(double x, double y) : x(x), y(y) {}

    Point(const Point &p) : x(p.x), y(p.y) {}

    double x;
    double y;

    Point &operator+=(const Point &p) {
        x += p.x;
        y += p.y;
        return *this;
    }

    Point &operator-=(const Point &p) {
        x -= p.x;
        y -= p.y;
        return *this;
    }

    Point &operator*=(double d) {
        x *= d;
        y *= d;
        return *this;
    }

    Point &operator/=(double d) {
        if (std::abs(d) < EPS) {
//            std::cout << "divide by 0";
            return *this;
        }
        x /= d;
        y /= d;
        return *this;
    }

    Point operator-() const {
        return Point(-x, -y);
    }

    double abs() const {
        return sqrt(x * x + y * y);
    }

    void rotate(const Point &center, double angle) {
        angle *= M_PI / 180;
        *this -= center;
        double new_x = x * cos(angle) - y * sin(angle);
        y = x * sin(angle) + y * cos(angle);
        x = new_x;
        *this += center;
    }

    void reflect(const Point &center) {
        x += 2 * (center.x - x);
        y += 2 * (center.y - y);
    }

    void scale(const Point &center, double coefficient) {
        x += (coefficient - 1) * (x - center.x);
        y += (coefficient - 1) * (y - center.y);
    }

    void reflect(const Line &axis);
};

Point operator+(const Point &p1, const Point &p2) {
    Point copy = p1;
    copy += p2;
    return copy;
}

Point operator*(const Point &p, double d) {
    Point copy = p;
    copy *= d;
    return copy;
}

Point operator*(double d, const Point &p) {
    Point copy = p;
    copy *= d;
    return copy;
}

Point operator/(const Point &p, double d) {
    Point copy = p;
    copy /= d;
    return copy;
}

Point operator/(double d, const Point &p) {
    Point copy = p;
    copy /= d;
    return copy;
}

Point operator-(const Point &p1, const Point &p2) {
    Point copy = p1;
    copy -= p2;
    return copy;
}

bool operator==(const Point &p1, const Point &p2) {
    return (std::abs(p1.x - p2.x) < EPS) && (std::abs(p1.y - p2.y) < EPS);
}

bool operator!=(const Point &p1, const Point &p2) {
    return !(p1 == p2);
}

std::ostream &operator<<(std::ostream &out, const Point &p) {
    out << "(" << p.x << "," << p.y << ")";
    return out;
}

struct Vector : Point {
    using Point::x;
    using Point::y;

    void norm() {
        double abs = this->abs();
        if (abs < EPS) {
//            std::cout << "abs = 0" << '\n';
            return;
        }
        x /= abs;
        y /= abs;
    }

    explicit Vector(const Point &p, bool norm = true) : Point::Point(p.x, p.y) {
        if (norm) this->norm();
    }

    explicit Vector(double x, double y, bool norm = true) : Point::Point(x, y) {
        if (norm) this->norm();
    }

    Vector(const Vector &v, bool norm = true) : Point::Point(v.x, v.y) { if (norm) this->norm(); }

    explicit operator bool() const {
        return !(std::abs(x) < EPS && std::abs(y) < EPS);
    }

};

double scalarProduct(const Vector &v1, const Vector &v2) {
    return v1.x * v2.x + v1.y * v2.y;
}

double vectorProduct(const Vector &v1, const Vector &v2) {
    return v1.x * v2.y - v1.y * v2.x;
}

class Line {
public:
    Line(const Point &p1, const Point &p2) : a(p2 - p1, true), r0(p1) {}

    Line(double k, double b) : a(1, k, true), r0(0, b) {}

    Line(double k, const Point &p) : a(1, k, true), r0(p) {}

    Line(const Point &p, double k) : a(1, k, true), r0(p) {}

    Vector GetA() const { return a; }

    Point GetR0() const { return r0; }

private:
    Vector a;
    Point r0;
};

void Point::reflect(const Line &axis) {
    double a, b, c, p;
    a = axis.GetA().y;
    b = -axis.GetA().x;
    c = axis.GetA().x * axis.GetR0().y - axis.GetA().y * axis.GetR0().x;
    Vector norm = Vector(a, b, true);
    p = (a * x + b * y + c);
    *this -= 2 * p / sqrt(a * a + b * b) * norm;
}

bool operator==(const Line &l1, const Line &l2) {
    Vector r = Vector(l2.GetR0() - l1.GetR0(), true);
    Vector a = l1.GetA();
    return ((a == l2.GetA()) || (a == -l2.GetA())) && (Vector(l2.GetR0() - l1.GetR0(), false) || (r == a) || (r == -a));
}

bool operator!=(const Line &l1, const Line &l2) {
    return !(l1 == l2);
}

class Shape {
public:
    virtual double perimeter() = 0;

    virtual double area() = 0;

    virtual bool isCongruentTo(const Shape &another) = 0;

    virtual bool isSimilarTo(const Shape &another) = 0;

    virtual bool containsPoint(const Point &p) = 0;

    virtual bool operator==(const Shape &another) const = 0;

    bool operator!=(const Shape &s) const { return !(*(this) == s); }

    virtual void rotate(const Point &center, double angle) = 0;

    virtual void reflect(const Point &center) = 0;

    virtual void reflect(const Line &axis) = 0;

    virtual void scale(const Point &center, double coefficient) = 0;

    virtual ~Shape() = default;
};

struct Polygon : Shape {
    Polygon() = default;

    template<typename ...T>
    Polygon(const Point &p, T... args): Polygon(args...) {
        verts.push_back(p);
    }

    Polygon(const std::vector<Point> &v) : verts(v) {
//        memcpy(&verts[0], &v[0], sizeof(Point) * v.size());
    }

    ~Polygon() override = default;

    double perimeter() override {
        double p = 0;
        size_t n = verticesCount();
        for (size_t i = 1; i < n; ++i) {
            p += (verts[i - 1] - verts[i]).abs();
        }
        return p;
    }

    double area() override {
        double s = 0;
        size_t n = verticesCount();
        for (size_t i = 0; i < n; ++i) {
            s += (verts[i].x * (verts[(i + 1) % n].y - verts[(i + n - 1) % n].y));
        }
        return std::abs(s) / 2;
    }

    bool containsPoint(const Point &p) override {
//        std::cerr<<'2';
//        std::cerr<<p.x<<','<<p.y<<"||";
//        for (size_t i = 0; i < verticesCount(); ++i) {
//            std::cerr<<verts[i].x<<','<<verts[i].y<<"|";
//        }
//        std::cerr<<']';

        size_t intersections = 0;
        size_t n = verticesCount();
        double x1 = p.x, x2 = 1000.0001, y1 = p.y, y2 = 0.001;
        for (size_t i = 1; i <= n; ++i) {
            double x3 = verts[i % n].x, x4 = verts[i - 1].x, y3 = verts[i % n].y, y4 = verts[i - 1].y;
            double det = (y4 - y3) * (x1 - x2) - (x4 - x3) * (y1 - y2);
            if (std::abs(det) < EPS) // прямые парал или сопад ( проверяем, лежит ли вершина на другом отрезке)
                intersections += (
                        (std::abs((x1 * y2 - x2 * y1) * (x4 - x3) - (x3 * y4 - x4 * y3) * (x2 - x1)) < EPS) &&
                        (std::abs((x1 * y2 - x2 * y1) * (y4 - y3) - (x3 * y4 - x4 * y3) * (y2 - y1)) < EPS));
            else { // правило Крамера
                double k1 = ((x4 - x2) * (y4 - y3) - (x4 - x3) * (y4 - y2)) / det;
                double k2 = ((x1 - x2) * (y4 - y2) - (x4 - x2) * (y1 - y2)) / det;
                intersections += (-EPS < k1 && k1 < 1 + EPS && -EPS < k2 && k2 < 1 + EPS);
            }
        }
        return intersections % 2;
    }

    bool isCongruentTo(const Shape &s) override {
        const Polygon *p = dynamic_cast<const Polygon *>(&s);
        size_t n = verticesCount();
        if (p == nullptr || p->verticesCount() != n) return false;
        bool congruent = false;
        size_t i = 1;
        while (!congruent && i <= n) {
            bool t = true;
            for (size_t j = 1; j < n; ++j) {
                t &= (std::abs((verts[(i - 1 + j) % n] - verts[(i + j) % n]).abs() -
                               (p->verts[(i - 1 + j) % n] - p->verts[(i + j) % n]).abs()) < EPS);
            }
            congruent |= t;
            ++i;
        }
        i = 1;
        while (!congruent && i <= n) { // если записаны в последовательности наооборот
            bool t = true;
            for (size_t j = 1; j < n; ++j) {
                t &= (std::abs((verts[(n - 1 - j + i - 1) % n] - verts[(n - 1 - j + i) % n]).abs() -
                               (p->verts[(i - 1 + j) % n] - p->verts[(i + j) % n]).abs()) < EPS);
            }
            congruent |= t;
            ++i;
        }
        return congruent;
    }

    bool isSimilarTo(const Shape &s) override {
        const Polygon *p = dynamic_cast<const Polygon *>(&s);
        size_t n = verticesCount();
        if (p == nullptr || p->verticesCount() != n) return false;
        bool similar = false;
        size_t i = 1;
        double len1 = (verts[i - 1] - verts[i]).abs();
        while (!similar && i <= n) {
            bool t = true;
            double k = len1 / (p->verts[i - 1] - p->verts[i % n]).abs();
            for (size_t j = 1; j < n; ++j) {
                t &= (std::abs(k - (verts[(i - 1 + j) % n] - verts[(i + j) % n]).abs() /
                                   (p->verts[(i - 1 + j) % n] - p->verts[(i + j) % n]).abs()) < EPS);
            }
            similar |= t;
            ++i;
        }
        i = 1;
        while (!similar && i <= n) { // если записаны в последовательности наооборот
            bool t = true;
            double k = len1 / (p->verts[i - 1] - p->verts[i % n]).abs();
            for (size_t j = 1; j < n; ++j) {
                t &= (std::abs(k - (verts[(n - 1 - j + i - 1) % n] - verts[(n - 1 - j + i) % n]).abs() /
                                   (p->verts[(i - 1 + j) % n] - p->verts[(i + j) % n]).abs()) < EPS);
            }
            similar |= t;
            ++i;
        }
        return similar;
    }


    size_t verticesCount() const { return verts.size(); }

    std::vector<Point> getVertices() const { return verts; }

    bool isConvex() const {
        bool signVec = vectorProduct(Vector(verts[1] - verts[0], false), Vector(verts[2] - verts[1], false)) >= 0;
        bool isConvex = true;
        size_t i = 3;
        size_t n = verticesCount();
        while (i < n && isConvex) {
            bool t = signVec;
            signVec =
                    vectorProduct(Vector(verts[i - 1] - verts[i - 2], false), Vector(verts[i] - verts[i - 1], false)) >=
                    0;
            isConvex &= (signVec == t);
            ++i;
        }
        return isConvex;
    }

    bool operator==(const Shape &s) const override {
        const Polygon *p = dynamic_cast<const Polygon *>(&s);
        size_t n = verticesCount();
        if (p == nullptr || p->verticesCount() != n) return false;
        bool equal = false;
        size_t i = 0;

        while (!equal && i < n) {
            bool t = true;
            for (size_t j = 0; j < n; ++j) {
                t &= (p->verts[(j + i) % n] == verts[j]);
            }
            equal |= t;
            ++i;
        }
        i = 0;
        while (!equal && i < n) { // если записаны в последовательности наооборот
            bool t = true;
            for (size_t j = 0; j < n; ++j) {
                t &= (p->verts[(n - 1 - j + i) % n] == verts[j]);
            }
            equal |= t;
            ++i;
        }
        return equal;
    }

//    bool operator!=(const Shape &s) const { return !((*this) == s); }

    void rotate(const Point &center, double angle) override {
        size_t n = verticesCount();
        for (size_t i = 0; i < n; ++i) verts[i].rotate(center, angle);
    }

    void reflect(const Point &center) override {
        size_t n = verticesCount();
        for (size_t i = 0; i < n; ++i) verts[i].reflect(center);
    }

    void reflect(const Line &axis) override {
        size_t n = verticesCount();
        for (size_t i = 0; i < n; ++i) verts[i].reflect(axis);
    }

    void scale(const Point &center, double coefficient) override {
        size_t n = verticesCount();
        for (size_t i = 0; i < n; ++i) verts[i].scale(center, coefficient);
    }

protected:
    std::vector<Point> verts;
};

struct Ellipse : Shape {
public:
    Ellipse(const Point &f1, const Point &f2, double s) : f1(f1), f2(f2), a(s / 2) {}

    ~Ellipse() override = default;

    std::pair<Point, Point> focuses() const { return {f1, f2}; }

    std::pair<Line, Line> directrices() const {
        double x = a / eccentricity();
        auto n = Vector((f1 - f2).y, -(f1 - f2).x, true);
        auto v = Vector(f1 - f2, true);
        auto p1 = center() - v * x;
        auto p2 = center() + v * x;
        return {Line(p1, p1 + n), Line(p2, p2 + n)};
    }

    double eccentricity() const {
        return c() / a;
    }

    Point center() const { return (f1 + f2) / 2; }

    bool operator==(const Shape &s) const override {
        const Ellipse *e = dynamic_cast<const Ellipse *>(&s);
        if (e == nullptr) return false;
        return (a == e->a) && (((f1 == e->f1) && (f2 == e->f2)) || ((f1 == e->f2) && (f2 == e->f1)));
    }


    double perimeter() override {
        double _b = b();
//        double ab = (a - _b);
//        return 4 * (M_PI * a * _b + ab*ab) / (a + _b);
        return M_PI * (3 * (a + _b) - sqrt((3 * a + _b) * (a + 3 * _b)));
    }

    double area() override {
        return M_PI * a * b();
    }

    bool containsPoint(const Point &p) override {
//        std::cerr<<"aaaaa";
//        double MF1 = (f1 - p).abs();
//        double MF2 = (f2 - p).abs();
        return ((f1 - p).abs() + (f2 - p).abs() - 2 * a < EPS);
    }

    bool isCongruentTo(const Shape &s) override {
        const Ellipse *e = dynamic_cast<const Ellipse *>(&s);
        if (e == nullptr) return false;
        Vector v1(f2 - f1, false);
        Vector v2(e->f2 - e->f1, false);
        return (std::abs(v2.abs() - v1.abs()) < EPS) && (std::abs(e->a - a) < EPS);
    }

    bool isSimilarTo(const Shape &s) override {
        const Ellipse *e = dynamic_cast<const Ellipse *>(&s);
        if (e == nullptr) return false;
        Vector v1(f2 - f1, false);
        Vector v2(e->f2 - e->f1, false);
        return (v2.abs() * a - v1.abs() * e->a) < EPS; // отношение
//        (v2 == v1) || (v2 == -v1)) &&
    }

    void rotate(const Point &center, double angle) override {
        f1.rotate(center, angle);
        f2.rotate(center, angle);
    }

    void reflect(const Point &center) override {
        f1.reflect(center);
        f2.reflect(center);
    }

    void reflect(const Line &axis) override {
        f1.reflect(axis);
        f2.reflect(axis);
    }

    void scale(const Point &center, double coefficient) override {
        f1.scale(center, coefficient);
        f2.scale(center, coefficient);
        a *= coefficient;
    }


protected:
    Point f1;
    Point f2;
    double a;

    double c() const { return (f1 - f2).abs() / 2; }

    double b() const {
        double _c = c();
        return sqrt(a * a - _c * _c);
    }

};

struct Circle : Ellipse {
    using Ellipse::a;

    ~Circle() override = default;

    Circle(const Point &c, double r) : Ellipse::Ellipse(c, c, 2 * r) {}

    double radius() const { return a; }
};

std::ostream &operator<<(std::ostream &out, const Circle &r) {
    out << r.center() << " " << r.radius() << '\n';
    return out;
}

struct Rectangle : Polygon {
    Rectangle(const Point &p1, const Point &p2, double ratio) : Polygon::Polygon(p1, p1, p2, p2) {
        if (ratio > 1) ratio = 1 / ratio;
        double angle = atan(ratio) * 180 / M_PI;
        auto diag = Vector(p2 - p1, false);
        Vector v1(diag, true);
        Vector v2(-diag, true);
        v1.rotate(Point(0, 0), -angle);
        v2.rotate(Point(0, 0), -angle);
        Point p3 = p1 + scalarProduct(v1, Vector(diag, false)) * v1;
        Point p4 = p2 + scalarProduct(v2, Vector(-diag, false)) * v2;
        Polygon::verts[0] = p4;
        Polygon::verts[2] = p3;
    }

    ~Rectangle() override = default;

    Point center() const {
        return (verts[0] + verts[2]) / 2;
    }

    std::pair<Line, Line> diagonals() const { return {Line(verts[0], verts[2]), Line(verts[3], verts[4])}; }


};

std::ostream &operator<<(std::ostream &out, const Rectangle &r) {
    std::vector<Point> verts = r.getVertices();
    out << verts[0] << " " << verts[1] << " " << verts[2] << " " << verts[3] << '\n';
    return out;
}

struct Square : Rectangle {
    Square(const Point &p1, const Point &p2) : Rectangle::Rectangle(p1, p2, 1) {}

    ~Square() override = default;

    Circle circumscribedCircle() const {
        return Circle((verts[0] + verts[2]) / 2, sqrt(2) / 2 * (verts[0] - verts[1]).abs());
    }

    Circle inscribedCircle() const { return Circle((verts[0] + verts[2]) / 2, (verts[0] - verts[1]).abs() / 2); }

};


struct Triangle : Polygon {
    Triangle(const Point &p1, const Point &p2, const Point &p3) : Polygon::Polygon(p1, p2, p3) {}

    ~Triangle() override = default;

    Circle circumscribedCircle() const {
        Point c = GetCircumscribedCircleCenter();
        return Circle(c, (c - verts[0]).abs());
    } // опис

    Circle inscribedCircle() const {
        double xA = verts[0].x, yA = verts[0].y, xB = verts[1].x, yB = verts[1].y, xC = verts[2].x, yC = verts[2].y;
//        double x0 = ((xA + xB - 2 * xC) * ((yC - yA) * (xB + xC - 2 * xA) + xA * (yB + yC - 2 * yA)) -
//                     xC * (xB + xC - 2 * xA) * (yA + yB - 2 * yC)) /
//                    ((yB + yC - 2 * yA) * (xA + xB - 2 * xC) - (xB + xC - 2 * xA) * (yA + yB - 2 * yC));
//        double y0 = ((x0 - xA) * (yB + yC - 2 * yA) / (xB + xC - 2 * xA)) + yA;

        double lC = sqrt((xB - xA) * (xB - xA) + (yB - yA) * (yB - yA));
        double lA = sqrt((xC - xB) * (xC - xB) + (yC - yB) * (yC - yB));
        double lB = sqrt((xC - xA) * (xC - xA) + (yC - yA) * (yC - yA));
        double S = std::abs(((xB - xA) * (yC - yA) - (xC - xA) * (yB - yA))) / 2;

//        return Circle(Point(x0, y0), S / (lC + lA + lB) * 2);
        return Circle(Point(((lA * xA + lB * xB + lC * xC) / (lA + lB + lC)),
                            ((lA * yA + lB * yB + lC * yC) / (lA + lB + lC))), S / (lC + lA + lB) * 2);
    } // впис

    Point centroid() const {
        return Point((verts[0] + verts[1]+ verts[2])/3);
    }

    Point orthocenter() const {
//        Point O = GetCircumscribedCircleCenter();
//        Point M = centroid();
//        return O + 3 * (M - O);
        double x1 = verts[0].x, x2 = verts[1].x, x3 = verts[2].x, y1 = verts[0].y, y2 = verts[1].y, y3 = verts[2].y;
        double den = x1 * (y2 - y3) - y1 * (x2 - x3) + x2 * y3 - x3 * y2;
        return Point((y1 * (x3 * x1 + y2 * y2 - x1 * x2 - y3 * y3) - (x2 * x3 + y1 * y1) * (y2 - y3) +
                      y2 * (x1 * x2 + y3 * y3) - y3 * (x3 * x1 + y2 * y2)) / den,
                     ((x2 - x3) * (x1 * x1 + y2 * y3) - x1 * (x2 * x2 + y3 * y1 - x3 * x3 - y1 * y2) +
                      (x2 * x2 + y3 * y1) * x3 - x2 * (x3 * x3 + y1 * y2)) / den);
    }

    Line EulerLine() const { return Line(GetCircumscribedCircleCenter(), orthocenter()); }

    Circle ninePointsCircle() const {
        return Triangle((verts[0] + verts[1]) / 2, (verts[1] + verts[2]) / 2,
                        (verts[2] + verts[0]) / 2).circumscribedCircle();
    } // окружность Эйлера

private:
    Point GetCircumscribedCircleCenter() const {
        double a2 = (verts[0].x * verts[0].x + verts[0].y * verts[0].y);
        double b2 = (verts[1].x * verts[1].x + verts[1].y * verts[1].y);
        double c2 = (verts[2].x * verts[2].x + verts[2].y * verts[2].y);
        double yba = (verts[0].y - verts[1].y);
        double ycb = (verts[1].y - verts[2].y);
        double yac = (verts[2].y - verts[0].y);
        double xab = (verts[0].x - verts[1].x);
        double xbc = (verts[1].x - verts[2].x);
        double xca = (verts[2].x - verts[0].x);
        double x0 = -(yba * c2 + ycb * a2 + yac * b2) / (2 * (xab * yac - yba * xca));
        double y0 = (xab * c2 + xbc * a2 + xca * b2) / (2 * (xab * yac - yba * xca));
        return Point(x0, y0);
    }

};
