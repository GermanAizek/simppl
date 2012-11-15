#include "simppl/ipc2.h"


INTERFACE(Interface)
{   
   Request<int> doSomething;
   Response<int> resultOfDoSomething;
   
   Request<int> doSomethingElse;
   Response<> resultOfDoSomethingElse;
   
   inline
   Interface()
    : INIT_REQUEST(doSomething)
    , INIT_REQUEST(doSomethingElse)
    , INIT_RESPONSE(resultOfDoSomething)
    , INIT_RESPONSE(resultOfDoSomethingElse)
   {
      doSomething >> resultOfDoSomething /*|| throwing()*/;
      doSomethingElse >> resultOfDoSomethingElse;
   }
};


struct Server : Skeleton<Interface>
{
   Server(const char* role)
    : Skeleton<Interface>(role)
   {      
      doSomething >> std::bind(&Server::handleDoSomething, this, _1);
   }
   
   void handleDoSomething(int i)
   {
      std::cout << "doing something with i=" << i << std::endl;  
      
      //throw RuntimeError(-i, "Shit also happens");   // also possible here
      if (i == 42)
      {
         respondWith(RuntimeError(-i, "Shit happens"));
      }
      else
         respondWith(resultOfDoSomething(i));
   }
};


struct Client : Stub<Interface>
{
   Client(const char* role)
    : Stub<Interface>(role, "unix:myserver")   // connect the client to 'myserver'
   {
      connected >> std::bind(&Client::handleConnected, this);
      
      // must distinuish between normal response and error handler
      // another possible solution would be to wrap the binder in an OnError(...) Function
      // which then could be used for type switching in the >> operator via specialization 
      
      resultOfDoSomething >> std::bind(&Client::handleResultDoSomething, this, _1, _2);
   }
   
   void handleConnected()
   {
      doSomething(42);
   }
   
   void handleResultDoSomething(const CallState& state, int response)
   {
      if (state)
      {
         std::cout << "Response is " << response << std::endl;
         disp().stop();
      }
      else
      {
         std::cout << "Error transmitted from server: request failed, message: " << state.what() << std::endl;
      
         doSomething(7);
      }
   }
};


void* server(void* dispatcher)
{
   Dispatcher& d = *(Dispatcher*)dispatcher;
   
   Server s1("myrole");
   d.addServer(s1);
   
   d.run();
   return 0;
}


int main()
{
   // start server dispatcher thread on unix path 'myserver'
   Dispatcher server_dispatcher("unix:myserver");
   
   pthread_t tid;
   pthread_create(&tid, 0, server, &server_dispatcher);
   while(!server_dispatcher.isRunning());   // wait for other thread
   
   // run client in separate thread (not really necessary, just for blocking interfaces)
   Dispatcher d;
   
   Client c("myrole");
   d.addClient(c);
   
   d.run();
   
   server_dispatcher.stop();
   pthread_join(tid, 0);
   
   return 0;

}
