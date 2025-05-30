// Copyright (c) 1997-2002  Max-Planck-Institute Saarbruecken (Germany).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org); you may redistribute it under
// the terms of the Q Public License version 1.0.
// See the file LICENSE.QPL distributed with CGAL.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/trunk/Nef_3/include/CGAL/Nef_3/OGL_helper.h $
// $Id: OGL_helper.h 56667 2010-06-09 07:37:13Z sloriot $
// 
//
// Author(s)     : Peter Hachenberger <hachenberger@mpi-sb.mpg.de>

// Modified for OpenSCAD

#pragma once

#include <CGAL/Nef_S2/OGL_base_object.h>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Nef_3/SNC_decorator.h>

#include "glview/system-gl.h"
#include <cstdlib>

// Overridden in CGAL_renderer
/*
#define CGAL_NEF3_OGL_MARKED_VERTEX_COLOR 183,232,92
#define CGAL_NEF3_OGL_MARKED_EDGE_COLOR 171,216,86
#define CGAL_NEF3_OGL_MARKED_FACET_COLOR  157,203,81
#define CGAL_NEF3_OGL_MARKED_BACK_FACET_COLOR  157,103,181

#define CGAL_NEF3_OGL_UNMARKED_VERTEX_COLOR 255,246,124
#define CGAL_NEF3_OGL_UNMARKED_EDGE_COLOR 255,236,94
#define CGAL_NEF3_OGL_UNMARKED_FACET_COLOR 249,215,44
#define CGAL_NEF3_OGL_UNMARKED_BACK_FACET_COLOR 249,115,144
*/

#ifdef _WIN32
#include <windows.h> // For the CALLBACK macro
#define CGAL_GLU_TESS_CALLBACK CALLBACK
#else
#define CGAL_GLU_TESS_CALLBACK 
#endif

#ifdef __APPLE__
#  include <AvailabilityMacros.h>
#endif

#if defined __APPLE__ && !defined MAC_OS_X_VERSION_10_5
    #define CGAL_GLU_TESS_DOTS ...
#else
    #define CGAL_GLU_TESS_DOTS
#endif


namespace CGAL {

namespace OGL {

// ----------------------------------------------------------------------------
// Drawable double types:
// ----------------------------------------------------------------------------

  typedef CGAL::Simple_cartesian<double> DKernel;  
  typedef DKernel::Point_3               Double_point;
  typedef DKernel::Vector_3              Double_vector;
  typedef DKernel::Segment_3             Double_segment;
  typedef DKernel::Aff_transformation_3  Affine_3;

  // DPoint = a double point including a mark
  class DPoint : public Double_point {
    bool m_;
  public:
    DPoint() {}
    DPoint(const Double_point& p, bool m) : Double_point(p) { m_ = m; }
    DPoint(const DPoint& p) : Double_point(p) { m_ = p.m_; }
    DPoint& operator=(const DPoint& p)
    { Double_point::operator=(p); m_ = p.m_; return *this; }
    bool mark() const { return m_; }
  };

  // DSegment = a double segment including a mark
  class DSegment : public Double_segment {
    bool m_;
  public:
    DSegment() {}
    DSegment(const Double_segment& s, bool m) : Double_segment(s) { m_ = m; }
    DSegment(const DSegment& s) : Double_segment(s) { m_ = s.m_; }
    DSegment& operator=(const DSegment& s)
    { Double_segment::operator=(s); m_ = s.m_; return *this; }
    bool mark() const { return m_; }
  };

  // Double_triple = a class that stores a triple of double
  // coordinates; we need a pointer to the coordinates in a C array
  // for OpenGL
  class Double_triple {
    typedef double*       double_ptr;
    typedef const double* const_double_ptr;
    double coords_[3];
  public:
    Double_triple() 
    { coords_[0]=coords_[1]=coords_[2]=0.0; }
    Double_triple(double x, double y, double z)
    { coords_[0]=x; coords_[1]=y; coords_[2]=z; }
    Double_triple(const Double_triple& t)
    { coords_[0]=t.coords_[0];
      coords_[1]=t.coords_[1];
      coords_[2]=t.coords_[2];
    }
    Double_triple& operator=(const Double_triple& t)
    { coords_[0]=t.coords_[0];
      coords_[1]=t.coords_[1];
      coords_[2]=t.coords_[2];
      return *this; }
    operator double_ptr() const 
    { return const_cast<Double_triple&>(*this).coords_; }
    double operator[](unsigned i) 
    { CGAL_assertion(i<3); return coords_[i]; }
  }; // Double_triple

