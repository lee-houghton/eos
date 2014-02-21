#include "stmt.hpp"

using namespace Eos;

namespace Eos {
    struct FetchOperation : Operation<Statement, FetchOperation> {
        FetchOperation::FetchOperation() {
            EOS_DEBUG_METHOD();
        }

        static Handle<Value> New(Statement* owner, const Arguments& args) {
            EOS_DEBUG_METHOD();

            if (args.Length() < 1)
                return ThrowError("Too few arguments");

            (new FetchOperation())->Wrap(args.Holder());
            return args.Holder();
        }

        void CallbackOverride(SQLRETURN ret) {
            EOS_DEBUG_METHOD();

            if (!SQL_SUCCEEDED(ret) && ret != SQL_NO_DATA)
                return CallbackErrorOverride(ret);

            EOS_DEBUG(L"Final Result: %hi\n", ret);

            Handle<Value> argv[] = { 
                Undefined(),
                ret == SQL_NO_DATA ? False() : True()
            };
            
            GetCallback()->Call(Context::GetCurrent()->Global(), 2, argv);
        }

        static const char* Name() { return "FetchOperation"; }

    protected:
        SQLRETURN CallOverride() {
            EOS_DEBUG_METHOD();

            return SQLFetch(
                Owner()->GetHandle());
        }
    };
}

Handle<Value> Statement::Fetch(const Arguments& args) {
    EOS_DEBUG_METHOD();

    if (args.Length() < 1)
        return ThrowError("Statement::Fetch() requires a callback");

    Handle<Value> argv[] = { handle_, args[0] };
    return Begin<FetchOperation>(argv);
}

Persistent<FunctionTemplate> Operation<Statement, FetchOperation>::constructor_;
namespace { ClassInitializer<FetchOperation> ci; }