#include "stdio.h"
#include "stdlib.h"
#include "prefix_lambda.h"
#include "../cps_interpreter.h"

KonTrampoline* KON_EvalPrefixCallcc(KonState* kstate, KN expression, KonEnv* env, KonContinuation* cont)
{
    KON_DEBUG("meet prefix marcro callcc");
    KON_DEBUG("rest words %s", KON_StringToCstr(KON_ToFormatString(kstate, expression, true, 0, "  ")));

    // set current continuation as argument
    KN argList = KON_CONS(kstate, (KN)cont, KON_NIL);
    KN proc = KON_DCR(expression);
    if (KON_IS_REFERENCE(proc)) {
        proc = KON_EnvLookup(kstate, env, KON_SymbolToCstr(proc));
    }
    // TODO assert proc is a procedure
    if (!KON_IS_PROCEDURE(proc)) {
        // throw exception
    }
    KonTrampoline* bounce = KON_ApplyCompositeLambda(kstate, proc, argList, env, cont);

    return bounce;
}
