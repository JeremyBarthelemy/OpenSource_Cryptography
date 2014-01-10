/* SMTSIM simulator.
   
   Copyright (C) 1994-1999 by Dean Tullsen (tullsen@cs.ucsd.edu)
   ALL RIGHTS RESERVED.

   SMTSIM is distributed under the following conditions:

     You may make copies of SMTSIM for your own use and modify those copies.

     All copies of SMTSIM must retain all copyright notices contained within.

     You may not sell SMTSIM or distribute SMTSIM in conjunction with a
     commercial product or service without the express written consent of
     Dean Tullsen.

   THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
   IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
   PURPOSE.

Significant parts of the SMTSIM simulator were written by Gun Sirer
(before it became the SMTSIM simulator) and by Jack Lo (after it became
the SMTSIM simulator).  Therefore the following copyrights may also apply:

Copyright (C) Jack Lo
Copyright (C) E. Gun Sirer

Pieces of this code may have been derived from Jim Larus\' SPIM simulator,
which contains the following copyright:

==============================================================
   Copyright (C) 1990-1998 by James Larus (larus@cs.wisc.edu).
   ALL RIGHTS RESERVED.

   SPIM is distributed under the following conditions:

     You may make copies of SPIM for your own use and modify those copies.

     All copies of SPIM must retain my name and copyright notice.

     You may not sell SPIM or distributed SPIM in conjunction with a
     commercial product or service without the expressed written consent of
     James Larus.

   THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
   IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
   PURPOSE.
===============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sim-assert.h"
#include "main.h"
#include "cache.h"
#include "cache-array.h"
#include "tlb-array.h"
#include "cache-queue.h"
#include "cache-req.h"
#include "cache-params.h"
#include "core-resources.h"
#include "coherence-mgr.h"
#include "context.h"
#include "dyn-inst.h"
#include "sim-params.h"
#include "stash.h"
#include "prog-mem.h"
#include "app-state.h"
#include "app-mgr.h"
#include "sim-cfg.h"
#include "assoc-array.h"
#include "app-stats-log.h"
#include "mem-unit.h"
#include "prefetch-streambuf.h"
#include "deadblock-pred.h"
#include "mshr.h"


#define DEBUG 1
#if defined(DEBUG)
#   define DEBUG_CACHE_EVENTS   1
#   define DEBUG_TLBS           1
#   define TEST_CREQ_INVARIANT  0       // somewhat expensive
#else
#   define DEBUG_CACHE_EVENTS   1
#   define DEBUG_TLBS           0
#   define TEST_CREQ_INVARIANT  0
#endif


static int dtlb_lookup(CoreResources * restrict core, AppState * restrict as,
                       LongAddr addr);
static int itlb_lookup(CoreResources * restrict core, AppState * restrict as,
                       LongAddr addr);

static u64 miss_queue_full = 0;


// WARNING: reply bus may be a just a pointer copy of the request bus,
// in which case it is NOT owned and must NOT be destroyed by itself.
CoreBus *SharedCoreRequestBus;  // global var, definitively owns bus object
CoreBus *SharedCoreReplyBus;    // never NULL, but may be an alias

CacheArray *SharedL2Cache;      // May be NULL
CacheArray *SharedL3Cache;      // May be NULL
MemUnit *SharedMemUnit;
CoherenceMgr *GlobalCoherMgr;

static CacheQueue *CacheQ;

/* event holders for event-driven simulation of memory hierarchy */
static int FreeHolderCount;

static i64 totmem=0, totmemdelay=0;


const char *CacheSource_names[] = { 
    "None",
    "L1_ICache", "L1_DCache", "L1_D_I", "L1_DStreamBuf", "L1_Ds_I",
    "L1_Ds_D", "L1_Ds_D_I", "WB", "Coher", "L2", "L3", NULL
};
const char *CacheAction_names[] = { 
    "L1FILL", "BUS_REQ", "BUS_REPLY", "BUS_WB",
    "COHER_WBI_L1", "COHER_WBI_L2_UP", "COHER_WBI_L2_DOWN",
    "COHER_WAIT", "COHER_REPLY",
    "L2ACCESS", "L2FILL", "L2_WB", "L3ACCESS", "L3FILL", "L3_WB", 
    "MEMACCESS", "MEM_WB", NULL
};


const char * 
fmt_creq_static(const struct CacheRequest *creq)
{
    static char buf[2048];
    int limit = NELEM(buf);
    int used = 0;

    // (Pretend this is a closure, that acts like printf().  whee!)
    // C has variadic macros, but C++ doesn't; we really should allow for
    // C++ compilation
    #define FMT2(fmt, arg) \
        used += e_snprintf(buf + used, limit - used, fmt, arg)
    #define FMT1(str) FMT2("%s", str)

    if (!creq) {
        FMT1("(null)");
        return buf;
    }

    FMT2("created %s", fmt_i64(creq->create_time));
    FMT2(" begin %s", fmt_i64(creq->request_time));
    if (creq->blocked)
        FMT1("(blocked)");
    FMT2(" addr %s", fmt_laddr(creq->base_addr));
    FMT2(" action %s", CacheAction_names[creq->action]);
    FMT2(" access_type %s", CacheAccessType_names[creq->access_type]);

    if ((creq->cores[0].core && (creq->cores[0].src == CSrc_Coher)) ||
        creq->coher_wb_type) {
        FMT2(" coher_wb_type %s",
             CoherAccessResult_names[creq->coher_wb_type]);
    }

    if (creq->service_level != SERVICED_UNKNOWN)
        FMT2(" service_level %s", fmt_service_level(creq->service_level));

    FMT1(" cores {");
    for (int cnum = 0; creq->cores[cnum].core; cnum++) {
        FMT2(" C%i", creq->cores[cnum].core->core_id);
        FMT2("/%s", CacheSource_names[creq->cores[cnum].src]);
    }
    FMT1(" }");

    if (creq->irequestor) {
        FMT1(" ireq {");
        const context * restrict ctx = creq->irequestor;
        while (ctx) {
            FMT2(" T%i", ctx->id);
            ctx = ctx->mergethread;
        }
        FMT1(" }");
    }

    if (creq->drequestor) {
        FMT1(" dreq {");
        const activelist * restrict inst = creq->drequestor;
        while (inst) {
            FMT2(" T%d", inst->thread);
            FMT2("s%d", inst->id);
            inst = inst->mergeinst;
        }
        FMT1(" }");
    }

    if (creq->dependent_coher) {
        const CacheRequest * restrict dep_creq = creq->dependent_coher;
        FMT1(" dep_cohe {");
        while (dep_creq) {
            const CoreResources * restrict dep_core = dep_creq->cores[0].core;
            FMT2(" C%d", (dep_core) ? dep_core->core_id : -1);
            FMT2("(%s", CacheAction_names[dep_creq->action]);
            FMT2("@%s)", fmt_i64(dep_creq->request_time));
            dep_creq = dep_creq->dependent_coher;
        }
        FMT1(" }");
    }

    if (creq->coher_for) {
        FMT1(" coher_for ");
        const CoreResources * restrict for_core =
            creq->coher_for->cores[0].core;
        FMT2("C%d", (for_core) ? for_core->core_id : -1);
        FMT2("(%s", CacheAction_names[creq->coher_for->action]);
        FMT2("@%s)", fmt_i64(creq->coher_for->request_time));
    }

    if (creq->blocked_apps[0]) {
        FMT1(" blocked_apps {");
        for (int anum = 0; creq->blocked_apps[anum]; anum++) 
            FMT2(" %i", creq->blocked_apps[anum]->app_id);
        FMT1(" }");
    }

    if (creq->is_dirty_fill) 
        FMT1(" is_dirty_fill");

    if (creq->coher_data_seen) 
        FMT1(" coher_data_seen");

    #undef FMT1
    #undef FMT2

    return buf;
}


const char *
fmt_service_level(int service_level)
{
    static char buf[16];
    switch (service_level) {
    case SERVICED_UNKNOWN:
        e_snprintf(buf, NELEM(buf), "UNKNOWN");
        break;
    case SERVICED_MEM:
        e_snprintf(buf, NELEM(buf), "MEM");
        break;
    case SERVICED_COHER:
        e_snprintf(buf, NELEM(buf), "COHER");
        break;
    case SERVICED_NONE:
        e_snprintf(buf, NELEM(buf), "NONE");
        break;
    case 1:
    case 2:
    case 3:
        e_snprintf(buf, NELEM(buf), "L%d", service_level);
        break;
        break;
    default:
        abort_printf("invalid service_level value (%d)\n", service_level);
    }
    return buf;
}


#if defined(DEBUG) && DEBUG_CACHE_EVENTS
    static void 
    dump_creq(const CacheRequest *creq, const char *tag)
    {
        if (debug)
            printf("cache:, %s, %s, %s\n",
                   fmt_i64(cyc), tag, fmt_creq_static(creq));
    }
#else
#   define dump_creq(creq, tag) ((void) 0)
#endif  // DEBUG_CACHE_EVENTS


/* ireq must not already be in creq */
static void
add_ireq_to_creq(CacheRequest *creq, context *ireq)
{
    if (!creq->irequestor) {
        creq->irequestor = ireq;
    } else {
        context *prev = creq->irequestor;
        sim_assert(prev != ireq);
        while (prev->mergethread) {
            prev = prev->mergethread;
            sim_assert(prev != ireq);
        }
        prev->mergethread = ireq;
    }
    ireq->mergethread = NULL;
    ireq->imiss_cache_entry = creq;
}


/* dreq must not already be in creq */
static void
add_dreq_to_creq(CacheRequest *creq, activelist *dreq)
{
    if (!creq->drequestor) {
        creq->drequestor = dreq;
    } else {
        activelist *prev = creq->drequestor;
        sim_assert(prev != dreq);
        while (prev->mergeinst) {
            prev = prev->mergeinst;
            sim_assert(prev != dreq);
        }
        prev->mergeinst = dreq;
    }
    dreq->mergeinst = NULL;
    dreq->dmiss_cache_entry = creq;
}


static CacheSource
merge_cache_sources(CacheSource old_source, CacheSource new_source)
{
    CacheSource result = old_source;
    if (old_source == new_source) {
        // no change
    } else if (CACHE_SOURCE_L1_ONLY(old_source) &&
        CACHE_SOURCE_L1_ONLY(new_source)) {
        result |= new_source;
    } else {
        abort_printf("unable to merge new CacheSource '%s' with existing"
                     "'%s'\n", CacheSource_names[new_source],
                     CacheSource_names[old_source]);
    }
    return result;
}


/* Okay if core already in creq */
static void
add_core_to_creq(CacheRequest *creq, CoreResources *core,
                 CacheSource core_source)
{
    int cnum;

    for (cnum = 0; creq->cores[cnum].core; cnum++) {
        if (creq->cores[cnum].core == core) {
            creq->cores[cnum].src =
                merge_cache_sources(creq->cores[cnum].src, core_source);
            break;
        }
    }

    sim_assert(cnum < GlobalParams.num_cores);

    if (creq->cores[cnum].core != core) {
        creq->cores[cnum].core = core;
        creq->cores[cnum].src = core_source;
        creq->cores[cnum + 1].core = NULL;
    }
}


static int
core_subscribed(const CacheRequest *creq, const CoreResources *core)
{
    for (int cnum = 0; creq->cores[cnum].core; cnum++) {
        if (creq->cores[cnum].core == core)
            return 1;
    }
    return 0;
}


static int
creq_single_core(const CacheRequest *creq)
{
    return creq->cores[0].core && !creq->cores[1].core;
}


void
initcache(void) 
{
    int i;

    FreeHolderCount = GlobalParams.mem.cache_request_holders;
    for (i=0; i < FreeHolderCount; i++) {
        CacheFreeHolders[i] = &CacheHolders[i];
        // Purposefully violates creq_invariant 
        CacheHolders[i].action = (CacheAction) -1;
    }

    if (!(CacheQ = cacheq_create())) {
        goto fail;
    }

    {
        SharedCoreRequestBus = corebus_create();
        if (GlobalParams.mem.split_bus) {
            SharedCoreReplyBus = corebus_create();
        } else {
            SharedCoreReplyBus = SharedCoreRequestBus;
        }
    }
    
    if (!GlobalParams.mem.private_l2caches) {
        if (!(SharedL2Cache = cache_create(-2, GlobalParams.mem.l2cache_geom,
                                           &GlobalParams.mem.l2cache_timing,
                                           NULL, NULL, cyc)))
            goto fail;
    }

    if (GlobalParams.mem.use_l3cache) {
        if (!(SharedL3Cache = cache_create(-3, GlobalParams.mem.l3cache_geom,
                                           &GlobalParams.mem.l3cache_timing,
                                           NULL, NULL, cyc)))
            goto fail;
    } else {
        SharedL3Cache = NULL;
    }

    SharedMemUnit = memunit_create(&GlobalParams.mem.main_mem, cyc);
    
    return;

fail:
    fprintf(stderr, "initcache (%s:%i): failure creating shared cache "
            "structures\n", __FILE__, __LINE__);
    exit(1);
}


void
init_coher(void) 
{
    if ((GlobalParams.num_cores > 1) && GlobalParams.mem.use_coherence) 
        GlobalCoherMgr = cm_create();
}


static void 
free_cache_request(CacheRequest *old) 
{
    dump_creq(old, "free");

    sim_assert(!old->drequestor);
    sim_assert(!old->irequestor);

    // If this isn't true, we've left an app blocked forever!
    sim_assert(!old->blocked_apps[0]);

    // If there's a dependent_coher still linked it, it will wait forever
    sim_assert(!old->dependent_coher);

    assert_ifthen(TEST_CREQ_INVARIANT, creq_invariant(old, 1));

    // Set something which violates creq_invariant(), to help notice
    // if this gets re-used by accident.  See also initcache().
    old->action = (CacheAction) -1;

    sim_assert(FreeHolderCount < GlobalParams.mem.cache_request_holders);
    CacheFreeHolders[FreeHolderCount] = old;
    FreeHolderCount++;
}


static CacheRequest *
get_c_request_holder(i64 request_time, LongAddr base_addr,
                     CacheAccessType access_type,
                     CacheAction action, CacheSource source,
                     CoreResources *first_core) 
{
    CacheRequest *new;

    while (FreeHolderCount < 1) {
        /* didn't find one -- we don't want this ever to happen, so it is
           okay if we take drastic action.  If this happens, bump up HOLDERS */
        /*stalling whole processor to get CacheRequest structures! */
        if (miss_queue_full == 0) {
            // This is historical behavior; we could just as easily use
            // malloc() / free() for individual cache holders, and I'm not
            // sure why we don't.  We'll at least log that something bizarre is
            // going on, the first time it occurs.
            err_printf("%s WARNING: stalling entire system to free up some "
                       "CacheRequest_holders; this is abnormal.  (first "
                       "instance at cyc %s)\n", __func__, fmt_now());
        }
        cyc++;
        miss_queue_full++;
        process_cache_queues();
    }
    new = CacheFreeHolders[FreeHolderCount - 1];
    FreeHolderCount--;

    sim_assert(request_time >= -1);     // -1: "don't know yet"
    new->request_time = request_time;
    new->serial_num = -1;
    new->blocked = 0;
    new->action = action;
    new->base_addr = base_addr;
    new->access_type = access_type;
    new->coher_accessed = 0;
    new->coher_wb_type = Coher_NoStall;
    new->service_level = SERVICED_UNKNOWN;
    new->create_time = cyc;
    new->irequestor = NULL;
    new->drequestor = NULL;
    new->dependent_coher = NULL;

    new->cores[0].core = first_core;
    new->cores[0].src = source;
    new->cores[1].core = NULL;

    new->blocked_apps[0] = NULL;

    new->coher_for = NULL;
    new->is_dirty_fill = 0;
    new->coher_data_seen = 0;
    //DEBUGPRINTF("CACHETRACE, %s, %s, %s, %s, %s, %s \n", fmt_i64(cyc), base_addr, access_type, action, source, *first_core);
   // printf("CACHETRACE1: , %s, %s, %s, %s, %s, %i\n", fmt_i64(cyc), fmt_laddr(base_addr), CacheSource_names[source], CacheAction_names[action], CacheAccessType_names[access_type], first_core->core_id);
//	if (base_addr != NULL & source >= 0)
//{	printf("CACHETRACE1: , %s, %s, %s, %s, %s, %i\n", fmt_i64(cyc), fmt_laddr(base_addr), CacheSource_names[source], CacheAction_names////[action], CacheAccessType_names[access_type], first_core->core_id);}
//	else
//{	printf("CACHETRACE1: , %s, %s, %s, %s, %s, %i\n", fmt_i64(cyc), source, action, access_type, first_core->core_id);
//}
    //DEBUGPRINTF("CACHETRACE, %s, %s, %s, %s, %s, %s \n", fmt_i64(cyc), base_addr, access_type, action, source, *first_core);
    // currently, creq_invariant won't accept request_time==-1
    //assert_ifthen(TEST_CREQ_INVARIANT, creq_invariant(new, 1));

    return new;
}


static void
enqueue_creq(CacheRequest *creq, const char *msg)
{
    static i64 serial_num = 0;

    assert_ifthen(TEST_CREQ_INVARIANT, creq_invariant(creq, 1));
    creq->serial_num = serial_num;
    serial_num++;
    dump_creq(creq, msg);
    cacheq_enqueue(CacheQ, creq);
}


static void
place_in_cache_queue(CacheRequest *creq) 
{
    int num_cores = GlobalParams.num_cores;
    int fork_cores;

    sim_assert(creq->request_time >= 0);

    if (GlobalParams.mem.private_l2caches) {
        fork_cores = (creq->action == L2FILL);
    } else {
        fork_cores = (creq->action == L1FILL);
    }

    if (fork_cores && creq->cores[1].core) {
        // temporary linked-list heads
        activelist *dreq = creq->drequestor;
        context *ireq = creq->irequestor;
        CacheRequest *reqs[num_cores];          // core_id -> requests
        int core_order[num_cores + 1];          // subscriber order, -1 ends
        int i;

        // We shouldn't have single requests which span cores, when coherence
        // is enabled; each core's traffic should go through cm_access() on
        // its own and then wait for each other via "dependent_coher" linkage
        sim_assert(!GlobalCoherMgr);

        for (i = 0; i < num_cores; i++) 
            reqs[i] = NULL;

        for (i = 0; creq->cores[i].core; i++) {
            int core_id = creq->cores[i].core->core_id;
            core_order[i] = core_id;
            sim_assert(reqs[core_id] == NULL);
            if (i == 0) {
                // re-use creq for first core
                reqs[core_id] = creq;
            } else {
                // allocate new creqs for remaining cores
                reqs[core_id] =
                    get_c_request_holder(creq->request_time, creq->base_addr,
                                         creq->access_type, 
                                         creq->action, creq->cores[i].src, 
                                         creq->cores[i].core);
                reqs[core_id]->service_level = creq->service_level;
            }
        }

        core_order[i] = -1;

        // Throw out first request's linked requestors/cores
        creq->drequestor = NULL;
        creq->irequestor = NULL;
        creq->cores[1].core = NULL;     // Leave first request for this core

        // Now, re-link all requestors/cores to per-core requests as needed

        while (ireq) {
            int core_id = ireq->core->core_id;
            context *ireq_next = ireq->mergethread;
            add_ireq_to_creq(reqs[core_id], ireq);
            ireq = ireq_next;
        }

        while (dreq) {
            int core_id = Contexts[dreq->thread]->core->core_id;
            activelist *dreq_next = dreq->mergeinst;
            add_dreq_to_creq(reqs[core_id], dreq);
            dreq = dreq_next;
        }

        for (i = 0; core_order[i] >= 0; i++) {
            CacheRequest *req = reqs[core_order[i]];
            if (req)
                enqueue_creq(req, "enqueue-fork");
        }
    } else {
        enqueue_creq(creq, "enqueue");
    }
}


static void 
merge_irequest(CacheRequest *entry, LongAddr base_addr, context *ctx) 
{
    sim_assert(laddr_eq(entry->base_addr, base_addr));
    add_ireq_to_creq(entry, ctx);
    add_core_to_creq(entry, ctx->core, CSrc_L1_ICache);
    dump_creq(entry, "I-merged");
}


static void 
merge_drequest(CacheRequest *entry, LongAddr base_addr,
               activelist *meminst) 
{
    sim_assert(laddr_eq(entry->base_addr, base_addr));
    add_dreq_to_creq(entry, meminst);
    add_core_to_creq(entry, Contexts[meminst->thread]->core, CSrc_L1_DCache);
    dump_creq(entry, "D-merged");
}


// (ok if already present)
static void
add_blocked_app(CacheRequest * restrict creq, AppState * restrict as)
{
    int num_blocked = 0, already_regd = 0;
    while (creq->blocked_apps[num_blocked]) {
        num_blocked++;
        if (creq->blocked_apps[num_blocked] == as) {
            already_regd = 1;
            break;
        }
    }
    if (!already_regd) {
        // inefficient grow-by-constant, but we expect these to stay small
        creq->blocked_apps =
            erealloc(creq->blocked_apps,
                     (num_blocked + 2) * sizeof(creq->blocked_apps[0]));
        creq->blocked_apps[num_blocked] = as;
        creq->blocked_apps[num_blocked + 1] = NULL;
    }
}


