/* =============================================================================
**  This file is part of the mmg software package for the tetrahedral
**  mesh modification.
**  Copyright (c) Inria - IMB (Université de Bordeaux) - LJLL (UPMC), 2004- .
**
**  mmg is free software: you can redistribute it and/or modify it
**  under the terms of the GNU Lesser General Public License as published
**  by the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  mmg is distributed in the hope that it will be useful, but WITHOUT
**  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
**  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
**  License for more details.
**
**  You should have received a copy of the GNU Lesser General Public
**  License and of the GNU General Public License along with mmg (in
**  files COPYING.LESSER and COPYING). If not, see
**  <http://www.gnu.org/licenses/>. Please read their terms carefully and
**  use this copy of the mmg distribution only if you accept them.
** =============================================================================
*/

/**
 * \file mmgs/quality.c
 * \brief Functions to compute elements quality and edge lengths.
 * \author Charles Dapogny (LJLL, UPMC)
 * \author Cécile Dobrzynski (Inria / IMB, Université de Bordeaux)
 * \author Pascal Frey (LJLL, UPMC)
 * \author Algiane Froehly (Inria / IMB, Université de Bordeaux)
 * \version 5
 * \copyright GNU Lesser General Public License.
 * \todo Doxygen documentation
 */

#include "mmgs.h"

extern char  ddb;

#define COS145   -0.81915204428899


/* return 0: triangle ok, 1: needle, 2: obtuse; ia: edge problem */
char typelt(MMG5_pPoint p[3],char *ia) {
  double   h1,h2,h3,hmi,hma,ux,uy,uz,vx,vy,vz,wx,wy,wz,dd;

  ux = p[1]->c[0] - p[0]->c[0];
  uy = p[1]->c[1] - p[0]->c[1];
  uz = p[1]->c[2] - p[0]->c[2];
  h1 = ux*ux + uy*uy + uz*uz;

  vx = p[2]->c[0] - p[0]->c[0];
  vy = p[2]->c[1] - p[0]->c[1];
  vz = p[2]->c[2] - p[0]->c[2];
  h2 = vx*vx + vy*vy + vz*vz;
  hmi = h1;
  hma = h2;
  *ia = 2;
  if ( h1 > h2 ) {
    hmi = h2;
    hma = h1;
    *ia = 1;
  }
  wx = p[2]->c[0] - p[1]->c[0];
  wy = p[2]->c[1] - p[1]->c[1];
  wz = p[2]->c[2] - p[1]->c[2];
  h3 = wx*wx + wy*wy + wz*wz;
  if ( h3 < hmi ) {
    hmi = h3;
    *ia = 0;
  }
  else if ( h3 > hma )
    hma = h3;

  /* needle */
  if ( hmi < 0.01 * hma )  return(1);

  /* check obtuse angle */
  dd = (ux*vx + uy*vy + uz*vz) / sqrt(h1*h2);
  if ( dd < COS145 ) {
    *ia = 0;
    return(2);
  }
  dd = (vx*wx + vy*wy + vz*wz) / sqrt(h2*h3);
  if ( dd < COS145 ) {
    *ia = 2;
    return(2);
  }
  dd = -(ux*wx + uy*wy + uz*wz) / sqrt(h1*h3);
  if ( dd < COS145 ) {
    *ia = 1;
    return(2);
  }

  return(0);
}

