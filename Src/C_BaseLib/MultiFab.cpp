
#include <winstd.H>
#include <algorithm>
#include <cfloat>
#include <iostream>
#include <iomanip>
#include <map>
#include <limits>

#include <BLassert.H>
#include <MultiFab.H>
#include <ParallelDescriptor.H>
#include <Profiler.H>
#include <ParmParse.H>

//
// Set default values in Initialize()!!!
//
bool MultiFab::check_for_nan;
bool MultiFab::check_for_inf;

namespace
{
    bool initialized = false;
}

MultiFabCopyDescriptor::MultiFabCopyDescriptor ()
    :
    FabArrayCopyDescriptor<FArrayBox>() {}

MultiFabCopyDescriptor::~MultiFabCopyDescriptor () {}

void
MultiFab::Add (MultiFab&       dst,
	       const MultiFab& src,
	       int             srccomp,
	       int             dstcomp,
	       int             numcomp,
	       int             nghost)
{
    BL_ASSERT(dst.boxArray() == src.boxArray());
    BL_ASSERT(dst.distributionMap == src.distributionMap);
    BL_ASSERT(dst.nGrow() >= nghost && src.nGrow() >= nghost);

    for (MFIter mfi(dst); mfi.isValid(); ++mfi)
    {
        Box bx = BoxLib::grow(mfi.validbox(),nghost);

        if (bx.ok())
            dst[mfi].plus(src[mfi], bx, bx, srccomp, dstcomp, numcomp);
    }
}

void
MultiFab::Copy (MultiFab&       dst,
                const MultiFab& src,
                int             srccomp,
                int             dstcomp,
                int             numcomp,
                int             nghost)
{
    BL_ASSERT(dst.boxArray() == src.boxArray());
    BL_ASSERT(dst.distributionMap == src.distributionMap);
    BL_ASSERT(dst.nGrow() >= nghost && src.nGrow() >= nghost);

    for (MFIter mfi(dst); mfi.isValid(); ++mfi)
    {
        Box bx = BoxLib::grow(mfi.validbox(),nghost);

        if (bx.ok())
            dst[mfi].copy(src[mfi], bx, srccomp, bx, dstcomp, numcomp);
    }
}

void
MultiFab::Subtract (MultiFab&       dst,
		    const MultiFab& src,
		    int             srccomp,
		    int             dstcomp,
		    int             numcomp,
		    int             nghost)
{
    BL_ASSERT(dst.boxArray() == src.boxArray());
    BL_ASSERT(dst.distributionMap == src.distributionMap);
    BL_ASSERT(dst.nGrow() >= nghost && src.nGrow() >= nghost);

    for (MFIter mfi(dst); mfi.isValid(); ++mfi)
    {
        Box bx = BoxLib::grow(mfi.validbox(),nghost);

        if (bx.ok())
            dst[mfi].minus(src[mfi], bx, bx, srccomp, dstcomp, numcomp);
    }
}

void
MultiFab::Multiply (MultiFab&       dst,
		    const MultiFab& src,
		    int             srccomp,
		    int             dstcomp,
		    int             numcomp,
		    int             nghost)
{
    BL_ASSERT(dst.boxArray() == src.boxArray());
    BL_ASSERT(dst.distributionMap == src.distributionMap);
    BL_ASSERT(dst.nGrow() >= nghost && src.nGrow() >= nghost);

    for (MFIter mfi(dst); mfi.isValid(); ++mfi)
    {
        Box bx = BoxLib::grow(mfi.validbox(),nghost);

        if (bx.ok())
            dst[mfi].mult(src[mfi], bx, bx, srccomp, dstcomp, numcomp);
    }
}

void
MultiFab::Divide (MultiFab&       dst,
		  const MultiFab& src,
		  int             srccomp,
		  int             dstcomp,
		  int             numcomp,
		  int             nghost)
{
    BL_ASSERT(dst.boxArray() == src.boxArray());
    BL_ASSERT(dst.distributionMap == src.distributionMap);
    BL_ASSERT(dst.nGrow() >= nghost && src.nGrow() >= nghost);

    for (MFIter mfi(dst); mfi.isValid(); ++mfi)
    {
        Box bx = BoxLib::grow(mfi.validbox(),nghost);

        if (bx.ok())
            dst[mfi].divide(src[mfi], bx, bx, srccomp, dstcomp, numcomp);
    }
}

void
MultiFab::plus (Real val,
                int  nghost)
{
    plus(val,0,n_comp,nghost);
}