// Release any blocked apps (from cache_register_blocked_app)
static void
release_blocked_apps(CacheRequest *creq)
{
    int anum;
    for (anum = 0; creq->blocked_apps[anum]; anum++) {
        AppState *blocked_app = creq->blocked_apps[anum];
        appmgr_signal_missdone(GlobalAppMgr, blocked_app);
        // Redundant in some cases
        if (GlobalLongMemLogger) {
            longmem_log_complete(GlobalLongMemLogger, blocked_app->app_id,
                                 -1);
        }
    }
    creq->blocked_apps[0] = NULL;
}


// Register a coherence-related dependence between two cache requests;
// "waiter_creq" will be linked from "targ_creq", where it will wait until
// "targ_creq" has reached an appropriate stage, at which point "waiter_creq"
// will be re-submitted to the cache request queue.  Dependent requests will
// be re-awakened in order.  (If waiter_creq itself has dependent creq's
// linked in, the links are preserved.)
// 
static void 
append_dep_coher(CacheRequest *targ_creq, CacheRequest *waiter_creq)
{
    sim_assert(laddr_eq(targ_creq->base_addr, waiter_creq->base_addr));
    {
        CacheRequest *last_dep = targ_creq;
        while (last_dep->dependent_coher) {
            last_dep = last_dep->dependent_coher;
        }
        last_dep->dependent_coher = waiter_creq;
    }
    {
        sim_assert(cache_action_incore(waiter_creq->action));
        sim_assert(creq_single_core(waiter_creq));
    }
    dump_creq(targ_creq, "coher-dep-add");
}


// release one dependent coher. request
//   release_this_req: 0 means "release the next dep-cohe child of creq"
//   release_this_req: !0 means "release creq itself"; caller must unlink
static void 
release_dep_coher(CacheRequest *creq, int release_this_req, i64 min_time)
{
    CacheRequest *dep_creq;
    if (release_this_req) {
        dep_creq = creq;                        // Caller must unlink
    } else {
        dep_creq = creq->dependent_coher;
        creq->dependent_coher = NULL;           // Unlink from parent 
    }
    if (dep_creq->request_time < min_time)
        dep_creq->request_time = min_time;
    dump_creq(dep_creq, "coher-dep-release");
    // cacheq_dequeue_blocked(CacheQ, dep_creq);
    // dep_creq->blocked = 0;
    place_in_cache_queue(dep_creq);
}


// Remove the dependent_coher chain from "creq", splitting it into multiple
// chains by the core ID of each request.  The head of each chain (i.e., the
// earliest request for that core) is stored the corresponding index of
// per_core_heads[].
static void
separate_dep_coher(CacheRequest *creq, CacheRequest **per_core_heads,
                   int num_cores)
{
    CacheRequest *per_core_tails[num_cores];
    for (int i = 0; i < num_cores; i++) {
        per_core_heads[i] = NULL;
        per_core_tails[i] = NULL;
    }
    CacheRequest *walk = creq->dependent_coher;
    creq->dependent_coher = NULL;
    while (walk) {
        CacheRequest *next = walk->dependent_coher;
        walk->dependent_coher = NULL;
        sim_assert(creq_single_core(walk));
        int walk_core = walk->cores[0].core->core_id;
        if (!per_core_heads[walk_core]) {       // first dep-cohe for this core
            per_core_heads[walk_core] = walk;
        } else {
            per_core_tails[walk_core]->dependent_coher = walk;
        }
        per_core_tails[walk_core] = walk;
        walk = next;
    }
}


// Merge the lists of data and instructions requests waiting in "src_creq",
// into "targ_creq".  They are unlinked from "src_creq", which is then freed.
static void 
merge_and_free_creq(CacheRequest *targ_creq, CacheRequest *src_creq)
{
    sim_assert(laddr_eq(targ_creq->base_addr, src_creq->base_addr));
    {
        activelist *miss_inst = src_creq->drequestor;
        src_creq->drequestor = NULL;
        while (miss_inst) {
            activelist *next_inst = miss_inst->mergeinst;
            add_dreq_to_creq(targ_creq, miss_inst);     // inefficient
            miss_inst = next_inst;
        }
    }

    {
        context *miss_ctx = src_creq->irequestor;
        src_creq->irequestor = NULL;
        while (miss_ctx) {
            context *next_ctx = miss_ctx->mergethread;
            add_ireq_to_creq(targ_creq, miss_ctx);      // inefficient
            miss_ctx = next_ctx;
        }
    }

    {
        for (int i = 0; src_creq->cores[i].core; ++i) {
            add_core_to_creq(targ_creq, src_creq->cores[i].core,
                             src_creq->cores[i].src);
        }
    }


    if (src_creq->dependent_coher) {
        // crufty: we know append_dep_coher() will link the entire chain
        // of dependent requests, so no need to walk over it
        append_dep_coher(targ_creq, src_creq->dependent_coher);
        src_creq->dependent_coher = NULL;
    }

    for (int anum = 0; src_creq->blocked_apps[anum]; anum++) {
        add_blocked_app(targ_creq, src_creq->blocked_apps[anum]);
    }
    src_creq->blocked_apps[0] = NULL;

    dump_creq(targ_creq, "merged");
    assert_ifthen(TEST_CREQ_INVARIANT, creq_invariant(targ_creq, 1));
    free_cache_request(src_creq);
}


static void
enq_evict_writeback(CoreResources *evict_core, const CacheEvicted *evicted,
                    CacheAction action, i64 ready_time)
{
    sim_assert((action == L2_WB) || (action == L3_WB) ||
               (action == BUS_WB) || (action == MEM_WB));
    CacheRequest *wb_request =
        get_c_request_holder(ready_time, evicted->base_addr, Cache_Write, 
                             action, CSrc_WB, evict_core);
    place_in_cache_queue(wb_request);
}

//Output to update the ready_time for the NUCA case
static int DelayCalc(const CacheRequest * creq)
{
	int cur_bank;
	int Temp;
	
	Temp = fmt_laddr(creq->base_addr) - '0';
	
	cur_bank = Temp % 16;

	printf("cur_bank:%i, cur_address:%s", cur_bank, fmt_laddr(creq->base_addr));
 
	switch (cur_bank)
	{
		case 0:
		return 20+(4*(16/4));
		case 1:
		return 20+(4*(1/4));
		case 2:
		return 20+(4*(2/4)); 
		case 3:
		return 20+(4*(3/4));
		case 4:
		return 20+(4*(4/4));
		case 5:
		return 20+(4*(5/4));
		case 6:
		return 20+(4*(6/4));
		case 7:
		return 20+(4*(7/4));
		case 8:
		return 20+(4*(8/4));
		case 9:
		return 20+(4*(9/4));
		case 10:
		return 20+(4*(10/4));
		case 11:
		return 20+(4*(11/4));
		case 12:
		return 20+(4*(12/4));
		case 13:
		return 20+(4*(13/4));
		case 14:
		return 20+(4*(14/4));
		case 15:
		return 20+(4*(15/4));
		default:
		return 20+(4*(0/4));
	}
 
}


// Generate and enqueue coherence-related writeback/invalidate requests to
// peers.
//
// We'll cheesily use the "access_type" field of the generated requests to
// encode whether the reply contains data at all, and whether that data is
// dirty:
//   Cache_Read: no data
//   Cache_ReadExcl: clean data
//   Cache_Write: dirty data
// They will start out as Cache_Read, and then be changed to the other
// types as peer caches check their copies.
static void
enq_coher_peer_msgs(CacheRequest *for_creq,
                    CoherAccessResult stall_type,
                    const CoherWaitInfo *peer_info, i64 ready_time)
{
    sim_assert(COHER_STALLS_FOR_PEERS(stall_type));
    sim_assert(ENUM_OK(CoherAccessResult, stall_type));
    CacheAction peer_action = (GlobalParams.mem.private_l2caches) ?
        COHER_WBI_L2_UP : COHER_WBI_L1;
    for (int peer_idx = 0; peer_idx < peer_info->node_count; peer_idx++) {
        int peer_core_id = peer_info->nodes[peer_idx];
        sim_assert(IDX_OK(peer_core_id, CoreCount));
        CoreResources *peer_core = Cores[peer_core_id];
        CacheRequest *peer_creq = 
            get_c_request_holder(ready_time, for_creq->base_addr, Cache_Read,
                                 peer_action, CSrc_Coher, peer_core);
        peer_creq->coher_for = for_creq;
        peer_creq->coher_wb_type = stall_type;
        place_in_cache_queue(peer_creq);
    }
}

static int
coher_reply_has_data(const CacheRequest * restrict creq)
{
    return (creq->access_type == Cache_ReadExcl) ||
        (creq->access_type == Cache_Write);
}
static int
coher_reply_has_dirty_data(const CacheRequest * restrict creq)
{
    return (creq->access_type == Cache_Write);
}

static void
update_coher_creq_for_yield(CacheRequest * restrict creq,
                            CacheFillOutcome yield_result)
{
    sim_assert(creq_single_core(creq));
    sim_assert(creq->cores[0].src == CSrc_Coher);
    switch (yield_result) {
    case CacheFill_NoEvict:
        // yield found no data; no state change
        break;
    case CacheFill_EvictClean:
        // yield found clean data; if creq indicates "no data", update it
        if (creq->access_type == Cache_Read)
            creq->access_type = Cache_ReadExcl;
        break;
    case CacheFill_EvictDirty:
        // yield found dirty data; update creq to indicate need for WB
        sim_assert((creq->coher_wb_type == Coher_StallForWB) ||
                   (creq->coher_wb_type == Coher_StallForXfer));
        creq->access_type = Cache_Write;
        break;
    default:
        ENUM_ABORT(CacheFillOutcome, yield_result);
    }
 //printf("CACHETRACE: , %s, %s, %s, %s, %s, %i\n", fmt_i64(cyc), fmt_laddr(creq->base_addr), CacheSource_names[creq->cores[0].src], CacheAction_names[creq->action], CacheAccessType_names[creq->access_type], creq->cores[0].core->core_id);


}


static AppState *
first_request_app(const CacheRequest * restrict creq)
{
    AppState *result = NULL;
    // This is a little shady, but is only used for stats
    CacheSource source = (creq->cores[0].core) ? creq->cores[0].src :
        CSrc_None;
    if (CACHE_SOURCE_L1_CONTAINS(source, CSrc_L1_DCache)) {
        const activelist * restrict meminst = creq->drequestor;
        while (meminst) {
            if (meminst->as) {
                result = meminst->as;
                break;
            }
            meminst = meminst->mergeinst;
        }
    } else if (CACHE_SOURCE_L1_CONTAINS(source, CSrc_L1_ICache)) {
        if (creq->irequestor && creq->irequestor->as)
            result = creq->irequestor->as;
    }
    return result;
}


static void
log_app_l23_access(const CacheRequest * restrict creq,
                   int is_l3, int is_hit)
{
    // Bill this access to the first requesting app.  We ignore writeback
    // accesses in the per-app billing, though they still show up in the
    // per-cache stats.
    AppState * restrict miss_as = first_request_app(creq);
    if (miss_as) {
        ASE_HitRate * restrict hit_rate =
            (is_l3) ? &miss_as->extra->hitrate.l3cache :
            &miss_as->extra->hitrate.l2cache;
        hit_rate->acc++;
        if (is_hit)
            hit_rate->hits++;

        if (is_l3) {
            i64 * l3cache_acc = &miss_as->extra->l3cache_acc;
            (*l3cache_acc)++;   
        }
        else {
            i64 * l2cache_acc = &miss_as->extra->l2cache_acc;
            (*l2cache_acc)++;   
        }
    }
}


// query: does the given core possess the given block, from the perspective
// of the coherence manager?
// Returns:
//   0: NO.  The core does not have the given block cached, and there
//      is no outstanding traffic (inbound in-core fills, outbound writebacks
//      to on-core caches) which will allow the core to get a copy
//      without a future call to cm_access().  Note that this still allows for
//      outbound writebacks from core-private caches, which will write-around
//      to off-core caches, so long as the core must still use cm_access()
//      before re-gaining access.
//   1: MAYBE.  This isn't a guarantee of availability, but the block may
//      be cached, or there may be relevant traffic.
static int
core_has_coher_block_maybe(const CoreResources *core, LongAddr base_addr)
{
    int core_id = core->core_id;
    int result = 0;

    if (cache_access_ok(core->dcache, base_addr, Cache_Read) ||
        cache_access_ok(core->icache, base_addr, Cache_Read) ||
        (GlobalParams.mem.private_l2caches &&
         cache_access_ok(core->l2cache, base_addr, Cache_Read))) {
        // Found in cache probe.  (note that these probes don't search the
        // caches' outbound writeback buffers; however, such writebacks create
        // cache requests, which will be discovered by cacheq_find* calls
        // below.)
        result = 1;
    }
    if (!result && core->d_streambuf &&
        pfsg_access_ok(core->d_streambuf, base_addr, Cache_Read)) {
        result = 1;
    }
    if (!result) {
        CacheRequest *offcore_miss_creq = 
            cacheq_find(CacheQ, base_addr, CACHEQ_SHARED, CQFS_Miss);
        // Off-core misses have already been cleared for coherent access
        // through process_bus_req().
        if (offcore_miss_creq && offcore_miss_creq->coher_accessed) {
            result = 1;
        }
    }
    if (!result) {
        // On-core miss requests which are "inbound" have already been cleared
        // for coherent access, either when they went off-core at
        // process_bus_req(), or (transitively) when they hit on an
        // in-core cached copy.
        CacheRequest *oncore_miss_creq =
            cacheq_find(CacheQ, base_addr, core_id, CQFS_Miss);
        if (oncore_miss_creq &&
            (oncore_miss_creq->service_level != SERVICED_UNKNOWN)) {
            result = 1;
        }
    }
    if (0 && !result) {
        // Writebacks to on-core caches represent another place the data can
        // hide, if we do write-allocate.  (Currently, this isn't necessary,
        // since we always write-around if there's a miss at the WB target; a
        // WB will only actually write into a cache if the tag was already
        // present, in which case means we'll have already discovered it in
        // some other way.)
        CacheRequest **reqs = cacheq_find_multi(CacheQ, base_addr,
                                                core_id, CQFS_WB);
        if (reqs[0]) {
            result = 1;
        }
        free(reqs);
    }
    DEBUGPRINTF("core_has_coher_block_maybe(C%d, %s) -> %d\n",
                core->core_id, fmt_laddr(base_addr), result);
    return result;
}


// Called when some core-private resource discards a cache block.  If
// coherence is in use, and no other on-core resources hold the block, it
// can be removed from the CoherenceMgr.  (This is somewhat sketchy in that
// it effectively implements evict-notification messages for free.)
static void
cache_core_evict_maybe(const CoreResources *core, LongAddr base_addr)
{
    if (GlobalCoherMgr && !core_has_coher_block_maybe(core, base_addr)) {
        cm_evict_notify(GlobalCoherMgr, base_addr, core->core_id);
    }
}


static CacheBankOp
cache_access_to_bankop(CacheAccessType access_type)
{
    CacheBankOp result;
    switch (access_type) {
    case Cache_Read:            result = CacheBank_LookupR; break;
    case Cache_ReadExcl:        result = CacheBank_LookupREx; break;
    case Cache_Upgrade:         result = CacheBank_LookupUpgrade; break;
    default:
        ENUM_ABORT(CacheAccessType, access_type);
        result = CacheBank_LookupR;
    }
    return result;
}


static CacheAction
l1_route_down(void)
{
    return (GlobalParams.mem.private_l2caches) ? L2ACCESS : BUS_REQ;
}


static CacheAction
bus_route_down(void)
{
    return (GlobalParams.mem.private_l2caches) ?
        ((GlobalParams.mem.use_l3cache) ? L3ACCESS : MEMACCESS) : L2ACCESS;
}


static CacheAction
bus_route_up(void)
{
    return (GlobalParams.mem.private_l2caches) ? L2FILL : L1FILL;
}


static void
icache_replace(CacheRequest *for_creq, CoreResources *core, LongAddr base_addr,
               i64 start_time)
{
    CacheEvicted evicted;
    CacheFillOutcome fill_stat;
    int block_already_present = cache_access_ok(core->icache, base_addr,
                                                Cache_Read);
    laddr_set(evicted.base_addr, 0, 0);
    fill_stat = cache_fill(core->icache, base_addr, Cache_Read, &evicted);
    DEBUGPRINTF("cache: time %s addr %s fill, core %d I-cache evict: %s, %s\n",
                fmt_now(), fmt_laddr(base_addr), core->core_id,
                CacheFillOutcome_names[fill_stat],
                fmt_laddr(evicted.base_addr));
    sim_assert(fill_stat != CacheFill_EvictDirty);
    if (core->i_dbp) {
        if (fill_stat != CacheFill_NoEvict)
            dbp_block_kill(core->i_dbp, evicted.base_addr);
        // note: you'd think that fills for present blocks (e.g. upgrades)
        // wouldn't be possible in the I-cache, but they do occur in some
        // corner cases (e.g. shared block with this core in holders,
        // and an exclusive-permission prefetch is emitted, and maybe with
        // wrong-path D-stream writes)
        if (!block_already_present) {
            dbp_block_insert(core->i_dbp, base_addr);
        }
    }
    if (fill_stat != CacheFill_NoEvict) {
        cache_core_evict_maybe(core, evicted.base_addr);
    }
}


static void
dcache_replace(CacheRequest *for_creq, CoreResources *core, LongAddr base_addr,
               CacheAccessType access_type, i64 start_time, int discard_wb,
               int inhibit_pfaudit_fill)
{
    CacheEvicted evicted;
    CacheFillOutcome fill_stat;
    int block_already_present = cache_access_ok(core->dcache, base_addr,
                                                Cache_Read);
    laddr_set(evicted.base_addr, 0, 0);
    fill_stat = cache_fill(core->dcache, base_addr, access_type, &evicted);
    DEBUGPRINTF("cache: time %s addr %s fill, core %d D-cache evict: %s, %s\n",
                fmt_now(), fmt_laddr(base_addr), core->core_id,
                CacheFillOutcome_names[fill_stat],
                fmt_laddr(evicted.base_addr));
    if (core->d_dbp) {
        if (fill_stat != CacheFill_NoEvict)
            dbp_block_kill(core->d_dbp, evicted.base_addr);
        if (!block_already_present) {
            // skip fill-on-present-blocks (due to coherence upgrades)
            dbp_block_insert(core->d_dbp, base_addr);
        }
    }
    if (fill_stat == CacheFill_EvictDirty) {
        CacheAction wb_action = (GlobalParams.mem.private_l2caches)
            ? L2_WB : BUS_WB;
        if (!discard_wb) {
            enq_evict_writeback(core, &evicted, wb_action, start_time);
        } else {
            // just drop the WB request; this is shady, but grants some
            // additional freedom in limit studies, etc.
            DEBUGPRINTF("cache: dropping core %d evicted D-cache block %s\n",
                        core->core_id, fmt_laddr(evicted.base_addr));
            cache_wb_accepted(core->dcache, evicted.base_addr);
        }
        if (core->d_streambuf)
            pfsg_cache_dirty_evict(core->d_streambuf, evicted.base_addr);
    }
    if (fill_stat != CacheFill_NoEvict) {
        cache_core_evict_maybe(core, evicted.base_addr);
    }
}


static void
l2_replace(CacheRequest *for_creq, CoreResources *core, CacheArray *l2cache,
           LongAddr base_addr, CacheAccessType access_type, i64 start_time,
           int discard_wb)
{
    CacheEvicted evicted;
    CacheFillOutcome fill_stat;
    //int block_already_present = cache_access_ok(core->l2cache, base_addr,
    //                                            Cache_Read);
    laddr_set(evicted.base_addr, 0, 0);

    if (!GlobalParams.mem.private_l2caches && (access_type == Cache_Read))
        access_type = Cache_ReadExcl;
    if (!GlobalParams.mem.private_l2caches)
        core = NULL;

    fill_stat = cache_fill(l2cache, base_addr, access_type, &evicted);
    
    //printf("tick %s %s\n",fmt_now(), fmt_laddr(base_addr)); 

    DEBUGPRINTF("cache: time %s addr %s fill, L2 evict: %s, %s\n",
                fmt_now(), fmt_laddr(base_addr),
                CacheFillOutcome_names[fill_stat],
                fmt_laddr(evicted.base_addr));
    if (fill_stat == CacheFill_EvictDirty) {
        CacheAction wb_action;
        if (GlobalParams.mem.private_l2caches) {
            wb_action = BUS_WB;
        } else {
            wb_action = (GlobalParams.mem.use_l3cache) ? L3_WB : MEM_WB;
        }
        if (!discard_wb) {
            enq_evict_writeback(core, &evicted, wb_action, start_time);
        } else {
            // (see dcache_replace() )
          //printf("tick %s %s\n",fmt_now(), fmt_laddr(base_addr));  
	  DEBUGPRINTF("cache: dropping evicted L2-cache block %s\n",
                        fmt_laddr(evicted.base_addr));
            cache_wb_accepted(l2cache, evicted.base_addr);
        }
    }
    if (fill_stat != CacheFill_NoEvict) {
        if (GlobalParams.mem.private_l2caches)
            cache_core_evict_maybe(core, evicted.base_addr);
    }
}