static double calelt33_ani(MMG5_pMesh mesh,MMG5_pSol met,int iel) {
  MMG5_pTria    pt;
  double   cal,dd,abx,aby,abz,acx,acy,acz,bcx,bcy,bcz,rap,det;
  double  *a,*b,*c,*ma,*mb,*mc,n[3],m[6];
  int      ia,ib,ic;
  char     i;

  pt = &mesh->tria[iel];
  ia = pt->v[0];
  ib = pt->v[1];
  ic = pt->v[2];

  ma = &met->m[6*ia+1];
  mb = &met->m[6*ib+1];
  mc = &met->m[6*ic+1];

  dd  = 1.0 / 3.0;
  for (i=0; i<6; i++)
    m[i] = dd * (ma[i] + mb[i] + mc[i]);
  det = m[0]*(m[3]*m[5] - m[4]*m[4]) - m[1]*(m[1]*m[5] - m[2]*m[4]) + m[2]*(m[1]*m[4] - m[2]*m[3]);
  if ( det < _MMG5_EPSD2 )  return(0.0);

  a = &mesh->point[ia].c[0];
  b = &mesh->point[ib].c[0];
  c = &mesh->point[ic].c[0];

  /* area */
  abx = b[0] - a[0];
  aby = b[1] - a[1];
  abz = b[2] - a[2];
  acx = c[0] - a[0];
  acy = c[1] - a[1];
  acz = c[2] - a[2];
  bcx = c[0] - b[0];
  bcy = c[1] - b[1];
  bcz = c[2] - b[2];

  n[0] = (aby*acz - abz*acy) * (aby*acz - abz*acy);
  n[1] = (abz*acx - abx*acz) * (abz*acx - abx*acz);
  n[2] = (abx*acy - aby*acx) * (abx*acy - aby*acx);
  cal  = sqrt(n[0] + n[1] + n[2]);
  if ( cal > _MMG5_EPSD ) {
    dd    = 1.0 / cal;
    n[0] *= dd;
    n[1] *= dd;
    n[2] *= dd;
    /* length */
    rap  = m[0]*abx*abx + m[3]*aby*aby + m[5]*abz*abz + 2.0*(m[1]*abx*aby + m[2]*abx*abz + m[4]*aby*abz);
    rap += m[0]*acx*acx + m[3]*acy*acy + m[5]*acz*acz + 2.0*(m[1]*acx*acy + m[2]*acx*acz + m[4]*acy*acz);
    rap += m[0]*bcx*bcx + m[3]*bcy*bcy + m[5]*bcz*bcz + 2.0*(m[1]*bcx*bcy + m[2]*bcx*bcz + m[4]*bcy*bcz);
    /* quality */
    if ( rap > _MMG5_EPSD )
      return(sqrt(det)*cal / rap);
    else
      return(0.0);
  }
  else
    return(0.0);
}

/* quality = surf / sigma(length_edges) */
inline double _MMG5_caltri_ani(MMG5_pMesh mesh,MMG5_pSol met,MMG5_pTria ptt) {
  double        rap,anisurf,l[3];
  int           ia,ib,ic;

  ia = ptt->v[0];
  ib = ptt->v[1];
  ic = ptt->v[2];

  anisurf = surftri_ani(mesh,met,ptt);

  l[0] = _MMG5_lenedg_ani(mesh,met,ib,ic,( ptt->tag[0] & MG_GEO ));
  l[1] = _MMG5_lenedg_ani(mesh,met,ia,ic,( ptt->tag[1] & MG_GEO ));
  l[2] = _MMG5_lenedg_ani(mesh,met,ia,ib,( ptt->tag[2] & MG_GEO ));

  rap = l[0]*l[0] + l[1]*l[1] + l[2]*l[2];
  if ( rap < _MMG5_EPSD ) return(0.0);
  return (anisurf / rap);
}

/* quality = surf / sigma(length_edges) */
inline double _MMG5_caltri_iso(MMG5_pMesh mesh,MMG5_pSol met,MMG5_pTria ptt) {
  double   *a,*b,*c,cal,abx,aby,abz,acx,acy,acz,bcx,bcy,bcz,rap;
  int       ia,ib,ic;

  ia = ptt->v[0];
  ib = ptt->v[1];
  ic = ptt->v[2];

  a = &mesh->point[ia].c[0];
  b = &mesh->point[ib].c[0];
  c = &mesh->point[ic].c[0];

  /* area */
  abx = b[0] - a[0];
  aby = b[1] - a[1];
  abz = b[2] - a[2];
  acx = c[0] - a[0];
  acy = c[1] - a[1];
  acz = c[2] - a[2];
  bcx = c[0] - b[0];
  bcy = c[1] - b[1];
  bcz = c[2] - b[2];

  cal  = (aby*acz - abz*acy) * (aby*acz - abz*acy);
  cal += (abz*acx - abx*acz) * (abz*acx - abx*acz);
  cal += (abx*acy - aby*acx) * (abx*acy - aby*acx);

  if ( cal < _MMG5_EPSD2 )  return(0.0);

  /* qual = 2.*surf / length */
  rap  = abx*abx + aby*aby + abz*abz;
  rap += acx*acx + acy*acy + acz*acz;
  rap += bcx*bcx + bcy*bcy + bcz*bcz;

  if ( rap < _MMG5_EPSD2 )  return(0.0);

  return(sqrt(cal) / rap);
}