  static std::ostream& operator << (std::ostream& os,
				    const Double_triple& t)
  { os << "(" << t[0] << "," << t[1] << "," << t[2] << ")";
    return os; }


  // DFacet stores the facet cycle vertices in a continuus C array
  // of three double components, this is necessary due to the OpenGL
  // tesselator input format !
  class DFacet {
    typedef std::vector<Double_triple>   Coord_vector;
    typedef std::vector<unsigned>        Cycle_vector;
    Coord_vector    coords_;  // stores all vertex coordinates
    Cycle_vector    fc_ends_; // stores entry points of facet cycles
    Double_triple   normal_; // stores normal and mark of facet
    bool            mark_;

  public:
    typedef Coord_vector::iterator Coord_iterator;
    typedef Coord_vector::const_iterator Coord_const_iterator;

    DFacet() {}

    void push_back_vertex(double x, double y, double z)
    { coords_.push_back(Double_triple(x,y,z)); }

    DFacet(const DFacet& f) 
    { coords_  = f.coords_;
      fc_ends_ = f.fc_ends_;
      normal_  = f.normal_;
      mark_    = f.mark_;
    }

    DFacet& operator=(const DFacet& f) 
    { coords_ =  f.coords_;
      fc_ends_ = f.fc_ends_;
      normal_ =  f.normal_;
      mark_    = f.mark_;
      return *this; 
    }

    ~DFacet()
    { coords_.clear(); fc_ends_.clear(); }

    void push_back_vertex(const Double_point& p)
    { push_back_vertex(p.x(),p.y(),p.z()); }
   
    void set_normal(double x, double y, double z, bool m)
    { double l = sqrt(x*x + y*y + z*z);
      normal_ = Double_triple(x/l,y/l,z/l); mark_ = m; }

    double dx() const { return normal_[0]; }
    double dy() const { return normal_[1]; }
    double dz() const { return normal_[2]; }
    bool mark() const { return mark_; }
    double* normal() const 
    { return static_cast<double*>(normal_); }

    void new_facet_cycle()
    { fc_ends_.push_back(coords_.size()); }
    
    unsigned number_of_facet_cycles() const
    { return fc_ends_.size(); }

    Coord_iterator facet_cycle_begin(unsigned i) 
    { CGAL_assertion(i<number_of_facet_cycles());
      if (i==0) return coords_.begin();
      else return coords_.begin()+fc_ends_[i]; }

    Coord_iterator facet_cycle_end(unsigned i) 
    { CGAL_assertion(i<number_of_facet_cycles());
      if (i<fc_ends_.size()-1) return coords_.begin()+fc_ends_[i+1];
      else return coords_.end(); }

    Coord_const_iterator facet_cycle_begin(unsigned i) const
    { CGAL_assertion(i<number_of_facet_cycles());
      if (i==0) return coords_.begin();
      else return coords_.begin()+fc_ends_[i]; }

    Coord_const_iterator facet_cycle_end(unsigned i) const
    { CGAL_assertion(i<number_of_facet_cycles());
      if (i<fc_ends_.size()-1) return coords_.begin()+fc_ends_[i+1];
      else return coords_.end(); }

    void debug(std::ostream& os = std::cerr) const
    { os << "DFacet, normal=" << normal_ << ", mark=" << mark() << std::endl;
      for(unsigned i=0; i<number_of_facet_cycles(); ++i) {
	os << "  facet cycle ";
	// put all vertices in facet cycle into contour:
	Coord_const_iterator cit;
	for(cit = facet_cycle_begin(i); cit != facet_cycle_end(i); ++cit)
	  os << *cit;
	os << std::endl;
      }
    }
    
  }; // DFacet


// ----------------------------------------------------------------------------
// OGL Drawable Polyhedron:
// ----------------------------------------------------------------------------

