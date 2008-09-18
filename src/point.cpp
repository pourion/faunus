#include "faunus/point.h"

namespace Faunus {

  point::point() { clear(); }
  point::point(double xx, double yy, double zz) {
    x=xx;
    y=yy;
    z=zz;
  }

  void point::clear() { x=y=z=0; }
  double point::dot(point &p) { return (x*p.x + y*p.y + z*p.z); }

  double point::len() {
    double l2=x*x+y*y+z*z;
    return (l2!=0) ? sqrt(l2) : 0;
  }

  void point::ranunit(slump &slp) {
    point u;
    double r=2;
    while (r > 1.) { //Generate a random unit vector
      u.x=slp.random_one();
      u.y=slp.random_one();
      u.z=slp.random_one();
      r=sqrt(u.x*u.x+u.y*u.y+u.z*u.z);
    }
    x=u.x/r;
    y=u.y/r;
    z=u.z/r;
  }

  void point::operator+=(point p) {
    x += p.x;
    y += p.y;
    z += p.z;
  }

  point point::operator-() {
    point o;
    o.x=-x; o.y=-y; o.z=-z;
    return o;
  }

  point point::operator*(point p) {
    point o;
    o.x = p.x * x;
    o.y = p.y * y;
    o.z = p.z * z;
    return o;
  }
  point point::operator*(double s) {
    point o;
    o.x = x*s;
    o.y = y*s;
    o.z = z*s;
    return o;
  }

  point point::operator+(double d) {
    point o;
    o.x = x+d;
    o.y = y+d;
    o.z = z+d;
    return o;
  }


  point point::operator-(point p) {
    point o;
    o.x = this->x - p.x;
    o.y = this->y - p.y;
    o.z = this->z - p.z;
    return o;
  }

  point point::operator+(point p) {
    point o;
    o.x = x + p.x;
    o.y = y + p.y;
    o.z = z + p.z;
    return o;
  }

  string point::str() {
    std::stringstream s;
    s.setf(std::ios::fixed);
    s.precision(2);
    s << x << "," << y << "," << z;
    return "[" + s.str() + "]";
  }


  /*
     void point::random_unitv() {
     double r=2;
     while (r > 1.) { //random unit vector
     x=random_one();
     y=random_one();
     z=random_one();
     r=sqrt(x*x+y*y+z*z);
     };
     x=x/r;
     y=y/r;
     z=z/r;
     };
     */

  //------------------ PARTICLE -----------------------
  particle::particle() {
    charge=mw=radius=0;
    hydrophobic=false;
    id=GHOST;
  }
  void particle::operator=(point p) {
    x=p.x;
    y=p.y;
    z=p.z;
  }
  double particle::volume() { return 4.18879*radius*radius*radius; }
  double particle::mw2vol(double rho) { return 1.6606*rho*mw; }
  double particle::mw2rad(double rho) { return pow( 3.*mw2vol(rho)/4./3.14, 0.3333 ); }
  std::ostream &operator<<(std::ostream &out, point &p) {
    out << p.str();
    return out;
  }

  //---------------- SPHERICAL -----------------
  spherical::spherical(double radial, double zenith, double azimuthal) {
    r=radial;
    theta=zenith;
    phi=azimuthal;
  }
}//namespace