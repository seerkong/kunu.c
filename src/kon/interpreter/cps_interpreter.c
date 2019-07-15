#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <tbox/tbox.h>
#include "cps_interpreter.h"


KonTrampoline* ApplySubjVerbAndObjects(KonState* kstate, KN subj, KN argList, KonEnv* env, KonContinuation* cont)
{
    KN subjFmtStr = KON_ToFormatString(kstate, subj, false, 0, " ");
    KN objectsFmtStr = KON_ToFormatString(kstate, argList, false, 0, " ");

    kon_debug("subj: %s, objects: %s", KON_StringToCstr(subjFmtStr), KON_StringToCstr(objectsFmtStr));
    KN firstObj = kon_car(argList);
    // KON_MakeString(kstate, "ihojh");
    KN applyResult = KON_NULL;
    if (kon_is_syntax_marker(firstObj)) {
        KonProcedure* subjProc = (KonProcedure*)subj;
        if (CAST_Kon(SyntaxMarker, firstObj)->Type == KON_SYNTAX_MARKER_APPLY) {
            // TODO assert subj is a procedure
            if (subjProc->Type == KON_NATIVE_FUNC) {
                KonNativeFuncRef funcRef = subjProc->NativeFuncRef;
                applyResult = (*funcRef)(kstate, kon_cdr(argList));
            }
        }
    }
    
    KonTrampoline* bounce = AllocBounceWithType(KON_TRAMPOLINE_RUN);
    bounce->Run.Value = applyResult;
    bounce->Run.Cont = cont;
    return bounce;
}


KN TbVectorToKonList(KonState* kstate, tb_vector_ref_t clauseWords)
{
    KN clause = KON_NIL;
    
    tb_size_t clauseHead = tb_iterator_head(clauseWords);
    tb_size_t clauseItor = tb_iterator_tail(clauseWords);
    do {
        // the previous item
        clauseItor = tb_iterator_prev(clauseWords, clauseItor);
        
        KN item = tb_iterator_item(clauseWords, clauseItor);
        if (item == NULL) {
            break;
        }
        clause = kon_cons(kstate, item, clause);
        
    } while (clauseItor != clauseHead);
    return clause;
}

////
// @param sentenceRestWords sentence words except subject,
//        if a sentence is {"zhangsan " name | upcase}, then this
//        param should be {name | upcase}
KN SplitClauses(KonState* kstate, KN sentenceRestWords)
{
    // TODO parse symbol type verb
    
    
    tb_vector_ref_t clauseListVec = tb_vector_init(TB_VECTOR_GROW_SIZE, tb_element_long());
    
    tb_vector_ref_t clauseVec = tb_vector_init(TB_VECTOR_GROW_SIZE, tb_element_long());

    KonListNode* iter = sentenceRestWords;
    
    int state = 1; // 1 need verb, 2 need objects
    do {
        KN item = kon_car(iter);
        
        if (state == 1) {
            if (kon_is_syntax_marker(item)
                && CAST_Kon(SyntaxMarker, item)->Type != KON_SYNTAX_MARKER_CLAUSE_END
            ) {
                tb_vector_insert_tail(clauseVec, item);
                state = 2;
            }
            else if (kon_is_vector(item)
                || KON_IsList(item)
                || kon_is_cell(item)
                || kon_is_symbol(item)
                || kon_is_quote(item)
                || kon_is_quasiquote(item)
                || kon_is_unquote(item)
            ) {
                tb_vector_insert_tail(clauseVec, item);
            }
            else {
                // TODO throw exception
            }
        }
        else {
            // meet ;
            if (kon_is_syntax_marker(item)
                && CAST_Kon(SyntaxMarker, item)->Type == KON_SYNTAX_MARKER_CLAUSE_END
            ) {
                tb_vector_insert_tail(clauseListVec, clauseVec);
                // reset state
                clauseVec = tb_vector_init(TB_VECTOR_GROW_SIZE, tb_element_ptr(kon_vector_item_ptr_free, "ClauseVec"));
                state = 1;
            }
            else {
                tb_vector_insert_tail(clauseVec, item);
            }
        }
        
        
        iter = kon_cdr(iter);
    } while (iter != KON_NIL);
    
    tb_vector_insert_tail(clauseListVec, clauseVec);

    tb_size_t clauseListHead = tb_iterator_head(clauseListVec);
    tb_size_t clauseListItor = tb_iterator_tail(clauseListVec);
    
    KN result = KON_NIL;
    do {
        // the previous item
        clauseListItor = tb_iterator_prev(clauseListVec, clauseListItor);
        
        tb_vector_ref_t clauseWords = (tb_vector_ref_t)tb_iterator_item(clauseListVec, clauseListItor);
        if (clauseWords == NULL) {
            break;
        }
        KN clause = TbVectorToKonList(kstate, clauseWords);
        result = kon_cons(kstate, clause, result);
        
    } while (clauseListItor != clauseListHead);
    return result;
}