void
MultiFab::plus (Real       val,
                const Box& region,
                int        nghost)
{
    plus(val,region,0,n_comp,nghost);
}

void
MultiFab::mult (Real val,
                int  nghost)
{
    mult(val,0,n_comp,nghost);
}

void
MultiFab::mult (Real       val,
                const Box& region,
                int        nghost)
{
    mult(val,region,0,n_comp,nghost);
}

void
MultiFab::invert (Real numerator,
                  int  nghost)
{
    invert(numerator,0,n_comp,nghost);
}

void
MultiFab::invert (Real       numerator,
                  const Box& region,
                  int        nghost)
{
    invert(numerator,region,0,n_comp,nghost);
}

void
MultiFab::negate (int nghost)
{
    negate(0,n_comp,nghost);
}

void
MultiFab::negate (const Box& region,
                  int        nghost)
{
    negate(region,0,n_comp,nghost);
}

void
MultiFab::Initialize ()
{
    if (initialized) return;
    //
    // Set initial values!!!
    //
    MultiFab::check_for_nan = false;
    MultiFab::check_for_inf = false;

    ParmParse pp("multifab");

    pp.query("check_for_nan", check_for_nan);
    pp.query("check_for_inf", check_for_inf);

    BoxLib::ExecOnFinalize(MultiFab::Finalize);

    initialized = true;
}

void
MultiFab::Finalize ()
{
    initialized = false;
}

MultiFab::MultiFab ()
{
    Initialize();
}

MultiFab::MultiFab (const BoxArray& bxs,
                    int             ncomp,
                    int             ngrow,
                    FabAlloc        alloc)
    :
    FabArray<FArrayBox>(bxs,ncomp,ngrow,alloc)
{
    Initialize();

    if ((check_for_nan || check_for_inf) && alloc == Fab_allocate) setVal(0);
}

void
MultiFab::operator= (const Real& r)
{
    setVal(r);
}

void
MultiFab::define (const BoxArray& bxs,
                  int             nvar,
                  int             ngrow,
                  FabAlloc        alloc)
{
    this->FabArray<FArrayBox>::define(bxs,nvar,ngrow,alloc);

    if ((check_for_nan || check_for_inf) && alloc == Fab_allocate) setVal(0);
}

void
MultiFab::define (const BoxArray&            bxs,
                  int                        nvar,
                  int                        ngrow,
                  const DistributionMapping& dm,
                  FabAlloc                   alloc)
{
    this->FabArray<FArrayBox>::define(bxs,nvar,ngrow,dm,alloc);

    if ((check_for_nan || check_for_inf) && alloc == Fab_allocate) setVal(0);
}

bool 
MultiFab::contains_nan (int scomp,
                        int ncomp,
                        int ngrow) const
{
    BL_ASSERT(scomp >= 0);
    BL_ASSERT(scomp + ncomp <= nComp());
    BL_ASSERT(ncomp >  0 && ncomp <= nComp());
    BL_ASSERT(ngrow >= 0 && ngrow <= nGrow());

    bool r = false;

    for (MFIter mfi(*this); mfi.isValid() && !r; ++mfi)
    {
        const Box bx = BoxLib::grow(mfi.validbox(),ngrow);

        if (this->FabArray<FArrayBox>::get(mfi.index()).contains_nan(bx,scomp,ncomp))
            r = true;
    }

    ParallelDescriptor::ReduceBoolOr(r);

    return r;
}

bool 
MultiFab::contains_nan () const
{
    return contains_nan(0,nComp(),nGrow());
}

bool 
MultiFab::contains_inf (int scomp,
                        int ncomp,
                        int ngrow) const
{
    BL_ASSERT(scomp >= 0);
    BL_ASSERT(scomp + ncomp <= nComp());
    BL_ASSERT(ncomp >  0 && ncomp <= nComp());
    BL_ASSERT(ngrow >= 0 && ngrow <= nGrow());

    bool r = false;

    for (MFIter mfi(*this); mfi.isValid() && !r; ++mfi)
    {
        const Box bx = BoxLib::grow(mfi.validbox(),ngrow);

        if (this->FabArray<FArrayBox>::get(mfi.index()).contains_inf(bx,scomp,ncomp))
            r = true;
    }

    ParallelDescriptor::ReduceBoolOr(r);

    return r;
}

bool 
MultiFab::contains_inf () const
{
    return contains_inf(0,nComp(),nGrow());
}