/* Same quality function but puts a sign according to deviation to normal to vertices */
inline double caleltsig_ani(MMG5_pMesh mesh,MMG5_pSol met,int iel) {
  MMG5_pTria    pt;
  MMG5_pPoint   pa,pb,pc;
  double   ps1,ps2,abx,aby,abz,acx,acy,acz,dd,rap,anisurf;
  double   n[3],pv[3],l[3],*ncomp,*a,*b,*c;
  int      ia,ib,ic;

  pt = &mesh->tria[iel];
  ia = pt->v[0];
  ib = pt->v[1];
  ic = pt->v[2];

  pa = &mesh->point[ia];
  pb = &mesh->point[ib];
  pc = &mesh->point[ic];

  a = &pa->c[0];
  b = &pb->c[0];
  c = &pc->c[0];

  /* area */
  abx = b[0] - a[0];
  aby = b[1] - a[1];
  abz = b[2] - a[2];
  acx = c[0] - a[0];
  acy = c[1] - a[1];
  acz = c[2] - a[2];

  pv[0] = aby*acz - abz*acy;
  pv[1] = abz*acx - abx*acz;
  pv[2] = abx*acy - aby*acx;

  dd   = pv[0]*pv[0] + pv[1]*pv[1] + pv[2]*pv[2];
  if ( dd < _MMG5_EPSD )  return(0.0);
  dd = 1.0 / sqrt(dd);

  if ( !MG_EDG(pa->tag) ) {
    memcpy(n,&pa->n[0],3*sizeof(double));
    ps1 = n[0]*pv[0]+n[1]*pv[1]+n[2]*pv[2];
    ps1 *= dd;
  }
  else if ( !MG_EDG(pb->tag) ) {
    memcpy(n,&pb->n[0],3*sizeof(double));
    ps1 = n[0]*pv[0]+n[1]*pv[1]+n[2]*pv[2];
    ps1 *= dd;
  }
  else if ( !MG_EDG(pc->tag) ) {
    memcpy(n,&pc->n[0],3*sizeof(double));
    ps1 = n[0]*pv[0]+n[1]*pv[1]+n[2]*pv[2];
    ps1 *= dd;
  }
  else {
    memcpy(n,&mesh->xpoint[pa->ig].n1[0],3*sizeof(double));
    if ( !(pa->tag & MG_REF) ) {
      ncomp = &mesh->xpoint[pa->ig].n2[0];
      ps1 = n[0]*pv[0]+n[1]*pv[1]+n[2]*pv[2];
      ps1 *= dd;
      ps2 = ncomp[0]*pv[0]+ncomp[1]*pv[1]+ncomp[2]*pv[2];
      ps2 *= dd;
      if ( fabs(1.0-fabs(ps1)) > fabs(1.0-fabs(ps2)) ) {
        memcpy(n,ncomp,3*sizeof(double));
        ps1 = ps2;
      }
    }
  }

  /* if orientation is reversed with regards to orientation of vertices */
  if ( ps1 < 0.0 )  return(-1.0);

  anisurf = surftri_ani(mesh,met,pt);
  if ( anisurf == 0.0 )  return(-1.0);

  l[0] = _MMG5_lenedg_ani(mesh,met,ib,ic,( pt->tag[0] & MG_GEO ));
  l[1] = _MMG5_lenedg_ani(mesh,met,ia,ic,( pt->tag[1] & MG_GEO ));
  l[2] = _MMG5_lenedg_ani(mesh,met,ia,ib,( pt->tag[2] & MG_GEO ));

  rap = l[0]*l[0] + l[1]*l[1] + l[2]*l[2];
  if ( rap < _MMG5_EPSD )  return(0.0);
  return(anisurf / rap);
}