KonContinuation* AllocContinuationWithType(KonContinuationType type)
{
    KonContinuation* cont = (KonContinuation*)malloc(sizeof(KonContinuation));
    assert(cont);
    cont->Type = type;
    return cont;
}

KonTrampoline* AllocBounceWithType(KonBounceType type)
{
    KonTrampoline* bounce = (KonTrampoline*)malloc(sizeof(KonTrampoline));
    assert(bounce);
    bounce->Type = type;
    return bounce;
}

// pop the top continuation
// the bounce continuation should be cont->Cont
KonTrampoline* KON_RunContinuation(KonState* kstate, KonContinuation* cont, KN val)
{
    // all sentences finished, return last value
    if (kon_continuation_type(cont) == KON_CONT_RETURN) {
        KonTrampoline* bounce = AllocBounceWithType(KON_TRAMPOLINE_LAND);
        bounce->Land.Value = val;
        return bounce;
    }
    else if (kon_continuation_type(cont) == KON_CONT_EVAL_SENTENCE_LIST) {
        KN lastSentenceVal = val;
        KN env = cont->Env;
        KN restSentences = cont->EvalSentenceList.RestSentenceList;
        if (restSentences == KON_NIL) {
            // block sentences all finished
            KonTrampoline* bounce = AllocBounceWithType(KON_TRAMPOLINE_RUN);
            bounce->Run.Cont = cont->Cont;
            bounce->Run.Value = lastSentenceVal;
            return bounce;
        }
        else {
            return KON_EvalSentences(kstate, restSentences, env, cont->Cont);
//            KonTrampoline* bounce = AllocBounceWithType(KON_TRAMPOLINE_BLOCK);
//
//            KonContinuation* k = AllocContinuationWithType(KON_CONT_EVAL_SENTENCE_LIST);
//            k->Cont = cont->Cont;
//            k->Env = cont->Env;
//            k->EvalSentenceList.RestSentenceList = kon_cdr(restSentences);
//
//            bounce->Bounce.Value = kon_car(restSentences);
//            bounce->Bounce.Cont = k;
//            bounce->Bounce.Env = cont->Env;
//            return bounce;
        }
    }
    // else if (kon_continuation_type(cont) == KON_CONT_EVAL_SENTENCE) {
    //     // eval subj first if have a sentence
    //     KN words = val;
    //     KonTrampoline* bounce = AllocBounceWithType(KON_TRAMPOLINE_SUBJ);
            
    //     KonContinuation* k = AllocContinuationWithType(KON_CONT_EVAL_SUBJ);
    //     k->Cont = cont;
    //     k->Env = cont->Env;
    //     k->EvalSubj.RestWordList = kon_cdr(words);
        
    //     bounce->Bounce.Value = kon_car(words);
    //     bounce->Bounce.Cont = k;
    //     bounce->Bounce.Env = cont->Env;
    //     return bounce;
    // }
    else if (kon_continuation_type(cont) == KON_CONT_EVAL_SUBJ) {
        // subj evaled, now should eval clauses
        KN subj = val;
        KN restWords = cont->EvalSubj.RestWordList;

        if (restWords == KON_NIL) {
            // no other words besids subj, is a sentence like {"abc"}
            // finish this sentence. use subj as return val
            KonTrampoline* bounce = AllocBounceWithType(KON_TRAMPOLINE_RUN);
            bounce->Run.Cont = cont->Cont;
            bounce->Run.Value = subj;
            return bounce;
        }
        else {
            // split sub clauses
            // TODO split % xxx; .xxx a a ; | xx aaa; sss
            // as {{% xxx} {. xxx a a} {| xx aaa} {sss}}
            KN clauses = SplitClauses(kstate, restWords);
            
            if (clauses == KON_NIL) {
                // no clauses like {{a}}
                KonTrampoline* bounce = AllocBounceWithType(KON_TRAMPOLINE_RUN);
                bounce->Run.Cont = cont->Cont;
                bounce->Run.Value = subj;
                return bounce;
            }

            // eval the first clause
            KonContinuation* k = AllocContinuationWithType(KON_CONT_EVAL_CLAUSE_LIST);
            k->Cont = cont->Cont;
            k->Env = cont->Env;
            k->EvalClauseList.RestClauseList = kon_cdr(clauses);
            
            KonTrampoline* bounce = AllocBounceWithType(KON_TRAMPOLINE_CLAUSE_LIST);
            bounce->SubjBounce.Subj = subj;
            bounce->SubjBounce.Cont = k;
            bounce->SubjBounce.Env = cont->Env;
            bounce->SubjBounce.Value = kon_car(clauses);
            return bounce;
        }
        
    }
    else if (kon_continuation_type(cont) == KON_CONT_EVAL_CLAUSE_LIST) {
        // last clause eval finshed, eval next clause
        // last clause eval result is the subj of the next clause
        KN subj = val;
        KN restClauseList = cont->EvalClauseList.RestClauseList;
        if (restClauseList == KON_NIL) {
            // no other clauses, is a sentence like {writeln % "abc"}
            // finish this sentence. use last clause eval result as return val
            KonTrampoline* bounce = AllocBounceWithType(KON_TRAMPOLINE_RUN);
            bounce->Run.Cont = cont->Cont;
            bounce->Run.Value = subj;
            return bounce;
        }
        else {
            // eval the next clause
            KonContinuation* k = AllocContinuationWithType(KON_CONT_EVAL_CLAUSE_LIST);
            k->Cont = cont->Cont;
            k->Env = cont->Env;
            k->EvalClauseList.RestClauseList = kon_cdr(restClauseList);
            
            KonTrampoline* bounce = AllocBounceWithType(KON_TRAMPOLINE_CLAUSE_LIST);
            bounce->SubjBounce.Subj = subj;
            bounce->SubjBounce.Cont = k;
            bounce->SubjBounce.Env = cont->Env;
            bounce->SubjBounce.Value = kon_car(restClauseList);
            return bounce;
        }
    }
    


    else if (kon_continuation_type(cont) == KON_CONT_EVAL_CLAUSE_ARGS) {
        KN lastArgEvaled = val;
        KN subj = cont->EvalClauseArgs.Subj;
        KN restArgList = cont->EvalClauseArgs.RestArgList;
        KN evaledArgList = cont->EvalClauseArgs.EvaledArgList;
        // NOTE! the evaluated arg list here is reverted saved
        // should reverted back when apply the arguments
        evaledArgList = kon_cons(kstate, lastArgEvaled, evaledArgList);

        if (restArgList == KON_NIL) {
            // this clause args all eval finished
            KN argList = Kon_ListRevert(kstate, evaledArgList);
            return ApplySubjVerbAndObjects(kstate, subj, argList, cont->Env, cont->Cont);
        }
        else {
            // eval the next clause
            KonContinuation* k = AllocContinuationWithType(KON_CONT_EVAL_CLAUSE_ARGS);
            k->Cont = cont->Cont;
            k->Env = cont->Env;
            k->EvalClauseArgs.Subj = subj;
            k->EvalClauseArgs.RestArgList = kon_cdr(restArgList);
            k->EvalClauseArgs.EvaledArgList = evaledArgList;
            
            KonTrampoline* bounce = AllocBounceWithType(KON_TRAMPOLINE_ARG_LIST);
//            bounce->SubjBounce.Subj = subj;
            bounce->Bounce.Cont = k;
            bounce->Bounce.Env = cont->Env;
            bounce->Bounce.Value = kon_car(restArgList);
            return bounce;
        }
    }

    else {
        // TODO throw error
    }
}