static
void
AbortOnNaN (const FArrayBox& fab)
{
    std::cout << fab << std::endl;
    BoxLib::Abort("FArrayBox contains a NaN");
}

static
void
AbortOnInf (const FArrayBox& fab)
{
    std::cout << fab << std::endl;
    BoxLib::Abort("FArrayBox contains a Inf");
}

const FArrayBox&
MultiFab::operator[] (int K) const
{
    BL_ASSERT(defined(K));

    const FArrayBox& fab = this->FabArray<FArrayBox>::get(K);

    if (check_for_nan && fab.contains_nan())
        AbortOnNaN(fab);

    if (check_for_inf && fab.contains_inf())
        AbortOnInf(fab);

    return fab;
}

FArrayBox&
MultiFab::operator[] (int K)
{
    BL_ASSERT(defined(K));

    FArrayBox& fab = this->FabArray<FArrayBox>::get(K);

    if (check_for_nan && fab.contains_nan())
        AbortOnNaN(fab);

    if (check_for_inf && fab.contains_inf())
        AbortOnInf(fab);

    return fab;
}

Real
MultiFab::min (int comp,
               int nghost) const
{
    BL_ASSERT(nghost >= 0 && nghost <= n_grow);

    Real mn = std::numeric_limits<Real>::max();

    for (MFIter mfi(*this); mfi.isValid(); ++mfi)
    {
        mn = std::min(mn,get(mfi).min(BoxLib::grow(mfi.validbox(),nghost),comp));
    }

    ParallelDescriptor::ReduceRealMin(mn);

    return mn;
}

Real
MultiFab::min (const Box& region,
               int        comp,
               int        nghost) const
{
    BL_ASSERT(nghost >= 0 && nghost <= n_grow);

    Real mn = std::numeric_limits<Real>::max();

    for ( MFIter mfi(*this); mfi.isValid(); ++mfi)
    {
        Box b = BoxLib::grow(mfi.validbox(),nghost) & region;

        if (b.ok())
            mn = std::min(mn, get(mfi).min(b,comp));
    }

    ParallelDescriptor::ReduceRealMin(mn);

    return mn;
}

Real
MultiFab::max (int comp,
               int nghost) const
{
    BL_ASSERT(nghost >= 0 && nghost <= n_grow);

    Real mn = -std::numeric_limits<Real>::max();

    for ( MFIter mfi(*this); mfi.isValid(); ++mfi)
    {
        mn = std::max(mn, get(mfi).max(BoxLib::grow(mfi.validbox(),nghost),comp));
    }

    ParallelDescriptor::ReduceRealMax(mn);

    return mn;
}

Real
MultiFab::max (const Box& region,
               int        comp,
               int        nghost) const
{
    BL_ASSERT(nghost >= 0 && nghost <= n_grow);

    Real mn = -std::numeric_limits<Real>::max();

    for (MFIter mfi(*this); mfi.isValid(); ++mfi)
    {
        Box b = BoxLib::grow(mfi.validbox(),nghost) & region;

        if (b.ok())
            mn = std::max(mn, get(mfi).max(b,comp));
    }

    ParallelDescriptor::ReduceRealMax(mn);

    return mn;
}

IntVect
MultiFab::minIndex (int comp,
                    int nghost) const
{
    BL_ASSERT(nghost >= 0 && nghost <= n_grow);

    IntVect loc;

    Real mn = std::numeric_limits<Real>::max();

    for (MFIter mfi(*this); mfi.isValid(); ++mfi)
    {
        const Box  box = BoxLib::grow(mfi.validbox(),nghost);
        const Real lmn = get(mfi).min(box,comp);

        if (lmn < mn)
        {
            mn  = lmn;
            loc = get(mfi).minIndex(box,comp);
        }
    }

    const int NProcs = ParallelDescriptor::NProcs();

    if (NProcs > 1)
    {
        Array<Real> mns(1);
        Array<int>  locs(1);

        if (ParallelDescriptor::IOProcessor())
        {
            mns.resize(NProcs);
            locs.resize(NProcs*BL_SPACEDIM);
        }

        const int IOProc = ParallelDescriptor::IOProcessorNumber();

        ParallelDescriptor::Gather(&mn, 1, mns.dataPtr(), 1, IOProc);

        BL_ASSERT(sizeof(IntVect) == sizeof(int)*BL_SPACEDIM);

        ParallelDescriptor::Gather(loc.getVect(), BL_SPACEDIM, locs.dataPtr(), BL_SPACEDIM, IOProc);

        if (ParallelDescriptor::IOProcessor())
        {
            mn  = mns[0];
            loc = IntVect(D_DECL(locs[0],locs[1],locs[2]));

            for (int i = 1; i < NProcs; i++)
            {
                if (mns[i] < mn)
                {
                    mn = mns[i];

                    const int j = BL_SPACEDIM * i;

                    loc = IntVect(D_DECL(locs[j+0],locs[j+1],locs[j+2]));
                }
            }
        }

        ParallelDescriptor::Bcast(const_cast<int*>(loc.getVect()), BL_SPACEDIM, IOProc);
    }

    return loc;
}