static void
l3_replace(CacheRequest *for_creq, CacheArray *l3cache, LongAddr base_addr,
           i64 start_time)
{
    CacheAccessType access_type = Cache_ReadExcl;
    CacheEvicted evicted;
    CacheFillOutcome fill_stat;
    laddr_set(evicted.base_addr, 0, 0);

    fill_stat = cache_fill(l3cache, base_addr, access_type, &evicted);
    
  printf("CACHETRACE: , %s, %s, %s, %s, %s, %i, %s, %s\n", fmt_i64(cyc), fmt_laddr(for_creq->base_addr), CacheSource_names[for_creq->cores[0].src], CacheAction_names[for_creq->action], CacheAccessType_names[for_creq->access_type], for_creq->cores[0].core->core_id, CacheFillOutcome_names[fill_stat], CoherAccessResult_names[for_creq->coher_wb_type]);

    //DEBUGPRINTF("cache: time %s addr %s fill, L3 evict: %s, %s\n",
    //            fmt_now(), fmt_laddr(base_addr),
    //            CacheFillOutcome_names[fill_stat],
    //            fmt_laddr(evicted.base_addr));
    if (fill_stat == CacheFill_EvictDirty) {
        // Core is NULL: L3 cache is off-core
        enq_evict_writeback(NULL, &evicted, MEM_WB, start_time);
    }
    if (fill_stat != CacheFill_NoEvict) {
    }  


}


static void
process_ifill_context(CoreResources *core, CacheRequest *creq, context *ctx,
                      i64 ready_time)
{
    sim_assert(ctx->imiss_cache_entry == creq);
    ctx->imiss_cache_entry = NULL;
    ctx->icache_sim.service_level = creq->service_level;
    ctx->icache_sim.latency = ready_time - ctx->icache_sim.last_startcyc;

    LongAddr fetch_addr;
    laddr_set(fetch_addr, ctx->as->npc, ctx->as->app_master_id);
    cache_align_addr(core->icache, &fetch_addr);

    if ((ctx->fetchcycle == MAX_CYC) &&
        (laddr_eq(fetch_addr, creq->base_addr))) {
        ctx->fetchcycle = ready_time;
        DEBUGPRINTF("T%d can resume fetching at %s, I miss complete\n",
                    ctx->id, fmt_i64(ctx->fetchcycle));
    } else {
        fprintf(stderr, "T%d I miss complete but not needed -- error\n",
                ctx->id);
        sim_abort();
    }

    mshr_cfree_inst(core->inst_mshr, creq->base_addr, ctx->id);

    if (ctx->long_mem_stat != LongMem_None) {
        DEBUGPRINTF("T%d long_mem_op completing at %s\n", ctx->id,
                    fmt_i64(ready_time));
        ctx->long_mem_stat = LongMem_Completing;
        appmgr_signal_missdone(GlobalAppMgr, ctx->as);
        if (GlobalLongMemLogger)
            longmem_log_complete(GlobalLongMemLogger, ctx->as->app_id,
                                 ctx->id);
    }
}


static void
process_l1fill_ifill(CacheRequest *creq)
{
    CoreResources *core = creq->cores[0].core;
    CacheArray *icache = core->icache;
    i64 ready_time;

    ready_time = cache_update_bank(icache, creq->base_addr, cyc,
                                   CacheBank_Fill);
    icache_replace(creq, core, creq->base_addr, ready_time);
    sim_assert(cache_access_ok(icache, creq->base_addr, Cache_Read));

    if (creq->irequestor) {
        context *mergecontext = creq->irequestor;
        creq->irequestor = NULL;
        process_ifill_context(core, creq, mergecontext, ready_time);
        
        while (mergecontext->mergethread) {
            context *nextcontext = mergecontext->mergethread;
            mergecontext->mergethread = NULL;
            mergecontext = nextcontext;

            ready_time = 
                cache_update_bank(icache, creq->base_addr,
                                  cyc, CacheBank_FillCont);
            process_ifill_context(core, creq, mergecontext, ready_time);
        }
    }
}


// Caller of this must be sure to wake up any "blocked_app", even if there are
// no instructions left in the creq due to flushing!
static void
process_dfill_inst(CacheRequest *creq, CoreResources *core,
                   activelist *meminst, i64 ready_time)
{
    context * restrict ctx = Contexts[meminst->thread];

    sim_assert(meminst->status & MEMORY);
    meminst->dmiss_cache_entry = NULL;
    meminst->dcache_sim.service_level = creq->service_level;

    if (meminst->mem_flags & SMF_Write) 
        cache_mark_dirty(core->dcache, creq->base_addr);

    {
        sim_assert(ready_time >= 0);

        // mem_resolve()/etc set donecycle based on ready_time
        if (meminst->fu == SYNCH)
            synch_mem_resolve(core, meminst, ready_time);
        else
            mem_resolve(core, meminst, ready_time);

        mshr_cfree_data(core->data_mshr, creq->base_addr, ctx->id,
                        meminst->id);

        meminst->dcache_sim.latency = meminst->donecycle - meminst->addrcycle;

        if ((ctx->long_mem_stat != LongMem_None) &&
            (meminst->id == ctx->next_to_commit)) {
            DEBUGPRINTF("T%ds%d long_mem_op completing at %s\n", ctx->id,
                        meminst->id, fmt_i64(ready_time));
            if (ctx->long_mem_stat == LongMem_FlushedBlocked)
                ctx->fetchcycle = ready_time;
            ctx->long_mem_stat = LongMem_Completing;
            appmgr_signal_missdone(GlobalAppMgr, ctx->as);
            if (GlobalLongMemLogger)
                longmem_log_complete(GlobalLongMemLogger, ctx->as->app_id,
                                     ctx->id);
        }
        if (1) {
            // Count only the cycles spent waiting for memory,
            // not including address generation.
            i64 mem_delay = meminst->dcache_sim.latency;
            sim_assert(meminst->addrcycle != MAX_CYC);
            sim_assert(mem_delay >= 0);
            totmem++;
            totmemdelay += mem_delay;
            DEBUGPRINTF("T%ds%d total mem_delay %s\n", meminst->thread,
                        meminst->id, fmt_i64(mem_delay));
            if (meminst->as) {
                meminst->as->extra->mem_delay.delay_sum += mem_delay;
                meminst->as->extra->mem_delay.sample_count++;
            }
        }
    }

    if (core->d_dbp) {
        LongAddr addr;
        laddr_set(addr, (meminst->mem_flags & SMF_Read) ? meminst->srcmem
                  : meminst->destmem, creq->base_addr.id);
        dbp_mem_exec(core->d_dbp, meminst->pc, addr);
    }
}


static void
process_l1fill_dfill(CacheRequest *creq)
{
    CoreResources *core = creq->cores[0].core;
    CacheArray *dcache = core->dcache;
    i64 ready_time;

    ready_time = cache_update_bank(dcache, creq->base_addr, cyc, 
                                   CacheBank_Fill);
    dcache_replace(creq, core, creq->base_addr, creq->access_type, ready_time,
                   0, 0);
    sim_assert(cache_access_ok(dcache, creq->base_addr, creq->access_type));

    if (creq->drequestor) {
        activelist *meminst = creq->drequestor;
        activelist *nextinst = meminst->mergeinst;
        creq->drequestor = NULL;
        meminst->mergeinst = NULL;
        process_dfill_inst(creq, core, meminst, ready_time);

        while (nextinst) {
            meminst = nextinst;
            nextinst = meminst->mergeinst;
            meminst->mergeinst = NULL;

            ready_time =
                cache_update_bank(dcache, creq->base_addr, cyc,
                                  CacheBank_FillCont);
            process_dfill_inst(creq, core, meminst, ready_time);
        }
    }
}


// A data-carrying reply has arrived in-core, with data for one or both of
// the split level-1 caches.  Assuming a common in-core interconnect,
// immediately trigger fills for the appropriate caches.
static void
process_l1fill(CacheRequest *creq)
{
    CoreResources * restrict core = creq->cores[0].core;
    CacheSource source = creq->cores[0].src;
    CacheArray * restrict icache = core->icache;
    CacheArray * restrict dcache = core->dcache;
    PFStreamGroup * restrict d_streambuf = core->d_streambuf;
    int icache_full, dcache_full, dstreambuf_full;      // flags

    sim_assert(creq_single_core(creq));
    sim_assert(CACHE_SOURCE_L1_ONLY(source));

    icache_full = (source & CSrc_L1_ICache) &&
        cache_wb_buffer_full(icache);
    dcache_full = (source & CSrc_L1_DCache) &&
        cache_wb_buffer_full(dcache);
    dstreambuf_full = (source & CSrc_L1_DStreamBuf) &&
        0;        // place-holder in case we need it later

    if (icache_full | dcache_full | dstreambuf_full) {
        if (icache_full)
            cache_log_wbfull_conflict(icache);
        if (dcache_full)
            cache_log_wbfull_conflict(dcache);

        // At least one of the L1 targets was unable to fill (say, due to a
        // full WB buffer).  We'll cruftily push the request back into the
        // cache queue to try again (poll) later.
        DEBUGPRINTF("cache: time %s addr %s, L1 fill blocked: I %d, D %d, "
                    "retrying later\n", fmt_now(), fmt_laddr(creq->base_addr),
                    icache_full, dcache_full);
        creq->request_time = cyc + 1;
        place_in_cache_queue(creq);
        return;
    }

    // Proceed with fill, releases, etc.

    if (source & CSrc_L1_ICache)
        process_l1fill_ifill(creq);
    if (source & CSrc_L1_DCache)
        process_l1fill_dfill(creq);
    if (source & CSrc_L1_DStreamBuf) {
        pfsg_pf_fill(d_streambuf, creq->base_addr, creq->access_type,
                     cyc, (source & CSrc_L1_DCache));
    }

    {
        int free_inst_mshr = (source & CSrc_L1_ICache);
        int free_data_mshr = (source & CSrc_L1_DCache);
        // We need to free the MSHR producer entries allocated in the service
        // of this request, but not if later dependent_coher requests have
        // been merged and have allocated consumers within those MSHRs.  (This
        // is a consequence of the simulator historically doing merging
        // independently of the MSHRs, which it probably shouldn't.)
        CacheRequest * restrict dep_creq = creq->dependent_coher;
        while (dep_creq) {
            if (laddr_eq(creq->base_addr, dep_creq->base_addr)) {
                sim_assert(creq_single_core(dep_creq));
                CacheSource dep_source = dep_creq->cores[0].src;
                if (dep_source & CSrc_L1_ICache)
                    free_inst_mshr = 0;
                if (dep_source & CSrc_L1_DCache)
                    free_data_mshr = 0;
            }
            dep_creq = dep_creq->dependent_coher;
        }

        if (free_inst_mshr)
            mshr_free_producer(core->inst_mshr, creq->base_addr);
        if (free_data_mshr)
            mshr_free_producer(core->data_mshr, creq->base_addr);
    }

    // Release the next coherence-dependent request within this core.
    // (There should be no cross-core dependences here, as those are
    // separated out in process_bus_reply() / process_coher_reply()).
    if (creq->dependent_coher) {
        CacheRequest * restrict dep_creq = creq->dependent_coher;
        CoreResources *dep_core = dep_creq->cores[0].core;
        sim_assert(creq_single_core(dep_creq));
        sim_assert(dep_core == creq->cores[0].core);
        sim_assert(laddr_eq(dep_creq->base_addr, creq->base_addr));
        // Currently, we only use same-core dependent creqs for merging
        // outbound requests, so this should be true (for now)
        sim_assert(dep_creq->action == l1_route_down());
        release_dep_coher(creq, 0, cyc);     // unlinks and re-enqueues
    }

    release_blocked_apps(creq);
    free_cache_request(creq);
}
   

// Test: is it legal to merge a request of type "new_access" with one of
// type "earlier_access" (for the same block)?
static int
cache_access_mergeable(CacheAccessType earlier_access,
                       CacheAccessType new_access)
{
    int prevent_merge = 0;
    if ((earlier_access == Cache_Read) && (new_access != Cache_Read))
        prevent_merge = 1;
    return !prevent_merge;
}


// Walk the dep-coher chain of "creq" after it's been promoted to a "stronger"
// access, _conditionally_ merging chained requests which were originally set
// to come afterwards, but which can now be satistfied by the stronger
// access.
static void
merge_promoted_dep_coher(CacheRequest * restrict creq)
{
    //const char *fname = "merge_promoted_dep_coher";
    sim_assert(creq_single_core(creq));
    CacheRequest *walk_creq = creq->dependent_coher;

    creq->dependent_coher = NULL;       // unlink to prevent confusion

    while (walk_creq) { 
        sim_assert(creq_single_core(walk_creq));
        sim_assert(walk_creq->cores[0].core == creq->cores[0].core);
        sim_assert(laddr_eq(walk_creq->base_addr, creq->base_addr));
        if (cache_access_mergeable(creq->access_type, 
                                   walk_creq->access_type)) {
            CacheRequest *next = walk_creq->dependent_coher;
            // NULL out, to prevent merge_and_free_creq from monkeying with
            // the dependent_coher pointers (since this function is managing
            // them during traversal)
            walk_creq->dependent_coher = NULL;
            merge_and_free_creq(creq, walk_creq);       // frees walk_creq
            walk_creq = next;
        } else {
            break;
        }
    }

    creq->dependent_coher = walk_creq;  // link in any non-mergeable leftovers
}


static void
process_bus_req_nocoher(CacheRequest * restrict creq)
{
    CoreResources *req_core = creq->cores[0].core;
    sim_assert(creq_single_core(creq));
    sim_assert(!req_core->params.coher_mgr);

    CacheRequest * restrict pending_req = 
        cacheq_find(CacheQ, creq->base_addr, CACHEQ_SHARED, CQFS_Miss);
    if (pending_req) {
        sim_assert(pending_req != creq);
        // Merge with an already-pending request for the same block
        merge_and_free_creq(pending_req, creq);
        creq = NULL;
    } else {
        i64 bus_done_cyc = corebus_access(req_core->request_bus,
                                          GlobalParams.mem.bus_request_time);
        // Proceed down normally
        creq->request_time = bus_done_cyc;
        creq->action = bus_route_down();
        place_in_cache_queue(creq);
    }
}


static void
process_bus_req_coher(CacheRequest * restrict creq)
{
    CoreResources *req_core = creq->cores[0].core;
    CacheSource req_source = creq->cores[0].src;
    sim_assert(creq_single_core(creq));
    sim_assert(req_core->params.coher_mgr);

    CoherWaitInfo *peer_info = NULL;
    int req_cache_id = req_core->core_id;   // crufty assumption
    CoherAccessType coher_access_type;
    CoherAccessResult coher_result;
    int have_write_perm;
    i64 bus_done_cyc = -1;                      // -1: n/a or unknown

//printf("CACHETRACE: , %s, %s, %s, %s, %s, %i, %s\n", fmt_now(), fmt_laddr(creq->base_addr), CacheSource_names[creq->cores[0].src], CacheAction_names[creq->action], CacheAccessType_names[creq->access_type], creq->cores[0].core->core_id, CoherAccessResult_names[creq->coher_wb_type]);

    switch (creq->access_type) {
    case Cache_Read:
        coher_access_type =
            (CACHE_SOURCE_L1_ONLY(req_source) && 
             (req_source == CSrc_L1_ICache)) ? Coher_InstRead : Coher_DataRead;
        break;
    case Cache_ReadExcl:
    case Cache_Upgrade:
        coher_access_type = Coher_DataReadExcl;
        break;
    case Cache_Write:
        abort_printf("Cache_Write request seen at BUS_REQ (C%d addr %s); "
                     "shouldn't happen with write-back cache\n",
                     req_core->core_id, fmt_laddr(creq->base_addr));
        coher_access_type = 0;
        break;
    default:
        ENUM_ABORT(CacheAccessType, creq->access_type);
        coher_access_type = 0;
    }

    sim_assert(!creq->coher_accessed);
    coher_result =
        cm_access(req_core->params.coher_mgr, creq->base_addr,
                  req_cache_id, coher_access_type, &peer_info,
                  &have_write_perm);

 // 

    if (coher_result != Coher_EntryBusy) {
        // Don't send out on the bus, if another core has an
        // outstanding request which might conflict with this one. (This
        // assumes that we passively track the tags of all outstanding peer
        // traffic, ala the SGI Challenge.)  If so, we'll defer our own bus
        // transaction until that request has completed.  In addition to
        // avoiding the need to spin on bus retries, this will keep us from
        // attempting to invalidate the earlier requestor's tags before they
        // complete a memory instruction, which could lead to deadlock if not
        // handled carefully.
        creq->coher_accessed = 1;
        bus_done_cyc = corebus_access(req_core->request_bus,
                                      GlobalParams.mem.bus_request_time);
    }

    switch (coher_result) {
    case Coher_NoStall:
        // No stalls for peers or older traffic; proceed as ordinary access
        if (creq->coher_data_seen) {
            // request doesn't need to wait for anything; reply is ready
            creq->service_level = SERVICED_COHER;
            creq->action = bus_route_up();
        } else {
            // send down normally
            creq->action = bus_route_down();
            cm_shared_request(req_core->params.coher_mgr, creq->base_addr);
        }
        creq->request_time = bus_done_cyc;
        place_in_cache_queue(creq);
        break;
    case Coher_EntryBusy:
        {
            // A coherence request is already outstanding.  We'll find the
            // request that's blocking the forward progress of this request --
            // there must be exactly one -- and line up there, to retry
            // BUS_REQ when it completes.
            CacheRequest * restrict conflict_creq =
                cacheq_find(CacheQ, creq->base_addr, CACHEQ_SHARED, CQFS_Miss);
            if (!conflict_creq) {
                abort_printf("error: coher access for block at %s "
                             "found busy entry, but no outstanding "
                             "CacheRequest; creq: %s\n",
                             fmt_laddr(creq->base_addr),
                             fmt_creq_static(creq));
            }
            sim_assert(conflict_creq != creq);
            sim_assert(conflict_creq->cores[0].core != req_core);
            // "creq" will re-enter BUS_REQ when conflict_creq completes
            append_dep_coher(conflict_creq, creq);
        }
        break;
    case Coher_StallForInvl:
    case Coher_StallForWB:  
    case Coher_StallForXfer:
    case Coher_StallForShared:
        {
            // Must stall for coherence traffic from peer(s)
            sim_assert(peer_info);
            sim_assert(peer_info->node_count > 0);
            // We need to make sure that the WBI does not arrive before
            // any already-released fills for this same address.
            i64 wbi_arrival_cyc = corebus_sync_prepare(req_core->reply_bus);
            // Also, don't let it sneak back in time if the reply bus was idle
            wbi_arrival_cyc = MAX_SCALAR(wbi_arrival_cyc, bus_done_cyc);
            enq_coher_peer_msgs(creq, coher_result, peer_info,
                                wbi_arrival_cyc);
            // This request (creq) must now wait for responses from all
            // indicated peers; however, it must also remain visible to future
            // calls to ordered_l1_find_and_merge().  We'll mark it "blocked"
            // and then re-enqueue it, so that it's available for searching.
            // Once the final reply comes in at process_coher_reply(), we'll
            // release it, at which time it can proceed with L2FILL/DFILL/etc.
            creq->action = COHER_WAIT;
            creq->blocked = 1;
            place_in_cache_queue(creq);
            coherwaitinfo_destroy(peer_info);
            peer_info = NULL;
        }
        break;
    default:
        ENUM_ABORT(CoherAccessResult, coher_result);
    }
	//printf("CACHETRACE: , %s, %s, %s, %s, %s, %i, %s\n", fmt_i64(cyc), fmt_laddr(creq->base_addr), CacheSource_names[creq->cores[0].src], CacheAction_names[creq->action], CacheAccessType_names[creq->access_type], creq->cores[0].core->core_id, coher_result);
    if ((coher_result != Coher_EntryBusy) && 
        (coher_access_type != Coher_DataReadExcl) && have_write_perm) {
        // The coherence manager has granted us write permission
        // (exclusive access), even though it was not requested; this
        // allows us to avoid unnecessary upgrade misses.  We update
        // "creq" to account for this, and allow the cache to be updated
        // appropriately at fill-time.
        DEBUGPRINTF("cache: promoting C%d's access to addr %s to "
                    "exclusive\n", req_core->core_id, 
                    fmt_laddr(creq->base_addr));
        creq->access_type = Cache_ReadExcl;
        if (creq->dependent_coher) {
            // Conditionally merge dependent-coher requests, if the
            // newly-promoted request can now handle them.
            merge_promoted_dep_coher(creq);
        }
    }
}



// Send a miss-service request out over the interconnect (bus).  
static void
process_bus_req(CacheRequest * restrict creq)
{
    CoreResources *req_core = creq->cores[0].core;
    if (req_core->params.coher_mgr) {
        process_bus_req_coher(creq);
    } else {
        process_bus_req_nocoher(creq);
    }
}


