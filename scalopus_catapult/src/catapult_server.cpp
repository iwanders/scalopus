/*
  Copyright (c) 2018-2019, Ivor Wanders
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

  * Neither the name of the author nor the names of contributors may be used to
    endorse or promote products derived from this software without specific
    prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "scalopus_catapult/catapult_server.h"
#include <seasocks/IgnoringLogger.h>
#include <seasocks/PrintfLogger.h>
#include <seasocks/Server.h>

namespace scalopus
{
CatapultServer::CatapultServer(CatapultBackend::Ptr backend) : backend_{ backend }
{
  logger_ = std::make_shared<ss::IgnoringLogger>();
}

void CatapultServer::setLogger(std::shared_ptr<ss::Logger> logger)
{
  logger_ = std::move(logger);
}

void CatapultServer::setDefaultLogger()
{
  setLogger(std::make_shared<ss::PrintfLogger>(ss::Logger::Level::Warning));
}

void CatapultServer::start(std::size_t port)
{
  // Create the server with the provided logger specification.
  server_ = std::make_shared<ss::Server>(logger_);

  // Set the send buffer to to the specified value, this is merely the limit, it's not necissarily allocated or used.
  server_->setClientBufferSize(max_buffer_);

  server_->addWebSocketHandler("/devtools/page/bar", backend_);  // needed for chrom(e/ium) 65.0+
  server_->addWebSocketHandler("/devtools/browser", backend_);   // needed for chrome 60.0
  server_->addPageHandler(backend_);                             // This is retrieved in the overview page.

  // Set the function that's to be used to execute functions on the seasocks thread.
  backend_->setExecutor(
      [weak_server = std::weak_ptr<ss::Server>(server_)](scalopus::CatapultBackend::Runnable&& runnable) {
        auto server = weak_server.lock();
        if (server != nullptr)
        {
          server->execute(std::move(runnable));
        }
      });

  // start listening, create the worker thread.
  server_->startListening(port);
  thread_ = std::thread([&] { server_->loop(); });
}

void CatapultServer::setMaxBuffersize(std::size_t max_buffer)
{
  max_buffer_ = max_buffer;
}

CatapultServer::~CatapultServer()
{
  server_->terminate();
  thread_.join();
}
}  // namespace scalopus
