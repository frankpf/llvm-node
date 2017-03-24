//
// Created by Micha Reiser on 24.03.17.
//

#include "constant-int.h"
#include "llvm-context.h"

NAN_MODULE_INIT(ConstantIntWrapper::Init) {
    auto constantInt = Nan::GetFunction(Nan::New(constantIntTemplate())).ToLocalChecked();
    Nan::Set(target, Nan::New("ConstantInt").ToLocalChecked(), constantInt);
}

NAN_METHOD(ConstantIntWrapper::New) {
    if (!info.IsConstructCall()) {
        return Nan::ThrowTypeError("Class Constructor ConstantInt cannot be invoked without new");
    }

    if (info.Length() != 1 || !info[0]->IsExternal()) {
        return Nan::ThrowTypeError("ConstantInt constructor needs to be called with: constantInt: external");
    }

    auto* constantInt = static_cast<llvm::ConstantInt*>(v8::External::Cast(*info[0])->Value());
    auto* wrapper = new ConstantIntWrapper { constantInt };
    wrapper->Wrap(info.This());

    info.GetReturnValue().Set(info.This());
}

NAN_METHOD(ConstantIntWrapper::get) {
    if (info.Length() != 2 || !LLVMContextWrapper::isInstance(info[0]) || !info[1]->IsNumber()) {
        return Nan::ThrowTypeError("get needs to be called with: context: LLVMContext, value: number");
    }

    auto& context = LLVMContextWrapper::FromValue(info[0])->getContext();
    int64_t number = Nan::To<int64_t >(info[1]).FromJust();

    auto* constant = llvm::ConstantInt::get(context, llvm::APInt { 32, static_cast<uint64_t>(number), true } );

    info.GetReturnValue().Set(ConstantIntWrapper::of(constant));
}

NAN_METHOD(ConstantIntWrapper::getTrue) {
    if (info.Length() != 1 || !LLVMContextWrapper::isInstance(info[0])) {
        return Nan::ThrowTypeError("getTrue needs to be called with: context: LLVMContext");
    }

    auto& context = LLVMContextWrapper::FromValue(info[0])->getContext();
    auto* constant = llvm::ConstantInt::getTrue(context);

    info.GetReturnValue().Set(ConstantIntWrapper::of(constant));
}

NAN_METHOD(ConstantIntWrapper::getFalse) {
    if (info.Length() != 1 || !LLVMContextWrapper::isInstance(info[0])) {
        return Nan::ThrowTypeError("getFalse needs to be called with: context: LLVMContext");
    }

    auto& context = LLVMContextWrapper::FromValue(info[0])->getContext();
    auto* constant = llvm::ConstantInt::getFalse(context);

    info.GetReturnValue().Set(ConstantIntWrapper::of(constant));
}

NAN_GETTER(ConstantIntWrapper::getValueApf) {
    auto* wrapper = ConstantIntWrapper::FromValue(info.Holder());
    auto value = wrapper->getConstantInt()->getValue();

    info.GetReturnValue().Set(Nan::New(value.signedRoundToDouble()));
}

llvm::ConstantInt *ConstantIntWrapper::getConstantInt() {
    return static_cast<llvm::ConstantInt*>(getValue());
}

v8::Local<v8::Object> ConstantIntWrapper::of(llvm::ConstantInt *constantInt) {
    auto constructorFunction = Nan::GetFunction(Nan::New(constantIntTemplate())).ToLocalChecked();
    v8::Local<v8::Value> args[1] = { Nan::New<v8::External>(constantInt) };
    auto instance = Nan::NewInstance(constructorFunction, 1, args).ToLocalChecked();

    Nan::EscapableHandleScope escapeScpoe {};
    return escapeScpoe.Escape(instance);
}

Nan::Persistent<v8::FunctionTemplate>& ConstantIntWrapper::constantIntTemplate() {
    static Nan::Persistent<v8::FunctionTemplate> functionTemplate {};

    if (functionTemplate.IsEmpty()) {
        auto localTemplate = Nan::New<v8::FunctionTemplate>(ConstantIntWrapper::New);
        localTemplate->SetClassName(Nan::New("ConstantInt").ToLocalChecked());
        localTemplate->InstanceTemplate()->SetInternalFieldCount(1);
        localTemplate->Inherit(Nan::New(constantTemplate()));

        Nan::SetMethod(localTemplate, "get", ConstantIntWrapper::get);
        Nan::SetMethod(localTemplate, "getFalse", ConstantIntWrapper::getFalse);
        Nan::SetMethod(localTemplate, "getTrue", ConstantIntWrapper::getTrue);
        Nan::SetAccessor(localTemplate->InstanceTemplate(), Nan::New("value").ToLocalChecked(), ConstantIntWrapper::getValueApf);

        functionTemplate.Reset(localTemplate);
    }

    return functionTemplate;
}