// Walk the "dependent_coher" chain in creq, seperating it into per-core
// sub-chains; chains for other cores are unlinked and released, while the
// chain for "creq"'s core is left intact for later release by
// process_l1fill().
static void
release_offcore_dep_coher(CacheRequest *creq, i64 min_time)
{
    if (creq->dependent_coher) {
        int num_cores = GlobalParams.num_cores;
        int native_core_id = creq->cores[0].core->core_id;
        CacheRequest *per_core_reqs[num_cores];
        // We'll separate out dependent_coher requests per-core;
        // dependent_coher requests native to the core that's being serviced
        // are left untouched (to be awakened by process_l1fill()), while
        // other cores' requests are awakened and re-enqueued.
        separate_dep_coher(creq, per_core_reqs, num_cores);
        for (int core_id = 0; core_id < num_cores; core_id++) {
            if (core_id == native_core_id) {
                creq->dependent_coher = per_core_reqs[core_id];
            } else if (per_core_reqs[core_id]) {
                CacheRequest *peer_req = per_core_reqs[core_id];
                sim_assert((peer_req->action == BUS_REQ) ||
                           (peer_req->action == bus_route_up()));
                release_dep_coher(peer_req, 1, min_time);
            }
        }
    }
}


// (data-carrying reply from below)
static void
process_bus_reply(CacheRequest *creq)
{
    i64 xfer_done;

    // With coherence enabled, we shouldn't have cross-core inst/data requests
    // here; they should have been left separate, to coordinate access through
    // the coherence manager.
    assert_ifthen(GlobalCoherMgr, creq_single_core(creq));

    xfer_done = corebus_access(creq->cores[0].core->reply_bus, 
                               GlobalParams.mem.bus_transfer_time);
    creq->request_time = xfer_done;
    creq->action = bus_route_up();

    if (creq->cores[0].core->params.coher_mgr) {
        cm_shared_reply(creq->cores[0].core->params.coher_mgr,
                        creq->base_addr);
    }

    if (creq->dependent_coher) {
        // Also in process_coher_reply()
        // (make sure we wait until the /end/ of the bus transfer, to keep
        // a dependent COHER_WBI from getting ahead of this reply)
        // (should we have a corebus_sync_prepare() here?)
        release_offcore_dep_coher(creq, xfer_done);
    }

    place_in_cache_queue(creq);
}


static void
process_bus_wb(CacheRequest *creq)
{
    i64 bus_done_cyc;
    sim_assert(creq_single_core(creq));
    CoreResources *solo_core = creq->cores[0].core;
    bus_done_cyc = corebus_access(solo_core->request_bus,
                                  GlobalParams.mem.bus_transfer_time);
    if (GlobalParams.mem.private_l2caches) {
        cache_wb_accepted(solo_core->l2cache, creq->base_addr);
        creq->request_time = bus_done_cyc + 
            solo_core->params.private_l2cache.timing.miss_penalty;
        creq->action = (GlobalParams.mem.use_l3cache) ? L3_WB : MEM_WB;
    } else {
        cache_wb_accepted(solo_core->dcache, creq->base_addr);
        creq->request_time = bus_done_cyc + 
            solo_core->params.dcache.timing.miss_penalty;
        creq->action = L2_WB;
    }
    place_in_cache_queue(creq);
}


static void
process_coher_wbi_l1(CacheRequest *creq)
{
    CoreResources *core = creq->cores[0].core;
    CacheArray *icache = core->icache, *dcache = core->dcache;
    CacheFillOutcome yield_stat;
    // The planned flag that says whether L1 resources need to be inval'd;
    // WARNING: WE MAY DECIDE TO INVALIDATE ANYWAY, and that needs to be
    // handled by WBI_L2_DOWN (private l2) or COHER_REPLY (shared L2).  (They
    // can detect this by checking access_type, due to the changes made by
    // update_coher_creq_for_yield() to signal dirtiness.)
    int invalidate_after = COHER_WB_NEEDS_INVAL(creq->coher_wb_type);

    sim_assert(creq_single_core(creq));
    sim_assert(COHER_STALLS_FOR_PEERS(creq->coher_wb_type));
    sim_assert(creq->coher_for != NULL);

    int dcache_full = cache_wb_buffer_full(dcache);
    int icache_full = cache_wb_buffer_full(icache);

    if (dcache_full)
        cache_log_wbfull_conflict(dcache);
    if (icache_full)
        cache_log_wbfull_conflict(icache);

    if (dcache_full || icache_full) {
        DEBUGPRINTF("cache: time %s addr %s, L1 WBI found full WB, "
                    "retrying later\n", fmt_now(), fmt_laddr(creq->base_addr));
        creq->request_time = cyc + 1;
        place_in_cache_queue(creq);
        return;
    }

    // see comments at MSHR test in process_coher_wbi_l2_up()
    if (mshr_any_producer(core->inst_mshr, creq->base_addr)) {
        DEBUGPRINTF("cache: time %s addr %s, note: WBI_L1 matches "
                    "outstanding Inst MSHR\n", fmt_now(),
                    fmt_laddr(creq->base_addr));
    }
    if (mshr_any_producer(core->data_mshr, creq->base_addr)) {
        DEBUGPRINTF("cache: time %s addr %s, note: WBI_L1 matches "
                    "outstanding Data MSHR\n", fmt_now(),
                    fmt_laddr(creq->base_addr));
    }

    yield_stat = cache_coher_yield(dcache, creq->base_addr, invalidate_after,
                                   0);
    creq->request_time = cache_update_bank(dcache, creq->base_addr, cyc,
                                           CacheBank_CoherPull);
    update_coher_creq_for_yield(creq, yield_stat);

    if (core->d_dbp && invalidate_after && (yield_stat != CacheFill_NoEvict))
        dbp_block_kill(core->d_dbp, creq->base_addr);

    if ((yield_stat == CacheFill_EvictDirty) && !invalidate_after) {
        // A simple downgrade won't be enough for the I-cache or streambuf,
        // since they may contain a stale copy of this block.  We don't
        // enforce D->I coherence, but we need to be more careful with the
        // streambuf, since it can later satisfy D-misses.  (This forced
        // invalidate obviates the need for a pfsg_cache_dirty_evict() call.)
        //
        // process_coher_reply() / process_coher_wbi_l2_down() need to handle
        // this "surprise invalidate" from the I-cache.
        invalidate_after = 1;
    }

    if (1) {
        // icache will never need to be downgraded, but may need invalidation;
        // also, it can be useful to source data for peer-to-peer transfer
        //int had_data_before_i_yield = coher_reply_has_data(creq);
        CacheFillOutcome i_yield_stat = 
            cache_coher_yield(icache, creq->base_addr, invalidate_after, 0);
        sim_assert(i_yield_stat != CacheFill_EvictDirty);
        if (core->i_dbp && invalidate_after &&
            (i_yield_stat != CacheFill_NoEvict))
            dbp_block_kill(core->i_dbp, creq->base_addr);
        i64 i_ready_time = cache_update_bank(icache, creq->base_addr, cyc,
                                             CacheBank_CoherPull);
        creq->request_time = MAX_SCALAR(creq->request_time, i_ready_time);
        update_coher_creq_for_yield(creq, i_yield_stat);

    }

    if (core->d_streambuf)
        pfsg_coher_yield(core->d_streambuf, creq->base_addr, invalidate_after);

    creq->action =
        (GlobalParams.mem.private_l2caches) ? COHER_WBI_L2_DOWN : COHER_REPLY;
    place_in_cache_queue(creq);
}


// Don't invalidate the L2 yet; we must invalidate it after we've invalidated
// the L1.  Otherwise, the L1 might write-back the data while our L1
// invalidate message is on the way.  We'll still sync all the ports, to be
// sure that this message won't precede any L2->L1 fill that's already been
// sent up.  (We'll sync the L1 banks as well, to be sure.)
static void
process_coher_wbi_l2_up(CacheRequest *creq)
{
    CoreResources *core = creq->cores[0].core;
    CacheArray *l2cache = core->l2cache;
    
    sim_assert(GlobalParams.mem.private_l2caches);
    sim_assert(creq_single_core(creq));
    sim_assert(COHER_STALLS_FOR_PEERS(creq->coher_wb_type));

    if (mshr_any_producer(core->private_l2mshr, creq->base_addr)) {
        // If the coherence system is working right, the presence of this
        // WBI implies that 1) the request which it being done on behalf of
        // has "won" access to the block before the miss we now have
        // outstanding, and 2) the coherence system will still send us a
        // copy when it's our turn, so we should leave the MSHR allocated
        // for eventual receipt of that fill.
      //printf("tick %s %s\n",fmt_now(), fmt_laddr(creq->base_addr));  
      DEBUGPRINTF("cache: time %s addr %s, note: WBI_L2_UP matches "
                    "outstanding L2 MSHR\n", fmt_now(),
                    fmt_laddr(creq->base_addr));
    }

    creq->request_time = cache_update_bank(l2cache, creq->base_addr, cyc,
                                           CacheBank_CoherSync);
    cache_coher_lockout(l2cache, creq->base_addr);      // block future hits
    creq->action = COHER_WBI_L1;
    place_in_cache_queue(creq);
}


// If the L1 had a copy of this block, it's been invalidated; it may also
// have been dirty.  We'll now invalidate the L2, being sure to sync the
// ports first so that the invalidate applies after any L2->L1 writeback.
//
// Note that we're still overlooking the case where we "chase the writeback"
// to L2, but an intervening L2 replacement has evicted that too; the actual
// data that we would need to transfer to our peer then would be in the
// L3/memory writeback, but we'll ignore that and be content with consistent
// L2 tag state.
static void
process_coher_wbi_l2_down(CacheRequest *creq)
{
    CoreResources *core = creq->cores[0].core;
    CacheArray *l2cache = core->l2cache;
    CacheFillOutcome yield_stat;
    const int invalidate_after = COHER_WB_NEEDS_INVAL(creq->coher_wb_type);
    
    sim_assert(GlobalParams.mem.private_l2caches);
    sim_assert(creq_single_core(creq));
    sim_assert(creq->coher_for != NULL);

    if (cache_wb_buffer_full(l2cache)) {
      //printf("tick %s %s\n",fmt_now(), fmt_laddr(creq->base_addr));  
      DEBUGPRINTF("cache: time %s addr %s, L2 WBI found full WB, "
                    "retrying later\n", fmt_now(), fmt_laddr(creq->base_addr));
        cache_log_wbfull_conflict(l2cache);
        creq->request_time = cyc + 1;
        place_in_cache_queue(creq);
        return;
    }

    // (icache implicitly invalidated if dirty data is WB'd)
    cache_wb_accepted(core->icache, creq->base_addr);   // accept ack
    cache_wb_accepted(core->dcache, creq->base_addr);

    yield_stat = cache_coher_yield(l2cache, creq->base_addr, invalidate_after,
                                   0);
    creq->request_time = cache_update_bank(l2cache, creq->base_addr, cyc,
                                           CacheBank_CoherPull);
    update_coher_creq_for_yield(creq, yield_stat);
	
//printf("CACHETRACE: , %s, %s, %s, %s, %s, %i, %s\n", fmt_i64(cyc), fmt_laddr(creq->base_addr), CacheSource_names[creq->cores[0].src], CacheAction_names[creq->action], CacheAccessType_names[creq->access_type], creq->cores[0].core->core_id, CoherAccessResult_names[creq->coher_wb_type]);

    creq->action = COHER_REPLY;
    place_in_cache_queue(creq);
}


static void
process_coher_wait(CacheRequest *creq)
{
    dump_creq(creq, "crashing");
    abort_printf("process_coher_wait(): this should never be called, "
                 "creq %p should be blocked (%s)\n", creq,
                 (creq->blocked) ? "and it is" : "but it isn't");
}


// A private cache has yielded privately-held data in response to a request
// from another core, for coherence reasons.  The data may still be "clean",
// in which case only an acknowledgement is sent over the interconnect; if the
// data is dirty, the entire block is sent, and a write-back request is
// propagated down the memory hierarchy.
//
// Note that creq->access_type is being overloaded to indicate the
// present/clean/dirty state of the data in question; see
// enq_coher_peer_msgs() for details.
static void
process_coher_reply(CacheRequest * restrict creq)
{
    CoreResources *reply_core = creq->cores[0].core;
    i64 bus_done_cyc;
    int full_block_transfer;
    int peer_has_data = coher_reply_has_data(creq);
    int wb_to_lower_level;
    CacheRequest * restrict blocked_req = creq->coher_for;
    CacheSource blocked_source;
    int blocked_had_data;       // flag: original requestor started with a copy

    if (debug) {
        CacheRequest *test_creq =
            cacheq_find(CacheQ, creq->base_addr, CACHEQ_SHARED, CQFS_Miss);
        sim_assert(test_creq != creq);
        sim_assert(test_creq == blocked_req);   // we should be the only one
    }
	
    sim_assert(creq_single_core(creq));
    sim_assert(ENUM_OK(CoherAccessResult, creq->coher_wb_type));
    sim_assert(COHER_STALLS_FOR_PEERS(creq->coher_wb_type));

    sim_assert(blocked_req != NULL);
    sim_assert(creq_single_core(blocked_req));
    blocked_source = blocked_req->cores[0].src;
    blocked_had_data = (blocked_req->access_type == Cache_Upgrade);

    if (GlobalParams.mem.private_l2caches) {
        cache_wb_accepted(reply_core->l2cache, creq->base_addr);
    } else {
        if (COHER_WB_NEEDS_INVAL(creq->coher_wb_type) ||
            coher_reply_has_dirty_data(creq)) {
            // (icache implicitly invalidated if dirty data is WB'd)
            cache_wb_accepted(reply_core->icache, creq->base_addr);
        }
        cache_wb_accepted(reply_core->dcache, creq->base_addr);
    }

    // access_type set to Write if needed, in update_coher_creq_for_yield()
    wb_to_lower_level = coher_reply_has_dirty_data(creq);
    if (wb_to_lower_level && (creq->coher_wb_type == Coher_StallForXfer) &&
        CACHE_SOURCE_L1_ONLY(blocked_source) &&
        (blocked_source == CSrc_L1_DCache)) {
        // When performing a core->core transfer of exclusive, modified data,
        // we're free to keep the data "dirty" at the destination and
        // eliminate the lower-level writeback.  (We must ensure we only
        // do this if the blocked requestor can handle dirty data, e.g.
        // a D-cache.)
        sim_assert(!blocked_req->is_dirty_fill);
        blocked_req->is_dirty_fill = 1;
        blocked_req->access_type = Cache_Write;
        wb_to_lower_level = 0;
        DEBUGPRINTF("cache: marking blocked request as "
                    "dirty-fill, cancelling writeback; creq: %s\n",
                    fmt_creq_static(blocked_req));
    }

    full_block_transfer = 0;
    if (peer_has_data && !blocked_req->coher_data_seen) {
        blocked_req->coher_data_seen = 1;
        // If the original requestor does not have the data, the first
        // responding cache which does have a copy, transfers its copy
        // instead of just an ACK.  In some cases (e.g. core 1 missing on
        // a block that core 2 holds dirty), core 2 would implicitly
        // transfer its copy of the data; however, in some other cases,
        // this is slightly psychic and/or magical.
        if (blocked_had_data &&
            !COHER_STALL_FOR_EXCL(blocked_req->coher_wb_type)) {
            // Sketchy: we assume that if the blocked requestor started out
            // with a copy of the data (e.g.  for an upgrade miss), AND that
            // the coherence result from the access implies that block wasn't
            // held exclusively by the previous owner(s), that the original
            // requestor still has a copy of the data, and hence only needs an
            // ack to continue.  This isn't necessarily true since the
            // original requestor may have since evicted that data as part of
            // normal operation.  Right now we just ignore the cases where
            // that happens, and magically place the missing data in-cache.
        } else {
            full_block_transfer = 1;
        }
    }

    assert_ifthen(wb_to_lower_level, full_block_transfer);

    bus_done_cyc = corebus_access(reply_core->reply_bus,
                                  ((full_block_transfer) ?
                                   GlobalParams.mem.bus_transfer_time :
                                   GlobalParams.mem.bus_request_time));
    if (wb_to_lower_level) {
        CacheRequest * restrict wb_req;
        CacheAction wb_action;
        int miss_penalty;
        if (GlobalParams.mem.private_l2caches) {
            miss_penalty = reply_core->params.private_l2cache
                .timing.miss_penalty;
            wb_action = (GlobalParams.mem.use_l3cache) ? L3_WB : MEM_WB;
        } else {
            miss_penalty = reply_core->params.dcache.timing.miss_penalty;
            wb_action = L2_WB;
        }

        wb_req = get_c_request_holder(bus_done_cyc + miss_penalty,
                                      creq->base_addr, Cache_Write,
                                      wb_action, CSrc_WB, reply_core);
//printf("CACHETRACE: , %s, %s, %s, %s, %s, %i\n", fmt_now(), fmt_laddr(wb_req->base_addr), CacheSource_names[wb_req->cores[0].src], CacheAction_names[wb_req->action], CacheAccessType_names[wb_req->access_type], creq->cores[0].core->core_id);
        place_in_cache_queue(wb_req);
    }

    int reply_cache_id = reply_core->core_id;   // crufty assumption
    int was_final = 
        cm_peer_reply(reply_core->params.coher_mgr, creq->base_addr,
                      reply_cache_id);

    if (was_final) {
        // If we've failed to locate a copy of the data after having consulted
        // possible sharers, fall back to requesting from a lower level,
        // instead of magically replying with non-existant data (whoops).
        int fetch_from_lower_level = !blocked_req->coher_data_seen;
        sim_assert(!(fetch_from_lower_level && wb_to_lower_level));

        // The final coherence-reply has returned; we can release the
        // blocked request.
        dump_creq(blocked_req, "unblocking");
        cacheq_dequeue_blocked(CacheQ, blocked_req);
        blocked_req->blocked = 0;
        blocked_req->coher_data_seen = 0;   // reset since we're done with this

        // Now to decide where to send it next
        if (fetch_from_lower_level) {
            // Proceed down as a normal miss, leave dependent_coher ops blocked
            // (they'll be released when blocked_req gets back to
            // process_bus_reply(), later).
            blocked_req->action = bus_route_down();
            cm_shared_request(reply_core->params.coher_mgr, creq->base_addr);
        } else {
            blocked_req->action = bus_route_up();
            if (blocked_req->dependent_coher) {
                // Similar to process_bus_reply(), this request is now being
                // sent up "privately", so we'll release other cores' traffic
                // (cyc+1: since we're making a decision this cycle, don't
                // apply it until the next to avoid shoot-through)
                release_offcore_dep_coher(blocked_req, cyc + 1);
            }
            blocked_req->service_level = SERVICED_COHER;
        }
        blocked_req->request_time = bus_done_cyc;
        place_in_cache_queue(blocked_req);
    }
	
    sim_assert(!creq->dependent_coher);         // shouldn't merge on this
    free_cache_request(creq);
}


static void
process_l2access(CacheRequest *creq)
{
    i64 ready_time;
    CacheLOutcome cache_stat;
    CoreResources *first_core = creq->cores[0].core;
    CacheArray *l2cache = first_core->l2cache;
    const int use_l3cache = GlobalParams.mem.use_l3cache;
    const int private_l2 = GlobalParams.mem.private_l2caches;
    CacheAccessType access_type = creq->access_type;
    int is_first_upgrade = 0; // flag: UpgradeMiss here, but full misses above

    assert_ifthen(private_l2, creq_single_core(creq));


    if (!private_l2 && (access_type == Cache_Read))
        access_type = Cache_ReadExcl;

    if (private_l2 && !mshr_is_avail(first_core->private_l2mshr,
                                     creq->base_addr)) {
        // we just spin for availability; this is inefficient and sad
      //printf("tick %s %s\n",fmt_now(), fmt_laddr(creq->base_addr));

      DEBUGPRINTF("cache: time %s addr %s, L2 access blocked (MSHR full), "
                    "retrying later\n", fmt_now(), fmt_laddr(creq->base_addr));
        ++first_core->private_l2mshr_confs;
        creq->request_time = cyc + 1;   // sad :(
        place_in_cache_queue(creq);
        return;
    }

    cache_stat = cache_lookup(l2cache, creq->base_addr, access_type, NULL);
    ready_time = cache_update_bank(l2cache, creq->base_addr, cyc, cache_access_to_bankop(access_type));
    //printf("tick %s %s\n",fmt_now(), fmt_laddr(creq->base_addr));
    DEBUGPRINTF("cache: time %s addr %s,", fmt_now(),
                fmt_laddr(creq->base_addr));
    if (private_l2) { DEBUGPRINTF(" core %d", first_core->core_id); }
    DEBUGPRINTF(" L2 access %s: %s, ready at %s\n",
                CacheAccessType_names[access_type],
                CacheLOutcome_names[cache_stat], fmt_i64(ready_time));
    assert_ifthen(!private_l2, (cache_stat != Cache_UpgradeMiss));
    assert_ifthen(!GlobalCoherMgr, (cache_stat != Cache_UpgradeMiss));

    if (cache_stat == Cache_Hit) {
        creq->request_time = ready_time;
        if (GlobalParams.mem.private_l2caches) {
            creq->action = L1FILL;
        } else {
            creq->action = BUS_REPLY;
        }
        creq->service_level = 2;
        sim_assert(cache_access_ok(l2cache, creq->base_addr, access_type));
        if (private_l2 && (access_type == Cache_Read) &&
            cache_access_ok(l2cache, creq->base_addr, Cache_ReadExcl)) {
          //printf("tick %s %s\n",fmt_now(), fmt_laddr(creq->base_addr));  
	  DEBUGPRINTF("cache: promoting C%d's access to addr %s to "
                        "exclusive on private L2 excl hit\n",
                        first_core->core_id,
                        fmt_laddr(creq->base_addr));
            creq->access_type = access_type = Cache_ReadExcl;
            if (creq->dependent_coher)
                merge_promoted_dep_coher(creq);
        }
    } else if ((cache_stat == Cache_Miss) ||
               (cache_stat == Cache_UpgradeMiss)) {
        if (private_l2) {
            // MSHR: we won't allocate a seperate consumer per-source;
            // that's overkill for a private L2 with just a fixed-sized handful
            // of L1 caches above it, we can just assume you'd have them
            // all share a single entry, and stick a bit-mask in it.
            MshrAllocOutcome mshr_stat = 
                mshr_alloc_nestedcache(first_core->private_l2mshr,
                                       creq->base_addr, 0);
            if (mshr_stat != MSHR_AllocNew) {
                // shouldn't be MSHR_ReuseOld -- should've been merged
                // shouldn't be MSHR_Full -- was just checked, above
                abort_printf("unexpected MSHR alloc result (%s), "
                             "for L2 miss, base_addr %s\n",
                             ENUM_STR(MshrAllocOutcome, mshr_stat),
                             fmt_laddr(creq->base_addr));
            }

            creq->request_time = ready_time + first_core->params.private_l2cache.timing.miss_penalty;
            creq->action = BUS_REQ;
        } else {
            sim_assert(cache_stat != Cache_UpgradeMiss); // (shared cache)
            creq->request_time = ready_time + GlobalParams.mem.l2cache_timing.miss_penalty;
            creq->action = (use_l3cache) ? L3ACCESS : MEMACCESS;
        }
        if ((cache_stat == Cache_UpgradeMiss) && 
            (access_type != Cache_Upgrade)) {
            creq->access_type = access_type = Cache_Upgrade;
            is_first_upgrade = 1;
        }
    } else if (cache_stat == Cache_CoherBusy) {
      //printf("tick %s %s\n",fmt_now(), fmt_laddr(creq->base_addr));  
      DEBUGPRINTF("cache: scheduling L2ACCESS for retry on CoherBusy\n");
        sim_assert(private_l2 && GlobalCoherMgr);
        creq->request_time = ready_time;
        place_in_cache_queue(creq);
        return;
    } else {
        ENUM_ABORT(CacheLOutcome, cache_stat);
    }

    log_app_l23_access(creq, 0, cache_stat == Cache_Hit);
    place_in_cache_queue(creq);
}