/* Same quality function but puts a sign according to deviation to normal to vertices */
inline double caleltsig_iso(MMG5_pMesh mesh,MMG5_pSol met,int iel) {
  MMG5_pTria     pt;
  MMG5_pPoint    pa,pb,pc;
  double   *a,*b,*c,cal,abx,aby,abz,acx,acy,acz,bcx,bcy,bcz,rap;
  double    n[3],*ncomp,pv[3],ps1,ps2,sqcal,invsqcal;
  int       ia,ib,ic;

  pt = &mesh->tria[iel];
  ia = pt->v[0];
  ib = pt->v[1];
  ic = pt->v[2];

  pa = &mesh->point[ia];
  pb = &mesh->point[ib];
  pc = &mesh->point[ic];

  a = &pa->c[0];
  b = &pb->c[0];
  c = &pc->c[0];

  /* area */
  abx = b[0] - a[0];
  aby = b[1] - a[1];
  abz = b[2] - a[2];
  acx = c[0] - a[0];
  acy = c[1] - a[1];
  acz = c[2] - a[2];
  bcx = c[0] - b[0];
  bcy = c[1] - b[1];
  bcz = c[2] - b[2];

  pv[0] = aby*acz - abz*acy;
  pv[1] = abz*acx - abx*acz;
  pv[2] = abx*acy - aby*acx;

  cal   = pv[0]*pv[0] + pv[1]*pv[1] + pv[2]*pv[2];
  sqcal = sqrt(cal);
  ps1   = 0.0;

  if ( sqcal < _MMG5_EPSD )  return(0.0);
  invsqcal = 1.0 / sqcal;

  if ( !MG_EDG(pa->tag) ) {
    memcpy(n,&pa->n[0],3*sizeof(double));
    ps1 = n[0]*pv[0]+n[1]*pv[1]+n[2]*pv[2];
    ps1 *= invsqcal;
  }
  else if ( !MG_EDG(pb->tag) ) {
    memcpy(n,&pb->n[0],3*sizeof(double));
    ps1 = n[0]*pv[0]+n[1]*pv[1]+n[2]*pv[2];
    ps1 *= invsqcal;
  }
  else if ( !MG_EDG(pc->tag) ) {
    memcpy(n,&pc->n[0],3*sizeof(double));
    ps1 = n[0]*pv[0]+n[1]*pv[1]+n[2]*pv[2];
    ps1 *= invsqcal;
  }
  else {
    memcpy(n,&mesh->xpoint[pa->ig].n1[0],3*sizeof(double));
    if ( !(pa->tag & MG_REF) ) {
      ncomp = &mesh->xpoint[pa->ig].n2[0];
      ps1 = n[0]*pv[0]+n[1]*pv[1]+n[2]*pv[2];
      ps1 *= invsqcal;
      ps2 = ncomp[0]*pv[0]+ncomp[1]*pv[1]+ncomp[2]*pv[2];
      ps2 *= invsqcal;
      if ( fabs(1.0-fabs(ps1)) > fabs(1.0-fabs(ps2)) ) {
        memcpy(n,ncomp,3*sizeof(double));
        ps1 = ps2;
      }
    }
  }

  /* if orientation is reversed with regards to orientation of vertex */
  if ( ps1 < 0.0 )  return(-1.0);
  if ( cal > _MMG5_EPSD ) {
    /* qual = 2.*surf / length */
    rap  = abx*abx + aby*aby + abz*abz;
    rap += acx*acx + acy*acy + acz*acz;
    rap += bcx*bcx + bcy*bcy + bcz*bcz;
    if ( rap > _MMG5_EPSD )
      return(sqrt(cal) / rap);
    else
      return(0.0);
  }
  else
    return(0.0);
}