IntVect
MultiFab::maxIndex (int comp,
                    int nghost) const
{
    BL_ASSERT(nghost >= 0 && nghost <= n_grow);

    IntVect loc;

    Real mx = -std::numeric_limits<Real>::max();

    for (MFIter mfi(*this); mfi.isValid(); ++mfi)
    {
        const Box  box = BoxLib::grow(mfi.validbox(),nghost);
        const Real lmx = get(mfi).max(box,comp);

        if (lmx > mx)
        {
            mx  = lmx;
            loc = get(mfi).maxIndex(box,comp);
        }
    }

    const int NProcs = ParallelDescriptor::NProcs();

    if (NProcs > 1)
    {
        Array<Real> mxs(1);
        Array<int>  locs(1);

        if (ParallelDescriptor::IOProcessor())
        {
            mxs.resize(NProcs);
            locs.resize(NProcs*BL_SPACEDIM);
        }

        const int IOProc = ParallelDescriptor::IOProcessorNumber();

        ParallelDescriptor::Gather(&mx, 1, mxs.dataPtr(), 1, IOProc);

        BL_ASSERT(sizeof(IntVect) == sizeof(int)*BL_SPACEDIM);

        ParallelDescriptor::Gather(loc.getVect(), BL_SPACEDIM, locs.dataPtr(), BL_SPACEDIM, IOProc);

        if (ParallelDescriptor::IOProcessor())
        {
            mx  = mxs[0];
            loc = IntVect(D_DECL(locs[0],locs[1],locs[2]));

            for (int i = 1; i < NProcs; i++)
            {
                if (mxs[i] > mx)
                {
                    mx = mxs[i];

                    const int j = BL_SPACEDIM * i;

                    loc = IntVect(D_DECL(locs[j+0],locs[j+1],locs[j+2]));
                }
            }
        }

        ParallelDescriptor::Bcast(const_cast<int*>(loc.getVect()), BL_SPACEDIM, IOProc);
    }

    return loc;
}

Real
MultiFab::norm0 (int comp, const BoxArray& ba) const
{
    Real nm0 = std::numeric_limits<Real>::min();

    std::vector< std::pair<int,Box> > isects;

    for (MFIter mfi(*this); mfi.isValid(); ++mfi)
    {
        ba.intersections(mfi.validbox(),isects);

        for (int i = 0, N = isects.size(); i < N; i++)
        {
            nm0 = std::max(nm0, get(mfi).norm(isects[i].second, 0, comp, 1));
        }
    }
 
    ParallelDescriptor::ReduceRealMax(nm0);
 
    return nm0;
}

Real
MultiFab::norm0 (int comp) const
{
    Real nm0 = std::numeric_limits<Real>::min();

    for (MFIter mfi(*this); mfi.isValid(); ++mfi)
    {
        nm0 = std::max(nm0, get(mfi).norm(mfi.validbox(), 0, comp, 1));
    }

    ParallelDescriptor::ReduceRealMax(nm0);

    return nm0;
}

Real
MultiFab::norm2 (int comp) const
{
    Real nm2 = 0.e0;

    for (MFIter mfi(*this); mfi.isValid(); ++mfi)
    {
        const Real nm_grid = get(mfi).norm(mfi.validbox(), 2, comp, 1);

        nm2 += nm_grid*nm_grid;
    }

    ParallelDescriptor::ReduceRealSum(nm2);

    nm2 = std::sqrt(nm2);

    return nm2;
}
 
Real
MultiFab::norm1 (int comp, int ngrow) const
{
    Real nm1 = 0.e0;

    for (MFIter mfi(*this); mfi.isValid(); ++mfi)
    {
        nm1 += get(mfi).norm(BoxLib::grow(mfi.validbox(),ngrow), 1, comp, 1);
    }

    ParallelDescriptor::ReduceRealSum(nm1);

    return nm1;
}