  inline void CGAL_GLU_TESS_CALLBACK beginCallback(GLenum which)
  { glBegin(which); }

  inline void CGAL_GLU_TESS_CALLBACK endCallback(void)
  { glEnd(); }

  inline void CGAL_GLU_TESS_CALLBACK errorCallback(GLenum errorCode)
  { const GLubyte *estring;
    estring = gluErrorString(errorCode);
    fprintf(stderr, "Tessellation Error: %s\n", estring);
    std::exit (0);
  }

  inline void CGAL_GLU_TESS_CALLBACK vertexCallback(GLvoid* vertex,
			                            GLvoid* user)
  { GLdouble* pc(static_cast<GLdouble*>(vertex));
    GLdouble* pu(static_cast<GLdouble*>(user));
    //    CGAL_NEF_TRACEN("vertexCallback coord  "<<pc[0]<<","<<pc[1]<<","<<pc[2]);
    //    CGAL_NEF_TRACEN("vertexCallback normal "<<pu[0]<<","<<pu[1]<<","<<pu[2]);
    glNormal3dv(pu);
    glVertex3dv(pc); 
  }

  inline void CGAL_GLU_TESS_CALLBACK combineCallback(GLdouble coords[3], GLvoid *[4], GLfloat [4], GLvoid **dataOut)
  { static std::list<GLdouble*> pcache;
    if (dataOut) {
      GLdouble *n = new GLdouble[3];
      n[0] = coords[0];
      n[1] = coords[1];
      n[2] = coords[2];
      pcache.push_back(n);
      *dataOut = n;
    } else {
      for (std::list<GLdouble*>::const_iterator i = pcache.begin(); i != pcache.end(); i++)
        delete[] *i;
      pcache.clear();
    }
  }


 enum { SNC_AXES};
 
 class Polyhedron : public OGL_base_object {
 protected:
    std::list<DPoint>    vertices_;
    std::list<DSegment>  edges_;
    std::list<DFacet>    halffacets_;

    GLuint         object_list_;
    bool init_;

    Bbox_3  bbox_;

    std::vector<bool> switches;

    typedef std::list<DPoint>::const_iterator   Vertex_iterator;
    typedef std::list<DSegment>::const_iterator Edge_iterator;
    typedef std::list<DFacet>::const_iterator   Halffacet_iterator;

  public:
    Polyhedron() : bbox_(-1,-1,-1,1,1,1), switches(1) { 
      object_list_ = 0;
      init_ = false;
      switches[SNC_AXES] = false; 
    }

    ~Polyhedron() 
    { if (object_list_) glDeleteLists(object_list_, 4); }

    void push_back(const Double_point& p, bool m) {
        vertices_.push_back(DPoint(p,m));
    }
    void push_back(const Double_segment& s, bool m) 
    { edges_.push_back(DSegment(s,m)); }
    void push_back(const DFacet& f) 
    { halffacets_.push_back(f); }
 
    void toggle(int index) override { 
      switches[index] = !switches[index]; 
    }
    
    void set_style(int index) override {
    }

    bool is_initialized() const { return init_; }

    Bbox_3  bbox() const { return bbox_; }
    Bbox_3& bbox()       { return bbox_; }

    // Overridden in CGAL_renderer
    virtual CGAL::Color getVertexColor(Vertex_iterator v) const
    {
      PRINTD("getVertexColor()");
	(void)v;
//	CGAL::Color cf(CGAL_NEF3_OGL_MARKED_VERTEX_COLOR),
//	  ct(CGAL_NEF3_OGL_UNMARKED_VERTEX_COLOR); // more blue-ish
//	CGAL::Color c = v->mark() ? ct : cf;
	CGAL::Color c(0,0,200);
	return c;
    }

    virtual void draw(bool) const = 0;

    // Overridden in CGAL_renderer
    virtual CGAL::Color getEdgeColor(Edge_iterator e) const
    {
      PRINTD("getEdgeColor)");
	(void)e;
//	CGAL::Color cf(CGAL_NEF3_OGL_MARKED_EDGE_COLOR),
//	  ct(CGAL_NEF3_OGL_UNMARKED_EDGE_COLOR); // more blue-ish
//	CGAL::Color c = e->mark() ? ct : cf;
	// Overridden in CGAL_renderer
	CGAL::Color c(200,0,0);
	return c;
    }



