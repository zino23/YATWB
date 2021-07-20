#include "channel.h"

#include "event_loop.h"
#include "logging.h"

const int Channel::NoneEvent = 0;
const int Channel::ReadEvent = POLLIN | POLLPRI;
const int Channel::WriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd)
    : loop_(loop),
      fd_(fd),
      events_(0),
      revents_(0),
      index_(-1),
      events_handling_(false) {}

Channel::~Channel() { assert(!events_handling_); }

void Channel::update() { loop_->updateChannel(this); }

// Use nonblocking I/O
void Channel::handleEvents() {
  events_handling_ = true;
  if (revents_ & POLLNVAL) {
    LOG << "Channel::handle_events() POLLNVAL";
  }

  if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
    LOG << "Channel::handle_event() POLLHUP";
    if (close_cb_) close_cb_();
  }

  if (revents_ & (POLLERR | POLLNVAL)) {
    if (error_cb_) error_cb_();
  }

  if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
    if (read_cb_) read_cb_();
  }

  if (revents_ & POLLOUT) {
    if (write_cb_) write_cb_();
  }
  events_handling_ = false;
}