void
MultiFab::minus (const MultiFab& mf,
                 int             strt_comp,
                 int             num_comp,
                 int             nghost)
{
    BL_ASSERT(boxarray == mf.boxarray);
    BL_ASSERT(strt_comp >= 0);
    BL_ASSERT(num_comp > 0);
#ifndef NDEBUG
    int lst_comp = strt_comp + num_comp - 1;
#endif
    BL_ASSERT(lst_comp < n_comp && lst_comp < mf.n_comp);
    BL_ASSERT(nghost <= n_grow && nghost <= mf.n_grow);

    for (MFIter mfi(*this); mfi.isValid(); ++mfi)
    {
        Box bx = BoxLib::grow(mfi.validbox(),nghost);

        get(mfi).minus(mf[mfi], bx, strt_comp, strt_comp, num_comp);
    }
}

void
MultiFab::divide (const MultiFab& mf,
		  int             strt_comp,
		  int             num_comp,
		  int             nghost)
{
    BL_ASSERT(boxarray == mf.boxarray);
    BL_ASSERT(strt_comp >= 0);
    BL_ASSERT(num_comp > 0);
#ifndef NDEBUG
    int lst_comp = strt_comp + num_comp - 1;
#endif
    BL_ASSERT(lst_comp < n_comp && lst_comp < mf.n_comp);
    BL_ASSERT(nghost <= n_grow && nghost <= mf.n_grow);

    for (MFIter mfi(*this); mfi.isValid(); ++mfi)
    {
        Box bx = BoxLib::grow(mfi.validbox(),nghost);

        get(mfi).divide(mf[mfi], bx, strt_comp, strt_comp, num_comp);
    }
}

void
MultiFab::plus (Real val,
                int  comp,
                int  num_comp,
                int  nghost)
{
    BL_ASSERT(nghost >= 0 && nghost <= n_grow);
    BL_ASSERT(comp+num_comp <= n_comp);
    BL_ASSERT(num_comp > 0);

    for (MFIter mfi(*this); mfi.isValid(); ++mfi)
    {
        get(mfi).plus(val,BoxLib::grow(mfi.validbox(),nghost),comp,num_comp);
    }
}

void
MultiFab::plus (Real       val,
                const Box& region,
                int        comp,
                int        num_comp,
                int        nghost)
{
    BL_ASSERT(nghost >= 0 && nghost <= n_grow);
    BL_ASSERT(comp+num_comp <= n_comp);
    BL_ASSERT(num_comp > 0);

    for (MFIter mfi(*this); mfi.isValid(); ++mfi)
    {
        Box b = BoxLib::grow(mfi.validbox(),nghost) & region;

        if (b.ok())
            get(mfi).plus(val,b,comp,num_comp);
    }
}

void
MultiFab::plus (const MultiFab& mf,
                int             strt_comp,
                int             num_comp,
                int             nghost)
{
    BL_ASSERT(boxarray == mf.boxarray);
    BL_ASSERT(strt_comp >= 0);
    BL_ASSERT(num_comp > 0);
#ifndef NDEBUG
    int lst_comp = strt_comp + num_comp - 1;
#endif
    BL_ASSERT(lst_comp < n_comp && lst_comp < mf.n_comp);
    BL_ASSERT(nghost <= n_grow && nghost <= mf.n_grow);

    for (MFIter mfi(*this); mfi.isValid(); ++mfi)
    {
        Box bx = BoxLib::grow(mfi.validbox(),nghost);

        get(mfi).plus(mf[mfi], bx, strt_comp, strt_comp, num_comp);
    }
}

void
MultiFab::mult (Real val,
                int  comp,
                int  num_comp,
                int  nghost)
{
    BL_ASSERT(nghost >= 0 && nghost <= n_grow);
    BL_ASSERT(comp+num_comp <= n_comp);
    BL_ASSERT(num_comp > 0);

    for (MFIter mfi(*this); mfi.isValid(); ++mfi)
    {
        get(mfi).mult(val, BoxLib::grow(mfi.validbox(),nghost),comp,num_comp);
    }
}

void
MultiFab::mult (Real       val,
                const Box& region,
                int        comp,
                int        num_comp,
                int        nghost)
{
    BL_ASSERT(nghost >= 0 && nghost <= n_grow);
    BL_ASSERT(comp+num_comp <= n_comp);
    BL_ASSERT(num_comp > 0);

    for (MFIter mfi(*this); mfi.isValid(); ++mfi)
    {
        Box b = BoxLib::grow(mfi.validbox(),nghost) & region;

        if (b.ok())
            get(mfi).mult(val, b, comp, num_comp);
    }
}