    // Overridden in CGAL_renderer
    virtual CGAL::Color getFacetColor(Halffacet_iterator /*f*/) const {
      CGAL::Color c(0,200,0);
      return c;
    }

    void debug(std::ostream& os = std::cerr) const
    {
      os << "OGL::Polyhedron" << std::endl;
      os << "Vertices:" << std::endl;
      Vertex_iterator v;
      for(v=vertices_.begin();v!=vertices_.end();++v) 
	os << "  "<<*v<<", mark="<<v->mark()<<std::endl;
      os << "Edges:" << std::endl;
      Edge_iterator e;
      for(e=edges_.begin();e!=edges_.end();++e)
	os << "  "<<*e<<", mark="<<e->mark()<<std::endl;
      os << "Facets:" << std::endl;
      Halffacet_iterator f;
      for(f=halffacets_.begin();f!=halffacets_.end();++f) {
	f->debug(); os << std::endl;
      }
      os << std::endl;
    }

  }; // Polyhedron

  template<typename Nef_polyhedron>
  class Nef3_Converter { 
    typedef typename Nef_polyhedron::SNC_structure           SNC_structure;
    typedef CGAL::SNC_decorator<SNC_structure>               Base;
    
    public:
    typedef typename SNC_structure::Vertex_const_iterator Vertex_const_iterator; 
    typedef typename SNC_structure::Halfedge_const_iterator Halfedge_const_iterator; 
    typedef typename SNC_structure::Halffacet_const_iterator Halffacet_const_iterator; 
    typedef typename SNC_structure::Halffacet_cycle_const_iterator Halffacet_cycle_const_iterator;
    
    typedef typename SNC_structure::Object_const_handle Object_const_handle;
    typedef typename SNC_structure::SHalfedge_const_handle SHalfedge_const_handle; 
    typedef typename SNC_structure::SHalfloop_const_handle SHalfloop_const_handle; 
    
    typedef typename SNC_structure::Vertex_const_handle Vertex_const_handle; 
    typedef typename SNC_structure::Halfedge_const_handle Halfedge_const_handle; 
    typedef typename SNC_structure::Halffacet_const_handle Halffacet_const_handle;
    
    typedef typename SNC_structure::Point_3 Point_3;
    typedef typename SNC_structure::Vector_3 Vector_3;
    typedef typename SNC_structure::Segment_3 Segment_3;
    typedef typename SNC_structure::Plane_3 Plane_3;
    typedef typename SNC_structure::Mark Mark;
    typedef typename SNC_structure::SHalfedge_around_facet_const_circulator 
      SHalfedge_around_facet_const_circulator;
    
  private:
    static OGL::Double_point double_point(const Point_3& p)
      { return OGL::Double_point(CGAL::to_double(p.x()),
				 CGAL::to_double(p.y()),
				 CGAL::to_double(p.z())); }
    
    static OGL::Double_segment double_segment(const Segment_3& s)
      { return OGL::Double_segment(double_point(s.source()),
				   double_point(s.target())); }
    
    static void draw(Vertex_const_handle v, const Nef_polyhedron& , 
		     CGAL::OGL::Polyhedron& P) { 
      Point_3 bp = v->point();
      //    CGAL_NEF_TRACEN("vertex " << bp);
      P.push_back(double_point(bp), v->mark()); 
    }
    
    static void draw(Halfedge_const_handle e, const Nef_polyhedron& ,
		     CGAL::OGL::Polyhedron& P) { 
      Vertex_const_handle s = e->source();
      Vertex_const_handle t = e->twin()->source();
      Segment_3 seg(s->point(),t->point());
      //    CGAL_NEF_TRACEN("edge " << seg);
      P.push_back(double_segment(seg), e->mark()); 
    }
    
    static bool same_orientation(Plane_3 p1, Plane_3 p2) {
      if(p1.a() != 0)
        return CGAL::sign(p1.a()) == CGAL::sign(p2.a());
      if(p1.b() != 0)
        return CGAL::sign(p1.b()) == CGAL::sign(p2.b());
      return CGAL::sign(p1.c()) == CGAL::sign(p2.c());
    }

