#ifndef SIMPPL_INTERFACE_H
#define SIMPPL_INTERFACE_H


#include <cstdint>
#include <vector>
#include <map>

#include "simppl/detail/parented.h"
#include "simppl/detail/basicinterface.h"
#include "simppl/detail/serversignalbase.h"
#include "simppl/detail/serverrequestbasesetter.h"


namespace simppl
{
   
namespace ipc
{
   
// forward decls
template<typename...> struct ClientRequest;
template<typename...> struct ClientResponse;
template<typename...> struct ClientSignal;
template<typename, typename> struct ClientAttribute;

template<typename...> struct ServerRequest;
template<typename...> struct ServerResponse;
template<typename...> struct ServerSignal;
template<typename, typename> struct ServerAttribute;

struct ServerRequestBase;
struct ServerRequestBaseSetter;


template<template<typename...> class RequestT>
struct InterfaceBase;


template<>
struct InterfaceBase<ClientRequest> : detail::BasicInterface
{
   inline
   InterfaceBase()
   {
      // NOOP
   }
};


template<>
struct InterfaceBase<ServerRequest> : detail::BasicInterface
{
   std::map<uint32_t, ServerRequestBase*> container_;
   std::map<uint32_t, detail::ServerSignalBase*> signals_;
};

}   // namespace ipc

}   // namespace simppl


/// make sure to NOT put this macro into a namespace
#define INTERFACE(iface) \
   template<template<typename...> class Request, \
            template<typename...> class Response, \
            template<typename...> class Signal, \
            template<typename,typename=simppl::ipc::OnChange> class Attribute> \
      struct iface; \
            \
   namespace simppl { namespace ipc { \
      template<> struct InterfaceNamer<iface<ClientRequest, ClientResponse, ClientSignal, ClientAttribute> > { static inline const char* const name() { return #  iface ; } }; \
      template<> struct InterfaceNamer<iface<ServerRequest, ServerResponse, ServerSignal, ServerAttribute> > { static inline const char* const name() { return #  iface ; } }; \
   } } \
   \
   template<template<typename...> class Request, \
            template<typename...> class Response, \
            template<typename...> class Signal, \
            template<typename,typename=simppl::ipc::OnChange> class Attribute> \
      struct iface : public simppl::ipc::InterfaceBase<Request>

#define INIT_REQUEST(request) \
   request(# request, this)

#define INIT_RESPONSE(response) \
   response()

#define INIT_SIGNAL(signal) \
   signal(# signal, this)

// an attribute is nothing more that an encapsulated signal
// #define INIT_ATTRIBUTE(attr) \
//   attr(# name, ((simppl::ipc::InterfaceBase<Request>*)this)->signals_)


template<typename... T, typename... T2>
inline
void operator>> (simppl::ipc::ClientRequest<T...>& request, simppl::ipc::ClientResponse<T2...>& response)
{
   assert(!request.handler_);
   request.handler_ = &response;
}


template<typename... T, typename... T2>
inline
void operator>> (simppl::ipc::ServerRequest<T...>& request, simppl::ipc::ServerResponse<T2...>& response)
{
   assert(!request.hasResponse());
   
   simppl::ipc::detail::ServerRequestBaseSetter::setHasResponse(request);
   response.allowedRequests_.insert(&request);   
}

#endif   // SIMPPL_INTERFACE_H