void
MultiFab::invert (Real numerator,
                  int  comp,
                  int  num_comp,
                  int  nghost)
{
    BL_ASSERT(nghost >= 0 && nghost <= n_grow);
    BL_ASSERT(comp+num_comp <= n_comp);
    BL_ASSERT(num_comp > 0);

    for (MFIter mfi(*this); mfi.isValid(); ++mfi)
    {
        get(mfi).invert(numerator, BoxLib::grow(mfi.validbox(),nghost),comp,num_comp);
    }
}

void
MultiFab::invert (Real       numerator,
                  const Box& region,
                  int        comp,
                  int        num_comp,
                  int        nghost)
{
    BL_ASSERT(nghost >= 0 && nghost <= n_grow);
    BL_ASSERT(comp+num_comp <= n_comp);
    BL_ASSERT(num_comp > 0);

    for (MFIter mfi(*this); mfi.isValid(); ++mfi)
    {
        Box b = BoxLib::grow(mfi.validbox(),nghost) & region;

        if (b.ok())
            get(mfi).invert(numerator,b,comp,num_comp);
    }
}

void
MultiFab::negate (int comp,
                  int num_comp,
                  int nghost)
{
    BL_ASSERT(nghost >= 0 && nghost <= n_grow);
    BL_ASSERT(comp+num_comp <= n_comp);

    for (MFIter mfi(*this); mfi.isValid(); ++mfi)
    {
        get(mfi).negate(BoxLib::grow(mfi.validbox(),nghost),comp,num_comp);
    }
}

void
MultiFab::negate (const Box& region,
                  int        comp,
                  int        num_comp,
                  int        nghost)
{
    BL_ASSERT(nghost >= 0 && nghost <= n_grow);
    BL_ASSERT(comp+num_comp <= n_comp);

    for (MFIter mfi(*this); mfi.isValid(); ++mfi)
    {
        Box b = BoxLib::grow(mfi.validbox(),nghost) & region;

        if (b.ok())
            get(mfi).negate(b,comp,num_comp);
    }
}

void
BoxLib::linInterpAddBox (MultiFabCopyDescriptor& fabCopyDesc,
                         BoxList*                returnUnfilledBoxes,
                         Array<FillBoxId>&       returnedFillBoxIds,
                         const Box&              subbox,
                         MultiFabId              faid1,
                         MultiFabId              faid2,
                         Real                    t1,
                         Real                    t2,
                         Real                    t,
                         int                     src_comp,
                         int                     dest_comp,
                         int                     num_comp,
                         bool                    extrap)
{
    const Real teps = (t2-t1)/1000.0;

    BL_ASSERT(extrap || ( (t>=t1-teps) && (t <= t2+teps) ) );

    if (t >= t1-teps && t <= t1+teps)
    {
        returnedFillBoxIds.resize(1);
        returnedFillBoxIds[0] = fabCopyDesc.AddBox(faid1,
                                                   subbox,
                                                   returnUnfilledBoxes,
                                                   src_comp,
                                                   dest_comp,
                                                   num_comp);
    }
    else if (t > t2-teps && t < t2+teps)
    {
        returnedFillBoxIds.resize(1);
        returnedFillBoxIds[0] = fabCopyDesc.AddBox(faid2,
                                                   subbox,
                                                   returnUnfilledBoxes,
                                                   src_comp,
                                                   dest_comp,
                                                   num_comp);
    }
    else
    {
        returnedFillBoxIds.resize(2);
        BoxList tempUnfilledBoxes(subbox.ixType());
        returnedFillBoxIds[0] = fabCopyDesc.AddBox(faid1,
                                                   subbox,
                                                   returnUnfilledBoxes,
                                                   src_comp,
                                                   dest_comp,
                                                   num_comp);
        returnedFillBoxIds[1] = fabCopyDesc.AddBox(faid2,
                                                   subbox,
                                                   &tempUnfilledBoxes,
                                                   src_comp,
                                                   dest_comp,
                                                   num_comp);
        //
        // The boxarrays for faid1 and faid2 should be the
        // same so only use returnUnfilledBoxes from one AddBox here.
        //
    }
}