KonTrampoline* KON_EvalSentences(KonState* kstate, KN sentences, KN env, KonContinuation* cont)
{
    if (sentences == KON_NIL) {
        // TODO throw error
        // should not go here
        return KON_NULL;
    }
    else {
        KonTrampoline* bounce = AllocBounceWithType(KON_TRAMPOLINE_BLOCK);
        
        KonContinuation* k = AllocContinuationWithType(KON_CONT_EVAL_SENTENCE_LIST);
        k->Cont = cont;
        k->Env = cont->Env;
        k->EvalSentenceList.RestSentenceList = kon_cdr(sentences);
        
        bounce->Bounce.Value = kon_car(sentences);
        bounce->Bounce.Cont = k;
        bounce->Bounce.Env = cont->Env;
        return bounce;
    }
}

bool IsSelfEvaluated(KN source)
{
    if (KON_IS_FIXNUM(source)
        || KON_IS_FLONUM(source)
        || kon_is_string(source)
        || kon_is_syntax_marker(source)
        || (kon_is_symbol(source) && CAST_Kon(Symbol, source)->Type == KON_SYM_STRING)
        || kon_is_quote(source)
    ) {
        return true;
    }
    else {
        return false;
    }
}

KN KON_ProcessSentences(KonState* kstate, KN sentences, KN env)
{
    // TODO add step count when debug
    KN formated = KON_ToFormatString(&kstate, sentences, true, 0, "  ");
    //  KN formated = KON_ToFormatString(&kstate, root, false, 0, " ");
    printf("%s\n", KON_StringToCstr(formated));
    
    KonContinuation* firstCont = AllocContinuationWithType(KON_CONT_RETURN);
    firstCont->Env = env;
    KonTrampoline* bounce = KON_EvalSentences(kstate, sentences, env, firstCont);

    while (kon_bounce_type(bounce) != KON_TRAMPOLINE_LAND) {
        if (kon_bounce_type(bounce) == KON_TRAMPOLINE_RUN) {
            KonTrampoline* oldBounce = bounce;
            KN value = bounce->Run.Value;
            KonContinuation* cont = bounce->Run.Cont;
            free(oldBounce);
            bounce = KON_RunContinuation(kstate, cont, value);
        }
        else if (kon_bounce_type(bounce) == KON_TRAMPOLINE_BLOCK) {
            KonTrampoline* oldBounce = bounce;
            KonContinuation* cont = bounce->Bounce.Cont;
            KN env = bounce->Bounce.Env;
            KN sentence = bounce->Bounce.Value;

            if (KON_IsList(sentence)) {
                // passed a sentence like {writeln % "abc" "efg"}
                KN words = sentence;
                bounce = AllocBounceWithType(KON_TRAMPOLINE_SUBJ);
                    
                KonContinuation* k = AllocContinuationWithType(KON_CONT_EVAL_SUBJ);
                k->Cont = cont;
                k->Env = cont->Env;
                k->EvalSubj.RestWordList = kon_cdr(words);
                
                bounce->Bounce.Value = kon_car(words);  // get subj word
                bounce->Bounce.Cont = k;
                bounce->Bounce.Env = cont->Env;
            }
            else if (kon_is_symbol(sentence)) {
                // a code block like { {!local a 4} a}
                // TODO asert should be a SYM_IDENTIFIER
                // env lookup this val
                KN val = KON_EnvLookup(kstate, env, KON_SymbolToCstr(sentence));
                assert(val != KON_NULL);
                bounce = KON_RunContinuation(kstate, cont, val);
            }
            else if (IsSelfEvaluated(sentence)) {
                bounce = KON_RunContinuation(kstate, cont, sentence);
            }
        }
        else if (kon_bounce_type(bounce) == KON_TRAMPOLINE_SUBJ) {
            KonTrampoline* oldBounce = bounce;
            KonContinuation* cont = bounce->Bounce.Cont;
            KN env = bounce->Bounce.Env;
            KN subj = bounce->Bounce.Value;
            if (KON_IsList(subj)) {
                // the subj position is a list, should be evaluated
                // like the first sentence in this eg block {{ {zhangsan name} |upcase }}
                KN words = subj;
                bounce = AllocBounceWithType(KON_TRAMPOLINE_SUBJ);
                    
                KonContinuation* k = AllocContinuationWithType(KON_CONT_EVAL_SUBJ);
                k->Cont = cont;
                k->Env = cont->Env;
                k->EvalSubj.RestWordList = kon_cdr(words);
                
                bounce->Bounce.Value = kon_car(words);
                bounce->Bounce.Cont = k;
                bounce->Bounce.Env = cont->Env;
            }
            else if (kon_is_symbol(subj)) {
                // TODO env lookup this val
                // KN val = KON_MakeString(kstate, "writeln");
                KN val = KON_EnvLookup(kstate, env, KON_SymbolToCstr(subj));
                assert(val != KON_NULL);
                bounce = KON_RunContinuation(kstate, cont, val);
            }
            // TODO quasiquote unquote, etc.
            
            else if (IsSelfEvaluated(subj)) {
                bounce = KON_RunContinuation(kstate, cont, subj);
            }
        }
        else if (kon_bounce_type(bounce) == KON_TRAMPOLINE_CLAUSE_LIST) {
            
            KonTrampoline* oldBounce = bounce;
            KonContinuation* cont = bounce->SubjBounce.Cont;
            KN env = bounce->SubjBounce.Env;
            KN clauseArgList = bounce->SubjBounce.Value;
            KN subj = bounce->SubjBounce.Subj;
            KN firstArg = kon_car(clauseArgList);

            if (kon_is_syntax_marker(firstArg)) {
                // % . |
                // this kind bouce value is a clause word list like {% "a" "b"}

                KonContinuation* k = AllocContinuationWithType(KON_CONT_EVAL_CLAUSE_ARGS);
                k->Cont = cont;
                k->Env = cont->Env;
                k->EvalClauseArgs.Subj = subj;
                k->EvalClauseArgs.RestArgList = kon_cdr(clauseArgList);
                k->EvalClauseArgs.EvaledArgList = KON_NIL;

                bounce = AllocBounceWithType(KON_TRAMPOLINE_ARG_LIST);
                bounce->Bounce.Cont = k;
                bounce->Bounce.Env = cont->Env;
                bounce->Bounce.Value = kon_car(clauseArgList); // the first arg is % or . or |

            }
            else if (kon_is_symbol(clauseArgList)) {
                // a clause like {length} in sentence {{"abc" length}}
                // should dispatch this msg to the object's visitor protocol

            }
            
            
        }

        else if (kon_bounce_type(bounce) == KON_TRAMPOLINE_ARG_LIST) {
            // eval each argment
            KonTrampoline* oldBounce = bounce;
            KonContinuation* cont = bounce->Bounce.Cont;
            KN env = bounce->Bounce.Env;
            KN arg = bounce->Bounce.Value;
            if (KON_IsList(arg)) {
                // the subj position is a list, should be evaluated
                // like the first sentence in this eg block {{ {zhangsan name} |upcase }}
                KN words = arg;
                bounce = AllocBounceWithType(KON_TRAMPOLINE_SUBJ);
                    
                KonContinuation* k = AllocContinuationWithType(KON_CONT_EVAL_SUBJ);
                k->Cont = cont;
                k->Env = cont->Env;
                k->EvalSubj.RestWordList = kon_cdr(words);
                
                bounce->Bounce.Value = kon_car(words);
                bounce->Bounce.Cont = k;
                bounce->Bounce.Env = cont->Env;
            }
            else if (kon_is_vector(arg)) {
                // TODO !!! verify cell inner content
                // whether have Quasiquote, Expand, Unquote, KON_SYM_VAR node
                bounce = KON_RunContinuation(kstate, cont, arg);
            }
            else if (kon_is_table(arg)) {
                // TODO !!! verify cell inner content key, value
                // whether have Quasiquote, Expand, Unquote, KON_SYM_VAR node
                bounce = KON_RunContinuation(kstate, cont, arg);
            }
            else if (kon_is_cell(arg)) {
                // TODO !!! verify cell inner content(tag, list, vector, table )
                // whether have Quasiquote, Expand, Unquote, KON_SYM_VAR node
                bounce = KON_RunContinuation(kstate, cont, arg);
            }
            else if (kon_is_quote(arg)) {
                // TODO
            }
            else if (kon_is_quasiquote(arg)) {
                // TODO
            }
            else if (kon_is_expand(arg)) {
                // TODO
            }
            else if (kon_is_unquote(arg)) {
                // TODO
            }
            else if (kon_is_symbol(arg)) {
                // env lookup this val
                KN val = KON_EnvLookup(kstate, env, KON_SymbolToCstr(arg));;
                assert(val != KON_NULL);
                bounce = KON_RunContinuation(kstate, cont, val);
            }
            else if (IsSelfEvaluated(arg)) {
                bounce = KON_RunContinuation(kstate, cont, arg);
            }
        }
    }
    return bounce->Land.Value;
}
