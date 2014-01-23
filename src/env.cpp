#include "env.hpp"
#include "conn.hpp"

using namespace Eos;

void Environment::Init(Handle<Object> exports) {
    auto ft = FunctionTemplate::New(New);
    constructor_ = Persistent<FunctionTemplate>::New(ft);

    ft->SetClassName(String::NewSymbol("Environment"));
    ft->InstanceTemplate()->SetInternalFieldCount(1);

    // The exported constructor can only be called on values made by the internal constructor.
    auto sig0 = Signature::New(ft);

    EOS_SET_METHOD(ft, "newConnection", Environment, NewConnection, sig0);
    EOS_SET_METHOD(ft, "free", Environment, Free, sig0);

    exports->Set(String::NewSymbol("Environment"), ft->GetFunction(), ReadOnly);
}

Environment::Environment(SQLHENV hEnv) : hEnv_(hEnv) {
    EOS_DEBUG_METHOD();
}

Handle<Value> Environment::New(const Arguments& args) {
    EOS_DEBUG_METHOD();

    HandleScope scope;

    if (!args.IsConstructCall())
        return scope.Close(constructor_->GetFunction()->NewInstance());

    SQLHENV hEnv;
    auto ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
    if (!SQL_SUCCEEDED(ret))
        return ThrowException(Exception::Error(String::New("Unable to allocate environment handle")));
    
    ret = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3_80, SQL_IS_UINTEGER);
    if (!SQL_SUCCEEDED(ret)) {
        Local<Value> exception = Eos::GetLastError(SQL_HANDLE_ENV, hEnv);
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        return ThrowException(exception);
    }

    Environment* env = new Environment(hEnv);
    env->Wrap(args.Holder());

    return scope.Close(args.Holder());
}

namespace {
    Handle<Value> ThrowClosed() {
        return ThrowException(Exception::Error(String::New("The environment has been closed.")));
    }
}

Handle<Value> Environment::NewConnection(const Arguments& args) {
    EOS_DEBUG_METHOD();

    if (!hEnv_)
        return ThrowClosed();

    Handle<Value> argv[1] = { handle_ };
    return Connection::Constructor()->GetFunction()->NewInstance(1, argv);
}

// Frees the environment handle. This will fail with HY010 if there are still 
// connection handles allocated to this environment. Closing an environment
// twice is allowed.
Handle<Value> Environment::Free(const Arguments& args) {
    EOS_DEBUG_METHOD();

    auto ret = hEnv_.Free();
    if (!SQL_SUCCEEDED(ret))
        return ThrowException(GetLastError());

    return Undefined();
}

Environment::~Environment() {
    EOS_DEBUG_METHOD();
}

Persistent<FunctionTemplate> Environment::constructor_;

namespace { ClassInitializer<Environment> x; }