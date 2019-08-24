#include "stdio.h"
#include "stdlib.h"
#include "prefix_if.h"
#include "../cps_interpreter.h"
#include "prefix_func.h"
#include "prefix_lambda.h"
#include "prefix_blk.h"
extern KN UnBoxAccessorValue(KN konValue);

KonTrampoline* ApplyProcArguments(KonState* kstate, KonProcedure* proc, KN argList, KonEnv* env, KonContinuation* cont)
{
    KonTrampoline* bounce;
    // TODO assert subj is a procedure
    if (proc->Type == KON_NATIVE_FUNC) {
        KonNativeFuncRef funcRef = proc->NativeFuncRef;
        KN applyResult = (*funcRef)(kstate, argList);
        bounce = AllocBounceWithType(kstate, KON_TRAMPOLINE_RUN);
        bounce->Run.Value = applyResult;
        bounce->Cont = cont;
    }
    else if (proc->Type == KON_NATIVE_OBJ_METHOD) {
        // treat as plain procedure when apply arg list
        // the first item in arg list is the object
        KonNativeFuncRef funcRef = proc->NativeFuncRef;
        KN applyResult = (*funcRef)(kstate, argList);
        bounce = AllocBounceWithType(kstate, KON_TRAMPOLINE_RUN);
        bounce->Run.Value = applyResult;
        bounce->Cont = cont;
    }
    else if (proc->Type == KON_COMPOSITE_LAMBDA) {
        bounce = KON_ApplyCompositeLambda(kstate, proc, argList, env, cont);
    }
    else if (proc->Type == KON_COMPOSITE_FUNC) {
        bounce = KON_ApplyCompositeFunc(kstate, proc, argList, env, cont);
    }
    else if (proc->Type == KON_COMPOSITE_BLK) {
        bounce = KON_ApplyCompositeBlk(kstate, proc, KON_NIL, env, cont);
    }
    else if (proc->Type == KON_COMPOSITE_OBJ_METHOD) {
        // treat as plain procedure when apply arg list
        // the first item in arg list is the object
        bounce = KON_ApplyCompositeLambda(kstate, proc, argList, env, cont);
    }
    return bounce;
}

KonTrampoline* AfterApplyArgsExprEvaled(KonState* kstate, KN evaledValue, KonContinuation* contBeingInvoked)
{
    KonEnv* env = contBeingInvoked->Env;
    KonContinuation* cont = contBeingInvoked->Cont;
    KxHashTable* memo = contBeingInvoked->Native.MemoTable;
    KN applySym = KxHashTable_AtKey(memo, "ApplySym");

    KON_DEBUG("applySym %s", KON_StringToCstr(KON_ToFormatString(kstate, applySym, true, 0, "  ")));

    KN proc = KON_UNDEF;
    // lookup procedure in env
    if (KON_IS_IDENTIFIER(applySym) || KON_IS_REFERENCE(applySym)) {
        proc = KON_EnvLookup(kstate, env, KON_SymbolToCstr(applySym));
    }
    else if (KON_IS_ACCESSOR(applySym)) {
        proc = UnBoxAccessorValue(applySym);
    }

    // evaledValue is a data
    if (KON_IS_QUOTE_LIST(evaledValue)) {
        evaledValue = KON_UNBOX_QUOTE(evaledValue);
    }

    KON_DEBUG("proc args %s", KON_StringToCstr(KON_ToFormatString(kstate, evaledValue, true, 0, "  ")));
    

    KonTrampoline* bounce = ApplyProcArguments(kstate, proc, evaledValue, env, cont);
    return bounce;
}

KonTrampoline* AfterApplySymExprEvaled(KonState* kstate, KN evaledValue, KonContinuation* contBeingInvoked)
{
    KonEnv* env = contBeingInvoked->Env;
    KxHashTable* memo =  KxHashTable_ShadowClone(contBeingInvoked->Native.MemoTable);
    KN applyArgsExpr = KxHashTable_AtKey(memo, "ApplyArgsExpr");

    KonTrampoline* bounce;
    KonContinuation* k = AllocContinuationWithType(kstate, KON_CONT_NATIVE_CALLBACK);
    k->Cont = contBeingInvoked->Cont;
    k->Env = env;

    KxHashTable_PutKv(memo, "ApplySym", evaledValue);

    k->Native.MemoTable = memo;
    k->Native.Callback = AfterApplyArgsExprEvaled;

    bounce = KON_EvalExpression(kstate, applyArgsExpr, env, k);

    return bounce;
}


// first arg should be a expr that returns a symbol
// second arg should be a expr that returns a quoted list
// 3rd arg is optional. is the apply env
KonTrampoline* KON_EvalPrefixApply(KonState* kstate, KN expression, KonEnv* env, KonContinuation* cont)
{
    KON_DEBUG("meet prefix marcro apply");
    KON_DEBUG("rest words %s", KON_StringToCstr(KON_ToFormatString(kstate, expression, true, 0, "  ")));
    
    KN applySymExpr = KON_DCNR(expression);
    KN applyArgsExpr = KON_DCNNR(expression);

    KON_DEBUG("applySymExpr %s", KON_StringToCstr(KON_ToFormatString(kstate, applySymExpr, true, 0, "  ")));
    KON_DEBUG("applyArgsExpr %s", KON_StringToCstr(KON_ToFormatString(kstate, applyArgsExpr, true, 0, "  ")));


    KonContinuation* k = AllocContinuationWithType(kstate, KON_CONT_NATIVE_CALLBACK);
    k->Cont = cont;
    k->Env = env;

    KxHashTable* memo = KxHashTable_Init(4);
    KxHashTable_PutKv(memo, "ApplyArgsExpr", applyArgsExpr);

    k->Native.MemoTable = memo;
    k->Native.Callback = AfterApplySymExprEvaled;
    KON_DEBUG("before KON_EvalExpression");
    KonTrampoline* bounce = KON_EvalExpression(kstate, applySymExpr, env, k);

    return bounce;
}