void
BoxLib::linInterpFillFab (MultiFabCopyDescriptor& fabCopyDesc,
                          const Array<FillBoxId>& fillBoxIds,
                          MultiFabId              faid1,
                          MultiFabId              faid2,
                          FArrayBox&              dest,
                          Real                    t1,
                          Real                    t2,
                          Real                    t,
                          int                     src_comp,   // these comps need to be removed
                          int                     dest_comp,  // from this routine
                          int                     num_comp,
                          bool                    extrap)
{
    const Real teps = (t2-t1)/1000.0;

    BL_ASSERT(extrap || ( (t>=t1-teps) && (t <= t2+teps) ) );

    if (t >= t1-teps && t <= t1+teps)
    {
        fabCopyDesc.FillFab(faid1, fillBoxIds[0], dest);
    }
    else if (t > t2-teps && t < t2+teps)
    {
        fabCopyDesc.FillFab(faid2, fillBoxIds[0], dest);
    }
    else
    {
        BL_ASSERT(dest_comp + num_comp <= dest.nComp());

        FArrayBox dest1(dest.box(), dest.nComp());
        dest1.setVal(Real(1.e30)); // FIXME - Whats a better value?
        FArrayBox dest2(dest.box(), dest.nComp());
        dest2.setVal(Real(1.e30)); // FIXME - Whats a better value?
        fabCopyDesc.FillFab(faid1, fillBoxIds[0], dest1);
        fabCopyDesc.FillFab(faid2, fillBoxIds[1], dest2);
        dest.linInterp(dest1,
                       dest1.box(),
                       src_comp,
                       dest2,
                       dest2.box(),
                       src_comp,
                       t1,
                       t2,
                       t,
                       dest.box(),
                       dest_comp,
                       num_comp);
    }
}

void
MultiFab::FillBoundary (int  scomp,
                        int  ncomp,
                        bool local,
                        bool cross)
{
    if ( n_grow <= 0 ) return;

    if ( local )
    {
        //
        // Do what you can with the FABs you own.  No parallelism allowed.
        //
        const BoxArray&            ba     = boxArray();
        const DistributionMapping& DMap   = DistributionMap();
        const int                  MyProc = ParallelDescriptor::MyProc();

        std::vector< std::pair<int,Box> > isects;

        for (MFIter mfi(*this); mfi.isValid(); ++mfi)
        {
            const int i = mfi.index();

            ba.intersections((*this)[mfi].box(),isects);

            for (int ii = 0, N = isects.size(); ii < N; ii++)
            {
                const Box& bx  = isects[ii].second;
                const int  iii = isects[ii].first;

                if (i != iii && DMap[iii] == MyProc)
                {
                    (*this)[i].copy((*this)[iii], bx, scomp, bx, scomp, ncomp);
                }
            }
        }
    }
    else
    {
        FabArray<FArrayBox>::FillBoundary(scomp,ncomp,cross);
    }
}

void
MultiFab::FillBoundary (bool local, bool cross)
{
    FillBoundary(0, n_comp, local, cross);
}

//
// Some useful typedefs.
//
typedef FabArrayBase::CopyComTag::CopyComTagsContainer CopyComTagsContainer;

typedef FabArrayBase::CopyComTag::MapOfCopyComTagContainers MapOfCopyComTagContainers;

