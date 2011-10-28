#include "faunus/geometry.h"
#include "faunus/inputfile.h"
#include "faunus/species.h"
#include "faunus/physconst.h"
#include "faunus/group.h"
#include "faunus/point.h"
#include <faunus/space.h>
#include <faunus/textio.h>

namespace Faunus {

  Space::Space(Geometry::Geometrybase &geoPtr) {
    assert(&geoPtr!=nullptr);
    geo=&geoPtr;
  }

  Space::~Space() {}

  double Space::charge() const {
    double z=0;
    for (auto &p_i : p)
      z+=p_i.charge;
    return z;
  }

  bool Space::check_vector() {
    assert(p.size()==trial.size());
    assert(p==trial);
    bool rc=false;
    if (p.size()==trial.size())
      for (size_t i=0; i<p.size(); i++) {
        if (p[i].x!=trial[i].x ||
            p[i].y!=trial[i].y ||
            p[i].z!=trial[i].z ||
            p[i].charge!=trial[i].charge)
        {
          rc=false;
          break;
        } else rc=true;
      }
    if (rc==false) {
      assert(!"Particle vectors corrupted");
      std::cerr << "# Fatal error: Particle vectors corrupted!!\n";
    }
    return rc;
  }

  GroupMolecular Space::insert(const p_vec &pin, int i) {
    assert(i==-1 && "Vector insertion at random position unimplemented.");
    assert(overlap()==false && "Particle overlap not allowed before inserting.");

    GroupMolecular g;
    if ( !pin.empty() ) {
      g.beg=p.size();
      g.end=g.beg-1;
      for (auto i : pin) {
        geo->boundary(i);
        p.push_back(i);
        trial.push_back(i);
        g.end++;
      }
      g.setMassCenter(*this);
      g.translate(*this, -g.cm);
      g.accept(*this); 
      Point a;
      while ( overlap()==true ) {
        geo->randompos(a);
        a=a-g.cm;
        geo->boundary(a);
        g.translate(*this,a);
        g.accept(*this);
      }
    }
    g.setMassCenter(*this);
    return g;
  }

  /*!
   * \param a Particle to insers
   * \param i Position in particle vector
   */
  bool Space::insert(particle a, int i) {
    if ( i==-1 || i>(int)p.size() )
      i=p.size();
    p.insert(p.begin()+i, a);
    trial.insert(trial.begin()+i, a);

    for (auto g_j : g) {
      if ( i < g_j->beg )
        g_j->beg++;
      if ( i <= g_j->end )
        g_j->end++;
    }
    return true;
  }

  bool Space::insert(string atomname, int n, spacekeys key) {
    particle a;
    a=atom[atomname];
    while (n>0) {
      geo->randompos(a);
      if (!overlap(a)) {
        insert(a,-1);
        n--;
      }
    }
    return true;
  }

  bool Space::overlap() const {
    for (auto i=p.begin(); i!=p.end()-1; ++i)
      for (auto j=i+1; j!=p.end(); ++j) {
        double s=i->radius + j->radius;
        if (geo->sqdist(*i,*j)<s*s)
          return true;
      }
    return false;
  }

  bool Space::overlap(const particle &a) const {
    for (auto &p_i : p) {
      double contact = a.radius + p_i.radius;
      if (geo->sqdist(a,p_i) < contact*contact)
        return true;
    }
    return false;
  }

  bool Space::remove(int i) {
    if (i>=(int)p.size())
      return false;
    p.erase( p.begin()+i );
    trial.erase( trial.begin()+i );
    for (auto gj : g) { //for size_t j=0; j<g.size(); j++) {  // move and reduce groups if appropriate
      if ( i<gj->beg )
        gj->beg--;
      if (i<=gj->end)
        gj->end--;
    }
    return true;
    }

    bool Space::save(string file) {
      using std::numeric_limits;
      std::ofstream fout( file.c_str() );
      if (fout) {
        fout.precision( numeric_limits<double>::digits10 + 1 );
        fout << p.size() << endl;
        for (auto p_i : p)
          fout << p_i << endl;
        fout << g.size() << endl;
        for (auto g_i : g)
          fout << *g_i << endl;
        fout.close();
        return true;
      }
      return false;
    }

    /*!
     * \param file Filename
     * \param resize True if the current geometry should be resized to match file content (default: false)
     */
    bool Space::load(string file, bool resize) {
      using namespace textio;
      int n;
      fin.close();
      fin.open( file.c_str() );
      if (fin) {
        fin >> n;
        if (resize==true)
          p.resize(n);
        if (n == (int)p.size() ) {
          for (int i=0; i<n; i++)
            p[i] << fin;
          trial=p;
          cout << indent(SUB) << "Read " << n << " space points from " << file << endl;
          fin >> n;
          if (n==(int)g.size()) {
            for (auto g_i : g) {
              *g_i << fin;
              g_i->setMassCenter(*this);
            }
            cout << indent(SUB) << "Read " << n << " groups from " << file << endl;
            return true;
          }
        }
      }
      cout << indent(SUB) << "Space data NOT read from file " << file << endl;
      return false;
    }

    int Space::enroll(Group &newgroup) {
      for (size_t i=0; i<g.size(); ++i)
        if (g[i]==&newgroup)
          return i;
      newgroup.setMassCenter(*this);
      g.push_back(&newgroup);
      trial=p;
      return g.size()-1; //return position if added group
    }

    string Space::info() {
      using namespace textio;
      static char w=25;
      double z=charge();
      std::ostringstream o;
      o << header("Simulation Space and Geometry") 
        << geo->info(w)
        << pad(SUB,w,"Number of particles") << p.size() << endl
        << pad(SUB,w,"Electroneutrality") << ((abs(z)>1e-7) ? "NO!" : "Yes") << " "  << z << endl
        << indent(SUB) << "Groups:" << endl;
      for (size_t i=0; i<g.size(); i++) {
        std::ostringstream range;
        range << "[" << g[i]->beg << "-" << g[i]->end << "]";
        o << indent(SUBSUB) << std::left
          << setw(6) << i+1
          << setw(17) << range.str()
          << setw(20) << g[i]->name
          << endl;
      }
      return o.str();
    }

  }//namespace