static void
process_l2fill(CacheRequest *creq)
{
    CoreResources *first_core = creq->cores[0].core;
    CacheArray *l2cache = first_core->l2cache;
    LongAddr base_addr = creq->base_addr;
    i64 ready_time;

    assert_ifthen(GlobalParams.mem.private_l2caches,
                  creq_single_core(creq));

    if (cache_wb_buffer_full(l2cache)) {
        
      //printf("tick %s %s\n",fmt_now(), fmt_laddr(creq->base_addr));

      DEBUGPRINTF("cache: time %s addr %s, L2 fill blocked (WB full), "
                    "retrying later\n", fmt_now(), fmt_laddr(base_addr));
        cache_log_wbfull_conflict(l2cache);
        creq->request_time = cyc + 1;
        place_in_cache_queue(creq);
        return;
    }

    ready_time = cache_update_bank(l2cache, base_addr, cyc,
                                   CacheBank_Fill);
    l2_replace(creq, first_core, l2cache, base_addr,
               creq->access_type, ready_time, 0);

    int any_consumers = 1;
    if (GlobalParams.mem.private_l2caches) {
        any_consumers = mshr_any_consumers(first_core->private_l2mshr,
                                           base_addr);
    }

    if (any_consumers) {
        creq->request_time = ready_time;
        if (GlobalParams.mem.private_l2caches) {
            creq->action = L1FILL;
        } else {
            creq->action = BUS_REPLY;
        }
        place_in_cache_queue(creq);
    } else {
        // for e.g. L2-targeted prefetches
        free_cache_request(creq);
        creq = NULL;
    }

    if (GlobalParams.mem.private_l2caches) {
        if (any_consumers) {
            mshr_cfree_nestedcache(first_core->private_l2mshr,
                                   base_addr, 0);
        }
        mshr_free_producer(first_core->private_l2mshr, base_addr);
    }
}


// Begin writeback _to_ L2
static void
process_l2wb(CacheRequest *creq)
{
    CacheArray *l2cache = creq->cores[0].core->l2cache;

    if (cache_wb_buffer_full(l2cache)) {
        // Only accept this WB request if we have room to pass it on
      //printf("tick %s %s\n",fmt_now(), fmt_laddr(creq->base_addr));   

      DEBUGPRINTF("cache: time %s addr %s, L2 WB blocked (WB full), "
                    "retrying later\n", fmt_now(), fmt_laddr(creq->base_addr));
        cache_log_wbfull_conflict(l2cache);
        creq->request_time = cyc + 1;
        place_in_cache_queue(creq);
        return;
    }

    int hit = cache_writeback(l2cache, creq->base_addr);
    i64 ready_time = 
        cache_update_bank(l2cache, creq->base_addr, cyc, CacheBank_WB);
    assert_ifthen(GlobalParams.mem.private_l2caches, creq_single_core(creq));

    //printf("tick %s %s\n",fmt_now(), fmt_laddr(creq->base_addr));

    DEBUGPRINTF("cache: time %s addr %s, L2 wb: %s, ready at %s\n",
                fmt_now(), fmt_laddr(creq->base_addr),
                (hit) ? "hit" : "miss", fmt_i64(ready_time));

    if (GlobalParams.mem.private_l2caches) {
        cache_wb_accepted(creq->cores[0].core->dcache, creq->base_addr);
    }

    if (hit) {
        // Writeback hit
        sim_assert(!creq->dependent_coher);     // Not used (and not handled)
        free_cache_request(creq);
    } else {
        // Miss: write-around, L2 writeback buffer allocated
        creq->request_time = ready_time + 
            GlobalParams.mem.l2cache_timing.miss_penalty;
        if (GlobalParams.mem.private_l2caches) {
            creq->action = BUS_WB;
        } else {
            creq->action = (GlobalParams.mem.use_l3cache) ? L3_WB : MEM_WB;
        }
        place_in_cache_queue(creq);
    }
}


static void
process_l3access(CacheRequest *creq)
{
    CacheArray *l3cache = SharedL3Cache;
    i64 ready_time;
    CacheAccessType access_type = Cache_ReadExcl;
    CacheLOutcome cache_stat;

    cache_stat = cache_lookup(l3cache, creq->base_addr, access_type, NULL); 
    ready_time = cache_update_bank(l3cache, creq->base_addr, cyc,
                          CacheBank_LookupREx);
    ready_time = ready_time + DelayCalc(creq);

    printf("tick %s %s %s\n",CacheAccessType_names[access_type],fmt_now(), fmt_laddr(creq->base_addr)); 

    DEBUGPRINTF("cache: time %s addr %s, L3 access %s: %s, ready at %s\n",
                fmt_now(), fmt_laddr(creq->base_addr),
                CacheAccessType_names[access_type],
                CacheLOutcome_names[cache_stat], fmt_i64(ready_time));

 printf("CACHETRACE: , %s, %s, %s, %s, %s, %i, %s, %s\n", fmt_now(), fmt_laddr(creq->base_addr), CacheSource_names[creq->cores[0].src], CacheAction_names[creq->action], CacheAccessType_names[creq->access_type], creq->cores[0].core->core_id, CacheLOutcome_names[cache_stat], CoherAccessResult_names[creq->coher_wb_type]);

    sim_assert(cache_stat != Cache_UpgradeMiss);

    if (cache_stat == Cache_Hit) {
        creq->request_time = ready_time;
        creq->action = (GlobalParams.mem.private_l2caches) ?
            BUS_REPLY : L2FILL;
        creq->service_level = 3;
        sim_assert(cache_access_ok(l3cache, creq->base_addr, access_type));
    } else if (cache_stat == Cache_Miss) {
        creq->request_time = ready_time +
            GlobalParams.mem.l3cache_timing.miss_penalty;
        creq->action = MEMACCESS;
    } else {
        sim_assert(cache_stat != Cache_UpgradeMiss); // (shared cache)
        abort_printf("unhandled cache_stat value %d\n", (int) cache_stat);
    }

    log_app_l23_access(creq, 1, cache_stat == Cache_Hit);
    place_in_cache_queue(creq);

}


static void
process_l3fill(CacheRequest *creq)
{
    CacheArray *l3cache = SharedL3Cache;
    i64 ready_time;
    //this is for L3 access trace
    //if (cyc < 1000000)
      //printf("tick %s %s\n",fmt_now(), fmt_laddr(creq->base_addr));

//printf("CACHETRACE: , %s, %s, %s, %s, %s, %i, %s\n", fmt_now(), fmt_laddr(creq->base_addr), CacheSource_names[creq->cores[0].src], CacheAction_names[creq->action], CacheAccessType_names[creq->access_type], creq->cores[0].core->core_id, CoherAccessResult_names[creq->coher_wb_type]);

    printf("tick %s %s\n",fmt_now(), fmt_laddr(creq->base_addr));

    if (cache_wb_buffer_full(l3cache)) {
        
      //printf("tick %s %s\n",fmt_now(), fmt_laddr(creq->base_addr));

      DEBUGPRINTF("cache: time %s addr %s, L3 fill blocked (WB full), "
                    "retrying later\n", fmt_now(), fmt_laddr(creq->base_addr));
        cache_log_wbfull_conflict(l3cache);
        creq->request_time = cyc + 1;
        place_in_cache_queue(creq);
        return;
    }

    ready_time = cache_update_bank(l3cache, creq->base_addr, cyc,
                                   CacheBank_Fill);
    ready_time = ready_time + DelayCalc(creq);
    l3_replace(creq, l3cache, creq->base_addr, ready_time);
    
    creq->request_time = ready_time;
    creq->action = (GlobalParams.mem.private_l2caches) ?
        BUS_REPLY : L2FILL;
    place_in_cache_queue(creq);

}


// Begin writeback _to_ L3
static void
process_l3wb(CacheRequest *creq)
{
    CacheArray *l3cache = SharedL3Cache;
    //this is for L3 access trace
    //if (cyc < 1000000)
    //  printf("tick %s %s\n",fmt_now(), fmt_laddr(creq->base_addr));



    printf("tick %s %s\n",fmt_now(), fmt_laddr(creq->base_addr));

    if (cache_wb_buffer_full(l3cache)) {
        // Only accept this WB request if we have room to pass it on
        
      //printf("tick %s %s\n",fmt_now(), fmt_laddr(creq->base_addr));

      DEBUGPRINTF("cache: time %s addr %s, L3 WB blocked (WB full), "
                    "retrying later\n", fmt_now(), fmt_laddr(creq->base_addr));
        cache_log_wbfull_conflict(l3cache);
        creq->request_time = cyc + 1;
        place_in_cache_queue(creq);

        return;
    }

    int hit = cache_writeback(l3cache, creq->base_addr);
    i64 ready_time = 
        cache_update_bank(l3cache, creq->base_addr, cyc, CacheBank_WB);
    ready_time = ready_time + DelayCalc(creq);

    DEBUGPRINTF("cache: time %s addr %s, L3 wb: %s, ready at %s\n",
                fmt_now(), fmt_laddr(creq->base_addr),
                (hit) ? "hit" : "miss", fmt_i64(ready_time));

    if (!GlobalParams.mem.private_l2caches)
        cache_wb_accepted(SharedL2Cache, creq->base_addr);

    if (hit) {
        sim_assert(!creq->dependent_coher);     // Not used (and not handled)
        free_cache_request(creq);
    } else {
         creq->request_time = ready_time +
            GlobalParams.mem.l3cache_timing.miss_penalty;
        creq->action = MEM_WB;
        place_in_cache_queue(creq);
    }


}


static void
process_memaccess(CacheRequest *creq)
{
    MemUnit *mu = SharedMemUnit;
    creq->request_time = memunit_access(mu, creq->base_addr, cyc,
                                        MemUnit_Read);
    

//


    creq->service_level = SERVICED_MEM;
    if (GlobalParams.mem.use_l3cache) {
        creq->action = L3FILL;
    } else {
        creq->action = (GlobalParams.mem.private_l2caches) ? 
            BUS_REPLY : L2FILL;
    }
    {
        // Writebacks are not billed per-application, though they're still
        // counted in the MemUnit stats.
        AppState * restrict as = first_request_app(creq);
        if (as)
            as->extra->mem_accesses++;
    }

    place_in_cache_queue(creq);

}


static void
process_memwb(CacheRequest *creq)
{
    MemUnit *mu = SharedMemUnit;
    if (GlobalParams.mem.use_l3cache) {
        cache_wb_accepted(SharedL3Cache, creq->base_addr);
    } else if (!GlobalParams.mem.private_l2caches) {
        cache_wb_accepted(SharedL2Cache, creq->base_addr);
    }
    


    creq->request_time = memunit_access(mu, creq->base_addr, cyc,
                                        MemUnit_Write);
    sim_assert(!creq->dependent_coher); // Not used (and not handled)
    free_cache_request(creq);

}

void
process_cache_queues(void) 
{
    static void (*creq_handlers[CacheAction_last])(CacheRequest *) = {
        /* The order is critical here, it MUST match the CacheAction enum */
        process_l1fill, 
        process_bus_req, process_bus_reply, process_bus_wb,
        process_coher_wbi_l1,
        process_coher_wbi_l2_up, process_coher_wbi_l2_down,
        process_coher_wait, process_coher_reply,
        process_l2access, process_l2fill, process_l2wb, process_l3access,
        process_l3fill, process_l3wb,
        process_memaccess, process_memwb };

    CacheRequest *next_entry;

    while ((next_entry = cacheq_dequeue_ready(CacheQ, cyc)) != NULL) {
        unsigned handler_num = next_entry->action;

        dump_creq(next_entry, "dequeue");
        
        if (handler_num < CacheAction_last) 
            creq_handlers[handler_num](next_entry);
        else {
            fprintf(stderr, "ILLEGAL CACHE REQUEST, action #%u\n", 
                    handler_num);
            sim_abort();
        }
    }
}


// non-modifying helper function: for a CacheRequest, including along its
// dependent_coher chain, compute and return the union of all L1-related
// CacheSource members for a given core.
static CacheSource
creq_l1_sources_from_core(const CacheRequest *search_creq,
                          const CoreResources *search_core)
{
    CacheSource result = CSrc_None;
    const CacheRequest *walk_creq = search_creq;
    while (walk_creq) {
        for (int core_idx = 0; walk_creq->cores[core_idx].core; ++core_idx) {
            const CacheRequestCore *cr_core = &(walk_creq->cores[core_idx]);
            if (cr_core->core == search_core) {
                CacheSource walk_source = cr_core->src;
                if (CACHE_SOURCE_L1_ONLY(walk_source)) {
                    result |= walk_source;
                }
            }
        }
        walk_creq = walk_creq->dependent_coher;
    }
    return result;
}


// another helper function for ordered_l1_find_and_merge(), which is already
// huge.  this checks that the mshr-allocation outcome after a merge involving
// an L1 cache is sane, and aborts if not.
static void
validate_merge_l1_mshr_stat(MshrAllocOutcome mshr_stat,
                            CacheSource pre_merge_sources,
                            CacheSource req_source,
                            const CacheRequest *result_creq)
{
    // mshr_stat shouldn't be MSHR_Full -- MSHR availability was checked in
    // get_next_thread() before for insts, and in queue_for_core() ->
    // mshr_wait() for data.
    //
    // mshr_stat should USUALLY be MSHR_ReuseOld; see below for an exception.
    //
    // mshr_stat USUALLY shouldn't be MSHR_AllocNew if we're using coherence
    // -- whatever we merged with should have already had a "producer" MSHR
    // entry allocated (and we don't do cross-core forwarding at L1).  There
    // is an exception: when an L1 miss from one L1 structure is merged
    // against a request from the same core, which originated from a different
    // L1 structure (ICache vs. DCache, DStreamBuf vs. DCache, etc.): the
    // first request from any L1 structure should always allocate a new
    // producer.
    //
    // Arguably, distinct-L1 stuff shouldn't get merged until it hits L2, but
    // it's probably a rare occurrence anyway.  Also, without coherence we
    // don't bother checking for ReuseOld vs. AllocNew, since merging is all
    // sloppy anyway.
    int accept = 0;
    if (!GlobalCoherMgr) {
        accept = (mshr_stat == MSHR_ReuseOld) || (mshr_stat == MSHR_AllocNew);
    } else {
        // assert that we can play L1 source bit-mask games
        sim_assert(CACHE_SOURCE_L1_ONLY(req_source | pre_merge_sources));
        const int special_case_added_new_l1 =
            !(req_source & pre_merge_sources);
        if (((mshr_stat == MSHR_ReuseOld) && !special_case_added_new_l1) ||
            ((mshr_stat == MSHR_AllocNew) && special_case_added_new_l1)) {
            // acceptable outcomes: ReuseOld when not adding a new L1, or
            // AllocNew when adding a new L1.
            accept = 1;
        }
    }
    
    if (!accept) {
        abort_printf("unexpected MSHR alloc result (%s), "
                     "pre_merge_sources %s, req_source %s "
                     "for merged cache access: %s\n",
                     MshrAllocOutcome_names[mshr_stat],
                     CacheSource_names[pre_merge_sources],
                     CacheSource_names[req_source],
                     fmt_creq_static(result_creq));
    }
}
    

// Return codes for ordered_l1_find_and_merge(), describing whether any
// merging has taken place, and if so, the relation between the requests.
// (consider R1 to be some earlier request, and R2 to be the proposed new
// request)
typedef enum {
    CacheMerge_NoMerge = 0,             // No merge; R2 can go ahead
    CacheMerge_WillAlsoSatisfy,         // R1 will-also-satisfy R2
    CacheMerge_MustPrecede,             // R1 must-precede R2, will not satisfy
    CacheMergeResult_last
} CacheMergeResult;
const char *CacheMergeResult_names[] = {
    "NoMerge", "WillAlsoSatisfy", "MustPrecede", NULL
};

