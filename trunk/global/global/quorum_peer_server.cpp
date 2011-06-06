/*
 * Copyright (C) Lichuang
 */
#include <eventrpc/base.h>
#include <eventrpc/log.h>
#include <eventrpc/rpc_server.h>
#include <eventrpc/dispatcher.h>
#include "fast_leader_election.h"
#include "quorum_peer.h"
#include "quorum_peer_server.h"
#include "quorum_peer_manager.h"
#include "protocol/leader_election.pb.h"
using namespace eventrpc;
namespace global {
class LeaderElectionServiceImpl : public LeaderElection {
 public:
  virtual void LeaderProposal(::google::protobuf::RpcController* controller,
                       const ::global::Notification* request,
                       ::global::Dummy* response,
                       ::google::protobuf::Closure* done);
};

class QuorumPeerServer::Impl {
 public:
  Impl(QuorumPeerServer *server);
  ~Impl();

  void set_quorumpeer_manager(QuorumPeerManager *manager);

  void Start();

 private:
  QuorumPeerServer *quorum_peer_server_;
  uint64 server_id_;
  QuorumPeer *quorum_peer_;
  QuorumPeerManager *quorum_peer_manager_;
  FastLeaderElection fast_leader_election_;
  RpcServer rpc_server_;
  Dispatcher dispatcher_;
};

QuorumPeerServer::Impl::Impl(QuorumPeerServer *server)
  : quorum_peer_server_(server),
    server_id_(0),
    quorum_peer_(NULL),
    quorum_peer_manager_(NULL),
    fast_leader_election_(server) {
}

QuorumPeerServer::Impl::~Impl() {
}

void QuorumPeerServer::Impl::set_quorumpeer_manager(QuorumPeerManager *manager) {
  quorum_peer_manager_ = manager;
  server_id_ = manager->my_server_id();
  quorum_peer_ = manager->FindQuorumPeerById(server_id_);
}

void QuorumPeerServer::Impl::Start() {
  dispatcher_.Start();
  rpc_server_.set_dispatcher(&dispatcher_);
  gpb::Service *service = new LeaderElectionServiceImpl();
  rpc_server_.rpc_method_manager()->RegisterService(service);
  rpc_server_.set_host_and_port(quorum_peer_->server_host_,
                                quorum_peer_->election_port_);
  rpc_server_.Start();
}

void LeaderElectionServiceImpl::LeaderProposal(
    ::google::protobuf::RpcController* controller,
    const ::global::Notification* request,
    ::global::Dummy* response,
    ::google::protobuf::Closure* done) {
}

QuorumPeerServer::QuorumPeerServer()
  : impl_(new Impl(this)) {
}

QuorumPeerServer::~QuorumPeerServer() {
  delete impl_;
}

void QuorumPeerServer::set_quorumpeer_manager(QuorumPeerManager *manager) {
  impl_->set_quorumpeer_manager(manager);
}

void QuorumPeerServer::Start() {
  impl_->Start();
}
};
