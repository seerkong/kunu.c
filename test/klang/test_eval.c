#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "../greatest.h"
#include "../../src/kon/kon.h"
#include <tbox/tbox.h>

KonState* kstate;
SUITE(suite);

TEST Native_And(void) {
    char* path = "~/lang/konscript/kon-c/samples/knative/and.kl";
    KON_EvalFile(kstate, path);
    PASS();
}

TEST Native_Blk(void) {
    char* path = "~/lang/konscript/kon-c/samples/knative/blk.kl";
    KON_EvalFile(kstate, path);
    PASS();
}

TEST Native_Cond(void) {
    char* path = "~/lang/konscript/kon-c/samples/knative/cond.kl";
    KON_EvalFile(kstate, path);
    PASS();
}

TEST Native_Do(void) {
    char* path = "~/lang/konscript/kon-c/samples/knative/do.kl";
    KON_EvalFile(kstate, path);
    PASS();
}

TEST Native_Func(void) {
    char* path = "~/lang/konscript/kon-c/samples/knative/func.kl";
    KON_EvalFile(kstate, path);
    PASS();
}

TEST Native_If(void) {
    char* path = "~/lang/konscript/kon-c/samples/knative/if.kl";
    KON_EvalFile(kstate, path);
    PASS();
}

TEST Native_Lambda(void) {
    char* path = "~/lang/konscript/kon-c/samples/knative/lambda.kl";
    KON_EvalFile(kstate, path);
    PASS();
}

TEST Native_Letset(void) {
    char* path = "~/lang/konscript/kon-c/samples/knative/letset.kl";
    KON_EvalFile(kstate, path);
    PASS();
}

TEST Native_Math(void) {
    char* path = "~/lang/konscript/kon-c/samples/knative/math.kl";
    KN result = KON_EvalFile(kstate, path);
    ASSERT_EQ(KON_FALSE, result);
    PASS();
}

TEST Native_Or(void) {
    char* path = "~/lang/konscript/kon-c/samples/knative/or.kl";
    KON_EvalFile(kstate, path);
    PASS();
}

TEST Native_Writeln(void) {
    char* path = "~/lang/konscript/kon-c/samples/knative/writeln.kl";
    KON_EvalFile(kstate, path);
    PASS();
}

TEST Native_CallCC(void) {
    char* path = "~/lang/konscript/kon-c/samples/knative/callcc.kl";
    KON_EvalFile(kstate, path);
    PASS();
}

TEST Native_For(void) {
    char* path = "~/lang/konscript/kon-c/samples/knative/for.kl";
    KON_EvalFile(kstate, path);
    PASS();
}

TEST Native_ForLoopList(void) {
    char* path = "~/lang/konscript/kon-c/samples/knative/for-loop-list.kl";
    KON_EvalFile(kstate, path);
    PASS();
}

TEST Native_Pipe(void) {
    char* path = "~/lang/konscript/kon-c/samples/knative/pipe.kl";
    KON_EvalFile(kstate, path);
    PASS();
}

TEST Native_AttrSlot(void) {
    char* path = "~/lang/konscript/kon-c/samples/knative/attr-slot.kl";
    KON_EvalFile(kstate, path);
    PASS();
}

TEST Native_MkDispatcher(void) {
    char* path = "~/lang/konscript/kon-c/samples/knative/mk-dispatcher.kl";
    KON_EvalFile(kstate, path);
    PASS();
}

TEST Native_Shell(void) {
    char* path = "~/lang/konscript/kon-c/samples/knative/sh.kl";
    KON_EvalFile(kstate, path);
    PASS();
}

TEST Native_ReturnFromRoot(void) {
    char* path = "~/lang/konscript/kon-c/samples/knative/return-from-root.kl";
    KN result = KON_EvalFile(kstate, path);
    ASSERT_EQ_FMT(KON_MAKE_FIXNUM(42), result, "%d");
    PASS();
}

TEST Native_ReturnFromLambda(void) {
    char* path = "~/lang/konscript/kon-c/samples/knative/return-from-lambda.kl";
    KN result = KON_EvalFile(kstate, path);
    ASSERT_EQ_FMT(KON_MAKE_FIXNUM(48), result, "%d");
    PASS();
}

TEST Native_ReturnFromFunc(void) {
    char* path = "~/lang/konscript/kon-c/samples/knative/return-from-func.kl";
    KN result = KON_EvalFile(kstate, path);
    ASSERT_EQ_FMT(KON_MAKE_FIXNUM(48), result, "%d");
    PASS();
}

TEST Native_ReturnFromBlk(void) {
    char* path = "~/lang/konscript/kon-c/samples/knative/return-from-blk.kl";
    KN result = KON_EvalFile(kstate, path);
    ASSERT_EQ_FMT(KON_MAKE_FIXNUM(42), result, "%d");
    PASS();
}

TEST Native_Quote(void) {
    char* path = "~/lang/konscript/kon-c/samples/knative/symbol.kl";
    KN result = KON_EvalFile(kstate, path);
    // ASSERT_EQ_FMT(KON_MAKE_FIXNUM(42), result, "%d");
    PASS();
}

TEST Native_Eval(void) {
    char* path = "~/lang/konscript/kon-c/samples/knative/eval.kl";
    KN result = KON_EvalFile(kstate, path);
    ASSERT_EQ_FMT(KON_MAKE_FIXNUM(4), result, "%d");
    PASS();
}

TEST Native_Apply(void) {
    char* path = "~/lang/konscript/kon-c/samples/knative/apply.kl";
    KN result = KON_EvalFile(kstate, path);
    ASSERT_EQ_FMT(KON_MAKE_FIXNUM(30), result, "%d");
    PASS();
}

SUITE(suite) {
    RUN_TEST(Native_Math);
    
    RUN_TEST(Native_Writeln);

    RUN_TEST(Native_And);
    RUN_TEST(Native_Or);

    RUN_TEST(Native_Letset);

    RUN_TEST(Native_If);
    RUN_TEST(Native_Do);
    RUN_TEST(Native_Cond);
    
    RUN_TEST(Native_Blk);
    RUN_TEST(Native_Func);
    RUN_TEST(Native_Lambda);

    RUN_TEST(Native_CallCC);

    RUN_TEST(Native_For);
    RUN_TEST(Native_ForLoopList);
    RUN_TEST(Native_Pipe);

    RUN_TEST(Native_ReturnFromRoot);
    RUN_TEST(Native_ReturnFromLambda);
    RUN_TEST(Native_ReturnFromFunc);
    RUN_TEST(Native_ReturnFromBlk);
    
    
    RUN_TEST(Native_Quote);

    RUN_TEST(Native_Eval);

    RUN_TEST(Native_Apply);
    
    // RUN_TEST(Native_AttrSlot);
    // RUN_TEST(Native_MkDispatcher);
    // RUN_TEST(Native_Shell);
}

GREATEST_MAIN_DEFS();

int main(int argc, char const* argv[])
{
    GREATEST_MAIN_BEGIN();
    greatest_set_verbosity(1);
    if (!tb_init(tb_null, tb_null)) {
        return NULL;
    }
    kstate = KON_Init();
    ENABLE_DEBUG = 0;
    RUN_SUITE(suite);

    KON_Finish(kstate);
    tb_exit();
    GREATEST_MAIN_END();
}