/*******************************************************************************
 *  Copyright 2012 MaidSafe.net limited                                        *
 *                                                                             *
 *  The following source code is property of MaidSafe.net limited and is not   *
 *  meant for external use.  The use of this code is governed by the licence   *
 *  file licence.txt found in the root of this directory and also on           *
 *  www.maidsafe.net.                                                          *
 *                                                                             *
 *  You are not free to copy, amend or otherwise use this source code without  *
 *  the explicit written permission of the board of directors of MaidSafe.net. *
 ******************************************************************************/
// Original author: Christopher M. Kohlhoff (chris at kohlhoff dot com)

#ifndef MAIDSAFE_RUDP_OPERATIONS_PROBE_OP_H_
#define MAIDSAFE_RUDP_OPERATIONS_PROBE_OP_H_

#include "boost/asio/handler_alloc_hook.hpp"
#include "boost/asio/handler_invoke_hook.hpp"
#include "boost/system/error_code.hpp"

namespace maidsafe {

namespace rudp {

namespace detail {

// Helper class to adapt a probe handler into a waiting operation.
template <typename ProbeHandler>
class ProbeOp {
 public:
  ProbeOp(ProbeHandler handler, const boost::system::error_code* ec)
    : handler_(handler),
      ec_(ec) {}

  ProbeOp(const ProbeOp& L) : handler_(L.handler_), ec_(L.ec_) {}

  ProbeOp & operator=(const ProbeOp& L) {
    // check for "self assignment" and do nothing in that case
    if (this != &L) {
      delete ec_;
      handler_ = L.handler_;
      ec_ = L.ec_;
    }
    return* this;
  }

  void operator()(boost::system::error_code ec) {
    if (boost::asio::error::timed_out == ec)
      handler_(ec);
    else
      handler_(*ec_);
  }

  friend void* asio_handler_allocate(size_t n, ProbeOp* op) {
    using boost::asio::asio_handler_allocate;
    return asio_handler_allocate(n, &op->handler_);
  }

  friend void asio_handler_deallocate(void* p, size_t n, ProbeOp* op) {
    using boost::asio::asio_handler_deallocate;
    asio_handler_deallocate(p, n, &op->handler_);
  }

  template <typename Function>
  friend void asio_handler_invoke(const Function& f, ProbeOp* op) {
    using boost::asio::asio_handler_invoke;
    asio_handler_invoke(f, &op->handler_);
  }

 private:
  ProbeHandler handler_;
  const boost::system::error_code* ec_;
};

}  // namespace detail

}  // namespace rudp

}  // namespace maidsafe

#endif  // MAIDSAFE_RUDP_OPERATIONS_PROBE_OP_H_