void
MultiFab::SumBoundary (int scomp,
                       int ncomp)
{
    if ( n_grow <= 0 ) return;

    FArrayBox                         fab;
    FabArrayBase::CopyComTag          tag;
    MapOfCopyComTagContainers         m_SndTags, m_RcvTags;
    std::map<int,int>                 m_SndVols, m_RcvVols;
    std::vector< std::pair<int,Box> > isects;

    const int                  MyProc = ParallelDescriptor::MyProc();
    const DistributionMapping& DMap   = DistributionMap();
    const BoxArray&            ba     = boxArray();
    MultiFab&                  mf     = *this;

    BoxArray gba = boxArray(); gba.grow(n_grow);

    for (int i = 0, M = ba.size(); i < M; i++)
    {
        const int dst_owner = DMap[i];

        gba.intersections(ba[i],isects);

        for (int ii = 0, N = isects.size(); ii < N; ii++)
        {
            const int  j         = isects[ii].first;
            const Box& bx        = isects[ii].second;
            const int  src_owner = DMap[j];

            if ( (i == j) || (dst_owner != MyProc && src_owner != MyProc) ) continue;

            tag.box      = bx;
            tag.fabIndex = i;
            tag.srcIndex = j;

            if (dst_owner == MyProc)
            {
                if (src_owner == MyProc)
                {
                    //
                    // Do the local work right here.
                    //
                    mf[i].plus(mf[j],bx,scomp,scomp,ncomp);
                }
                else
                {
                    FabArrayBase::SetRecvTag(m_RcvTags,src_owner,tag,m_RcvVols,bx);
                }
            }
            else if (src_owner == MyProc)
            {
                FabArrayBase::SetSendTag(m_SndTags,dst_owner,tag,m_SndVols,bx);
            }
        }
    }

#ifdef BL_USE_MPI
    if (ParallelDescriptor::NProcs() == 1) return;
    //
    // Do this before prematurely exiting if running in parallel.
    // Otherwise sequence numbers will not match across MPI processes.
    //
    const int SeqNum = ParallelDescriptor::SeqNum();

    if (m_SndTags.empty() && m_RcvTags.empty())
        //
        // No parallel work for this MPI process to do.
        //
        return;

    Array<MPI_Status>  stats;
    Array<int>         recv_from, index;
    Array<double*>     recv_data, send_data;
    Array<MPI_Request> recv_reqs, send_reqs;
    //
    // Post rcvs. Allocate one chunk of space to hold'm all.
    //
    double* the_recv_data = 0;

    FabArrayBase::PostRcvs(m_RcvTags,m_RcvVols,the_recv_data,recv_data,recv_from,recv_reqs,ncomp,SeqNum);
    //
    // Send the data.
    //
    for (MapOfCopyComTagContainers::const_iterator m_it = m_SndTags.begin(),
             m_End = m_SndTags.end();
         m_it != m_End;
         ++m_it)
    {
        std::map<int,int>::const_iterator vol_it = m_SndVols.find(m_it->first);

        BL_ASSERT(vol_it != m_SndVols.end());

        const int N = vol_it->second*ncomp;

        BL_ASSERT(N < std::numeric_limits<int>::max());

        double* data = static_cast<double*>(BoxLib::The_Arena()->alloc(N*sizeof(double)));
        double* dptr = data;

        for (CopyComTagsContainer::const_iterator it = m_it->second.begin(),
                 End = m_it->second.end();
             it != End;
             ++it)
        {
            const Box& bx = it->box;
            fab.resize(bx,ncomp);
            fab.copy(mf[it->srcIndex],bx,scomp,bx,0,ncomp);
            const int Cnt = bx.numPts()*ncomp;
            memcpy(dptr,fab.dataPtr(),Cnt*sizeof(double));
            dptr += Cnt;
        }
        BL_ASSERT(data+N == dptr);

        if (FabArrayBase::do_async_sends)
        {
            send_data.push_back(data);
            send_reqs.push_back(ParallelDescriptor::Asend(data,N,m_it->first,SeqNum).req());
        }
        else
        {
            ParallelDescriptor::Send(data,N,m_it->first,SeqNum);
            BoxLib::The_Arena()->free(data);
        }
    }
    //
    // Now receive and unpack FAB data as it becomes available.
    //
    const int N_rcvs = m_RcvTags.size();

    index.resize(N_rcvs);
    stats.resize(N_rcvs);

    for (int NWaits = N_rcvs, completed; NWaits > 0; NWaits -= completed)
    {
        ParallelDescriptor::Waitsome(recv_reqs, completed, index, stats);

        for (int k = 0; k < completed; k++)
        {
            const double* dptr = recv_data[index[k]];

            BL_ASSERT(dptr != 0);

            MapOfCopyComTagContainers::const_iterator m_it = m_RcvTags.find(recv_from[index[k]]);

            BL_ASSERT(m_it != m_RcvTags.end());

            for (CopyComTagsContainer::const_iterator it = m_it->second.begin(),
                     End = m_it->second.end();
                 it != End;
                 ++it)
            {
                const Box& bx = it->box;
                fab.resize(bx,ncomp);
                const int Cnt = bx.numPts()*ncomp;
                memcpy(fab.dataPtr(),dptr,Cnt*sizeof(double));
                mf[it->fabIndex].plus(fab,bx,0,scomp,ncomp);
                dptr += Cnt;
            }
        }
    }

    BoxLib::The_Arena()->free(the_recv_data);

    if (FabArrayBase::do_async_sends && !m_SndTags.empty())
        FabArrayBase::GrokAsyncSends(m_SndTags.size(),send_reqs,send_data,stats);

#endif /*BL_USE_MPI*/
}

void
MultiFab::SumBoundary ()
{
    SumBoundary(0, n_comp);
}