static CacheMergeResult
ordered_l1_find_and_merge(LongAddr base_addr, i64 addr_ready_cyc,
                          CacheAccessType access_type,
                          CacheSource req_source,
                          CoreResources *req_core, context *req_ctx,
                          int req_inst_or_neg1,
                          CacheRequest **pending_req_ret)
{
    // (merge_type is left as a naked int instead of a CacheMergeResult, since
    // it's conceptually different: merge_type is an internal thing saying
    // what this routine needs to do, CacheMergeResult is _why_.)
    int merge_type = 0; // 0: no merge, 1: merge into existing, 2: new dep_cohe
    CacheRequest *found_req = NULL;
    const CacheArray *l1cache = NULL;

    switch (req_source) {
    case CSrc_L1_ICache:
        l1cache = req_core->icache;
        break;
    case CSrc_L1_DCache:
        l1cache = req_core->dcache;
        break;
    case CSrc_L1_DStreamBuf:
        break;
    default:
        ENUM_ABORT(CacheSource, req_source);
    }
    
    {
        CacheRequest *oncore_found = 
            cacheq_find(CacheQ, base_addr, req_core->core_id, CQFS_Miss);
        CacheRequest *offcore_found = 
            cacheq_find(CacheQ, base_addr, CACHEQ_SHARED, CQFS_Miss);
        found_req = (oncore_found) ? oncore_found : offcore_found;
        // found_req may still be NULL
    }

    // If we're using coherence, even off-core requests should still have only
    // one subscribed core; other cores' off-core requests should have been
    // added to the dependent_coher chain by process_busreq(), instead of
    // merged into a common request.  (This isn't the only way to make things
    // safe, but it is a simple way.)
    assert_ifthen(GlobalCoherMgr && found_req, creq_single_core(found_req));

    // Get a mask of L1 sources from this core over the entire dep-coher
    // chain, before walking along to find a place to merge; even if we merge
    // with later requests on that chain, the MSHR producer entries allocated
    // by the earlier entries will be re-used.  (We also scan the
    // dependent_coher chain at fill, before freeing those producers.)  This
    // means that a single MSHR entry needs to handle the "load X, store X,
    // load X" case where the first load acquires only read permission, and so
    // the corresponding fill does not release the store or second load.
    // (Since our MSHRs don't actually do releasing, this is not currently a
    // concern.)
    const CacheSource pre_merge_sources =
        creq_l1_sources_from_core(found_req, req_core);

    if (found_req) {
        int subscription_found = 0;
        CacheRequest *walk_req = found_req;
        while (walk_req) {
            if (core_subscribed(walk_req, req_core)) {
                subscription_found = 1;
                found_req = walk_req;
            }
            walk_req = walk_req->dependent_coher;
        }
        // "found_req" points to the last (latest) request in the
        // dependent_coher to which req_core is subscribed; if req_core is not
        // subscribed to any, found_req points to the head of the chain.  If
        // found_req request subsumes this access, we'll merge this new access
        // into it; otherwise, we'll create a new request and add it to the
        // end of the dependent_coher chain.  However, when we're using
        // coherence, we will prohibit merging of requests across cores
        // (i.e. where this core is not already subscribed), since that could
        // allow some requests to bypass the coherence manager.
        if (cache_access_mergeable(found_req->access_type, access_type)
            && (!GlobalCoherMgr || subscription_found)) {
            merge_type = 1;             // merge into compatible request
        } else if (subscription_found) {
            // incompatible, but subscribed req: link newer as dependent
            merge_type = 2;
        } else {
            merge_type = 0;     // ignore cross-core, merge later at bus
        }
    }

    if ((merge_type != 0) && l1cache &&
        cache_access_ok(l1cache, base_addr, access_type)) {
        int found_includes_this_cache = 0;
        for (int i = 0; found_req->cores[i].core; ++i) {
            if (found_req->cores[i].core == req_core) {
                if ((found_req->cores[i].src & req_source) != 0) {
                    found_includes_this_cache = 1;
                }
                break;
            }
        }
        if (!found_includes_this_cache) {
            merge_type = 0;     // suppress merge, will proceed as hit
        }
    }

    CacheMergeResult result_code;
    CacheRequest *result_req;
    switch (merge_type) {
    case 0:     // No merge
        result_code = CacheMerge_NoMerge;
        result_req = NULL;
        break;

    case 1: {   // Merge individual fields, to be filled with existing req.
        if (req_ctx) {          // (may be NULL for e.g. prefetches)
            sim_assert((req_source == CSrc_L1_ICache) ||
                       (req_source == CSrc_L1_DCache));
            if (req_source == CSrc_L1_ICache) {
                req_ctx->icache_sim.was_merged = 1;
                merge_irequest(found_req, base_addr, req_ctx);
            } else {
                req_ctx->alist[req_inst_or_neg1].dcache_sim.was_merged = 1;
                merge_drequest(found_req, base_addr,
                               &req_ctx->alist[req_inst_or_neg1]);
            }
        } else {
            // with no irequest/drequest merges, we still need to be sure
            // this core gets subscribed.  (This comes up for prefetches.)
            add_core_to_creq(found_req, req_core, req_source);
        }
        result_code = CacheMerge_WillAlsoSatisfy;
        result_req = found_req;
        break;
    }

    case 2: {   // Create new request for this access, link it as dep. coher.
        CacheAction new_action = l1_route_down();
        CacheAccessType new_access_type =
            (access_type == Cache_Write) ? Cache_ReadExcl : access_type;
        CacheRequest *new_req = 
            get_c_request_holder(addr_ready_cyc, base_addr,
                                 new_access_type, new_action, req_source,
                                 req_core);
        if (req_ctx) {
            sim_assert((req_source == CSrc_L1_ICache) ||
                       (req_source == CSrc_L1_DCache));
            if (req_source == CSrc_L1_ICache) {
                merge_irequest(new_req, base_addr, req_ctx);
            } else {
                merge_drequest(new_req, base_addr,
                               &req_ctx->alist[req_inst_or_neg1]);
            }
        }
        append_dep_coher(found_req, new_req);
        result_code = CacheMerge_MustPrecede;
        result_req = new_req;
        break;
    }
    default:
        abort_printf("bad internal merge_type, %d\n", merge_type);
        result_code = 0;
        result_req = NULL;
        break;
    }

    sim_assert((result_code == CacheMerge_NoMerge) == (result_req == NULL));

    if ((result_code != CacheMerge_NoMerge) && l1cache) {
        // We're merging a request involving an L1 cache with some existing
        // request; we still need to allocate an MSHR entry for this new
        // consumer.  (PFStreamGroup requests don't need MSHRs at the moment)
        sim_assert((req_source == CSrc_L1_ICache) ||
                   (req_source == CSrc_L1_DCache));
        int is_from_icache = (req_source == CSrc_L1_ICache);
        MshrTable *mshr = (is_from_icache) ? req_core->inst_mshr :
            req_core->data_mshr;
        MshrAllocOutcome mshr_stat;
        if (req_ctx) {
            // normal request (producer+consumer)
            mshr_stat = (is_from_icache)
                ? mshr_alloc_inst(mshr, base_addr, req_ctx->id)
                : mshr_alloc_data(mshr, base_addr, req_ctx->id,
                                  req_inst_or_neg1);
        } else {
            // prefetch (producer only)
            mshr_stat = mshr_alloc_prefetch(mshr, base_addr);
        }

        #ifdef DEBUG
        validate_merge_l1_mshr_stat(mshr_stat, pre_merge_sources, req_source,
                                    result_req);
        #endif
    }

    if (pending_req_ret)
        *pending_req_ret = result_req;
    return result_code;
}


static int 
doiaccess_cache(LongAddr base_addr, int block_offset, context *ctx,
                CacheAccessType access_type, int *tlb_miss_ret)
{
    CoreResources *core = ctx->core;
    CacheArray *icache = core->icache;
    i64 ready_time;
    CacheLOutcome cache_stat;
    int penalty;
    int is_first_touch;
    int skip_tlb = 0;

    icache = core->icache;
 
    cache_stat = cache_lookup(icache, base_addr, access_type, &is_first_touch);
    ready_time =
        cache_update_bank(icache, base_addr, cyc, CacheBank_LookupR);
                "ready at %s\n", fmt_now(), fmt_laddr(base_addr), block_offset,
                core->core_id, CacheAccessType_names[access_type],
                CacheLOutcome_names[cache_stat], fmt_i64(ready_time));

    sim_assert(cache_stat != Cache_UpgradeMiss);     // (no excl-access insts)

    penalty = i64_to_int(ready_time - cyc);
    sim_assert(penalty >= 0);

    if (ctx->as) {
        ctx->as->extra->hitrate.icache.acc++;
        ctx->as->extra->icache_acc++;
        if (cache_stat == Cache_Hit)
            ctx->as->extra->hitrate.icache.hits++;
    }

    if (cache_stat == Cache_Hit) {
        int tlb_penalty = itlb_lookup(core, ctx->as, base_addr);
        if (skip_tlb)
            tlb_penalty = 0;
        penalty += tlb_penalty;
        ctx->icache_sim.service_level = 1;
        ctx->fetchcycle = cyc + penalty;
        ctx->as->extra->hitrate.itlb.acc++;
        ctx->as->extra->itlb_acc++;
        if (tlb_penalty == 0)
            ctx->as->extra->hitrate.itlb.hits++;
        *tlb_miss_ret = (tlb_penalty != 0);
        sim_assert(cache_access_ok(icache, base_addr, access_type));
    } else {
        int tlb_penalty = itlb_lookup(core, ctx->as, base_addr);
        CacheAccessType down_access_type = 
            (!GlobalCoherMgr) ? Cache_ReadExcl : Cache_Read;
        CacheRequest *creq;
        sim_assert(cache_stat == Cache_Miss);
        sim_assert(ctx->mergethread == NULL);
        /* Put a request in the cache queue */
        creq = get_c_request_holder(-1, base_addr, down_access_type,
                                    l1_route_down(), CSrc_L1_ICache, core);
        add_ireq_to_creq(creq, ctx);
        creq->request_time = cyc + penalty +
            core->params.icache.timing.miss_penalty + tlb_penalty;
        penalty = MEMDELAY_LONG;
        place_in_cache_queue(creq);
        MshrAllocOutcome mshr_stat =
            mshr_alloc_inst(core->inst_mshr, base_addr, ctx->id);
        if (mshr_stat != MSHR_AllocNew) {
            // shouldn't be MSHR_ReuseOld -- that should've been merged
            // shouldn't be MSHR_Full -- that was checked in fetch.c
            abort_printf("unexpected MSHR alloc result (%s), for non-merged"
                         " I-cache miss T%d/A%d base_addr %s\n",
                         MshrAllocOutcome_names[mshr_stat], ctx->id,
                         ctx->as->app_id, fmt_laddr(base_addr));
        }
        ctx->as->extra->hitrate.itlb.acc++;
        ctx->as->extra->itlb_acc++;
        if (tlb_penalty == 0)
            ctx->as->extra->hitrate.itlb.hits++;
        *tlb_miss_ret = (tlb_penalty != 0);
    }

    if (core->params.icache.prefetch_nextblock && is_first_touch) {
        LongAddr next_block_base = base_addr;
        next_block_base.a += GlobalParams.mem.cache_block_bytes;
        int pf_as_excl = 0;
        int pf_success;
        pf_success = 
            cachesim_prefetch_for_nextblock(core, next_block_base,
                                            pf_as_excl, CSrc_L1_ICache);
                    "excl %s: %s\n", core->core_id, fmt_laddr(next_block_base),
                    fmt_bool(pf_as_excl),
                    (pf_success) ? "submitted" : "rejected, dropping");
    }

    return penalty;
}


int 
doiaccess(mem_addr addr, context *ctx)
{
    CoreResources *core = ctx->core;
    CacheAccessType access_type = Cache_Read;
    CacheMergeResult merge_stat;
    CacheRequest *pending_req;
    int penalty;
    LongAddr base_addr;
    int block_offset;
    int tlb_miss = 0;

    laddr_set(base_addr, addr, ctx->as->app_master_id);
    cache_align_addr(core->icache, &base_addr);
    block_offset = addr - base_addr.a;

    ctx->icache_sim.was_merged = 0;

    merge_stat = 
        ordered_l1_find_and_merge(base_addr, cyc, access_type,
                                  CSrc_L1_ICache, core, ctx, -1, &pending_req);
    if (merge_stat != CacheMerge_NoMerge) {
        penalty = MEMDELAY_LONG;
    } else {
        penalty = doiaccess_cache(base_addr, block_offset, ctx, access_type,
                                  &tlb_miss);
    }

    sim_assert((penalty >= 0) || (penalty == MEMDELAY_LONG));
    if (penalty == MEMDELAY_LONG)
        ctx->fetchcycle = MAX_CYC; 
    ctx->icache_sim.last_startcyc = cyc;
    ctx->icache_sim.latency = (penalty == MEMDELAY_LONG) ? MAX_CYC: penalty;

    return penalty;
}


static void
daccess_memdelay(CoreResources *core, activelist *meminst)
{
    meminst->status = MEMORY;
}


static int
dodaccess_cache(LongAddr base_addr, int block_offset, int is_write,
                context *ctx, activelist *meminst, i64 addr_ready_cyc, 
                CacheAccessType access_type, int *tlb_miss_ret,
                int *upgrade_miss_ret, int *streambuf_hit_ret)
{
    CoreResources *core = ctx->core;
    CacheArray *dcache = core->dcache;
    i64 ready_time;
    CacheLOutcome cache_stat;
    int penalty;
    int skip_tlb = 0;
    int miss_d_streambuf_deferred_fill = 0;
    int is_first_touch;
    
    dcache = core->dcache;

    cache_stat = cache_lookup(dcache, base_addr, access_type, &is_first_touch);
    ready_time =
        cache_update_bank(dcache, base_addr, addr_ready_cyc,
                          (is_write) ? CacheBank_LookupW : CacheBank_LookupR);
    DEBUGPRINTF("cache: time %s addr %s +%d, core %d D-cache access %s: %s, "
                "ready at %s, first-touch %s\n", fmt_now(),
                fmt_laddr(base_addr),
                block_offset, core->core_id, 
                CacheAccessType_names[access_type],
                CacheLOutcome_names[cache_stat], fmt_i64(ready_time),
                fmt_bool(is_first_touch));

    assert_ifthen(!GlobalCoherMgr, (cache_stat != Cache_UpgradeMiss));

    *streambuf_hit_ret = 0;
    *upgrade_miss_ret = 0;

    if (core->d_streambuf && (cache_stat != Cache_Hit)) {
        CacheAccessType sb_access_type = 
            (access_type == Cache_Write) ? Cache_ReadExcl : access_type;
        int cast_max_access_type;
        // It's important to do the streambuf lookup even on upgrade
        // misses, so the streambuf has a chance to invalidate its copy
        if (pfsg_cache_miss(core->d_streambuf, base_addr, block_offset,
                            sb_access_type, meminst->pc,
                            &cast_max_access_type)) {
            DEBUGPRINTF("cache: C%d D-streambuf hit for block %s, "
                        "max access %s\n", core->core_id, fmt_laddr(base_addr),
                        CacheAccessType_names[cast_max_access_type]);
            *streambuf_hit_ret = 1;
            if (!cache_wb_buffer_full(dcache)) {
                if (cache_stat == Cache_UpgradeMiss) {
                    // Be sure to signal that this /was/ an upgrade miss,
                    // so that the pfaudit system doesn't freak out about
                    // an L1D-fill for a present block
                    *upgrade_miss_ret = 1;
                }
                cache_stat = Cache_Hit;
                ready_time = cache_update_bank(dcache, base_addr, cyc, 
                                               CacheBank_Fill);
                // inhibit_pfaudit_fill set: we'll do pfa_block_fill() from
                // dodaccess_pfaudit(), called after return.
                dcache_replace(NULL, core, base_addr, cast_max_access_type,
                               ready_time, 0, 1);
            } else {
                // Sigh, L1 can't accept fill traffic now, so set up a
                // deferred fill.
                DEBUGPRINTF("cache: C%d dcache busy on streambuf hit; "
                            "setting up deferred fill\n", core->core_id);
                cache_log_wbfull_conflict(dcache);
                sim_assert((cache_stat == Cache_Miss) ||
                           (cache_stat == Cache_UpgradeMiss));
                miss_d_streambuf_deferred_fill = 1;
            }
        }
    }

    penalty = i64_to_int(ready_time - addr_ready_cyc);
    sim_assert(penalty >= 0);

    if (meminst->as){
        meminst->as->extra->hitrate.dcache.acc++;
        meminst->as->extra->dcache_acc++;
    }

    if (cache_stat == Cache_Hit) {
        int tlb_penalty = dtlb_lookup(core, meminst->as, base_addr);
        if (skip_tlb)
            tlb_penalty = 0;
        penalty += tlb_penalty;
        totmem++;
        totmemdelay += penalty;
        meminst->dcache_sim.service_level = 1;
        if (meminst->as) {
            AppState *as = meminst->as;
            as->extra->hitrate.dcache.hits++;
            as->extra->hitrate.dtlb.acc++;
            as->extra->dtlb_acc++;
            if (tlb_penalty == 0)
                as->extra->hitrate.dtlb.hits++;
            as->extra->mem_delay.delay_sum += penalty;
            as->extra->mem_delay.sample_count++;
        }
        *tlb_miss_ret = (tlb_penalty != 0);
        sim_assert(cache_access_ok(dcache, base_addr, access_type));
    } else if ((cache_stat == Cache_Miss) ||
               (cache_stat == Cache_UpgradeMiss)) {
        int tlb_penalty = dtlb_lookup(core, meminst->as, base_addr);
        CacheRequest *creq;
        CacheAccessType down_access_type = 
            (!GlobalCoherMgr || is_write) ? Cache_ReadExcl : Cache_Read;
        creq = get_c_request_holder(-1, base_addr, down_access_type,
                                    l1_route_down(), CSrc_L1_DCache, core);
        add_dreq_to_creq(creq, meminst);
        creq->request_time = addr_ready_cyc + penalty +
            core->params.dcache.timing.miss_penalty + tlb_penalty;
        if (miss_d_streambuf_deferred_fill) {
            // Data is available from streambuf; retry until fill succeeds
            creq->service_level = 1;
            creq->action = L1FILL;
            creq->request_time -= core->params.dcache.timing.miss_penalty;
        }
        if (cache_stat == Cache_UpgradeMiss) {
            creq->access_type = access_type = Cache_Upgrade;
        }
        place_in_cache_queue(creq);
        daccess_memdelay(core, meminst);
        penalty = MEMDELAY_LONG;
        MshrAllocOutcome mshr_stat =
            mshr_alloc_data(core->data_mshr, base_addr, ctx->id, meminst->id);
        if (mshr_stat != MSHR_AllocNew) {
            // shouldn't be MSHR_ReuseOld -- that should've been merged
            // shouldn't be MSHR_Full -- that was checked in queue.c
            abort_printf("unexpected MSHR alloc result (%s), for non-merged"
                         " D-cache miss T%ds%d/A%d base_addr %s\n",
                         MshrAllocOutcome_names[mshr_stat], ctx->id,
                         meminst->id, ctx->as->app_id, fmt_laddr(base_addr));
        }
        if (meminst->as) {
            AppState *as = meminst->as;
            as->extra->hitrate.dtlb.acc++;
            as->extra->dtlb_acc++;
            if (tlb_penalty == 0)
                as->extra->hitrate.dtlb.hits++;
        }
        *tlb_miss_ret = (tlb_penalty != 0);
        *upgrade_miss_ret = (cache_stat == Cache_UpgradeMiss);
    } else {
        abort_printf("unhandled cache_stat value %d\n", (int) cache_stat);
    }

    if (core->params.dcache.prefetch_nextblock && is_first_touch) {
        LongAddr next_block_base = base_addr;
        next_block_base.a += GlobalParams.mem.cache_block_bytes;
        int pf_as_excl = (access_type != Cache_Read);
        int pf_success;
        pf_success = 
            cachesim_prefetch_for_nextblock(core, next_block_base,
                                            pf_as_excl, CSrc_L1_DCache);
        DEBUGPRINTF("cache: core %d D-cache prefetch_nextblock: %s "
                    "excl %s: %s\n", core->core_id, fmt_laddr(next_block_base),
                    fmt_bool(pf_as_excl),
                    (pf_success) ? "submitted" : "rejected, dropping");
    }
    core_ide=%d\n",penalty,base_addr,addr_ready_cyc,core->core_id);   
    return (penalty);
}


int
dodaccess(mem_addr addr, int is_write, context *ctx,
          activelist *meminst, i64 addr_ready_cyc) 
{
    CoreResources *core = ctx->core;
    CacheArray *dcache = core->dcache;
    int penalty = -1;
    CacheAccessType access_type  = (is_write) ? Cache_Write : Cache_Read;
    CacheMergeResult merge_stat;
    CacheRequest *pending_req;
    LongAddr base_addr;
    int block_offset;           // offset into block at base_addr
    int tlb_miss = 0;
    int upgrade_miss = 0;
    int streambuf_hit = 0;

    dcache = core->dcache;

    laddr_set(base_addr, addr, ctx->as->app_master_id);
    cache_align_addr(dcache, &base_addr);
    block_offset = addr - base_addr.a;

    if ((access_type == Cache_Read) && !GlobalCoherMgr)
        access_type = Cache_ReadExcl;

    merge_stat = 
        ordered_l1_find_and_merge(base_addr, addr_ready_cyc, access_type,
                                  CSrc_L1_DCache, core, ctx, meminst->id,
                                  &pending_req);

    if (merge_stat != CacheMerge_NoMerge) {
        // Request has been merged/enqueued in some fashion
        daccess_memdelay(core, meminst);
        penalty = MEMDELAY_LONG;
        if (merge_stat == CacheMerge_WillAlsoSatisfy) {
            for (int i = 0; pending_req->cores[i].core; ++i) {
                if (pending_req->cores[i].core == core) {
                    if (CACHE_SOURCE_L1_CONTAINS(pending_req->cores[i].src,
                                                 CSrc_L1_DStreamBuf)) {
                        pfsg_pf_merged(core->d_streambuf, base_addr, 1);
                        break;
                    }
                }
            }
        }
    } else {
        meminst->status = EXECUTING;
        penalty = dodaccess_cache(base_addr, block_offset, is_write, ctx,
                                  meminst, addr_ready_cyc, access_type,
                                  &tlb_miss, &upgrade_miss, &streambuf_hit);
    }

    if (meminst->addrcycle == MAX_CYC)          // (use earliest, if retry)
        meminst->addrcycle = addr_ready_cyc;
    meminst->dcache_sim.latency =
        (penalty == MEMDELAY_LONG) ? MAX_CYC : penalty;
    sim_assert((penalty >= 0) || (penalty == MEMDELAY_LONG));
    sim_assert(((meminst->status & MEMORY) != 0)
               == (penalty == MEMDELAY_LONG));

    if (core->d_dbp && (penalty != MEMDELAY_LONG)) {
        // (Only for "present" blocks; we'll catch the others at fill-time.)
        LongAddr orig_addr;
        laddr_set(orig_addr, base_addr.a + block_offset, base_addr.id);
        dbp_mem_exec(core->d_dbp, meminst->pc, orig_addr);
    }

    //if ((penalty >=29))
    //printf ("NOcache:   penalty= %d        base_addr=%d        addr_ready_cyc=%d      core_ide=%d\n",penalty,base_addr,addr_ready_cyc,core->core_id);


    //printf("penalty1 = %d\n",penalty);
    //printf("penalty = %d   %s\n",penalty,fmt_laddr(base_addr));   

    return penalty;
}