/* compute face normal */
inline int norpts(MMG5_pPoint p1,MMG5_pPoint p2,MMG5_pPoint p3,double *n) {
  double   dd,abx,aby,abz,acx,acy,acz,det;

  /* area */
  abx = p2->c[0] - p1->c[0];
  aby = p2->c[1] - p1->c[1];
  abz = p2->c[2] - p1->c[2];

  acx = p3->c[0] - p1->c[0];
  acy = p3->c[1] - p1->c[1];
  acz = p3->c[2] - p1->c[2];

  n[0] = aby*acz - abz*acy;
  n[1] = abz*acx - abx*acz;
  n[2] = abx*acy - aby*acx;
  det  = n[0]*n[0] + n[1]*n[1] + n[2]*n[2];
  if ( det > _MMG5_EPSD ) {
    dd = 1.0 / sqrt(det);
    n[0] *= dd;
    n[1] *= dd;
    n[2] *= dd;
    return(1);
  }
  else
    return(0);
}

/* coordinates of the center of incircle of p0p1p2 and its 'size' */
inline double incircle(MMG5_pPoint p0,MMG5_pPoint p1,MMG5_pPoint p2,double *o) {
  double   dd,r,rr;

  dd = 1.0 / 3.0;
  o[0] = dd * (p0->c[0] + p1->c[0] + p2->c[0]);
  o[1] = dd * (p0->c[1] + p1->c[1] + p2->c[1]);
  o[2] = dd * (p0->c[2] + p1->c[2] + p2->c[2]);

  rr = sqrt((p0->c[0]-o[0])*(p0->c[0]-o[0]) + (p0->c[1]-o[1])*(p0->c[1]-o[1]) \
            + (p0->c[2]-o[2])*(p0->c[2]-o[2]));

  r = sqrt((p1->c[0]-o[0])*(p1->c[0]-o[0]) + (p1->c[1]-o[1])*(p1->c[1]-o[1]) \
           + (p1->c[2]-o[2])*(p1->c[2]-o[2]));
  rr = MG_MAX(rr,r);

  r = sqrt((p2->c[0]-o[0])*(p2->c[0]-o[0]) + (p2->c[1]-o[1])*(p2->c[1]-o[1]) \
           + (p2->c[2]-o[2])*(p2->c[2]-o[2]));
  rr = MG_MAX(rr,r);

  return(rr);
}

inline double diamelt(MMG5_pPoint p0,MMG5_pPoint p1,MMG5_pPoint p2) {
  double  di,dd;

  di = (p1->c[0]-p0->c[0])*(p1->c[0]-p0->c[0]) + (p1->c[1]-p0->c[1])*(p1->c[1]-p0->c[1]) \
    + (p1->c[2]-p0->c[2])*(p1->c[2]-p0->c[2]);

  dd = (p2->c[0]-p0->c[0])*(p2->c[0]-p0->c[0]) + (p2->c[1]-p0->c[1])*(p2->c[1]-p0->c[1]) \
    + (p2->c[2]-p0->c[2])*(p2->c[2]-p0->c[2]);
  di = MG_MAX(di,dd);

  dd = (p2->c[0]-p1->c[0])*(p2->c[0]-p1->c[0]) + (p2->c[1]-p1->c[1])*(p2->c[1]-p1->c[1]) \
    + (p2->c[2]-p1->c[2])*(p2->c[2]-p1->c[2]);
  di = MG_MAX(di,dd);

  return(di);
}

