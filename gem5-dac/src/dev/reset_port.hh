/*
 * Copyright 2022 Google, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __DEV_RESET_PORT_HH__
#define __DEV_RESET_PORT_HH__

#include "sim/port.hh"

#include <string>

namespace gem5
{

class ResetResponsePortBase : public Port
{
  public:
    using Port::Port;
    virtual void requestReset() = 0;
};

template <class Device>
class ResetResponsePort : public ResetResponsePortBase
{
  public:
    ResetResponsePort(const std::string &name, PortID id, Device *dev) :
        ResetResponsePortBase(name, id), device(dev) {}
    void requestReset() override { device->requestReset(); }

  private:
    Device *device = nullptr;
};

class ResetRequestPort : public Port
{
  public:
    ResetRequestPort(const std::string &_name, PortID _id)
        : Port(_name, _id) {}
    void bind(Port &p) override;
    void unbind() override;
    void requestReset();

  private:
    ResetResponsePortBase *peer = nullptr;
};

} // namespace gem5

#endif // __DEV_RESET_PORT_HH__
