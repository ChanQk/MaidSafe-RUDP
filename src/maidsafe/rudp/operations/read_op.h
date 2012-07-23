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

#ifndef MAIDSAFE_RUDP_OPERATIONS_READ_OP_H_
#define MAIDSAFE_RUDP_OPERATIONS_READ_OP_H_

#include "boost/asio/handler_alloc_hook.hpp"
#include "boost/asio/handler_invoke_hook.hpp"
#include "boost/system/error_code.hpp"

namespace maidsafe {

namespace rudp {

namespace detail {

// Helper class to adapt a read handler into a waiting operation.
template <typename ReadHandler>
class ReadOp {
 public:
  ReadOp(ReadHandler handler, const boost::system::error_code* ec, const size_t* bytes_transferred)
      : handler_(handler),
        ec_(ec),
        bytes_transferred_(bytes_transferred) {}

  void operator()(boost::system::error_code) {
    handler_(*ec_, *bytes_transferred_);
  }

  friend void* asio_handler_allocate(size_t n, ReadOp* op) {
    using boost::asio::asio_handler_allocate;
    return asio_handler_allocate(n, &op->handler_);
  }

  friend void asio_handler_deallocate(void* p, size_t n, ReadOp* op) {
    using boost::asio::asio_handler_deallocate;
    asio_handler_deallocate(p, n, &op->handler_);
  }

  template <typename Function>
  friend void asio_handler_invoke(const Function& f, ReadOp* op) {
    using boost::asio::asio_handler_invoke;
    asio_handler_invoke(f, &op->handler_);
  }

 private:
  // Disallow assignment.
  ReadOp& operator=(const ReadOp&);

  ReadHandler handler_;
  const boost::system::error_code* ec_;
  const size_t* bytes_transferred_;
};

}  // namespace detail

}  // namespace rudp

}  // namespace maidsafe

#endif  // MAIDSAFE_RUDP_OPERATIONS_READ_OP_H_