static void
report_hit_latencies(const CoreResources *core, const char *pref)
{
    int lat = 0;
    lat += core->params.dcache.timing.access_time.latency;
    printf("%sHit times (excluding exec) from: L1=%d", pref, lat);
    lat += core->params.dcache.timing.miss_penalty;
    if (GlobalParams.mem.private_l2caches) {
        lat += core->params.private_l2cache.timing.access_time.latency;
    } else {
        lat += GlobalParams.mem.bus_request_time.latency;
        lat += GlobalParams.mem.l2cache_timing.access_time.latency;
        lat += GlobalParams.mem.bus_transfer_time.latency;
    }
    lat += core->params.dcache.timing.fill_time.latency;
    printf(", L2=%d", lat);
    if (GlobalParams.mem.use_l3cache) {
        if (GlobalParams.mem.private_l2caches) {
            lat += core->params.private_l2cache.timing.miss_penalty;
            lat += GlobalParams.mem.bus_request_time.latency;
            lat += GlobalParams.mem.l3cache_timing.access_time.latency;
            lat += GlobalParams.mem.bus_transfer_time.latency;
            lat += core->params.private_l2cache.timing.fill_time.latency;
        } else {
            lat += GlobalParams.mem.l2cache_timing.miss_penalty;
            lat += GlobalParams.mem.l3cache_timing.access_time.latency;
            lat += GlobalParams.mem.l2cache_timing.fill_time.latency;
        }
        printf(", L3=%d", lat);
        lat += GlobalParams.mem.l3cache_timing.miss_penalty;
        lat += GlobalParams.mem.l3cache_timing.fill_time.latency;
    } else {
        if (GlobalParams.mem.private_l2caches) {
            lat += GlobalParams.mem.bus_request_time.latency;
            lat += core->params.private_l2cache.timing.miss_penalty;
            lat += GlobalParams.mem.bus_transfer_time.latency;
            lat += core->params.private_l2cache.timing.fill_time.latency;
        } else {
            lat += GlobalParams.mem.l2cache_timing.miss_penalty;
            lat += GlobalParams.mem.l2cache_timing.fill_time.latency;
        }
    }
    lat += GlobalParams.mem.main_mem.read_time.latency;
    printf(", M=%d\n", lat);
}


// Report ideal (contention-free, zero protocol overhead) data-transfer
// bandwidth at various levels in the cache hierarchy.  Units are in
// bytes per cycle.
static void
report_core_bw(const CoreResources *core, const char *pref)
{
    const char *part_names[] = { "L1I", "L1D", "Bus", "L2", "L3", "Mem" };
    const char *op_names[] = { "access", "wb-to", "fill" };
    const int n_parts = NELEM(part_names);
    const int n_ops = NELEM(op_names);

    double bw[NELEM(part_names)][NELEM(op_names)];
    int xfer_sz[NELEM(part_names)][NELEM(op_names)];    // in bytes
    int xfer_para[NELEM(part_names)][NELEM(op_names)];
    
    // Bandwidth estimates are calculated in steps; first, bw[][] is set to
    // the inter-request time in cycles, then bank-parallelism and transfer
    // widths are accounted for.

    // L1 I
    bw[0][0] = core->params.icache.timing.access_time.interval;
    bw[0][1] = -1;       // no WBs to L1
    bw[0][2] = core->params.icache.timing.fill_time.interval;

    // L1 D
    bw[1][0] = core->params.dcache.timing.access_time.interval;
    bw[1][1] = -1;       // no WBs to L1
    bw[1][2] = core->params.dcache.timing.fill_time.interval;

    // Bus: cheat and use access/fill op titles for request/transfer times
    bw[2][0] = GlobalParams.mem.bus_request_time.interval;
    bw[2][1] = -1;
    bw[2][2] = GlobalParams.mem.bus_transfer_time.interval;

    // L2 (may be private)
    if (GlobalParams.mem.private_l2caches) {
        bw[3][0] = core->params.private_l2cache.timing.access_time.interval;
        bw[3][1] = core->params.private_l2cache.timing.access_time_wb.interval;
        bw[3][2] = core->params.private_l2cache.timing.fill_time.interval;
    } else {
        bw[3][0] = GlobalParams.mem.l2cache_timing.access_time.interval;
        bw[3][1] = GlobalParams.mem.l2cache_timing.access_time_wb.interval;
        bw[3][2] = GlobalParams.mem.l2cache_timing.fill_time.interval;
    }

    // L3 (may be unused)
    bw[4][0] = GlobalParams.mem.l3cache_timing.access_time.interval;
    bw[4][1] = GlobalParams.mem.l3cache_timing.access_time_wb.interval;
    bw[4][2] = GlobalParams.mem.l3cache_timing.fill_time.interval;

    // Memory
    bw[5][0] = GlobalParams.mem.main_mem.read_time.interval;
    bw[5][1] = GlobalParams.mem.main_mem.write_time.interval;
    bw[5][2] = -1;

    // Estimate transfer sizes (excluding most(?) overhead)
    for (int part = 0; part < n_parts; part++) {
        for (int op = 0; op < n_ops; op++) {
            int bytes = GlobalParams.mem.cache_block_bytes;
            if ((part == 0) && (op == 0)) {             // I-cache access
                bytes = GlobalParams.mem.inst_bytes;
            } else if ((part == 1) && (op == 0)) {      // D-cache access
                bytes = 8;                              // (ldq/stq: 8 bytes)
            } else if ((part == 2) && (op == 0)) {      // Bus req
                bytes = 8;      // 64-bit virt address?
            }
            xfer_sz[part][op] = bytes;
        }
    }

    // set parallelism limit (width) from banking/porting
    for (int part = 0; part < n_parts; part++) {
        for (int op = 0; op < n_ops; op++) {
            xfer_para[part][op] = 1;
        }
    }
    // L1 I
    xfer_para[0][0] = core->params.icache.geom->n_banks *
        (core->params.icache.geom->ports.r +
         core->params.icache.geom->ports.rw);
    xfer_para[0][1] = xfer_para[0][2] = core->params.icache.geom->n_banks *
        (core->params.icache.geom->ports.w +
         core->params.icache.geom->ports.rw);
    // L1 D
    xfer_para[1][0] = core->params.dcache.geom->n_banks *
        (core->params.dcache.geom->ports.r +
         core->params.dcache.geom->ports.rw);
    xfer_para[1][1] = xfer_para[1][2] = core->params.dcache.geom->n_banks *
        (core->params.dcache.geom->ports.w +
         core->params.dcache.geom->ports.rw);
    // L2
    if (GlobalParams.mem.private_l2caches) {
        xfer_para[3][0] = core->params.private_l2cache.geom->n_banks *
            (core->params.private_l2cache.geom->ports.r +
             core->params.private_l2cache.geom->ports.rw);
        xfer_para[3][1] = xfer_para[3][2] =
            core->params.private_l2cache.geom->n_banks *
            (core->params.private_l2cache.geom->ports.w +
             core->params.private_l2cache.geom->ports.rw);
    } else {
        xfer_para[3][0] = GlobalParams.mem.l2cache_geom->n_banks *
            (GlobalParams.mem.l2cache_geom->ports.r +
             GlobalParams.mem.l2cache_geom->ports.rw);
        xfer_para[3][1] = xfer_para[3][2] =
            GlobalParams.mem.l2cache_geom->n_banks *
            (GlobalParams.mem.l2cache_geom->ports.w +
             GlobalParams.mem.l2cache_geom->ports.rw);
    }
    // L3
    xfer_para[4][0] = GlobalParams.mem.l3cache_geom->n_banks *
        (GlobalParams.mem.l3cache_geom->ports.r +
         GlobalParams.mem.l3cache_geom->ports.rw);
    xfer_para[4][1] = xfer_para[4][2] =
        GlobalParams.mem.l3cache_geom->n_banks *
        (GlobalParams.mem.l3cache_geom->ports.w +
         GlobalParams.mem.l3cache_geom->ports.rw);
    // Mem
    for (int op = 0; op < n_ops; op++)
        xfer_para[5][op] = GlobalParams.mem.main_mem.n_banks;

    for (int part = 0; part < n_parts; part++) {
        for (int op = 0; op < n_ops; op++) {
            if (bw[part][op] > 0) {
                bw[part][op] = xfer_sz[part][op] * xfer_para[part][op]
                    / bw[part][op];
            }
        }
    }

    printf("%sIdeal est. transfer bandwidth (bytes/cyc * banks * ports):\n",
           pref);
    printf("%s  %10s", pref, "part");
    for (int op = 0; op < NELEM(op_names); op++)
        printf("%10s", op_names[op]);
    printf("\n");
    for (int part = 0; part < NELEM(part_names); part++) {
        int part_swap = part;
        if (GlobalParams.mem.private_l2caches &&
            ((part == 2) || (part == 3))) {
            // Swap BUS and L2 print order, when using private L2s.
            part_swap = (part == 2) ? 3 : 2;
        }
        if (!GlobalParams.mem.use_l3cache && (part == 4)) {
            continue;
        }
        printf("%s  %10s", pref, part_names[part_swap]);
        for (int op = 0; op < NELEM(op_names); op++) {
            double val = bw[part_swap][op];
            if (val > 0) {
                printf(" %9g", val);
            } else {
                printf(" %9s", "na");
            }
        }
        printf("\n");
    }
}


static void 
print_cstats_core(CoreResources *core) 
{
    CacheStats i_stats, d_stats;
    TLBStats itlb_stats, dtlb_stats;
    int i;

    cache_get_stats(core->icache, &i_stats);
    cache_get_stats(core->dcache, &d_stats);
    tlb_get_stats(core->itlb, &itlb_stats);
    tlb_get_stats(core->dtlb, &dtlb_stats);

    printf("Core %i:\n", core->core_id);
    printf("  Reads: %s   Writes: %s\n", 
           fmt_i64(d_stats.reads + d_stats.reads_ex + i_stats.reads +
                   i_stats.reads_ex),
           fmt_i64(d_stats.writes));
    report_hit_latencies(core, "  ");
    report_core_bw(core, "  ");
    printf("  ICACHE: size: %d KB assoc: %d\n", 
           core->params.icache.geom->size_kb,
           core->params.icache.geom->assoc);
    if ((i_stats.hits+i_stats.misses) > 0)
        printf("  ICACHE: hits: %s misses: %s  Hit Ratio: %.2f%%\n",
               fmt_i64(i_stats.hits), fmt_i64(i_stats.misses), 
               (double) 100 * i_stats.hits / (i_stats.hits + i_stats.misses));
    printf("  ICACHE: coher misses: %s invalidates: %s\n"
           "          wbfull_confs: %s\n",
           fmt_i64(i_stats.coher_misses), 
           fmt_i64(i_stats.coher_invalidates),
           fmt_i64(i_stats.wbfull_confs));
    if (core->tcache) {
        TraceCacheStats t_stats;
        tc_get_stats(core->tcache, &t_stats);
        printf("  TrCACHE: entries: %ld assoc: %d inst/block: %d "
               "pred/block: %d path-assoc: %d trim-partial: %d\n",
           core->params.tcache.n_entries, core->params.tcache.assoc,
           core->params.tcache.block_insts, core->params.tcache.pred_per_block,
           core->params.tcache.is_path_assoc,
           core->params.tcache.trim_partial_hits);
        printf("  TrCACHE: hits: %s (%s par.) misses: %s fills: %s "
               "evicts: %s\n",
           fmt_i64(t_stats.trace_hits), fmt_i64(t_stats.partial_hits),
           fmt_i64(t_stats.trace_misses),
           fmt_i64(t_stats.fills), fmt_i64(t_stats.evicts));
        printf("  TrCACHE: Hit Ratio: %.2f%% inst/hit: %.2f "
           "pred/hit: %.2f\n",
           (100.0 * t_stats.trace_hits) / 
           (t_stats.trace_hits + t_stats.trace_misses),
           (double)t_stats.hit_insts / t_stats.trace_hits,
           (double)t_stats.hit_preds / t_stats.trace_hits);
    }
    printf("  DCACHE: size: %d KB assoc: %d\n", 
           core->params.dcache.geom->size_kb,
           core->params.dcache.geom->assoc);
    if ((d_stats.hits+d_stats.misses) > 0)
        printf("  DCACHE: hits: %s misses: %s writebacks: %s  "
               "Hit Ratio: %.2f%%\n",
               fmt_i64(d_stats.hits), fmt_i64(d_stats.misses),
               fmt_i64(d_stats.dirty_evicts), 
               (double) 100 * d_stats.hits/(d_stats.hits+d_stats.misses));
    printf("  DCACHE: coher misses: %s upgrade misses: %s\n"
           "          coher writebacks: %s invalidates: %s\n"
           "          wbfull_confs: %s coher_busy: %s \n",
           fmt_i64(d_stats.coher_misses), 
           fmt_i64(d_stats.upgrade_misses), 
           fmt_i64(d_stats.coher_writebacks), 
           fmt_i64(d_stats.coher_invalidates),
           fmt_i64(d_stats.wbfull_confs),
           fmt_i64(d_stats.coher_busy));
    if (GlobalParams.mem.private_l2caches) {
        CacheStats l2_stats;
        cache_get_stats(core->l2cache, &l2_stats);
        printf("  SCACHE: size: %d KB assoc: %d\n", 
               core->params.private_l2cache.geom->size_kb,
               core->params.private_l2cache.geom->assoc);
        if ((l2_stats.hits+l2_stats.misses) > 0)
            printf("  SCACHE: hits: %s misses: %s writebacks: %s  "
                   "Hit Ratio: %.2f%%\n",
                   fmt_i64(l2_stats.hits), fmt_i64(l2_stats.misses),
                   fmt_i64(l2_stats.dirty_evicts), 
                   (double) 100 * l2_stats.hits/
                   (l2_stats.hits+l2_stats.misses));
        printf("  SCACHE: coher misses: %s upgrade misses: %s \n"
               "          coher writebacks: %s invalidates: %s\n"
               "          wbfull_confs: %s coher_busy: %s\n",
               fmt_i64(l2_stats.coher_misses), 
               fmt_i64(l2_stats.upgrade_misses), 
               fmt_i64(l2_stats.coher_writebacks), 
               fmt_i64(l2_stats.coher_invalidates),
               fmt_i64(l2_stats.wbfull_confs),
               fmt_i64(l2_stats.coher_busy));
        printf("  Stalls for L2 MSHR conflicts: %s\n",
               fmt_i64(core->private_l2mshr_confs));
    }

    if (core->d_streambuf) {
        printf("  D-streambuf stats:\n");
        pfsg_print_stats(core->d_streambuf, stdout, "    ");
    }
    if (core->i_dbp) {
        printf("  I-deadblock stats:\n");
        dbp_print_stats(core->i_dbp, stdout, "    ");
    }
    if (core->d_dbp) {
        printf("  D-deadblock stats:\n");
        dbp_print_stats(core->d_dbp, stdout, "    ");
    }

    printf("  ITLB: size: %d, misses %s, miss rate %.2f\n",
           core->params.itlb_entries, fmt_i64(itlb_stats.misses), 
           (double) 100.0*itlb_stats.misses / (i_stats.hits+i_stats.misses));
    printf("  DTLB: size: %d, misses %s, miss rate %.2f\n",
           core->params.dtlb_entries, fmt_i64(dtlb_stats.misses), 
           (double) 100.0*dtlb_stats.misses / (d_stats.hits+d_stats.misses));

    printf("  icache bank util. ");
    for (i=0; i<core->params.icache.geom->n_banks ;i++) {
        CacheBankStats bank_stats;
        cache_get_bankstats(core->icache, cyc, i, &bank_stats);
        printf("%.3f ", bank_stats.util);
    }
    printf("\n");
    printf("  dcache bank util. ");
    for (i=0; i<core->params.dcache.geom->n_banks ;i++) {
        CacheBankStats bank_stats;
        cache_get_bankstats(core->dcache, cyc, i, &bank_stats);
        printf("%.3f ", (double) bank_stats.util);
    }
    printf("\n");
    if (GlobalParams.mem.private_l2caches) {
        printf("  L2 bank util. ");
        for (i=0;i<core->params.private_l2cache.geom->n_banks;i++) {
            CacheBankStats bank_stats;
            cache_get_bankstats(core->l2cache, cyc, i, &bank_stats);
            printf("%.3f ", bank_stats.util);
        }
        printf("\n");
    }
}


void 
print_cstats(void) 
{
    int i;

    printf("Cache Statistics\n");

    for (i = 0; i < CoreCount; i++)
        print_cstats_core(Cores[i]);

    printf("Stall for miss queue full %s cycles (%.2f%%)\n",
           fmt_u64(miss_queue_full), (double) miss_queue_full/cyc);
    if (!GlobalParams.mem.private_l2caches) {
        CacheStats l2_stats;
        cache_get_stats(SharedL2Cache, &l2_stats);
        printf("SCACHE: size: %d KB assoc: %d\n", 
               GlobalParams.mem.l2cache_geom->size_kb,
               GlobalParams.mem.l2cache_geom->assoc);
        if((l2_stats.hits+l2_stats.misses) > 0) {
            printf("SCACHE: hits: %s misses: %s  writebacks: %s  "
                   "Hit Ratio: %.2f%%\n",
                   fmt_i64(l2_stats.hits), fmt_i64(l2_stats.misses),
                   fmt_i64(l2_stats.dirty_evicts),
                   (double) 100*l2_stats.hits/(l2_stats.hits+l2_stats.misses));
        }
        printf("SCACHE: wbfull_confs: %s\n", fmt_i64(l2_stats.wbfull_confs));
    }
    if (GlobalParams.mem.use_l3cache) {
        CacheStats l3_stats;
        cache_get_stats(SharedL3Cache, &l3_stats);
        printf("3CACHE: size: %d KB assoc: %d\n",
               GlobalParams.mem.l3cache_geom->size_kb,
               GlobalParams.mem.l3cache_geom->assoc);
        if((l3_stats.hits+l3_stats.misses) > 0) {
            printf("3CACHE: hits: %s misses: %s  writebacks: %s  "
                   "Hit Ratio: %.2f%%\n",
                   fmt_i64(l3_stats.hits), fmt_i64(l3_stats.misses),
                   fmt_i64(l3_stats.dirty_evicts),
                   (double) 100*l3_stats.hits/(l3_stats.hits+l3_stats.misses));
        }
        printf("3CACHE: wbfull_confs: %s\n", fmt_i64(l3_stats.wbfull_confs));
    }
    printf("avg mem delay %.3f\n", (double) totmemdelay/totmem);
    if (!GlobalParams.mem.private_l2caches) {
        printf("L2 bank util. ");
        for (i=0;i<GlobalParams.mem.l2cache_geom->n_banks;i++) {
            CacheBankStats bank_stats;
            cache_get_bankstats(SharedL2Cache, cyc, i, &bank_stats);
            printf("%.3f ", bank_stats.util);
        }
        printf("\n");
    }
    {
        CoreBusStats bus_stats;
        if (GlobalParams.mem.split_bus) {
            corebus_get_stats(SharedCoreRequestBus, &bus_stats);
            printf("Request bus: %s xfers, %s syncs, %s idle_cyc, "
                   "%s sync_cyc, %.3f util\n",
                   fmt_i64(bus_stats.xfers), fmt_i64(bus_stats.syncs),
                   fmt_i64(bus_stats.idle_cyc), fmt_i64(bus_stats.sync_cyc),
                   bus_stats.util);
            corebus_get_stats(SharedCoreReplyBus, &bus_stats);
            printf("Reply bus: %s xfers, %s syncs, %s idle_cyc, "
                   "%s sync_cyc, %.3f util\n",
                   fmt_i64(bus_stats.xfers), fmt_i64(bus_stats.syncs),
                   fmt_i64(bus_stats.idle_cyc), fmt_i64(bus_stats.sync_cyc),
                   bus_stats.util);
        } else {
            corebus_get_stats(SharedCoreRequestBus, &bus_stats);
            printf("Unified bus: %s xfers, %s syncs, %s idle_cyc, "
                   "%s sync_cyc, %.3f util\n",
                   fmt_i64(bus_stats.xfers), fmt_i64(bus_stats.syncs),
                   fmt_i64(bus_stats.idle_cyc), fmt_i64(bus_stats.sync_cyc),
                   bus_stats.util);
        }
    }
    if (GlobalParams.mem.use_l3cache) {
        double l3util = 0;
        for (i=0;i<GlobalParams.mem.l3cache_geom->n_banks;i++) {
            CacheBankStats bank_stats;
            cache_get_bankstats(SharedL3Cache, cyc, i, &bank_stats);
            l3util += bank_stats.util;
        }
        l3util /= GlobalParams.mem.l3cache_geom->n_banks;
        printf("L3 util. = %.3f\n", l3util);
    }
    {
        MemUnitStats mem_stats;
        memunit_get_stats(SharedMemUnit, &mem_stats);
        printf("MemUnit stats: %s reads, %s writes\n", 
               fmt_i64(mem_stats.reads), fmt_i64(mem_stats.writes));
        printf("MemUnit bank util:");
        for (i = 0; i < GlobalParams.mem.main_mem.n_banks; i++) {
            MemBankStats bank_stats;
            memunit_get_bankstats(SharedMemUnit, cyc, i, &bank_stats);
            printf(" %.3f", bank_stats.util);
        }
        printf("\n");
    }
    {
        i64 calls = 0, gave_up = 0, cache_inj = 0, cache_wb_full = 0;
        for (i = 0; i < CoreCount; i++) {
            const CoreResources * restrict core = Cores[i];
            calls += core->cache_inject_stats.calls;
            gave_up += core->cache_inject_stats.gave_up;
            cache_inj += core->cache_inject_stats.cache_inj;
            cache_wb_full += core->cache_inject_stats.cache_wb_full;
        }
        if (calls) {
            printf("cachesim_oracle_inject_core: %s calls, %s gave_up, "
                   "%s cache_inj, %s cache_wb_full\n", fmt_i64(calls),
                   fmt_i64(gave_up), fmt_i64(cache_inj),
                   fmt_i64(cache_wb_full));
        }
    }
    {
        i64 calls = 0, gave_up = 0, cache_matches = 0;
        for (i = 0; i < CoreCount; i++) {
            const CoreResources * restrict core = Cores[i];
            calls += core->cache_discard_stats.calls;
            gave_up += core->cache_discard_stats.gave_up;
            cache_matches += core->cache_discard_stats.cache_matches;
        }
        if (calls) {
            printf("cachesim_oracle_discard_block: %s calls, %s gave_up, "
                   "%s cache_matches\n", fmt_i64(calls),
                   fmt_i64(gave_up), fmt_i64(cache_matches));
        }
    }
}