    static void draw(Halffacet_const_handle f, const Nef_polyhedron& ,
		     CGAL::OGL::Polyhedron& P) { 

      if (f->incident_volume()->mark()) return; // Skip halffaces facing solid volume
      OGL::DFacet g;
      Halffacet_cycle_const_iterator fc; // all facet cycles:
      CGAL_forall_facet_cycles_of(fc,f)
	if ( fc.is_shalfedge() ) { // non-trivial facet cycle
	  g.new_facet_cycle();
	  SHalfedge_const_handle h = fc;
	  SHalfedge_around_facet_const_circulator hc(h), he(hc);
	  CGAL_For_all(hc,he){ // all vertex coordinates in facet cycle
	    Point_3 sp = hc->source()->source()->point();
	    //	      CGAL_NEF_TRACEN(" ");CGAL_NEF_TRACEN("facet" << sp);
	    g.push_back_vertex(double_point(sp));
	  }
	}
      Vector_3 v = f->plane().orthogonal_vector();
      g.set_normal(CGAL::to_double(v.x()), 
		   CGAL::to_double(v.y()), 
		   CGAL::to_double(v.z()), 
		   f->mark());
      P.push_back(g);
    }
    
    // Returns the bounding box of the finite vertices of the polyhedron.
    // Returns $[-1,+1]^3$ as bounding box if no finite vertex exists.
    
    static Bbox_3  bounded_bbox(const Nef_polyhedron& N) {
      bool first_vertex = true;
      Bbox_3 bbox( -1.0, -1.0, -1.0, 1.0, 1.0, 1.0);
      Vertex_const_iterator vi;
      CGAL_forall_vertices(vi, N) {
	Point_3 p = vi->point();
	double x = CGAL::to_double(p.hx());
	double y = CGAL::to_double(p.hy());
	double z = CGAL::to_double(p.hz());
	double w = CGAL::to_double(p.hw());
	if (N.is_standard(vi)) {
	  if(first_vertex) {
	    bbox = Bbox_3(x/w, y/w, z/w, x/w, y/w, z/w);
	    first_vertex = false;
	  } else {
	    bbox = bbox + Bbox_3(x/w, y/w, z/w, x/w, y/w, z/w);
	    first_vertex = false;
	  }
	}
      }
      return bbox;
    }  
    
    static void set_R(Bbox_3& bbox, const Nef_polyhedron& N) {
      if(N.is_standard_kernel()) return;
      double size = abs(bbox.xmin());
      if(size < bbox.xmax()) size = bbox.xmax();
      if(size < bbox.ymin()) size = bbox.ymin();
      if(size < bbox.ymax()) size = bbox.ymax();
      if(size < bbox.zmin()) size = bbox.zmin();
      if(size < bbox.zmax()) size = bbox.zmax();
      N.set_size_of_infimaximal_box(size*50);
      //    CGAL_NEF_TRACEN("set infi box size to " << size);
      Vertex_const_iterator vi;
      CGAL_forall_vertices(vi, N)
	if(N.is_standard(vi))
	  return;
      bbox = Bbox_3(bbox.xmin()*10,bbox.ymin()*10,bbox.zmin()*10,
		    bbox.xmax()*10,bbox.ymax()*10,bbox.zmax()*10);
    }
  public:
    static void convert_to_OGLPolyhedron(const Nef_polyhedron& N, CGAL::OGL::Polyhedron* P) { 
      Bbox_3 bbox(bounded_bbox(N));
      set_R(bbox,N);
      P->bbox() = bbox;    
      Vertex_const_iterator v;
      CGAL_forall_vertices(v,*N.sncp()) draw(v,N,*P);
      Halfedge_const_iterator e;
      CGAL_forall_edges(e,*N.sncp()) draw(e,N,*P);
      Halffacet_const_iterator f;
      CGAL_forall_halffacets(f,*N.sncp()) draw(f,N,*P);
    }
    
  }; // Nef3_Converter

} // namespace OGL

} //namespace CGAL