/* print histogram of qualities */
void inqua(MMG5_pMesh mesh,MMG5_pSol met) {
  MMG5_pTria    pt;
  double   rap,rapmin,rapmax,rapavg,med;
  int      i,k,iel,ir,imax,nex,his[5];

  rapmin  = 1.0;
  rapmax  = 0.0;
  rapavg  = med = 0.0;
  iel     = 0;

  for (k=0; k<5; k++)  his[k] = 0;
  nex  = 0;
  for (k=1; k<=mesh->nt; k++) {
    pt = &mesh->tria[k];
    if ( !MG_EOK(pt) ) {
      nex++;
      continue;
    }
    if ( met->m )
      rap = ALPHAD * calelt33_ani(mesh,met,k);
    else
      rap = ALPHAD * _MMG5_caltri_iso(mesh,NULL,pt);
    if ( rap < rapmin ) {
      rapmin = rap;
      iel    = k;
    }
    if ( rap > 0.5 )  med++;
    if ( rap < BADKAL )  mesh->info.badkal = 1;
    rapavg += rap;
    rapmax  = MG_MAX(rapmax,rap);
    ir = MG_MIN(4,(int)(5.0*rap));
    his[ir] += 1;
  }

  fprintf(stdout,"\n  -- MESH QUALITY   %d\n",mesh->nt - nex);
  fprintf(stdout,"     BEST   %8.6f  AVRG.   %8.6f  WRST.   %8.6f (%d)\n",
          rapmax,rapavg / (mesh->nt-nex),rapmin,iel);
  if ( abs(mesh->info.imprim) < 5 )  return;

  /* print histo */
  fprintf(stdout,"     HISTOGRAMM:  %6.2f %% > 0.5\n",100.0*(med/(float)(mesh->nt-nex)));
  imax = MG_MIN(4,(int)(5.*rapmax));
  for (i=imax; i>=(int)(5*rapmin); i--) {
    fprintf(stdout,"     %5.1f < Q < %5.1f   %7d   %6.2f %%\n",
            i/5.,i/5.+0.2,his[i],100.*(his[i]/(float)(mesh->nt-nex)));
  }
}

/* print histogram of qualities */
void outqua(MMG5_pMesh mesh,MMG5_pSol met) {
  MMG5_pTria    pt;
  double   rap,rapmin,rapmax,rapavg,med;
  int      i,k,iel,ir,imax,nex,his[5];

  rapmin  = 1.0;
  rapmax  = 0.0;
  rapavg  = med = 0.0;
  iel     = 0;

  for (k=0; k<5; k++)  his[k] = 0;
  nex  = 0;
  for (k=1; k<=mesh->nt; k++) {
    pt = &mesh->tria[k];
    if ( !MG_EOK(pt) ) {
      nex++;
      continue;
    }

    ddb = ( k == 669 );
    if ( met->m )
      rap = ALPHAD * _MMG5_calelt(mesh,met,pt);
    else
      rap = ALPHAD * _MMG5_caltri_iso(mesh,NULL,pt);
    if ( rap < rapmin ) {
      rapmin = rap;
      iel    = k;
    }

    if ( rap > 0.5 )  med++;
    if ( rap < BADKAL )  mesh->info.badkal = 1;
    rapavg += rap;
    rapmax  = MG_MAX(rapmax,rap);
    ir = MG_MIN(4,(int)(5.0*rap));
    his[ir] += 1;
  }

  fprintf(stdout,"\n  -- MESH QUALITY   %d\n",mesh->nt - nex);
  fprintf(stdout,"     BEST   %8.6f  AVRG.   %8.6f  WRST.   %8.6f (%d)\n",
          rapmax,rapavg / (mesh->nt-nex),rapmin,iel);
  if ( abs(mesh->info.imprim) < 5 )  return;

  /* print histo */
  fprintf(stdout,"     HISTOGRAMM:  %6.2f %% > 0.5\n",100.0*(med/(float)(mesh->nt-nex)));
  imax = MG_MIN(4,(int)(5.*rapmax));
  for (i=imax; i>=(int)(5*rapmin); i--) {
    fprintf(stdout,"     %5.1f < Q < %5.1f   %7d   %6.2f %%\n",
            i/5.,i/5.+0.2,his[i],100.*(his[i]/(float)(mesh->nt-nex)));
  }
}