void 
zero_cstats(void) 
{
    int i;
    for (i = 0; i < CoreCount; i++) {
        CoreResources *core = Cores[i];
        cache_reset_stats(core->icache, cyc);
        cache_reset_stats(core->dcache, cyc);
        if (core->tcache)
            tc_reset_stats(core->tcache);
        if (GlobalParams.mem.private_l2caches)
            cache_reset_stats(core->l2cache, cyc);
    }

    if (!GlobalParams.mem.private_l2caches)
        cache_reset_stats(SharedL2Cache, cyc);
    if (GlobalParams.mem.use_l3cache)
        cache_reset_stats(SharedL3Cache, cyc);
}


/* TLB routines */

// "as" may be NULL
static int
itlb_lookup(CoreResources * restrict core, AppState * restrict as,
            LongAddr addr)
{
    int tlb_penalty = core->params.tlb_miss_penalty;
    int penalty;
    static int perfect_itlb = -1;

    if (perfect_itlb < 0)
        perfect_itlb = simcfg_get_bool("Hacking/perfect_tlbs");
    if (perfect_itlb)
        return 0;

    if (core->params.tlb_filter_invalid && (as != NULL) &&
        !tlb_probe(core->itlb, addr.a, addr.id) &&
        !pmem_get_base(as->pmem, addr.a)) {
        // TLB miss for an address which isn't mapped; we'll make it pay the
        // full penalty, but we won't insert it into the TLB.  (This doesn't
        // allow for auto-grow-down behavior on the stack, but we don't
        // do much fetching from the stack.  In fact, invalid fetch PCs should
        // be caught in fetch_for_core(), at stash_decode_inst() failure.)
        penalty = tlb_penalty;
    } else {
        penalty = i64_to_int(tlb_lookup(core->itlb, cyc, addr.a, addr.id,
                                        tlb_penalty, NULL));
    }

    if (DEBUG_TLBS && debug)
        printf("itlb: %s lookup %s base_addr %s -> %i\n",
               fmt_i64(cyc), fmt_laddr(addr),
               fmt_x64(tlb_calc_baseaddr(core->itlb, addr.a)), 
               penalty);

    return penalty;
}


// "as" may be NULL
static int 
dtlb_lookup(CoreResources * restrict core, AppState * restrict as,
            LongAddr addr)
{
    int penalty, is_hit;
    static int spill_miss_only = -1;
    static int perfect_dtlb = -1;
    int prevent_tlb_insert = 0;

    if (perfect_dtlb < 0)
        perfect_dtlb = simcfg_get_bool("Hacking/perfect_tlbs");
    if (perfect_dtlb)
        return 0;

    if (core->params.tlb_filter_invalid && (as != NULL) &&
        !tlb_probe(core->dtlb, addr.a, addr.id) &&
        !pmem_get_base(as->pmem, addr.a)) {
        // TLB miss for an address which isn't mapped; we'll make it pay the
        // full penalty, but we won't insert it into the TLB.  In a real chip,
        // it would never be inserted, since the TLB miss handler a) wouldn't
        // get invoked on a wrong-path access, and b) wouldn't find an invalid
        // address in the page table, with which to fill; instead you'd
        // get a segfault or similar.  (We'll cheat and blindly allow one page
        // of grow-down per mapped region, to allow for stack growth.)
        mem_addr next_seg = pmem_get_nextbase(as->pmem, addr.a);
        if (!next_seg || ((next_seg - addr.a) > GlobalParams.mem.page_bytes)) {
            prevent_tlb_insert = 1;
        }
    }

    if (prevent_tlb_insert) {
        penalty = core->params.tlb_miss_penalty;
    } else {
        penalty = i64_to_int(tlb_lookup(core->dtlb, cyc, addr.a, addr.id,
                                        core->params.tlb_miss_penalty,
                                        &is_hit));
    }

    if (DEBUG_TLBS && debug)
        printf("dtlb: %s lookup %s base_addr %s -> %i\n",
               fmt_i64(cyc), fmt_laddr(addr),
               fmt_x64(tlb_calc_baseaddr(core->dtlb, addr.a)),
               penalty);

    if (spill_miss_only < 0)
        spill_miss_only = simcfg_get_bool("Hacking/spill_dtlb_missonly");

    if ((!spill_miss_only || !is_hit) && (as)) {
        AppStateExtras * restrict ase = as->extra;
        ase->bmt.spill_dtlb.head++;
        if (ase->bmt.spill_dtlb.head >= ase->bmt.spill_dtlb.size)
            ase->bmt.spill_dtlb.head = 0;
        int next_idx = ase->bmt.spill_dtlb.head;
        if (ase->bmt.spill_dtlb.used < ase->bmt.spill_dtlb.size)
            ase->bmt.spill_dtlb.used++;
        ase->bmt.spill_dtlb.ents[next_idx].base_addr =
            tlb_calc_baseaddr(core->dtlb, addr.a);
        ase->bmt.spill_dtlb.ents[next_idx].ready_time = cyc + penalty;
    }

    return penalty;
}


/* brute force clean up of the cache queue after a branch mispredict */
void
clean_cache_queue_mispredict(context *ctx) 
{

    CacheRequest *entry = ctx->imiss_cache_entry;
    context *entrycontext;
    context **pointerto;

    sim_assert(entry != NULL);
    assert_ifthen(TEST_CREQ_INVARIANT, creq_invariant(entry, 1));

    entrycontext = entry->irequestor;
    pointerto = &(entry->irequestor);
    while (entrycontext) {
        if (entrycontext == ctx) {
            // unlink CacheRequest -> ... -> context
            *pointerto = entrycontext->mergethread;
            ctx->mergethread = NULL;
            // unlink context -> CacheRequest
            ctx->imiss_cache_entry = NULL;
            mshr_cfree_inst(ctx->core->inst_mshr, entry->base_addr, ctx->id);
            assert_ifthen(TEST_CREQ_INVARIANT, creq_invariant(entry, 0));
            return;
        }
        pointerto = &entrycontext->mergethread;
        entrycontext = entrycontext->mergethread;
    }

    /*should not get here */
    abort_printf("clean_cache_queue_mispredict: T%d "
                 "(A%d npc %s) points to imiss entry which doesn't "
                 "point back.  imiss entry: %s\n", ctx->id,
                 ((ctx->as) ? ctx->as->app_id : -1),
                 fmt_x64((ctx->as) ? ctx->as->npc : 0),
                 fmt_creq_static(entry));
}


static void
clean_creq_for_squash(CacheRequest *creq)
{
    activelist *d_req = creq->drequestor;
    activelist **d_req_prev = &(creq->drequestor);

    // disable recursive invariant check; we'll recurse it ourselves
    assert_ifthen(TEST_CREQ_INVARIANT, creq_invariant(creq, 0));

    while (d_req) {
        activelist *d_req_next = d_req->mergeinst;
        if (d_req->status & SQUASHED) {
            CoreResources *core = Contexts[d_req->thread]->core;
            // unlink CacheRequest -> ... -> activelist
            *d_req_prev = d_req_next;
            d_req->mergeinst = NULL;
            // unlink activelist -> CacheRequest
            d_req->dmiss_cache_entry = NULL;
            mshr_cfree_data(core->data_mshr, creq->base_addr, d_req->thread,
                            d_req->id);
        } else {
            d_req_prev = &(d_req->mergeinst);
        }
        d_req = d_req_next;
    }

    if (0) {
        // I if(0){}'d this out in revision 1.47.6.7 -> 1.47.6.8 without
        // noting why; in retrospect, it looks like it's redundant, due to the
        // flush_for_halt() -> reset_nextpc() ->
        // clean_cache_queue_mispredict() call chain already unlinking the
        // context in question from i_req, and clearing its
        // "imiss_cache_entry" field.
        context *i_req = creq->irequestor;
        context **i_req_prev = &(creq->irequestor);
        while (i_req) {
            context *i_req_next = i_req->mergethread;
            if (i_req->halting != CtxHalt_NoHalt) {
                *i_req_prev = i_req_next;
                i_req->mergethread = NULL;
                i_req->imiss_cache_entry = NULL;
                mshr_cfree_inst(i_req->core->inst_mshr, creq->base_addr,
                                i_req->id);
            } else {
                i_req_prev = &(i_req->mergethread);
            }
            i_req = i_req_next;
        }
    }
    if (creq->dependent_coher) {
        // recursively follows dependent_coher chain
        clean_creq_for_squash(creq->dependent_coher);
    }
    assert_ifthen(TEST_CREQ_INVARIANT, creq_invariant(creq, 0));
}


void 
clean_cache_queue_squash(void)
{
    CacheRequest *cacherequest;

    cacheq_iter_reset(CacheQ);

    while ((cacherequest = cacheq_iter_next(CacheQ))) {
        clean_creq_for_squash(cacherequest);
    }      
}


mem_addr 
calc_lock_paddr(context *ctx, mem_addr addr)
{
    mem_addr paddr = (addr >> GlobalParams.mem.cache_block_bytes_lg) << 
        GlobalParams.mem.cache_block_bytes_lg;
    mem_addr xlated_addr = u64_from_ptr(
        pmem_xlate_hack(ctx->as->pmem, paddr, 1, PMAF_NoExcept));

    sim_assert(!(paddr & 1));
    sim_assert(!(xlated_addr & 1));

    if (xlated_addr == 0) {
        // Invalid address; still okay to lock it, just keep it unique
        // Set low bit to indicate that this happened.
        sim_assert(ctx->as->app_master_id >= 0);
        xlated_addr = (((mem_addr) ctx->as->app_master_id) << 1) | 1;
    }

    return xlated_addr;
}


int
cache_register_blocked_app(struct context *ctx, int dmiss_alist_id)
{
    CacheRequest *creq;

    if (dmiss_alist_id >= 0) {
        // Blocked on D-cache miss
        activelist * restrict inst = &ctx->alist[dmiss_alist_id];
        sim_assert(inst->status & MEMORY);
        sim_assert(inst->dmiss_cache_entry);
        creq = inst->dmiss_cache_entry;
    } else {
        // Blocked on I-cache miss
        sim_assert(ctx->imiss_cache_entry);     // added since it seems right?
        creq = ctx->imiss_cache_entry;
    }
    if (creq) {
        add_blocked_app(creq, ctx->as);
        return 0;
    }
    return -1;
}


// Query: is "action" performed entirely within a single core, invisible to
// the rest of the system?
int
cache_action_incore(CacheAction action)
{
    int result;
    switch (action) {
    case L1FILL:
    case BUS_REQ:       // Odd, but while waiting for BUS_REQ, it's private
    case COHER_WBI_L1:
    case COHER_REPLY:
        result = 1;
        break;
    case L2ACCESS:
    case L2FILL:
    case L2_WB:
    case COHER_WBI_L2_UP:
    case COHER_WBI_L2_DOWN:
        result = GlobalParams.mem.private_l2caches;
        break;
    case BUS_REPLY:
    case BUS_WB:
    case L3ACCESS:
    case L3FILL:
    case L3_WB:
    case MEMACCESS:
    case MEM_WB:
    case COHER_WAIT:    // sneaky: we're forcing this "shared" for searching
        result = 0;
        break;
    default:
        result = 0;
        ENUM_ABORT(CacheAction, action);
    }
    return result;
}


int
creq_invariant(const struct CacheRequest *creq, int recurse)
{
    const char *fname = "creq_invariant";
    int ok = 1;
    int subd_cores[CoreCount];          // subscribed, core_id -> t/f

    // note: "ok" uses bitwise, not logical, ANDs (so be careful!)

    for (int i = 0; i < NELEM(subd_cores); i++)
        subd_cores[i] = 0;

    // request_time: we sometimes use -1 to mean "don't know yet"; we could
    // let that slide here
    ok &= (creq->request_time >= 0);

    ok &= ENUM_OK(CacheAction, creq->action);
    // base_addr: accept all
    ok &= ENUM_OK(CacheAccessType, creq->access_type);
    ok &= ENUM_OK(CoherAccessResult, creq->coher_wb_type);

    switch (creq->service_level) {
    case SERVICED_UNKNOWN:
    case SERVICED_MEM:
    case SERVICED_COHER:
    case SERVICED_NONE:
    case 1:
    case 2:
    case 3:
        break;
    default:
        ok = 0;
    }

    ok &= (creq->create_time >= 0);

    // we'll check after cores[] early here, in order to build up subd_cores[]
    // flags for later

    for (int i = 0; creq->cores[i].core; i++) {
        sim_assert(i < CoreCount);
        int core_id = creq->cores[i].core->core_id;
        ok &= !subd_cores[core_id];     // should only see it once
        subd_cores[core_id] = 1;
        ok &= ENUM_OK(CacheSource, creq->cores[i].src);
    }

    {
        const activelist * restrict dreq_inst = creq->drequestor;
        while (dreq_inst) {
            int core_id = Contexts[dreq_inst->thread]->core->core_id;
            ok &= (dreq_inst->dmiss_cache_entry == creq);
            ok &= subd_cores[core_id];
            dreq_inst = dreq_inst->mergeinst;
        }
    }

    {
        const context * restrict ireq_ctx = creq->irequestor;
        while (ireq_ctx) {
            int core_id = ireq_ctx->core->core_id;
            ok &= (ireq_ctx->imiss_cache_entry == creq);
            ok &= subd_cores[core_id];
            ireq_ctx = ireq_ctx->mergethread;
        }
    }


    if (recurse) {
        // recursive call: follow dependent_coher chain
        const CacheRequest * restrict dep_creq = creq->dependent_coher;
        while (dep_creq) {
            ok &= creq_invariant(dep_creq, recurse);
            dep_creq = dep_creq->dependent_coher;
        }
    }

    // blocked_apps: nothing to do
    // is_dirty_fill: nothing to do
    // coher_data_seen: nothing to do

    if (!ok) {
        err_printf("%s failed at time %s on req: %s\n", fname, fmt_now(),
                   fmt_creq_static(creq));
    }

    return ok;
}


// Generate and simulate a prefetch request from a given core.  In a sense,
// this is a non-idealized version of cachesim_oracle_inject_core(), or
// alternatively, a mix of dodaccess() and doiaccess() without a context or
// meminst associated.
//
// Returns nonzero iff the prefetch request is accepted and can proceed "now".
// This can reject a request if the needed cache resources are busy at the
// sim-time of the call, or if no MSHR is available.

static int
cachesim_prefetch_at_core(CoreResources *core, LongAddr base_addr,
                          int exclusive_access, CacheSource pf_source,
                          CacheMergeResult *merge_stat_ret,
                          CacheRequest **creq_ret)
{
    const char *fname = "cachesim_prefetch_at_core";
    CacheAccessType access_type  = (exclusive_access) ? Cache_ReadExcl :
        Cache_Read;
    const i64 addr_ready_cyc = cyc;
    CacheMergeResult merge_stat = CacheMerge_NoMerge;
    CacheRequest *active_creq = NULL;

    DEBUGPRINTF("cache: %s, C%d base_addr %s excl %d src %s\n",
                fname, core->core_id, fmt_laddr(base_addr), exclusive_access,
                CacheSource_names[pf_source]);

    if ((access_type == Cache_Read) && !GlobalCoherMgr)
        access_type = Cache_ReadExcl;

    // simplified combo of dodaccess+dodaccess_cache+doiaccess...

    CacheArray * restrict l1cache = NULL;
    MshrTable * restrict l1mshr = NULL;
    PFStreamGroup * restrict l1streambuf = NULL;

    // it'd be nice if we could select a specific TLB here, but the I vs. D
    // TLB lookups use two different functions
    switch (pf_source) {
    case CSrc_L1_ICache:
        l1cache = core->icache;
        l1mshr = core->inst_mshr;
        break;
    case CSrc_L1_DCache:
        l1cache = core->dcache;
        l1mshr = core->data_mshr;
        break;
    case CSrc_L1_DStreamBuf:
        l1streambuf = core->d_streambuf;
        break;
    default:
        ENUM_ABORT(CacheSource, pf_source);
    }

    if (l1mshr && !mshr_is_avail(l1mshr, base_addr)) {
        DEBUGPRINTF("cache: %s: out of %s-MSHRs, rejecting\n", fname,
                    CacheSource_names[pf_source]);
        goto mshr_full;
    }
    if (l1mshr) {
        static int hack_mshr_partition_enabled = -1;    // -1: unread, else 0/1
        static int max_prefetch_producers = 0;
        if (SP_F(hack_mshr_partition_enabled < 0)) {
            hack_mshr_partition_enabled =
                simcfg_get_bool("Hacking/L1MSHRPartition/enable");
            sim_assert(hack_mshr_partition_enabled >= 0);
            max_prefetch_producers = 
                simcfg_get_int("Hacking/L1MSHRPartition/"
                               "max_prefetch_producers");
            sim_assert(max_prefetch_producers >= 0);
        }
        int prefetch_count = mshr_count_prefetch_producers(l1mshr);
        sim_assert(prefetch_count <= max_prefetch_producers);// check for slips
        if (hack_mshr_partition_enabled &&
            (prefetch_count >= max_prefetch_producers)) {
            DEBUGPRINTF("cache: %s: %s-MSHR has %d prefetch-producers, "
                        "limit is %d, rejecting\n", fname,
                        CacheSource_names[pf_source], prefetch_count,
                        max_prefetch_producers);
            goto mshr_full;
        }
    }

    if (l1cache &&
        !cache_probebank_avail(l1cache, base_addr, addr_ready_cyc, 0)) {
        DEBUGPRINTF("cache: %s: bank not ready, rejecting\n", fname);
        goto cache_busy;
    }

    int l1_cache_hit = 0;
    i64 ready_time = addr_ready_cyc;

    if (l1cache) {
        // don't access cache on streambuf prefetch request
        CacheLOutcome cache_stat = cache_lookup(l1cache, base_addr,
                                                access_type, NULL);
        ready_time = 
            cache_update_bank(l1cache, base_addr, addr_ready_cyc,
                              cache_access_to_bankop(access_type));
        l1_cache_hit = (cache_stat == Cache_Hit);
    }

    if (l1_cache_hit) {
        // data present at L1; nothing to do
    } else if ((merge_stat =
                ordered_l1_find_and_merge(base_addr, addr_ready_cyc,
                                          access_type, pf_source, core, NULL,
                                          -1, &active_creq))
               != CacheMerge_NoMerge) {
        // responsibility merged into some existing request; MSHR alloc'd if
        // needed
        if (l1streambuf) {
            // streambuf loses or ties
            pfsg_pf_merged(l1streambuf, base_addr, 0);
        }
    } else {
        // create a new request, add it to cache simulator
        int tlb_penalty;
        switch (pf_source) {
        case CSrc_L1_ICache:
            tlb_penalty = itlb_lookup(core, NULL, base_addr);
            break;
        case CSrc_L1_DCache:
        case CSrc_L1_DStreamBuf:
            tlb_penalty = dtlb_lookup(core, NULL, base_addr);
            break;
        default:
            tlb_penalty = 0;
        }

        i64 next_req_time =
            MAX_SCALAR(ready_time + core->params.dcache.timing.miss_penalty,
                       addr_ready_cyc + tlb_penalty);
        active_creq =
            get_c_request_holder(next_req_time, base_addr, access_type,
                                 l1_route_down(), pf_source, core);
        place_in_cache_queue(active_creq);
        if (l1mshr) {
            MshrAllocOutcome mshr_stat = 
                mshr_alloc_prefetch(l1mshr, base_addr);
            if (mshr_stat != MSHR_AllocNew) {
                // shouldn't be MSHR_ReuseOld -- that should've been merged
                // shouldn't be MSHR_Full -- that was checked above
                ENUM_ABORT(MshrAllocOutcome, mshr_stat);
            }
        }
    }


    if (merge_stat_ret)
        *merge_stat_ret = merge_stat;
    if (creq_ret)
        *creq_ret = active_creq;
    return 1;

 mshr_full:
 cache_busy:
    return 0;
}


int
cachesim_prefetch_for_streambuf(struct CoreResources *core, LongAddr base_addr,
                                int exclusive_access, const void *inflight_id,
                                int stream_id, int entry_id)
{
    CacheMergeResult merge_stat = CacheMerge_NoMerge;
    CacheRequest *creq = NULL;
    int success = cachesim_prefetch_at_core(core, base_addr, exclusive_access,
                                            CSrc_L1_DStreamBuf, &merge_stat,
                                            &creq);
    return success;
}


int
cachesim_prefetch_for_nextblock(struct CoreResources *core, LongAddr base_addr,
                                int exclusive_access, CacheSource pf_source)
{
    CacheMergeResult merge_stat = CacheMerge_NoMerge;
    CacheRequest *creq = NULL;
    int success = cachesim_prefetch_at_core(core, base_addr, exclusive_access,
                                            pf_source, &merge_stat, &creq);
    return success;
}
